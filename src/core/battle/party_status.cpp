// H and number-row condition review outside the native targeting panel. The
// ordinary HUD replaces condition icons with queued commands, but the party's
// bodies still show Mini/Toad, poison tint, paralysis effects and danger poses.
// Observe completed native presentation; never build a hidden condition inventory.
// Native evidence and bounds: docs/re/ALLY_STATUS_20261026.md.
#include "party_status.h"
#include "battle_util.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../hooks.h"
#include "../il2cpp.h"
#include "../modtext.h"
#include "../refs.h"
#include "../unity.h"
#include <array>
#include <cmath>
#include <cstdint>

namespace ffpr::battle::party_status {
namespace {
ref::Class kColors{"Last.UI", "TextColors"};
ref::Class kGraphic{"UnityEngine.UI", "Graphic"};
ref::Method mColor{kGraphic, "get_color", 0};
ref::Class kPlayer{"Last.Battle", "BattlePlayerEntity"};
ref::Class kSprite{"Last.Battle", "BattleSpriteEntity"};
ref::Field fBody{kSprite, "characterSR"};
ref::Class kMesh{"Last.Battle", "EntityMeshRenderer"};
ref::Field fRenderer{kMesh, "meshRenderer"}, fSprite{kMesh, "sprite"};
ref::Field fAnimation{kPlayer, "spriteAnimBattle"};
ref::Class kAnimation{"Last.Battle", "SpriteAnimBattle"};
ref::Field fMotions{kAnimation, "<battleCharaSpriteList>k__BackingField"};
ref::Class kInstantiate{"Last.Battle", "InstantiateManager"};
ref::Method mChange{kInstantiate, "ChangePlayerMotions", 4};
ref::Class kSkin{"Last.Battle", "ChangeSkinColorConditionFunctionBase"};
ref::Field fSkinChanged{kSkin, "changedSkinColor"}, fSkinRenderer{kSkin, "effectRender"};
ref::Class kEffects{"Last.Battle", "ConditionEffectEntityController"};
ref::Field fEffects{kEffects, "conditionEffectList"};
ref::Field fEffectController{kSprite, "ConditionEffectEntityController"};
ref::Class kEffect{"Last.Battle", "ConditionEffectEntity"};
ref::Field fEffectType{kEffect, "conditionType"}, fEffectAsset{kEffect,"effectId"};
ref::Class kInspector{"Last.Battle.Effect","EffectReactionInspector"};
ref::Field fReaction{kInspector,"currentReaction"},fReactionType{kInspector,"currentType"},
           fReactionStarted{kInspector,"isStart"},fReactionEnded{kInspector,"isEnd"};
ref::Class kReaction{"Last.Battle.Effect","EffectReaction"};
ref::Field fReactionTargets{kReaction,"entityMeshRenderers"};
ref::Class kEffectBase{"Last.Battle", "EffectEntityBase"};
ref::Field fParticles{kEffectBase, "particleSystems"};
ref::Class kParticle{"UnityEngine", "ParticleSystem"};
ref::Method mPlaying{kParticle, "get_isPlaying", 0};
ref::Class kRenderer{"UnityEngine", "Renderer"};

constexpr int kFunctions = 128, kEffectsLimit = 64, kRendererWork = 64;
struct Color { float r, g, b, a; };
struct Form { void* entity = nullptr; void* motions = nullptr; int type = 0; };
std::array<Form,16> forms{};
ref::Class kConditionUI{"Last.UI","BattleConditionController"};
ref::Method mConditionText{kConditionUI,"SetConditionText",1};
struct TextCue {void* owner=nullptr;void* text=nullptr;int type=0;};
std::array<TextCue,32> text_cues{};
using SetText=void(*)(void*,void*,const void*);
SetText o_text;
void d_text(void* self,void* condition,const void* mi){
    o_text(self,condition,mi);
    hooks::guarded("PartyStatus.ConditionTextIdentity",[=]{
        if(!kConditionUI.is(self)||!unity::alive(self))return;
        TextCue* slot=nullptr;
        for(auto& cue:text_cues)if(cue.owner==self){slot=&cue;break;}
        if(!slot)for(auto& cue:text_cues)if(!cue.owner||!unity::alive(cue.owner)){slot=&cue;break;}
        if(slot)*slot={self,displayed::field(displayed::field(self,"view"),"conditionText"),condition_type(condition)};
    });
}
bool text_represents(void* owner,int type){
    for(const auto& cue:text_cues)if(cue.owner==owner && cue.type==type &&
        cue.text==displayed::field(displayed::field(owner,"view"),"conditionText"))
        return !displayed::text(cue.text).empty();
    return false;
}

int hp_condition(void* view)
{
    void* hp = displayed::field(view, "currentHpText");
    const auto value=displayed::text(hp);
    if (value.empty()) return 0;
    int packed = 0;
    // Color32 is four bytes, RGBA, in all six TextColors classes. Reuse the
    // existing bounded static-field copy; no class initializer is invoked here.
    const bool zero=value==L"0";
    if (!il2cpp::read_static_int(kColors.get(), zero?"Gray":"Red", packed)) return 0;
    const auto bits = static_cast<uint32_t>(packed);
    if ((bits>>24)==0) return 0; // uninitialized palette is not a danger cue
    const auto color = ref::call<Color>(mColor, hp);
    const auto matches = [](float actual, unsigned byte) {
        return std::isfinite(actual) && std::fabs(actual - byte / 255.0f) < 0.5f / 255.0f;
    };
    return matches(color.r,bits&255) && matches(color.g,(bits>>8)&255) &&
           matches(color.b,(bits>>16)&255) && color.a > 0 ? (zero?5:4) : 0;
}

void* motions(void* entity) { return ref::rd<void*>(ref::rd<void*>(entity,fAnimation),fMotions); }
using Change = void (*)(void*,void*,int32_t,int32_t,int32_t,const void*);
Change o_change;
void d_change(void* self,void* entity,int32_t character,int32_t job,int32_t type,const void* mi)
{
    void* before = hooks::guarded_r("PartyStatus.BeforeForm",static_cast<void*>(nullptr),[=] {
        return kPlayer.is(entity) && unity::alive(entity) ? motions(entity) : nullptr;
    });
    o_change(self,entity,character,job,type,mi);
    hooks::guarded("PartyStatus.CompletedForm",[=] {
        if (!kPlayer.is(entity) || !unity::alive(entity)) return;
        void* after = motions(entity);
        Form* found = nullptr;
        for (auto& form:forms) if (form.entity==entity) { found=&form; break; }
        // A failed load that kept the old dictionary cannot prove a new form.
        if (!after || (after==before && (!found || found->type!=type))) {
            if(found)*found={};
            return;
        }
        if (!found) for (auto& form:forms) if (!form.entity || !unity::alive(form.entity)) {found=&form;break;}
        if (found) *found={entity,after,type};
    });
}

bool body_visible(void* entity)
{
    if (!kPlayer.is(entity) || !unity::alive(entity)) return false;
    void* mesh = ref::rd<void*>(entity,fBody);
    return unity::alive(ref::rd<void*>(mesh,fSprite)) &&
           displayed::renderer_visible(ref::rd<void*>(mesh,fRenderer));
}

// Ready component/array reads only: no hierarchy enumeration or scene search.
// The shared action budget bounds the TOTAL renderer work across all effects.
bool effect_visible(void* effect,void* body,int& work)
{
    if (!displayed::container_visible(effect)) return false;
    if (work-- <= 0) return false;
    if (displayed::renderer_visible(unity::get_component(effect,kRenderer))) return true;
    // Shader-only Protect/Shell/Haste/Slow prefabs have no particle renderer.
    // The ready native reaction identifies its actual target meshes. A live
    // started reaction on this visible body is the presentation witness; the
    // effect ID or raw condition alone is never sufficient.
    if(work>0){
        --work;
        void* inspector=unity::get_component(effect,kInspector);
        const int kind=ref::rd<int>(inspector,fReactionType);
        if(inspector && ref::rd<bool>(inspector,fReactionStarted) && !ref::rd<bool>(inspector,fReactionEnded) &&
            (kind==2||kind==3||kind==9||kind==10||kind==13||kind==25)){
            void* targets=ref::rd<void*>(ref::rd<void*>(inspector,fReaction),fReactionTargets);
            const int n=unity::array_length(targets);
            if(n>=0&&n<=9)for(int i=0;i<n && work>0;++i){
                --work;if(unity::array_at(targets,i)==body && displayed::renderer_visible(ref::rd<void*>(body,fRenderer)))return true;
            }
        }
    }
    // The exact FFVI Silence68 prefab is a three-object, one-sprite-renderer
    // bubble rather than particles. This one verified small prefab permits a
    // single local renderer lookup; no scene or general hierarchy scan is used.
    if(work>0 && game::is(game::Id::FF6) && ref::rd<int>(effect,fEffectType)==6 && ref::rd<int>(effect,fEffectAsset)==68){
        --work;if(displayed::renderer_visible(unity::get_component_in_children(effect,kRenderer,false)))return true;
    }
    void* particles = ref::rd<void*>(effect,fParticles);
    const int count = unity::array_length(particles);
    if (count<0 || count>kRendererWork) return false;
    for (int i=0;i<count && work>0;++i) {
        --work;
        void* particle = unity::array_at(particles,i);
        if (unity::alive(particle) && ref::call<bool>(mPlaying,particle) &&
            displayed::renderer_visible(unity::get_component(particle,kRenderer))) return true;
    }
    return false;
}

int install()
{
    events::on(events::Kind::BattleEntering,[]{forms={};text_cues={};});
    events::on(events::Kind::BattleEnd,[]{forms={};text_cues={};});
    events::on(events::Kind::TitleScreen,[]{forms={};text_cues={};});
    return (hooks::attach(mChange,&d_change,&o_change)?1:0)+(hooks::attach(mConditionText,&d_text,&o_text)?1:0);
}
core::Install s_install{"Battle.PartyStatus",&install,109};
}

std::vector<std::wstring> extra_lines(void* player,void* view,bool body_conditions,void* condition_controller)
{
    std::vector<std::wstring> lines;
    const int hp_state=hp_condition(view);
    // FFVI also writes Critical into its rotating text column. Select the
    // native condition identity as producer; never compare speech strings.
    if(hp_state && !text_represents(condition_controller,hp_state))lines.push_back(T(hp_state==5?L"KO":L"Critical"));
    if (!body_conditions || !player || !kBattlePlayerData.is(player)) return lines;
    void* info = unit_info(player);
    void* entity = displayed::field(info,"<BattleSpriteEntity>k__BackingField");
    if (!body_visible(entity)) return lines;
    void* functions = info_functions(info);
    const int count = unity::list_count(functions);
    if (count<0 || count>kFunctions) return lines;
    std::array<int,kFunctions> represented{};
    int represented_count = 0;
    // A native condition identity can have stacked functions. One current
    // condition is one review entry, independent of its localized text.
    auto add = [&](void* condition) {
        const int id=condition_id(condition);
        for(int i=0;i<represented_count;++i)if(represented[i]==id)return;
        const auto name=condition_name(condition);
        if(!name.empty()){represented[represented_count++]=id;lines.push_back(name);}
    };
    int form_type=0;
    for(const auto& form:forms)if(form.entity==entity && form.motions==motions(entity)){form_type=form.type;break;}
    void* effects=ref::rd<void*>(ref::rd<void*>(entity,fEffectController),fEffects);
    const int effect_count=unity::list_count(effects);
    std::array<int,kEffectsLimit> rendered_types{};
    int rendered_count=0,work=kRendererWork;
    if(effect_count>=0 && effect_count<=kEffectsLimit)for(int i=0;i<effect_count && work>0;++i){
        void* effect=unity::list_at(effects,i);
        if(kEffect.is(effect) && effect_visible(effect,ref::rd<void*>(entity,fBody),work))rendered_types[rendered_count++]=ref::rd<int>(effect,fEffectType);
    }
    for(int i=0;i<count;++i){
        void* function=unity::list_at(functions,i);
        void* condition=function_condition(function);
        if(!condition)continue;
        const int type=condition_type(condition);
        if(type==4 || (type==5&&hp_state==5))continue; // HP color owns Critical, including priority/KO resets.
        bool shown=(type==form_type && (type==401 || type==402 || type==403 || type==601));
        if(kSkin.is(function) && ref::rd<bool>(function,fSkinChanged) &&
            displayed::renderer_visible(ref::rd<void*>(function,fSkinRenderer)))shown=true;
        for(int j=0;j<rendered_count && !shown;++j)shown=rendered_types[j]==type;
        if(shown)add(condition);
    }
    return lines;
}
}
