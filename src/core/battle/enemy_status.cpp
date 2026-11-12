// Optional speech for condition-specific enemy replacement sprites. Native
// condition and asset identity qualify a request, then SetSprite and the live
// body renderer establish presentation. Worker inputs contain only text.
#include "battle.h"
#include "battle_util.h"
#include "boss_visuals.h"
#include "../condition_labels.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../frame.h"
#include "../hooks.h"
#include "../say.h"
#include "../ui/preferences.h"
#include "../unity.h"
#include "../worker_job.h"
#include <array>
#include <climits>

namespace ffpr::battle::enemy_status {
namespace {
ref::Class kFunction{"Last.Battle","BattleConditionFunction"};
ref::Method mInit{kFunction,"Init",2};
ref::Class kSprite{"Last.Battle","BattleSpriteEntity"};
ref::Method mSetSprite{kSprite,"SetSprite",1};
ref::Field fCharacterRenderer{kSprite,"characterSR"};
ref::Class kMesh{"Last.Battle","EntityMeshRenderer"};
ref::Field fMeshSprite{kMesh,"sprite"};
ref::Field fMeshRenderer{kMesh,"meshRenderer"};
ref::Class kInstantiate{"Last.Battle","InstantiateManager"};
ref::Method mFromCondition{kInstantiate,"InitEnemyEntityFromCondition",2};
ref::Method mInitEnemy{kInstantiate,"InitEnemyEntity",2};

struct Form { game::Id title; int type; int asset; };
constexpr Form forms[]{
    {game::Id::FF2,402,129},
    {game::Id::FF4,401,26}, {game::Id::FF4,402,131}, {game::Id::FF4,403,237},
    {game::Id::FF5,401,296}, {game::Id::FF5,402,297},
    {game::Id::FF6,601,352},
};
int form_asset(int type) {
    // Verified against each installed condition-to-art dictionary and bundle.
    const auto title=game::current();
    for(const auto& form:forms)if(form.title==title&&form.type==type)return form.asset;
    return 0;
}
struct Label { int type=-1; std::wstring name; };
struct Binding {
    void* sprite=nullptr;
    uint64_t appearance_revision=0;
    uint64_t generation=0;
    std::wstring enemy;
    std::array<Label,3> conditions; // FFIV has the largest supported form set
};
std::array<Binding,64> bindings;
worker_job::Owner work;
uint64_t generation=0;
unsigned pending=0;

void retire() {
    ++generation;pending=0;work.interrupt();
}
void clear() {
    retire();work.cancel();frame::cancel_label("EnemyStatus.Visible");
    // Old owned strings are reclaimed only when a binding is reused.
}
Binding* binding_for_sprite(void* sprite) {
    if(!sprite)return nullptr;
    for(auto& binding:bindings)if(binding.generation==generation&&binding.sprite==sprite)return &binding;
    return nullptr;
}
void bind(void* unit,void* condition) {
    if(!frame::on_main_thread()||!unit||!condition||!kBattleEnemyData.is(unit))return;
    const int type=condition_type(condition);
    if(!form_asset(type))return;
    void* sprite=displayed::field(unit_info(unit),"<BattleSpriteEntity>k__BackingField");
    if(!unity::alive(sprite))return;
    auto* binding=binding_for_sprite(sprite);
    if(!binding)for(auto& candidate:bindings)if(!candidate.sprite||candidate.generation!=generation) {
        binding=&candidate;
        binding->sprite=sprite;
        ++binding->appearance_revision;binding->generation=generation;
        binding->conditions={};break;
    }
    if(!binding)return;
    binding->enemy=enemy_name(unit).substr(0,256);
    Label* label=nullptr;
    for(auto& at:binding->conditions)if(at.type==type){label=&at;break;}
    if(!label)for(auto& at:binding->conditions)if(at.type<0){label=&at;break;}
    if(label){label->type=type;label->name=condition_name(condition).substr(0,256);}
}
// Only this native-owner continuation retains the renderer witness.
void appearance(const Binding& binding,std::wstring label,std::function<int()> state) {
    if(!ui::prefs::enemy_status_announcements_enabled()||pending>=64||binding.enemy.empty()||label.empty())return;
    const auto owner_generation=generation;
    ++pending;
    frame::every_frame([owner_generation,name=binding.enemy,label=std::move(label),state=std::move(state)] {
        if(owner_generation!=generation)return false;
        if(!ui::prefs::enemy_status_announcements_enabled()){--pending;return false;}
        if(!in_battle())return true;
        const int ready=state();
        if(ready<0){--pending;return false;}
        if(!ready)return true;
        struct Input { std::wstring name,label; };
        worker_job::start(work,Input{name,label},[](Input& input){return input.name+L": "+input.label;},
            [owner_generation,state](std::wstring line) {
                if(owner_generation!=generation)return;
                --pending;
                if(in_battle()&&ui::prefs::enemy_status_announcements_enabled()&&state()==1)say(line,false);
            });
        return false;
    },"EnemyStatus.Visible",INT_MAX);
}

// In all six binaries the condition loader calls the ordinary asset loader,
// which assigns the sprite synchronously. Match those exact nested calls.
// A later sprite assignment has no authority to speak a condition label.
struct FormRequest { void* entity=nullptr; int type=-1; bool matched=false; };
thread_local FormRequest condition_request;
struct ConditionScope {
    FormRequest previous=condition_request;
    explicit ConditionScope(FormRequest request){condition_request=request;}
    ~ConditionScope(){condition_request=previous;}
};
using InitEnemy=void(*)(void*,void*,int32_t,const void*);
InitEnemy o_from_condition,o_init_enemy;
void d_from_condition(void* self,void* entity,int32_t type,const void* mi) {
    ConditionScope scope({entity,type,false});
    o_from_condition(self,entity,type,mi);
}
void d_init_enemy(void* self,void* entity,int32_t asset,const void* mi) {
    if(frame::on_main_thread())if(auto* binding=binding_for_sprite(entity)) {
        ++binding->appearance_revision;
    }
    const int type=condition_request.type;
    const bool matched=condition_request.entity==entity&&asset>0&&asset==form_asset(type);
    ConditionScope scope({entity,type,matched});
    o_init_enemy(self,entity,asset,mi);
}
using SetSprite=void(*)(void*,void*,const void*);
SetSprite o_set_sprite;
void set_condition_sprite(void* self,void* sprite,const void* mi) {
    if(!frame::on_main_thread()||condition_request.entity!=self||!condition_request.matched) {
        o_set_sprite(self,sprite,mi);return;
    }
    void* mesh=nullptr;
    void* before=nullptr;
    hooks::guarded("EnemyStatus.BeforeSprite",[&] {
        if(binding_for_sprite(self)) {
            mesh=ref::rd<void*>(self,fCharacterRenderer);
            before=ref::rd<void*>(mesh,fMeshSprite);
        }
    });
    o_set_sprite(self,sprite,mi);
    hooks::guarded("EnemyStatus.Transformation",[=] {
        auto* binding=binding_for_sprite(self);
        if(!binding||!sprite||!mesh||before==sprite||
           ref::rd<void*>(mesh,fMeshSprite)!=sprite)return;
        const int type=condition_request.type;
        const auto revision=binding->appearance_revision;
        std::wstring label;
        for(const auto& at:binding->conditions)if(at.type==type){label=at.name;break;}
        if(label.empty())label=condition_labels::fallback(type);
        appearance(*binding,std::move(label),[binding,revision,mesh,sprite] {
            if(binding->appearance_revision!=revision||!unity::alive(mesh)||
               ref::rd<void*>(mesh,fMeshSprite)!=sprite)return -1;
            return displayed::renderer_visible(ref::rd<void*>(mesh,fMeshRenderer))?1:0;
        });
    });
}
void d_set_sprite(void* self,void* sprite,const void* mi) {
    const auto change=boss_visuals::before_sprite(self,sprite);
    set_condition_sprite(self,sprite,mi);
    if(change.key)hooks::guarded("BossVisual.AfterSprite",[&]{boss_visuals::after_sprite(self,change);});
}
using Init=void(*)(void*,void*,void*,const void*);
Init o_init;
void d_init(void* self,void* unit,void* condition,const void* mi) {
    hooks::guarded("EnemyStatus.Identity",[=]{bind(unit,condition);});
    o_init(self,unit,condition,mi);
    hooks::guarded("EnemyStatus.IdentityReady",[=]{bind(unit,condition);});
}
int install() {
    events::on(events::Kind::BattleEntering,[]{retire();});
    events::on(events::Kind::BattleEnd,[]{clear();});
    events::on(events::Kind::TitleScreen,[]{clear();});
    int count=hooks::attach(mInit,&d_init,&o_init)?1:0;
    count+=hooks::attach(mFromCondition,&d_from_condition,&o_from_condition)?1:0;
    count+=hooks::attach(mInitEnemy,&d_init_enemy,&o_init_enemy)?1:0;
    return count+(hooks::attach(mSetSprite,&d_set_sprite,&o_set_sprite)?1:0);
}
core::Install s_install{"Battle.EnemyStatus",&install,117};
}
}
