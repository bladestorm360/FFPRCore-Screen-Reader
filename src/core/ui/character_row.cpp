// Character selections share a short focus read and an optional charge tail.
// Native pointers stay on the UI owner. Only copied speech reaches workers.
#include "character_row.h"
#include "preferences.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../say.h"
#include "../system/system.h"
#include "../text.h"
#include "../ui_util.h"
#include "../unity.h"

namespace ui::character_row {
namespace {
ref::Class kOverviewBase{"Serial.Template.UI", "StatusWindowControllerBase"};
ref::Field fOverviewState{kOverviewBase, "stateMachine"};
ref::Class kOverviewInput{"Serial.Template.UI.KeyInput", "StatusWindowControllerBase"};
ref::Field fOverviewCursor{kOverviewInput, "selectCursor"};
ref::Field fOverviewTargetCursor{kOverviewInput, "targetSelectCursor"};
void* g_content = nullptr;
void* g_cursor = nullptr;
void* g_window = nullptr;

bool focused()
{
    if (!ffpr::system::is_main_menu_state() || !unity::alive(g_content) ||
        !displayed::container_visible(g_cursor) ||
        !displayed::image_visible(displayed::field(g_cursor, "image"))) return false;
    // IsFocus is a native selection-lock/scale flag, not keyboard focus:
    // SelectInit explicitly clears it on the active character cursor. Instead
    // require this window's actual selection phase and its corresponding cursor.
    // List/None retain visible overview portraits behind other menus.
    if (g_window) {
        if (!displayed::container_visible(g_window)) return false;
        const int phase=ui::state_tag(g_window,fOverviewState);
        if (phase<2 || phase>4) return false;
        auto& field=phase==4 ? fOverviewTargetCursor : fOverviewCursor;
        if (g_cursor!=ref::rd<void*>(g_window,field)) return false;
    }
    void* view = displayed::field(g_content, "view");
    if (!displayed::container_visible(view)) return false;
    return displayed::cursor_matches(g_cursor,g_content);
}
int install()
{
    queries::define_text("CharacterRow.Details", [] { return details(); });
    events::on(events::Kind::MapChanging, [] { clear(); });
    events::on(events::Kind::BattleEntering, [] { clear(); });
    return 0;
}
core::Install s_install{"UI.CharacterRow", &install, 125};
}
void clear() { g_content = g_cursor = g_window = nullptr; }
std::wstring details()
{
    if (!focused()) return {};
    const auto charges = displayed::charge_lines(displayed::field(g_content, "view"));
    // Focus speech already owns name/vitals/overview. Details are just the
    // optional MP tail, also used after that line by Autodetail.
    return text::join(charges, L", ");
}
void announce(void* content, void* cursor, std::wstring line, void* overview_window)
{
    g_content = content; g_cursor = cursor; g_window = overview_window;
    const auto charges = displayed::charge_lines(displayed::field(content, "view"));
    if (prefs::auto_detail_enabled() && !charges.empty())
        ffpr::say_sequence({std::move(line), text::join(charges, L", ")}, true);
    else ffpr::say(line, true);
}
}
