// check-sources: text.cpp
// Runs the real FFV timer action against native-shaped objects without starting the game.
#include <cstddef>
#include <cstdio>
#include <map>
#include "../src/games/ff5/timers.cpp"

namespace fixture {
struct Widget {
    Widget* controller{};
    Widget* view{};
    Widget* minutes{};
    Widget* seconds{};
    Widget* group{};
    bool alive = true;
    bool shown = true;
    float alpha = 1.f;
    const wchar_t* value = L"";
};
struct Chord { input::Key key; input::Mod mod; };
Widget manager, timer, view, minutes, seconds, group;
Widget* instance = &manager;
bool ready = false;
int singleton_reads = 0, field_scans = 0, speech_count = 0;
std::wstring spoken;
std::map<std::string, actions::Fn> handlers;
std::map<std::string, Chord> chords;
float alpha(void* self, const void*) { return static_cast<Widget*>(self)->alpha; }
}

namespace ref {
Class::Class(const char* n, const char* type, uint8_t g, const char* lib)
    : ns(n), name(type), games(g), assembly(lib) {}
il2cpp::Class* Class::get() { return reinterpret_cast<il2cpp::Class*>(this); }
bool Class::is(void* ptr) { return ptr == &fixture::manager; }
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
int32_t Field::get()
{
    using fixture::Widget;
    const std::string_view n(name);
    if (n == "timerController") return offsetof(Widget, controller);
    if (n == "view") return offsetof(Widget, view);
    if (n == "canvasGroup") return offsetof(Widget, group);
    if (n == "playTimeMinuteText") return offsetof(Widget, minutes);
    if (n == "playTimeSecondText") return offsetof(Widget, seconds);
    return -1;
}
Method::Method(Class& c, const char* n, int args) : cls(c), name(n), argc(args) {}
il2cpp::Method* Method::get() { return nullptr; }
void* Method::ptr() { return std::string_view(name) == "get_alpha" ? reinterpret_cast<void*>(&fixture::alpha) : nullptr; }
void call_failed_note(const char*) {}
}
namespace il2cpp {
int parent_slot;
Class* class_parent(Class* cls)
{
    return cls == ffpr::ff5::kBattleTimerManager.get() ? reinterpret_cast<Class*>(&parent_slot) : nullptr;
}
void* read_static_ptr(Class* cls, const char* name)
{
    ++fixture::singleton_reads;
    return cls == reinterpret_cast<Class*>(&parent_slot) && std::string_view(name) == "instance" ? fixture::instance : nullptr;
}
}
namespace unity {
bool alive(void* widget) { return widget && static_cast<fixture::Widget*>(widget)->alive; }
bool active_in_hierarchy(void* widget) { return alive(widget) && static_cast<fixture::Widget*>(widget)->shown; }
}
namespace displayed {
bool container_visible(void* widget) { return unity::active_in_hierarchy(widget); }
std::wstring text(void* widget) { return container_visible(widget) ? static_cast<fixture::Widget*>(widget)->value : L""; }
}
namespace ffpr {
void say(std::wstring_view value, bool) { ++fixture::speech_count; fixture::spoken = value; }
}
namespace ffpr::ff5 { std::wstring text_of(void*) { return {}; } }
namespace field::cache { std::vector<void*> get_all(ref::Class&) { ++fixture::field_scans; return {}; } }
namespace field::map_state { std::optional<int> scenario_flag(int) { return {}; } }
namespace field { int current_map_id() { return 0; } }
namespace ffpr::battle { bool battle_presentation_ready() { return fixture::ready; } }
namespace ui::mod_menu { bool is_open() { return false; } }
namespace game { const Flags& flags() { static const auto enabled = [] { Flags f; f.timer_freeze = true; return f; }(); return enabled; } }
namespace core { Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {} }
namespace actions { void define(const char* name, Fn fn) { fixture::handlers.emplace(name, std::move(fn)); } }
namespace queries {
void define(const char*, std::function<bool()>) {}
bool is(const char*) { return false; }
}
namespace input {
void add_binding(Key key, Mod mod, Context, const char* action, const char*)
{
    fixture::chords.emplace(action, fixture::Chord{key, mod});
}
}
namespace hooks {
bool attach_raw(ref::Method&, void*, void**) { return true; }
void note_fault(const char*) {}
}
namespace mh {
bool available() { return false; }
MH_STATUS create(void*, void*, void**) { return MH_OK; }
MH_STATUS enable(void*) { return MH_OK; }
MH_STATUS disable(void*) { return MH_OK; }
}
namespace modtext {
std::wstring T(std::wstring_view key) { return std::wstring(key); }
std::wstring TF(std::wstring_view key, std::initializer_list<std::wstring>) { return std::wstring(key); }
}
namespace log_ { void write(const char*, ...) {} }

int main()
{
    using namespace fixture;
    int checks = 0, failures = 0;
    const auto check = [&](bool ok, const char* description) {
        ++checks;
        if (!ok) { ++failures; std::printf("FAIL: %s\n", description); }
    };
    check(ffpr::ff5::install() == 1, "existing timer hook remains installed");
    const auto& chord = chords.at("TimerHelper.AnnounceBattleTimer");
    check(chord.key == input::Key::T && chord.mod == input::Mod::Ctrl, "battle timer uses Ctrl+T");
    check(chords.at("TimerHelper.AnnounceActiveTimers").mod == input::Mod::None, "plain T is unchanged");
    check(chords.at("TimerHelper.ToggleTimerFreeze").key == input::Key::F5 &&
          chords.at("TimerHelper.ToggleTimerFreeze").mod == input::Mod::Shift, "freeze chord is unchanged");

    auto read_timer = handlers.at("TimerHelper.AnnounceBattleTimer");
    read_timer();
    check(singleton_reads == 0 && speech_count == 0, "no native read or speech before battle is ready");
    ready = true;
    const std::wstring hidden = L"No battle timer displayed";
    instance = nullptr;
    read_timer();
    check(spoken == hidden, "missing native owner stays absent");
    instance = &manager;
    manager.controller = &timer;
    timer.view = &view;
    view.minutes = &minutes;
    view.seconds = &seconds;
    view.group = &group;
    minutes.value = L"01";
    seconds.value = L"47";
    read_timer();
    check(spoken == L"1 minute 47 seconds" && field_scans == 0, "read only the visible battle digits");
    seconds.value = L"46";
    read_timer();
    check(spoken == L"1 minute 46 seconds", "each request reads the current display");
    timer.shown = false;
    read_timer();
    check(spoken == hidden, "hidden timer controller suppresses its old text");
    timer.shown = true;
    group.alpha = 0.f;
    read_timer();
    check(spoken == hidden, "faded canvas suppresses its old text");
    group.alpha = 1.f;
    seconds.shown = false;
    read_timer();
    check(spoken == hidden, "partial display is not read");
    seconds.shown = true;
    seconds.value = L"60";
    read_timer();
    check(spoken == hidden, "malformed seconds are not read");
    seconds.value = L"00";
    minutes.value = L"00";
    read_timer();
    check(spoken == L"0 seconds", "visible zero is read");
    manager.alive = false;
    read_timer();
    check(spoken == hidden && field_scans == 0, "destroyed owner cannot fall back to a field timer");
    std::printf("FFV battle timer: %d checks, %d failures.\n", checks, failures);
    return failures ? 1 : 0;
}
