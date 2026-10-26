#include "special_feedback.h"
#include "battle_util.h"
#include "../background.h"
#include "../core.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../unity.h"
#include <algorithm>
#include <atomic>

namespace ffpr::battle::special_feedback {
namespace {
ref::Class kBasic{"Last.Battle.Function", "BattleBasicFunction"};
ref::Class kAfterBasic{"Last.Battle.Function", "BattleBasicFunction", game::kFF1 | game::kFF2};
ref::Method mAfter{kAfterBasic, "CreateAfterViewEntity", 2};
ref::Method mAfterAttack{kAfterBasic, "CreateAfterAttackViewEntity", 2};
ref::Method mAfterBatch{kAfterBasic, "AfterViewDamage", 0};
ref::Class kAfterBase{"Last.Battle", "BattleBaseFunction", game::kFF1 | game::kFF2};
ref::Field fAfterResults{kAfterBase, "<IAfterCalcResultDic>k__BackingField"};
ref::Method mView{kBasic, "CreateViewEntity", 2};
ref::Method mAttack{kBasic, "CreateAttackViewEntity", 2};
ref::Class kBase{"Last.Battle", "BattleBaseFunction"};
ref::Field fFunctions{kBase, "<functionBases>k__BackingField"};
ref::Field fAct{kBase, "<battleActData>k__BackingField"};
ref::Class kFunction{"Last.Battle.Function", "FunctionBase"};
ref::Field fValue{kFunction, "<Value1>k__BackingField"};
ref::Class kAbsorption{"Last.Battle.Function", "AbsorptionFunction"};
ref::Class kMagicAbsorption{"Last.Battle.Function", "MagicAbsorptionFunction"};
ref::Class kDrain{"Last.Battle.Function", "BattleFunction13"};
ref::Field fAbsorption{kDrain, "absorptionType"};
// These are siblings of BattleFunction13, not subclasses. Resolve their own
// fields: FF2's layout puts this resource at 0x98, versus the base's 0xA8.
// They exist in all six builds; the native function provider chooses a variant.
ref::Class kDrain2{"Last.Battle.Function", "BattleFunction13FF2"};
ref::Class kDrain3{"Last.Battle.Function", "BattleFunction13FF3"};
ref::Field fAbsorption2{kDrain2, "absorptionType"}, fAbsorption3{kDrain3, "absorptionType"};
ref::Class kSwap{"Last.Battle.Function", "BattleFunction23", game::kFF2};
ref::Method mSwapView{kSwap, "CreateViewEntity", 2};
ref::Class kAct{"Last.Battle", "BattleActData"};
ref::Field fActor{kAct, "<AttackUnitData>k__BackingField"};
ref::Class kBasic2{"Last.Battle.Function", "BattleBasicFunction", game::kFF2};
ref::Method mUpdate{kBasic2, "UpdateParameter", 0};
ref::Class kProvider2{"Serial.FF2.Calc", "UniqueUpdateParameterProvider", game::kFF2};

// At most 64 actual battle targets and 16 functions per target. Read one rooted
// current native dictionary; never retain pointers or scan a whole scene.
void* target_value(void* dict, void* target) {
    const int count=unity::dict_count(dict);
    if(count<0||count>64)return nullptr;
    for(int i=0;i<count;++i) {
        uint64_t key=0,value=0;
        if(unity::dict_entry(dict,i,key,value)&&key==reinterpret_cast<uint64_t>(target))
            return reinterpret_cast<void*>(value);
    }
    return nullptr;
}
// Only a synchronous identity marks this native batch. The same FF2 function
// can render its original physical result, then a separate MP after-result.
thread_local void* g_after_batch_owner=nullptr;
bool split_primary(void* self) {
    if(g_after_batch_owner==self||!game::in(game::kFF1|game::kFF2)||!kDrain2.is(self))return false;
    const int count=unity::dict_count(ref::rd<void*>(self,fAfterResults));
    // A populated after dictionary is the native physical+MP split. Invalid
    // observations also preserve the primary native type; never guess MP.
    return count!=0;
}
int absorption_resource(void* self) {
    if(kDrain.is(self))return ref::rd<int32_t>(self,fAbsorption);
    if(kDrain2.is(self))return ref::rd<int32_t>(self,fAbsorption2);
    if(kDrain3.is(self))return ref::rd<int32_t>(self,fAbsorption3);
    return -1;
}
bool absorption_mp(void* self,void* target,bool primary) {
    // Percentage/wear absorption owns its resource on the outer function,
    // including the sibling FF2/FF3 variants. Target CalcResult can be plain Hit;
    // the separate attacker result is already MPRecovery. Do not infer from
    // damage amounts, monster names or changes to hidden parameter values.
    const int resource=absorption_resource(self);
    if(resource>=0)return resource==1&&(!primary||!split_primary(self));
    void* functions=target_value(ref::rd<void*>(self,fFunctions),target);
    const int count=unity::list_count(functions);
    if(count<1||count>16)return false;
    bool mp=false,hp=false;
    for(int i=0;i<count;++i) {
        void* fn=unity::list_at(functions,i);
        if(!kAbsorption.is(fn)&&!kMagicAbsorption.is(fn)) {
            if(primary)return false; // A physical main hit keeps its own type.
            continue;
        }
        if(ref::rd<int32_t>(fn,fValue)==2)mp=true;else hp=true;
    }
    return mp&&!hp; // Ambiguous combined effects retain the native type.
}
thread_local bool g_secondary_mp=false;
struct Scope {
    bool previous=g_secondary_mp;
    Scope(void* self,void* target,bool primary=false) {g_secondary_mp=absorption_mp(self,target,primary);}
    ~Scope(){g_secondary_mp=previous;}
};
using View=void(*)(void*,void*,bool,const void*);
View o_after=nullptr,o_after_attack=nullptr,o_swap=nullptr,o_view=nullptr,o_attack=nullptr;
using Batch=void(*)(void*,const void*);
Batch o_after_batch=nullptr;
void d_after_batch(void* self,const void* mi) {
    struct Restore {void* previous=g_after_batch_owner;~Restore(){g_after_batch_owner=previous;}} restore;
    g_after_batch_owner=self;
    o_after_batch(self,mi);
}
// Only identities cross the native update/view phases; names are owned strings.
// No retained native pointer is dereferenced later. Battle end clears unmatched
// records, and the fixed count rejects overload instead of growing without bound.
struct Applied {void* owner;void* target;std::wstring target_name,actor_name;};
std::vector<Applied> g_applied;
std::atomic<uint64_t> g_epoch{0};
thread_local void* g_update_owner=nullptr;
using Update=void(*)(void*,const void*);
Update o_update=nullptr;
void d_update(void* self,const void* mi) {
    struct UpdateScope {void* previous=g_update_owner;~UpdateScope(){g_update_owner=previous;}} scope;
    g_update_owner=kSwap.is(self)?self:nullptr;
    if(g_update_owner)std::erase_if(g_applied,[=](const Applied& row){return row.owner==self;});
    o_update(self,mi);
}
using Exchange=void(*)(void*,void*,void*,void*,int32_t,const void*);
Exchange o_exchange=nullptr;
void d_exchange(void* self,void* actor,void* target,void* key,int32_t target_type,const void* mi) {
    Applied applied{};
    hooks::guarded("Battle.SwapApplied.Capture",[&] {
        if(!g_update_owner||!actor||!target||ref::str(key)!=L"WM_CHANGE")return;
        void* targets=ref::rd<void*>(g_update_owner,fFunctions);
        const int count=unity::dict_count(targets);
        if(count<1||count>64||g_applied.size()>=64)return;
        void* actor_unit=ref::rd<void*>(ref::rd<void*>(g_update_owner,fAct),fActor);
        if(unit_parameter(actor_unit)!=actor)return;
        for(int i=0;i<count;++i) {
            uint64_t unit_key=0,value=0;
            if(!unity::dict_entry(targets,i,unit_key,value))continue;
            void* unit=reinterpret_cast<void*>(unit_key);
            if(unit_parameter(unit)!=target)continue;
            applied={g_update_owner,target,unit_name(unit),unit_name(actor_unit)};
            break;
        }
    });
    // This exact native overload performs WM_CHANGE's HP/MP exchange. Its other
    // keys are untouched; misses that never execute it create no success record.
    o_exchange(self,actor,target,key,target_type,mi);
    if(applied.owner)g_applied.push_back(std::move(applied));
}
void d_view(void* self,void* target,bool recovery,const void* mi) {
    Scope scope(self,target,true);o_view(self,target,recovery,mi);
}
void d_attack(void* self,void* target,bool recovery,const void* mi) {
    Scope scope(self,target,true);o_attack(self,target,recovery,mi);
}
void d_after(void* self,void* target,bool recovery,const void* mi) {
    Scope scope(self,target);o_after(self,target,recovery,mi);
}
void d_after_attack(void* self,void* target,bool recovery,const void* mi) {
    Scope scope(self,target);o_after_attack(self,target,recovery,mi);
}
void d_swap(void* self,void* target,bool recovery,const void* mi) {
    o_swap(self,target,recovery,mi);
    hooks::guarded("Battle.SwapFeedback",[=] {
        void* parameter=unit_parameter(target);
        const auto row=std::find_if(g_applied.begin(),g_applied.end(),[=](const Applied& item){return item.owner==self&&item.target==parameter;});
        if(row==g_applied.end())return;
        const auto target_name=row->target_name,actor_name=row->actor_name;
        const auto locale=modtext::lang();
        const auto epoch=g_epoch.load(std::memory_order_acquire);
        g_applied.erase(row);
        background::post([target_name,actor_name,locale,epoch] {
            if(g_epoch.load(std::memory_order_acquire)!=epoch)return;
            modtext::LocaleScope language(locale);
            ffpr::say(TF(L"{0}: HP and MP exchanged with {1}",{target_name,actor_name}),false);
        });
    });
}
int install() {
    g_applied.reserve(64); // Install/boot owns this allocation, before battle hooks.
    int n=0;
    n+=hooks::attach(mView,&d_view,&o_view)?1:0;
    n+=hooks::attach(mAttack,&d_attack,&o_attack)?1:0;
    if(game::in(game::kFF1|game::kFF2)) {
        n+=hooks::attach(mAfter,&d_after,&o_after)?1:0;
        n+=hooks::attach(mAfterAttack,&d_after_attack,&o_after_attack)?1:0;
        n+=hooks::attach(mAfterBatch,&d_after_batch,&o_after_batch)?1:0;
    }
    if(game::is(game::Id::FF2)) {
        n+=hooks::attach(mSwapView,&d_swap,&o_swap)?1:0;
        n+=hooks::attach(mUpdate,&d_update,&o_update)?1:0;
        auto* method=find_method_param_type(kProvider2.get(),"UpdateParameter",4,0,"CharacterParameterBase");
        n+=hooks::attach_addr(il2cpp::method_pointer(method),"Swap.UpdateParameter.parameters",reinterpret_cast<void*>(&d_exchange),reinterpret_cast<void**>(&o_exchange))?1:0;
        events::on(events::Kind::BattleEnd,[]{g_applied.clear();g_epoch.fetch_add(1,std::memory_order_acq_rel);});
    }
    return n;
}
core::Install s_install{"Battle.SpecialFeedback",&install,109};
int mp_type(int type,bool recovery) {
    if(type==2||type==5||type==6)return type;
    return type==4||recovery?6:5;
}
}
int damage_type(int type,bool recovery) {
    if(type==9&&game::is(game::Id::FF6))return recovery?6:5; // FF6 MpAbs.
    return g_secondary_mp?mp_type(type,recovery):type;
}
int owned_damage_type(void* owner,int type,bool recovery) {
    // Inlined native ViewEntity loops bypass CreateViewEntity and its resource
    // scope. Observe this individual function, not the whole action: Parasite's
    // primary physical result must keep HP even when held by that same FF2
    // absorption function; only its populated after-result batch supplies MP.
    // Pure MP functions without that split keep their own resource. No hit counting,
    // amount matching or parameter-change inference participates in this label.
    if(absorption_resource(owner)==1&&!split_primary(owner))return mp_type(type,recovery);
    return damage_type(type,recovery);
}
}
