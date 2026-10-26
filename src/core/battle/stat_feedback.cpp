// Observe native modifier application, not localized spell names or damage/HP
// arithmetic. All-six BattleStatusControl owns the ready aggregate dictionary;
// BattleUtility's typed mutation owns direct stat writes. Native pointers stay
// on their owner. The formatter receives copied names, directions and locale.
#include "stat_feedback.h"
#include "battle_util.h"
#include "../worker_job.h"
#include "../core.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../text.h"
#include "../unity.h"
#include <atomic>

namespace ffpr::battle::stat_feedback {
namespace {
ref::Class kInfo{"Last.Battle","BattleUnitDataInfo"};
ref::Field fTemporary{kInfo,"<BattleTempParameter>k__BackingField"};
ref::Class kTemporary{"Last.Battle","BattleTempParameter"};
ref::Field fModifiers{kTemporary,"parameterGroupDic"};
ref::Class kStatus{"Last.Battle","BattleStatusControl",static_cast<uint8_t>(game::kAll & ~game::kFF2)};
ref::Method mStatus{kStatus,"UpdateTempParamter",1};
ref::Class kUtility{"Last.Battle","BattleUtility"};
ref::Class kParameter{"Last.Data","CharacterParameterBase"};
// Same fields, name-resolved per installed binary. HP/MP operations are excluded:
// the native overload changes current resources, already owned by damage speech.
ref::Field fLevel{kParameter,"<TemporaryLevel>k__BackingField"};
ref::Field fMaxHp{kParameter,"<TemporaryMaxHp>k__BackingField"},fMaxMp{kParameter,"<TemporaryMaxMp>k__BackingField"};
ref::Field fPower{kParameter,"<TemporaryPower>k__BackingField"};
ref::Field fVitality{kParameter,"<TemporaryVitality>k__BackingField"};
ref::Field fAgility{kParameter,"<TemporaryAgility>k__BackingField"};
ref::Field fWeight{kParameter,"<TemporaryWeight>k__BackingField"};
ref::Field fIntelligence{kParameter,"<TemporaryIntelligence>k__BackingField"};
ref::Field fSpirit{kParameter,"<TemporarySpirit>k__BackingField"};
ref::Field fAttack{kParameter,"<TemporaryAttack>k__BackingField"};
ref::Field fDefense{kParameter,"<TemporaryDefense>k__BackingField"};
ref::Field fMagicDefense{kParameter,"<TemporaryAbilityDefense>k__BackingField"};
ref::Field fMagicEvasion{kParameter,"<TemporaryAbilityEvasionRate>k__BackingField"};
ref::Field fMagic{kParameter,"<TemporaryMagic>k__BackingField"};
ref::Field fLuck{kParameter,"<TemporaryLuck>k__BackingField"};
ref::Field fAccuracy{kParameter,"<TemporaryAccuracyRate>k__BackingField"};
ref::Field fEvasion{kParameter,"<TemporaryEvasionRate>k__BackingField"};
ref::Field fInterference{kParameter,"<TemporaryAbilityDisturbedRate>k__BackingField"};
ref::Field fCritical{kParameter,"<TemporaryCriticalRate>k__BackingField"};
ref::Field fDamage{kParameter,"<TemporaryDamageDiameter>k__BackingField"};
ref::Field fSpeed{kParameter,"temporaryAttackSpeed"};
ref::Field fDefenseCount{kParameter,"<TemporaryDefenseCount>k__BackingField"};
ref::Field fMagicDefenseCount{kParameter,"<TemporaryMagicDefenseCount>k__BackingField"};
ref::Field fFear{kParameter,"<TemporaryFear>k__BackingField"};
ref::Field fAccuracyCount{kParameter,"<TemporaryAccuracyCount>k__BackingField"};
ref::Field fMagicDefenseRate{kParameter,"<TemporaryAbilityDefenseRate>k__BackingField"};
ref::Field fEvasionCount{kParameter,"<TemporaryEvasionCount>k__BackingField"};
std::array<ref::Field*,slots> fields{{nullptr,&fLevel,&fMaxHp,&fMaxMp,&fPower,&fVitality,
    &fAgility,&fWeight,&fIntelligence,&fSpirit,&fAttack,&fDefense,&fMagicDefense,
    &fMagicEvasion,&fMagic,&fLuck,&fAccuracy,&fEvasion,&fInterference,&fCritical,
    nullptr,&fDamage,&fSpeed,&fDefenseCount,&fMagicDefenseCount,&fFear,
    &fAccuracyCount,&fMagicDefenseRate,&fEvasionCount}};
ref::Field bLevel{kParameter,"<BaseLevel>k__BackingField"},bHp{kParameter,"<BaseMaxHp>k__BackingField"},bMp{kParameter,"<BaseMaxMp>k__BackingField"};
ref::Field bPower{kParameter,"<BasePower>k__BackingField"},bVitality{kParameter,"<BaseVitality>k__BackingField"},bAgility{kParameter,"<BaseAgility>k__BackingField"};
ref::Field bWeight{kParameter,"<BaseWeight>k__BackingField"},bIntelligence{kParameter,"<BaseIntelligence>k__BackingField"},bSpirit{kParameter,"<BaseSpirit>k__BackingField"};
ref::Field bAttack{kParameter,"<BaseAttack>k__BackingField"},bDefense{kParameter,"<BaseDefense>k__BackingField"},bMagicDefense{kParameter,"<BaseAbilityDefense>k__BackingField"};
ref::Field bMagicEvasion{kParameter,"<BaseAbilityEvasionRate>k__BackingField"},bMagic{kParameter,"<BaseMagic>k__BackingField"},bLuck{kParameter,"<BaseLuck>k__BackingField"};
ref::Field bAccuracy{kParameter,"<BaseAccuracyRate>k__BackingField"},bEvasion{kParameter,"<BaseEvasionRate>k__BackingField"},bInterference{kParameter,"<BaseAbilityDisturbedRate>k__BackingField"};
ref::Field bCritical{kParameter,"<BaseCriticalRate>k__BackingField"},bDamage{kParameter,"<BaseDamageDiameter>k__BackingField"},bSpeed{kParameter,"<BaseAttackSpeed>k__BackingField"};
ref::Field bDefenseCount{kParameter,"<BaseDefenseCount>k__BackingField"},bMagicDefenseCount{kParameter,"<BaseMagicDefenseCount>k__BackingField"},bFear{kParameter,"<BaseFear>k__BackingField"};
ref::Field bAccuracyCount{kParameter,"<BaseAccuracyCount>k__BackingField"},bMagicDefenseRate{kParameter,"<BaseAbilityDefenseRate>k__BackingField"},bEvasionCount{kParameter,"<BaseEvasionCount>k__BackingField"};
std::array<ref::Field*,slots> base_fields{{nullptr,&bLevel,&bHp,&bMp,&bPower,&bVitality,
    &bAgility,&bWeight,&bIntelligence,&bSpirit,&bAttack,&bDefense,&bMagicDefense,
    &bMagicEvasion,&bMagic,&bLuck,&bAccuracy,&bEvasion,&bInterference,&bCritical,
    nullptr,&bDamage,&bSpeed,&bDefenseCount,&bMagicDefenseCount,&bFear,
    &bAccuracyCount,&bMagicDefenseRate,&bEvasionCount}};
using Changes=std::array<int64_t,slots>;
Changes field_values(void* parameter) {
    Changes result{};
    if(parameter)for(size_t i=1;i<slots;++i) {
        if(fields[i])result[i]+=ref::rd<int32_t>(parameter,*fields[i]);
        if(base_fields[i])result[i]+=ref::rd<int32_t>(parameter,*base_fields[i]);
    }
    return result;
}

// These are comparison identities only. No saved native pointer is dereferenced
// by a later callback/worker. Initial native refreshes populate names before
// BattleStart; BattleEntering/End/Title retire them, including queued output.
struct Binding {void* parameter=nullptr;std::wstring name;uint64_t generation=0;Changes fields{};};
std::array<Binding,64> bindings;
std::atomic<uint64_t> epoch{0}, rejected{0};
worker_job::Owner formatting;
unsigned pending=0;
std::atomic<bool> overflow{false};
bool active=false;
thread_local AddScope* adding=nullptr;
void remember(void* unit,void* parameter) {
    if(!unit||!parameter)return;
    const auto generation=epoch.load(std::memory_order_relaxed);
    for(const auto& row:bindings)if(row.generation==generation&&row.parameter==parameter)return;
    for(auto& row:bindings)if(!row.parameter||row.generation!=generation){row={parameter,unit_name(unit).substr(0,256),generation,field_values(parameter)};return;}
    rejected.fetch_add(1,std::memory_order_relaxed);
}
std::wstring name_for(void* parameter) {
    const auto generation=epoch.load(std::memory_order_relaxed);
    for(const auto& row:bindings)if(row.generation==generation&&row.parameter==parameter)return row.name;
    return {};
}
int slot(int type) {
    if(type>=1&&type<=24)return type;
    if(type>=201&&type<=204)return type-176;
    return 0;
}
bool supported(size_t id) {
    // Common master IDs1..25; FFII alone adds26..28. Current HP/MP are never
    // statistics feedback and retain their separate rendered-result producers.
    // Luck is internal critical-health bookkeeping, not a conveyed ability
    // effect. No parameter group in any of the six installed masters modifies
    // Luck. In FFIII the automatic Critical/KO refresh otherwise exposed the
    // temporary value as "Luck up/down". Keep its cache current but never speak
    // this raw field/dictionary slot without a separately verified UI producer.
    return id>0&&id<slots&&id!=15&&(id<=25||game::is(game::Id::FF2));
}
const wchar_t* key(size_t id,game::Id title) {
    switch(id) {
    case 1:return L"Level";case 4:return L"Strength";case 5:return L"Stamina";
    case 6:return L"Agility";case 7:return L"Weight";case 8:return L"Intellect";
    case 9:return L"Spirit";case 10:return L"Attack";case 11:return L"Defense";
    case 12:case 27:return L"Magic Defense";case 13:return L"Magic Evasion";
    case 14:return L"Magic";case 15:return L"Luck";case 16:return L"Accuracy";
    case 17:return L"Evasion";case 18:return L"Magic interference";
    case 19:return L"Critical rate";case 20:return L"Magic count";
    case 21:return L"Damage multiplier";case 22:return title==game::Id::FF1||title==game::Id::FF2||title==game::Id::FF3?L"Attack count":L"Attack speed";
    case 23:return L"Defense count";case 24:return L"Magic defense count";
    case 25:return L"Fear";case 26:return L"Accuracy count";case 28:return L"Evasion count";
    default:return nullptr;
    }
}
bool any(const Changes& changes) {for(auto value:changes)if(value)return true;return false;}
bool submit(std::wstring target,const Changes& changes,std::wstring condition={}) {
    if(target.empty()||!any(changes))return false;
    if(pending>=64) {
        rejected.fetch_add(1,std::memory_order_relaxed);overflow.store(true,std::memory_order_release);
        return false;
    }
    const auto language=modtext::lang();const auto generation=epoch.load(std::memory_order_acquire);
    const auto title=game::current();
    struct Input {std::wstring target,condition,language;Changes changes;game::Id title;uint64_t generation;};
    ++pending;
    // The established helper retains this owned event and retries a busy lane
    // through bounded frame admission. It never waits or formats inline. At
    // most64 independent results are retained; overflow is explicitly reported
    // once the formatter recovers instead of silently dropping an unlimited queue.
    return worker_job::start(formatting,Input{std::move(target),std::move(condition),language,changes,title,generation},
      [](Input& input) {
        if(epoch.load(std::memory_order_acquire)!=input.generation)return std::wstring();
        modtext::LocaleScope locale(input.language);std::vector<std::wstring> descriptions;
        if(!input.condition.empty())descriptions.push_back(input.condition);
        for(size_t i=1;i<slots;++i)if(input.changes[i]) {
            std::wstring label;
            if(i==2||i==3)label=TF(L"Maximum {0}",{T(i==2?L"HP":L"MP")});
            else if(const auto* stat=key(i,input.title))label=T(stat);
            if(!label.empty())descriptions.push_back(TF(input.changes[i]>0?L"{0} up":L"{0} down",{label}));
        }
        auto line=input.target+L": "+text::join(descriptions,L", ");
        if(overflow.exchange(false,std::memory_order_acq_rel))
            line=T(L"Statistic feedback is busy; some changes could not be read.")+L" "+line;
        return line;
      },[generation](std::wstring line) {
        if(epoch.load(std::memory_order_acquire)!=generation)return;
        if(pending)--pending;
        if(!line.empty())ffpr::say(line,false);
      },background::Lane::Data);
}
void changed(void* unit,const Changes& changes) {
    if(!active||!any(changes)||param_current_hp(unit_parameter(unit))<=0)return;
    if(adding&&adding->unit==unit) {
        for(size_t i=0;i<slots;++i)adding->changes[i]+=changes[i];
        return;
    }
    submit(unit_name(unit).substr(0,256),changes);
}
Changes changed_fields(void* parameter) {
    const auto current=field_values(parameter);
    const auto generation=epoch.load(std::memory_order_relaxed);
    for(auto& row:bindings)if(row.generation==generation&&row.parameter==parameter) {
        Changes difference{};
        for(size_t i=1;i<slots;++i)if(supported(i))difference[i]=current[i]-row.fields[i];
        row.fields=current;return difference;
    }
    return {};
}
using Status=void(*)(void*,void*,const void*);
Status original_status=nullptr;
void status(void* self,void* unit,const void* mi) {
    Snapshot before;hooks::guarded("Battle.StatBefore",[&]{before=capture(unit);});
    original_status(self,unit,mi);
    hooks::guarded("Battle.StatAfter",[&]{updated(unit,before);});
}
using Direct=void(*)(void*,int32_t,int32_t,const void*);
Direct original_direct=nullptr;
void direct(void* parameter,int32_t type,int32_t value,const void* mi) {
    const int id=slot(type);int32_t previous=0;bool ready=false;
    hooks::guarded("Battle.StatDirectBefore",[&] {
        ready=active&&parameter&&supported(id)&&id!=2&&id!=3&&fields[id]&&fields[id]->get()>=0;
        if(ready)previous=ref::rd<int32_t>(parameter,*fields[id]);
    });
    original_direct(parameter,type,value,mi);
    hooks::guarded("Battle.StatDirectAfter",[&] {
        if(!ready||!active)return;
        const int64_t difference=int64_t(ref::rd<int32_t>(parameter,*fields[id]))-previous;
        if(!difference)return;
        const auto generation=epoch.load(std::memory_order_relaxed);
        for(auto& row:bindings)if(row.generation==generation&&row.parameter==parameter){row.fields[id]+=difference;break;}
        if(param_current_hp(parameter)<=0)return; // KO teardown is not an applied effect
        Changes changes{};changes[id]=difference;
        if(adding&&unit_parameter(adding->unit)==parameter) {
            adding->changes[id]+=difference;return;
        }
        submit(name_for(parameter),changes);
    });
}
void reset() {
    cleanup();formatting.cancel();
    for(auto& row:bindings)row={};
}
int install() {
    int count=0;
    // Resolve this feature's metadata on the boot owner before native battle
    // callbacks use the cached offsets. No lazy class discovery in a stat write.
    fTemporary.get();fModifiers.get();for(auto* field:fields)if(field)field->get();for(auto* field:base_fields)if(field)field->get();
    if(!game::is(game::Id::FF2))count+=hooks::attach(mStatus,&status,&original_status)?1:0;
    // Both overloads have three arguments. Hook only the enum overload reached
    // by the int-ID overload too; otherwise one native write would be seen twice.
    auto* method=find_method_param_type(kUtility.get(),"ChangeTemporaryParameter",3,1,"ParameterType");
    count+=hooks::attach_addr(il2cpp::method_pointer(method),"Battle.StatDirect",reinterpret_cast<void*>(&direct),reinterpret_cast<void**>(&original_direct))?1:0;
    events::on(events::Kind::BattleEntering,[]{cleanup();}); // flags/epoch only
    events::on(events::Kind::BattleStart,[]{active=true;});
    events::on(events::Kind::BattleEnd,[]{reset();});
    events::on(events::Kind::TitleScreen,[]{reset();});
    return count;
}
core::Install registration{"Battle.StatFeedback",&install,109};
}

Snapshot capture(void* unit) {
    Snapshot result;
    void* info=unit_info(unit);result.parameter=info_parameter(info);
    if(!result.parameter)return result;
    remember(unit,result.parameter);
    result.temporary=ref::rd<void*>(info,fTemporary);
    if(!result.temporary)return result;
    void* dictionary=ref::rd<void*>(result.temporary,fModifiers);
    const int count=unity::dict_count(dictionary);
    if(count<0||count>64){rejected.fetch_add(1,std::memory_order_relaxed);return result;}
    for(int i=0;i<count;++i) {
        uint64_t id=0,value=0;
        if(!unity::dict_entry(dictionary,i,id,value,4,4))return result;
        const auto index=static_cast<int32_t>(id);
        if(index>0&&index<static_cast<int>(slots)&&supported(index))result.values[index]=static_cast<int32_t>(value);
    }
    result.valid=true;return result;
}
void observe_action(void* actor,const std::wstring& actor_name,void* target_array) {
    if(void* parameter=unit_parameter(actor)) {
        const auto generation=epoch.load(std::memory_order_relaxed);
        Binding* binding=nullptr;
        for(auto& row:bindings)if(row.generation==generation&&row.parameter==parameter){binding=&row;break;}
        if(!binding)for(auto& row:bindings)if(!row.parameter||row.generation!=generation){binding=&row;break;}
        if(binding) {
            if(binding->parameter!=parameter||binding->generation!=generation)binding->fields=field_values(parameter);
            binding->parameter=parameter;binding->name=actor_name.substr(0,256);binding->generation=generation;
        }
        else rejected.fetch_add(1,std::memory_order_relaxed);
    }
    const int count=unity::array_length(target_array);
    if(count<0||count>64){rejected.fetch_add(1,std::memory_order_relaxed);return;}
    for(int i=0;i<count;++i) {
        void* unit=unity::array_at(target_array,i);
        if(unit)remember(unit,unit_parameter(unit));
    }
}
void updated(void* unit,const Snapshot& before) {
    if(!before.valid)return;
    const auto after=capture(unit);
    if(!after.valid||after.parameter!=before.parameter||after.temporary!=before.temporary)return;
    Changes changes=changed_fields(after.parameter);
    for(size_t i=1;i<slots;++i)if(supported(i))changes[i]+=int64_t(after.values[i])-before.values[i];
    changed(unit,changes);
}
void finish_action(void* actor,void* target_array) {
    if(!active)return;
    const auto finish=[](void* unit){if(void* parameter=unit_parameter(unit))changed(unit,changed_fields(parameter));};
    finish(actor);
    const int count=unity::array_length(target_array);
    if(count<0||count>64){rejected.fetch_add(1,std::memory_order_relaxed);return;}
    for(int i=0;i<count;++i)if(void* unit=unity::array_at(target_array,i))finish(unit);
}
void cleanup(){active=false;epoch.fetch_add(1,std::memory_order_acq_rel);formatting.interrupt();pending=0;overflow.store(false,std::memory_order_release);}
AddScope::AddScope(void* owner):unit(owner),previous(adding){adding=this;}
AddScope::~AddScope(){adding=previous;hooks::guarded("Battle.StatAddComplete",[&]{if(!consumed&&active&&any(changes)&&param_current_hp(unit_parameter(unit))>0)submit(unit_name(unit).substr(0,256),changes);});}
bool announce_condition(void* unit,const std::wstring& name,int type) {
    if(!adding||adding->unit!=unit)return false;
    // Critical health, KO and petrification have their own visible condition
    // feedback. Their automatic parameter resets are not new ability effects.
    // Consume this native Add's accumulated stats, retaining the ordinary label.
    if(type==4||type==5||type==11){adding->consumed=true;return false;}
    if(!any(adding->changes))return false;
    // Imbibe's fallback already describes these two statistics. Select the
    // structured applied result by semantic type, without comparing speech text.
    const auto label=game::is(game::Id::FF2)&&type==203?std::wstring():name;
    adding->consumed=true;
    return submit(unit_name(unit).substr(0,256),adding->changes,label);
}
}
