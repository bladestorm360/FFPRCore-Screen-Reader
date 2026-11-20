#include "field_internal.h"
#include "field_api.h"
#include "object_cache.h"
#include "../core.h"
#include "../dialogue/dialogue.h"
#include "../displayed_ui.h"
#include "../frame.h"
#include "../modtext.h"
#include "../say.h"

namespace ffpr::dialogue {
namespace {
namespace r = ::field::r;

ref::Class kManager{"Last.Message", "MessageWindowManager"};
ref::Field fFieldWindow{kManager, "fieldWindowController"};
ref::Field fCurrentWindow{kManager, "currentWindowController"};
ref::Class kWindow{"Last.Message", "MessageWindowController"};
ref::Field fView{kWindow, "view"};
ref::Class kView{"Last.Message", "MessageWindowView"};
ref::Field fMessageText{kView, "messageText"};
ref::Class kClient{"Last.Management", "MessageWindowClient"};
ref::Method mClient{kClient, "Instance", 0};
ref::Field fMessageId{kClient, "<CurrentMessageId>k__BackingField"};

ref::Field fEventOwner{r::kFieldController, "eventHandle"};
ref::Class kEventProcedure{"Last.Map", "EventProcedure"};
ref::Field fRunAction{kEventProcedure, "runAction"};
ref::Class kEventAction{"Last.Map", "EventAction"};
ref::Class kScriptAction{"Last.Map", "EventActionScript"};
ref::Field fScriptName{kScriptAction, "scriptName"};
ref::Field fObserver{r::kFieldController, "interactiveTargetObserver"};
ref::Class kObserver{"Last.Map", "InteractiveTargetObserver"};
ref::Field fTarget{kObserver, "interactiveTargetEntity"};

ref::Class kSpriteEntity{"Last.Entity.Field", "FieldSpriteEntity"};
ref::Field fRenderer{kSpriteEntity, "spriteRenderer"};
ref::Class kRenderer{"UnityEngine", "SpriteRenderer"};
ref::Method mSprite{kRenderer, "get_sprite", 0};
ref::Class kSprite{"UnityEngine", "Sprite"};
ref::Method mTexture{kSprite, "get_texture", 0};

std::wstring field_message_key()
{
    auto* manager = ref::instance(kManager);
    auto* window = ref::rd<void*>(manager, fCurrentWindow);
    if (!window || window != ref::rd<void*>(manager, fFieldWindow) ||
        !displayed::visible(ref::rd<void*>(ref::rd<void*>(window, fView), fMessageText))) return {};
    // The six installed SetContent implementations have one native caller.
    // MessageWindowClient.Play records the corresponding ID before setting content.
    auto* client = ref::call_static<void*>(mClient);
    return ref::rd_str_bounded(client, fMessageId, 96);
}

bool matches_direct_talk(void* actor, const std::wstring& message)
{
    if (!r::kFieldNonPlayer.is(actor) || !::field::live_entity(actor)) return false;
    auto* property = ::field::entity_property(actor);
    return r::kPropertyTalk.is(property) && r::fEventScriptId &&
           ref::rd<int>(property, r::fEventScriptId) == 0 &&
           ref::rd_str_bounded(property, r::fTalkMessageKey, 96) == message &&
           displayed::renderer_visible(ref::rd<void*>(actor, fRenderer));
}

void* unique_actor(void* controller, const auto& matches)
{
    auto* actors = ref::rd<void*>(controller, r::fEntityList);
    const int count = unity::list_count(actors);
    if (count < 1 || count > 64) return nullptr;
    void* result = nullptr;
    for (int i = 0; i < count; ++i) {
        auto* actor = unity::list_at(actors, i);
        if (!matches(actor)) continue;
        if (result && result != actor) return nullptr;
        result = actor;
    }
    return result;
}

void* direct_talk_actor(void* controller, const std::wstring& message)
{
    auto* target = ref::rd<void*>(ref::rd<void*>(controller, fObserver), fTarget);
    if (matches_direct_talk(target, message)) return target;
    // When the indicator clears, require a unique visible actor with this key.
    return unique_actor(controller, [&](void* actor) { return matches_direct_talk(actor, message); });
}

void* scripted_talk_actor(void* controller, void* action, const std::wstring& message)
{
    // Garland's first two lines belong to his pre-battle scene. A separate
    // speaker and costume appear in its continuation back at Cornelia.
    if (!game::is(game::Id::FF1) || (message != L"MSG_HELPSERA_01" && message != L"MSG_HELPSERA_02") ||
        ref::rd_str_bounded(action, fScriptName, 48) != L"sc_e_0003" || ::field::current_map_id() != 72)
        return nullptr;
    return unique_actor(controller, [](void* actor) {
        return kSpriteEntity.is(actor) && ::field::live_entity(actor) &&
               ref::rd<int>(::field::entity_property(actor), r::fPropEntityId) == 32 &&
               displayed::renderer_visible(ref::rd<void*>(actor, fRenderer));
    });
}

std::wstring visible_art(void* actor)
{
    if (!actor) return {};
    auto* sprite = ref::call<void*>(mSprite, ref::rd<void*>(actor, fRenderer));
    if (!unity::alive(sprite)) return {};
    // All six packed field atlases place the authored costume key immediately
    // before the texture's final hash. Pose sprite names are shared by outfits.
    const auto texture = unity::name(ref::call<void*>(mTexture, sprite));
    if (texture.size() > 128 || !texture.starts_with(L"sactx-")) return {};
    const auto start = texture.find(L"-MO_FF");
    if (start == std::wstring::npos) return {};
    const auto end = texture.find(L'-', start + 1);
    if (end == std::wstring::npos || end - start > 49) return {};
    auto art = texture.substr(start + 1, end - start - 1);
    for (wchar_t& c : art) {
        if (c >= L'A' && c <= L'Z') c += L'a' - L'A';
        if (!((c >= L'a' && c <= L'z') || (c >= L'0' && c <= L'9') || c == L'_')) return {};
    }
    return art;
}
} // namespace

bool read_dialogue_appearance()
{
    if (!frame::on_main_thread() || !in_dialogue() || queries::is("GameStatePatches.IsInputTransitionBlocked")) return false;
    const auto message = field_message_key();
    if (message.empty()) return false;
    auto* map = ::field::cache::get(r::kFieldMap);
    auto* controller = ref::rd<void*>(map, r::fFieldController);
    if (!r::kFieldController.is(controller)) return false;
    auto* event_owner = ref::rd<void*>(controller, fEventOwner);
    if (!kEventProcedure.is(event_owner)) return false;
    auto* action = ref::rd<void*>(event_owner, fRunAction);
    if (!kEventAction.is(action) || !kScriptAction) return false;
    const bool scripted = kScriptAction.is(action);
    const auto art = visible_art(scripted ? scripted_talk_actor(controller, action, message)
                                          : direct_talk_actor(controller, message));
    if (art.empty()) return false;
    const auto key = L"Field appearance " + art;
    if (!modtext::has(key)) return false;
    say(T(key), true);
    return true;
}
} // namespace ffpr::dialogue
