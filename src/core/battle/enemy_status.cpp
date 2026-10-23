// Optional speech for native enemy status effects that actually render. Condition
// Init supplies identity only. New effects, native sprite replacement and native
// material changes each require actual renderer proof. No condition enumeration
// or HP-derived Critical. Native handles never enter the formatting worker.
#include "battle.h"
#include "battle_util.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../frame.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../ui/preferences.h"
#include "../unity.h"
#include "../worker_job.h"
#include <array>
#include <climits>

namespace ffpr::battle::enemy_status {
namespace {
ref::Class kFunction{"Last.Battle", "BattleConditionFunction"};
ref::Method mInit{kFunction,"Init",2};
ref::Field fUnit{kFunction,"<BattleUnitData>k__BackingField"};
ref::Class kSprite{"Last.Battle","BattleSpriteEntity"};
ref::Method mSetSprite{kSprite,"SetSprite",1};
ref::Field fCharacterRenderer{kSprite,"characterSR"};
ref::Class kMesh{"Last.Battle","EntityMeshRenderer"};
ref::Field fMeshSprite{kMesh,"sprite"};
ref::Field fMeshRenderer{kMesh,"meshRenderer"};
ref::Class kInstantiate{"Last.Battle","InstantiateManager"};
ref::Method mFromCondition{kInstantiate,"InitEnemyEntityFromCondition",2};
ref::Method mInitEnemy{kInstantiate,"InitEnemyEntity",2};
ref::Class kSkin{"Last.Battle","ChangeSkinColorConditionFunctionBase"};
ref::Method mSkinStart{kSkin,"Start",0};
ref::Method mSkinRemove{kSkin,"Remove",0};
ref::Field fSkinChanged{kSkin,"changedSkinColor"};
ref::Field fSkinRenderer{kSkin,"effectRender"};
ref::Class kEffects{"Last.Battle", "ConditionEffectEntityController"};
ref::Class kEffects4{"Last.Battle", "ConditionEffectEntityController",game::kFF1 | game::kFF2 | game::kFF4 | game::kFF5 | game::kFF6};
ref::Method mPlay{kEffects4,"Play",4};
ref::Class kEffects3{"Last.Battle", "ConditionEffectEntityController",game::kFF3};
ref::Method mPlay3{kEffects3,"Play",3};
ref::Field fEffects{kEffects,"conditionEffectList"};
ref::Class kEffect{"Last.Battle", "ConditionEffectEntity"};
ref::Field fType{kEffect,"conditionType"};
ref::Class kShader{"", "PlayShader"};
ref::Field fShaderTarget{kShader,"targetSR"};
ref::Class kRenderer{"UnityEngine","Renderer"};
struct Label { int type=-1;uint64_t generation=0;std::wstring name; };
struct Binding {
    void* controller=nullptr;
    void* sprite=nullptr;
    int transformation=-1;
    uint64_t appearance_revision=0;
    uint64_t skin_revision=0;
    uint64_t generation=0;
    std::wstring enemy;
    std::array<Label,64> conditions;
};
std::array<Binding,64> bindings;
worker_job::Owner work;
uint64_t generation=0;
unsigned pending=0;

void retire() {
    ++generation;pending=0;work.interrupt(); // transition prefix: scalar/epoch work only
}
void clear() {
    retire();work.cancel();frame::cancel_label("EnemyStatus.Visible");pending=0;
    // Fixed-capacity old label storage can be reused lazily. Avoid destroying
    // thousands of strings in a native loading/transition callback.
}
Binding* binding_for(void* controller) {
    if(!controller)return nullptr;
    for(auto& binding:bindings)if(binding.generation==generation && binding.controller==controller)return &binding;
    return nullptr;
}
void bind(void* unit,void* condition) {
    if(!frame::on_main_thread() || !unit || !condition || !kBattleEnemyData.is(unit))return;
    void* sprite=displayed::field(unit_info(unit),"<BattleSpriteEntity>k__BackingField");
    void* controller=displayed::field(sprite,"ConditionEffectEntityController");
    if(!unity::alive(controller))return;
    auto* binding=binding_for(controller);
    if(!binding)for(auto& candidate:bindings)if(!candidate.controller || candidate.generation!=generation){
        binding=&candidate;binding->controller=controller;binding->sprite=sprite;
        binding->transformation=-1;++binding->appearance_revision;binding->generation=generation;break;
    }
    if(!binding)return;
    binding->enemy=enemy_name(unit).substr(0,256);
    const int type=condition_type(condition);
    Label* label=nullptr;
    for(auto& at:binding->conditions)if(at.generation==generation && at.type==type){label=&at;break;}
    if(!label)for(auto& at:binding->conditions)if(at.type<0 || at.generation!=generation){label=&at;break;}
    if(label){label->type=type;label->generation=generation;label->name=condition_name(condition).substr(0,256);}
}
Binding* binding_for_sprite(void* sprite) {
    if(!sprite)return nullptr;
    for(auto& binding:bindings)if(binding.generation==generation && binding.sprite==sprite)return &binding;
    return nullptr;
}
// The predicate is retained ONLY in the native-owner registry/frame task. It can
// hold Unity handles; the formatter receives strings/locale and nothing else.
// -1 cancels, 0 waits for native rendering, 1 is currently visible.
void appearance(Binding& binding,std::wstring label,std::function<int()> state) {
    if(!ui::prefs::enemy_status_announcements_enabled() || pending>=64 || binding.enemy.empty())return;
    const auto owner_generation=generation;const auto name=binding.enemy;
    if(label.empty())label=T(L"Status effect");
    ++pending;
    frame::every_frame([owner_generation,name,label,state=std::move(state)] {
        if(owner_generation!=generation)return false;
        if(!ui::prefs::enemy_status_announcements_enabled()){--pending;return false;}
        const int ready=state();if(ready<0){--pending;return false;}
        if(!in_battle() || !ready)return true;
        struct Input{std::wstring name,label,locale;};
        worker_job::start(work,Input{name,label,modtext::lang()},[](Input& input){
            modtext::LocaleScope locale(input.locale);return input.name+L": "+input.label;
        },[owner_generation,state](std::wstring line){
            if(owner_generation!=generation)return;
            --pending;
            if(ui::prefs::enemy_status_announcements_enabled() && state()==1)say(line,false);
        });
        return false;
    },"EnemyStatus.Visible",INT_MAX);
}
// A native condition requests a transformed monster asset asynchronously. A
// later SetSprite is the actual presentation producer. Native restoration or a
// superseding request increments the revision and cancels the earlier request.
thread_local void* condition_initializing=nullptr;
struct ConditionScope {
    void* previous=condition_initializing;
    explicit ConditionScope(void* entity){condition_initializing=entity;}
    ~ConditionScope(){condition_initializing=previous;}
};
using InitEnemy=void(*)(void*,void*,int32_t,const void*);
InitEnemy o_from_condition,o_init_enemy;
void d_from_condition(void* self,void* entity,int32_t type,const void* mi) {
    if(frame::on_main_thread())if(auto* binding=binding_for_sprite(entity)) {
        binding->transformation=type;++binding->appearance_revision;
    }
    ConditionScope scope(entity);o_from_condition(self,entity,type,mi);
}
void d_init_enemy(void* self,void* entity,int32_t asset,const void* mi) {
    if(frame::on_main_thread() && entity!=condition_initializing)if(auto* binding=binding_for_sprite(entity)) {
        binding->transformation=-1;++binding->appearance_revision;
    }
    o_init_enemy(self,entity,asset,mi);
}
using SetSprite=void(*)(void*,void*,const void*);
SetSprite o_set_sprite;
void d_set_sprite(void* self,void* sprite,const void* mi) {
    if(!frame::on_main_thread()){o_set_sprite(self,sprite,mi);return;}
    void* mesh=nullptr;void* before=nullptr;
    hooks::guarded("EnemyStatus.BeforeSprite",[&]{if(binding_for_sprite(self)) {
        mesh=ref::rd<void*>(self,fCharacterRenderer);before=ref::rd<void*>(mesh,fMeshSprite);
    }});
    o_set_sprite(self,sprite,mi);
    hooks::guarded("EnemyStatus.Transformation",[=]{
        auto* binding=binding_for_sprite(self);
        if(!binding || !sprite || before==sprite || binding->transformation<0 ||
            ref::rd<void*>(mesh,fMeshSprite)!=sprite)return;
        const int type=binding->transformation;binding->transformation=-1;
        const auto revision=binding->appearance_revision;
        std::wstring label;
        for(const auto& at:binding->conditions)if(at.generation==generation && at.type==type){label=at.name;break;}
        appearance(*binding,std::move(label),[binding,revision,mesh,sprite]{
            if(binding->appearance_revision!=revision || !unity::alive(mesh) ||
                ref::rd<void*>(mesh,fMeshSprite)!=sprite)return -1;
            return displayed::renderer_visible(ref::rd<void*>(mesh,fMeshRenderer))?1:0;
        });
    });
}
using SkinStart=void(*)(void*,const void*);
SkinStart o_skin_start;
SkinStart o_skin_remove;
Binding* skin_owner(void* self) {
    void* sprite=displayed::field(unit_info(ref::rd<void*>(self,fUnit)),"<BattleSpriteEntity>k__BackingField");
    return binding_for_sprite(sprite);
}
void d_skin_remove(void* self,const void* mi) {
    hooks::guarded("EnemyStatus.MaterialRemoved",[=]{
        if(frame::on_main_thread() && kSkin.is(self))if(auto* binding=skin_owner(self))++binding->skin_revision;
    });
    o_skin_remove(self,mi);
}
void d_skin_start(void* self,const void* mi) {
    const bool was=hooks::guarded_r("EnemyStatus.BeforeMaterial",true,[=]{
        return frame::on_main_thread() && kSkin.is(self) && ref::rd<bool>(self,fSkinChanged);
    });
    o_skin_start(self,mi);
    hooks::guarded("EnemyStatus.Material",[=]{
        if(was || !frame::on_main_thread() || !kSkin.is(self) || !ref::rd<bool>(self,fSkinChanged))return;
        auto* binding=skin_owner(self);void* renderer=ref::rd<void*>(self,fSkinRenderer);
        if(!binding || !unity::alive(renderer))return;
        // Native skin palettes are not uniquely distinguishable across every
        // condition. Announce the actual changed material without hidden naming.
        const auto revision=++binding->skin_revision;
        appearance(*binding,T(L"Status effect"),[binding,revision,renderer]{
            // No managed condition-function pointer survives its native Start.
            // Remove/superseding material changes retire this renderer witness.
            if(binding->skin_revision!=revision || !unity::alive(renderer))return -1;
            return displayed::renderer_visible(renderer)?1:0;
        });
    });
}
bool unique_visual(int asset,int type) {
    const auto* record=data::game_json("conditionvisuals").get(std::to_wstring(asset));
    const auto* unique=record?record->get(L"unique"):nullptr;
    const auto* types=record?record->get(L"types"):nullptr;
    return unique && unique->b && types && types->arr.size()==1 && types->arr[0].n==type;
}
struct RenderWitness {
    void* effect=nullptr;
    void* shader=nullptr;
    std::vector<void*> renderers;
};
bool visible_effect(const RenderWitness& witness) {
    if(!displayed::container_visible(witness.effect))return false;
    for(void* renderer:witness.renderers)if(displayed::renderer_visible(renderer))return true;
    // Some native status prefabs tint the enemy via PlayShader instead of
    // drawing an independent sprite. Verify its assigned on-screen renderer.
    return unity::alive(witness.shader) && displayed::renderer_visible(ref::rd<void*>(witness.shader,fShaderTarget));
}
void created(void* controller,void* effect,int asset,int type) {
    if(!ui::prefs::enemy_status_announcements_enabled() || pending>=64)return;
    auto* binding=binding_for(controller);
    if(!binding || binding->enemy.empty())return;
    std::wstring label;
    if(unique_visual(asset,type))for(const auto& at:binding->conditions)if(at.generation==generation && at.type==type){label=at.name;break;}
    // Different statuses can share a visual. Describe that visible effect rather
    // than disclose which indistinguishable hidden condition produced it.
    if(label.empty())label=T(L"Status effect");
    const auto owner_generation=generation;
    const auto name=binding->enemy;
    auto witness=std::make_shared<RenderWitness>();witness->effect=effect;
    // Resolve this small effect prefab once. Waiting frames inspect only the
    // captured, lifetime-checked renderer handles, never repeat a tree search.
    witness->renderers=unity::components_in_children(effect,kRenderer,true);
    // All85 installed condition-effect bundles were inspected: the largest
    // contains36 serialized renderers (FF5), so32 would reject a real effect.
    if(witness->renderers.size()>64)return;
    witness->shader=unity::get_component_in_children(effect,kShader,true);
    if(witness->renderers.empty() && !witness->shader)return;
    ++pending;
    frame::every_frame([owner_generation,witness,type,name,label] {
        if(owner_generation!=generation)return false;
        void* effect=witness->effect;
        if(!ui::prefs::enemy_status_announcements_enabled() ||
            !displayed::container_visible(effect) || ref::rd<int32_t>(effect,fType)!=type){--pending;return false;}
        if(!in_battle())return true; // initial native effects can precede BattleStart
        if(!visible_effect(*witness))return true;
        struct Input {std::wstring name,label,locale;};
        worker_job::start(work,Input{name,label,modtext::lang()},[](Input& input) {
            modtext::LocaleScope locale(input.locale);
            return input.name+L": "+input.label;
        },[owner_generation,witness,type](std::wstring line) {
            if(owner_generation!=generation)return;
            --pending; // includes formatting/publication backlog, not just waiting frames
            if(ui::prefs::enemy_status_announcements_enabled() && visible_effect(*witness) &&
                ref::rd<int32_t>(witness->effect,fType)==type)say(line,false);
        });
        return false;
    },"EnemyStatus.Visible",INT_MAX);
}
using Init=void(*)(void*,void*,void*,const void*);
Init o_init;
void d_init(void* self,void* unit,void* condition,const void* mi) {
    hooks::guarded("EnemyStatus.Identity",[=]{bind(unit,condition);});
    o_init(self,unit,condition,mi);
    // Initial sprite construction can finish inside native Init. No worker ever
    // receives the unit, condition, controller or effect pointers.
    hooks::guarded("EnemyStatus.IdentityReady",[=]{bind(unit,condition);});
}
using Play=void(*)(void*,void*,int32_t,int32_t,bool,const void*);
Play o_play;
int before_play(void* self,bool enemy) {
    if(enemy && frame::on_main_thread() && kEffects.is(self) && ui::prefs::enemy_status_announcements_enabled())
        return unity::list_count(ref::rd<void*>(self,fEffects));
    return -1;
}
void after_play(void* self,int asset,int type,int before) {
    if(before<0)return;
    hooks::guarded("EnemyStatus.RenderedEffect",[=] {
        void* list=ref::rd<void*>(self,fEffects);const int count=unity::list_count(list);
        if(count<=before || count>64)return; // native repeated-Play guard created nothing
        void* effect=unity::list_at(list,count-1);
        if(unity::alive(effect) && ref::rd<int32_t>(effect,fType)==type)created(self,effect,asset,type);
    });
}
void d_play(void* self,void* prefab,int32_t asset,int32_t type,bool native_flag,const void* mi) {
    // FF1/2 call this flag isEnemy; FF4/5/6 use isCopyEffect instead. Enemy
    // identity always comes from the actual Init-bound BattleEnemyData owner.
    const int before=before_play(self,frame::on_main_thread() && binding_for(self)!=nullptr);
    o_play(self,prefab,asset,type,native_flag,mi);
    after_play(self,asset,type,before);
}
using Play3=void(*)(void*,void*,int32_t,int32_t,const void*);
Play3 o_play3;
void d_play3(void* self,void* prefab,int32_t asset,int32_t type,const void* mi) {
    // FF3 omits isEnemy entirely. Only an Init-bound BattleEnemyData controller
    // is eligible; never read a nonexistent fifth native argument.
    const int before=before_play(self,frame::on_main_thread() && binding_for(self)!=nullptr);
    o_play3(self,prefab,asset,type,mi);
    after_play(self,asset,type,before);
}
int install() {
    events::on(events::Kind::BattleEntering,[]{retire();});
    events::on(events::Kind::BattleEnd,[]{clear();});
    events::on(events::Kind::TitleScreen,[]{clear();});
    const bool play=game::is(game::Id::FF3)?hooks::attach(mPlay3,&d_play3,&o_play3):hooks::attach(mPlay,&d_play,&o_play);
    int n=(hooks::attach(mInit,&d_init,&o_init)?1:0)+(play?1:0);
    n+=hooks::attach(mFromCondition,&d_from_condition,&o_from_condition)?1:0;
    n+=hooks::attach(mInitEnemy,&d_init_enemy,&o_init_enemy)?1:0;
    n+=hooks::attach(mSetSprite,&d_set_sprite,&o_set_sprite)?1:0;
    n+=hooks::attach(mSkinStart,&d_skin_start,&o_skin_start)?1:0;
    return n+(hooks::attach(mSkinRemove,&d_skin_remove,&o_skin_remove)?1:0);
}
core::Install s_install{"Battle.EnemyStatus",&install,117};
}
}
