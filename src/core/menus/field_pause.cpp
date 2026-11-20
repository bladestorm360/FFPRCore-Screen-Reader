// FieldHelp.Open establishes the desktop pause list and its initial cursor in
// all six games. Read its visible selection once the native screen is ready.
#include "../core.h"
#include "../frame.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../system/system.h"
#include "../ui_util.h"
#include "../unity.h"
#include <cstdint>

namespace ffpr::menus::field_pause {
namespace {
ref::Class kController{"Last.UI.KeyInput", "FieldHelpController"};
ref::Class kOpen15{"Last.UI.KeyInput", "FieldHelpController", game::kFF1 | game::kFF2 | game::kFF3 | game::kFF4 | game::kFF5};
ref::Class kOpen6{"Last.UI.KeyInput", "FieldHelpController", game::kFF6};
ref::Class kView{"Last.UI.KeyInput", "FieldHelpView"};
ref::Class kCommand{"Last.UI.KeyInput", "CommonCommandContentController"};
ref::Field fView{kController, "view"};
ref::Field fCursor{kController, "selectCommandCursor"};
ref::Field fPauseRoot{kView, "pauseRoot"};
ref::Field fCommands{kView, "commandControllerList"};
ref::Method mOpen{kOpen15, "Open", 1};
ref::Method mOpen6{kOpen6, "Open", 2};
ref::Method mIsOpen{kController, "get_IsOpen", 0};
ref::Method mText{kCommand, "get_CommandText", 0};

using Open = void(*)(void*, void*, const void*);
using Open6 = void(*)(void*, void*, int32_t, const void*);
Open original_open;
Open6 original_open6;
uint64_t opening = 0;

void read_opening(void* controller, uint64_t ticket)
{
    if (ticket != opening || system::input_transition_blocked() || !unity::alive(controller) ||
        !unity::active_in_hierarchy(controller) || !ref::call<bool>(mIsOpen, controller)) return;
    void* view = ref::rd<void*>(controller, fView);
    void* root = view ? ref::rd<void*>(view, fPauseRoot) : nullptr;
    if (!root || !unity::active_in_hierarchy(root)) return;
    void* list = ref::rd<void*>(view, fCommands);
    const int count = unity::list_count(list);
    void* cursor = ref::rd<void*>(controller, fCursor);
    if (!cursor || count <= 0 || count > 3) return;
    const int index = ui::cursor_index(cursor);
    if (index < 0 || index >= count) return;
    void* command = unity::list_at(list, index);
    if (!command || !unity::active_in_hierarchy(command)) return;
    auto name = unity::ui_text(ref::call<void*>(mText, command));
    if (!name.empty()) ffpr::say_sequence({T(L"Pause"), std::move(name)}, true);
}

void schedule_opening(void* controller)
{
    hooks::guarded("FieldPause.Open", [&] {
        if (!kController.is(controller)) return;
        const auto ticket = ++opening;
        frame::next_frame([controller, ticket] {
            hooks::guarded("FieldPause.ReadOpening", [&] { read_opening(controller, ticket); });
        }, "FieldPause.ReadOpening");
    });
}

void on_open(void* controller, void* cancel, const void* mi)
{
    original_open(controller, cancel, mi);
    schedule_opening(controller);
}

void on_open6(void* controller, void* cancel, int32_t type, const void* mi)
{
    original_open6(controller, cancel, type, mi);
    schedule_opening(controller);
}

int install()
{
    for (auto event : {events::Kind::MapChanging, events::Kind::BattleEntering, events::Kind::TitleScreen})
        events::on(event, [] { ++opening; });
    if (game::is(game::Id::FF6)) return hooks::attach(mOpen6, &on_open6, &original_open6) ? 1 : 0;
    return hooks::attach(mOpen, &on_open, &original_open) ? 1 : 0;
}

core::Install registration{"Menus.FieldPause", &install, 512};
}
}
