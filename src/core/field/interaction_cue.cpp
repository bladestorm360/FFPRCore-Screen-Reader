// Native interaction-marker appearances, not proximity polling. PlayNear/Front
// own the range/selection decision; Hide and draw-disable retire that occurrence.
// Only copied labels/locale go to the formatting worker. See docs/MAP_LABELS.md.
#include "field_internal.h"
#include "event_labels.h"
#include "entity_translator.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../system/system.h"
#include "../ui/preferences.h"
#include "../worker_job.h"
#include <array>

namespace field::interaction_cue {
namespace {
ref::Class kIcon{"Last.Entity.Field", "FieldInteractiveIcon"};
ref::Method mNear{kIcon, "PlayNear", 0};
ref::Method mFront{kIcon, "PlayFront", 0};
ref::Method mHide{kIcon, "Hide", 0};
ref::Method mDraw{kIcon, "SetDrawEnable", 1};
ref::Field fOwner{kIcon, "OwnerInteractiveEntity"};
ref::Field fType{kIcon, "interactiveIconType"};
ref::Field fDraw{kIcon, "rendererEnable"};
ref::Class kSprite{"Last.Entity.Field", "FieldSpriteEntity"};
ref::Field fRenderer{kSprite, "spriteRenderer"};

struct Occurrence {
    void* icon = nullptr;
    void* object = nullptr;
    bool submitted = false;
    worker_job::Owner work;
};
// Only currently visible interaction opportunities occupy slots. Native Hide,
// draw-disable and scene retirement cancel pending work and immediately free them.
std::array<Occurrence,64> occurrences;
void retire(Occurrence& at) { at.work.cancel();at.icon=at.object=nullptr;at.submitted=false; }
void hide(void* icon) { for(auto& at:occurrences)if(at.icon==icon)retire(at); }
void clear() { for(auto& at:occurrences)retire(at); }
bool ready(void* icon) {
    return ffpr::system::is_field_navigation_ready() && unity::alive(icon) &&
        ref::rd<int32_t>(icon,fType)!=0 && ref::rd<bool>(icon,fDraw) &&
        displayed::renderer_visible(ref::rd<void*>(icon,fRenderer));
}
std::wstring object_label(void* object) {
    if(r::kFieldTresureBox.is(object))return T(L"Treasure Chest"); // type only, never contents
    if(r::kFieldNonPlayer.is(object)) {
        // NPC labels use the same prefix parser/translation as pathfinding.
        // Keep the descriptive base; script IDs and instance ordinals are not
        // useful when the native marker identifies this exact nearby actor.
        auto name=npc_display_label(entity_name_from_property(object)).base;
        return name.empty()?T(L"NPC"):name;
    }
    return safe_event_label(object,L"Interactive Object");
}
void observed(void* icon) {
    if(!ui::prefs::speech_enabled()) { hide(icon);return; }
    void* object=ref::rd<void*>(icon,fOwner);
    Occurrence* slot=nullptr;
    for(auto& at:occurrences)if(at.icon==icon) { slot=&at;break; }
    // These native calls can recur with the same animation every frame. The
    // event identity is icon+owner until Hide, not the contents of its label.
    if(slot && slot->object==object && slot->submitted)return;
    if(!object || !ready(icon))return;
    if(!slot)for(auto& at:occurrences)if(!at.icon) { slot=&at;break; }
    if(!slot)return; // not marked consumed; a later native update can retry
    retire(*slot);slot->icon=icon;slot->object=object;slot->submitted=true;
    struct Input { std::wstring label,locale; };
    worker_job::start(slot->work,Input{object_label(object),modtext::lang()},[](Input& input) {
        modtext::LocaleScope locale(input.locale);
        return TF(L"Interact: {0}",{input.label});
    },[icon,object](std::wstring line) {
        if(ready(icon) && ref::rd<void*>(icon,fOwner)==object)ffpr::say(line,false);
    });
}
using Play=void(*)(void*,const void*);
Play o_near,o_front,o_hide;
void d_near(void* self,const void* mi) { o_near(self,mi);hooks::guarded("Interaction.Near",[=]{if(kIcon.is(self))observed(self);}); }
void d_front(void* self,const void* mi) { o_front(self,mi);hooks::guarded("Interaction.Front",[=]{if(kIcon.is(self))observed(self);}); }
void d_hide(void* self,const void* mi) { o_hide(self,mi);hooks::guarded("Interaction.Hide",[=]{hide(self);}); }
using Draw=void(*)(void*,bool,const void*);
Draw o_draw;
void d_draw(void* self,bool enable,const void* mi) {
    o_draw(self,enable,mi);
    if(!enable)hooks::guarded("Interaction.DrawDisabled",[=]{hide(self);});
}
int install() {
    events::on(events::Kind::SceneLoaded,[]{if(!ffpr::system::is_field_navigation_ready())clear();});
    events::on(events::Kind::BattleEntering,[]{for(auto& at:occurrences)at.work.interrupt();});
    events::on(events::Kind::MapChanging,[]{for(auto& at:occurrences)at.work.interrupt();});
    events::on(events::Kind::PartyChanging,[]{for(auto& at:occurrences)at.work.interrupt();});
    events::on(events::Kind::MapChanged,[]{clear();});
    events::on(events::Kind::BattleStart,[]{clear();});
    int n=hooks::attach(mNear,&d_near,&o_near)?1:0;
    n+=hooks::attach(mFront,&d_front,&o_front)?1:0;
    n+=hooks::attach(mHide,&d_hide,&o_hide)?1:0;
    return n+(hooks::attach(mDraw,&d_draw,&o_draw)?1:0);
}
core::Install s_install{"Field.InteractionCue",&install,116};
}
}
