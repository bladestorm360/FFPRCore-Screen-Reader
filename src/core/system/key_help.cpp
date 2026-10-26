// Ports FF1 Menus/KeyHelpReader.cs: the controls display.
//   - Shift+I (AnnounceKeyHelp): reads the on-screen control-hint bar (KeyHelpController) at once.
//   - Arrows/WASD: step the "Gamepad/Keyboard Controls" pop-up one entry at a time, with a
//     navigation buffer armed ONLY by the config area's controls pop-up (open_controls_help); never
//     by the hint bar, which hijacked the title Options menu's arrows in FF1.
// The buffer is FF1 Core/NavigationBuffer.cs reduced to the flat list this reader uses (no groups).
#include <string>
#include <vector>
#include "core.h"
#include "hooks.h"
#include "log.h"
#include "modtext.h"
#include "refs.h"
#include "say.h"
#include "text.h"
#include "ui_util.h"
#include "unity.h"
#include "system/system.h"

namespace ffpr::system {
namespace {

ref::Class kKeyHelpController{"Last.UI.KeyInput", "KeyHelpController"};
ref::Method mInitialize{kKeyHelpController, "Initialize", 0};
ref::Method mUpdateView{kKeyHelpController,"UpdateView",1};
ref::Method mDestroy{kKeyHelpController,"OnDestroy",0};
using Initialize = void (*)(void*, const void*);
Initialize o_initialize;
void d_initialize(void* self, const void* mi) {
    o_initialize(self, mi);
    hooks::guarded("KeyHelp.Owner", [self] {
        if (kKeyHelpController.is(self)) register_controls_owner(self,false);
    });
}
using UpdateView=void(*)(void*,void*,const void*);
UpdateView o_update_view;
void d_update_view(void* self,void* values,const void* mi) {
    o_update_view(self,values,mi);
    hooks::guarded("KeyHelp.Published",[=]{register_controls_owner(self);});
}
Initialize o_destroy;
void d_destroy(void* self,const void* mi) {
    hooks::guarded("KeyHelp.Retired",[=]{retire_controls_owner(self);});
    o_destroy(self,mi);
}
// Battle Items/Equip and several ordinary screens render standalone widgets,
// not a KeyHelpController. Observe the common final renderer and lifecycle so
// Shift+I can read their actual glyph text/caption without searching the scene.
ref::Class kKeyIconController{"Last.UI.KeyInput","KeyIconController"};
ref::Method mIconInitialize{kKeyIconController,"Initialize",1};
ref::Method mIconRender{kKeyIconController,"UpdateView",4};
ref::Method mIconDestroy{kKeyIconController,"OnDestroy",0};
using IconInitialize=void(*)(void*,bool,const void*);
using IconRender=void(*)(void*,void*,void*,void*,bool,const void*);
IconInitialize o_icon_initialize;IconRender o_icon_render;Initialize o_icon_destroy;
void d_icon_initialize(void* self,bool change,const void* mi) {
    o_icon_initialize(self,change,mi);
    hooks::guarded("KeyHelp.WidgetOwner",[=]{register_controls_icon(self,false);});
}
void d_icon_render(void* self,void* message,void* second,void* keys,bool comma,const void* mi) {
    o_icon_render(self,message,second,keys,comma,mi);
    hooks::guarded("KeyHelp.WidgetPublished",[=]{register_controls_icon(self);});
}
void d_icon_destroy(void* self,const void* mi) {
    hooks::guarded("KeyHelp.WidgetRetired",[=]{retire_controls_icon(self);});
    o_icon_destroy(self,mi);
}

// NavigationBuffer over pre-rendered entries.
struct Buffer {
    std::vector<std::wstring> entries;
    int index = 0;
    bool empty() const { return entries.empty(); }
    int count() const { return static_cast<int>(entries.size()); }
    const std::wstring& current()
    {
        if (index < 0 || index >= count()) index = 0;
        return entries[static_cast<size_t>(index)];
    }
    const std::wstring& next() { index = (index + 1) % count(); return current(); }
    const std::wstring& previous() { if (--index < 0) index = count() - 1; return current(); }
    const std::wstring& top() { index = 0; return current(); }
    const std::wstring& bottom() { index = count() - 1; return current(); }
};

Buffer g_buffer;          // empty = no buffer (C# null)
bool g_primed = false;
// The controls pop-up state. g_help_owner validates the screen is still up, so a missed close
// cannot leave the KeyHelp key context stuck.
bool g_help_active = false;
void* g_help_owner = nullptr;   // ConfigKeysSettingController (identity; alive-checked)

void speak(const std::wstring& s)
{
    if (s.empty()) return;
    // Flat controls list: whole-buffer position (CurrentGroupPosition without groups).
    std::wstring line = g_buffer.empty() ? s : ui::menu_position(s, g_buffer.index, g_buffer.count());
    ffpr::say(line, true);
}

// Fallback orient-at-top if the open announce was missed.
const std::wstring& prime()
{
    g_primed = true;
    return g_buffer.current();
}

void navigate_next()
{
    if (g_buffer.empty()) return;
    speak(g_primed ? g_buffer.next() : prime());
}

void navigate_previous()
{
    if (g_buffer.empty()) return;
    speak(g_primed ? g_buffer.previous() : prime());
}

void jump_to_top()
{
    if (g_buffer.empty()) return;
    g_primed = true;
    speak(g_buffer.top());
}

void jump_to_bottom()
{
    if (g_buffer.empty()) return;
    g_primed = true;
    speak(g_buffer.bottom());
}

// AnnounceKeyHelp (Shift+I)
void announce_key_help()
{
    if(queries::is("Warehouse.IsActive")){actions::run("Warehouse.ReadControls");return;}
    if(queries::is("MapPanels.IsActive")){actions::run("MapPanels.ReadControls");return;}
    if(queries::is("BestiaryStateTracker.IsInDetail")){actions::run("Bestiary.ReadControls");return;}
    hooks::guarded("KeyHelpReader.AnnounceKeyHelp", [] { announce_current_controls(); });
}

int install()
{
    actions::define("KeyHelpReader.NavigateNext", [] { navigate_next(); });
    actions::define("KeyHelpReader.NavigatePrevious", [] { navigate_previous(); });
    actions::define("KeyHelpReader.JumpToTop", [] { jump_to_top(); });
    actions::define("KeyHelpReader.JumpToBottom", [] { jump_to_bottom(); });
    actions::define("KeyHelpReader.AnnounceKeyHelp", [] { announce_key_help(); });
    actions::define("KeyHelpReader.CloseControlsHelp", [] { close_controls_help(); });
    queries::define("KeyHelpReader.IsScreenActive", [] { return key_help_screen_active(); });

    // Requests/popups retire at scene changes. Lifecycle-owned bars may survive
    // additive preload and are still validated by their real view on each read.
    events::on(events::Kind::SceneLoaded, [] {
        cancel_controls_reading();
        close_controls_help();
    });
    int count=0;
    count+=hooks::attach(mInitialize,&d_initialize,&o_initialize)?1:0;
    count+=hooks::attach(mUpdateView,&d_update_view,&o_update_view)?1:0;
    count+=hooks::attach(mDestroy,&d_destroy,&o_destroy)?1:0;
    count+=hooks::attach(mIconInitialize,&d_icon_initialize,&o_icon_initialize)?1:0;
    count+=hooks::attach(mIconRender,&d_icon_render,&o_icon_render)?1:0;
    count+=hooks::attach(mIconDestroy,&d_icon_destroy,&o_icon_destroy)?1:0;
    return count;
}

core::Install s_install{"KeyHelpReader", &install, 100};

} // namespace

// OpenControlsHelp: the controls pop-up opened with its rendered entries; announce the first.
void open_controls_help(void* owner, const std::vector<std::wstring>& entries)
{
    g_help_owner = owner;
    if (entries.empty()) {
        g_buffer = Buffer{};
        g_help_active = false;
        g_primed = false;
        return;
    }
    g_buffer = Buffer{};
    g_buffer.entries = entries;
    g_help_active = true;
    g_primed = true;   // entry 0 is announced now; the first Down moves to entry 1
    speak(g_buffer.current());
}

// CloseControlsHelp: the pop-up closed / returned to the controls list.
void close_controls_help()
{
    g_help_active = false;
    g_help_owner = nullptr;
    g_buffer = Buffer{};
    g_primed = false;
}

// IsScreenActive: true while the controls pop-up is on screen.
bool key_help_screen_active()
{
    if (!g_help_active) return false;
    bool on_screen = false;
    bool ok = false;
    hooks::guarded("KeyHelpReader.IsScreenActive", [&] {
        on_screen = unity::alive(g_help_owner) && unity::active_in_hierarchy(g_help_owner);
        ok = true;
    });
    if (!ok || !on_screen) {
        close_controls_help();
        return false;
    }
    return true;
}

} // namespace ffpr::system
