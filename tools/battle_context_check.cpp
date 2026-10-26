// Real battle-state transition and native ChangeState detour, without a game runtime.
#include <cstdio>
#include <cstdlib>
#include "../src/core/battle/battle_state.cpp"
#include "../src/core/system/game_state.cpp"
namespace fixture { int calls = 0, scans = 0, starts = 0, entering = 0, ends = 0, resets = 0, state = 3; bool prefix_ready = false, fade = false, input_blocked = false, dialogue = false, queue_only = false, navigation_inside = false, cleaned_inside = false; }
int native_state(void*, const void*) { return fixture::state; }
namespace core { Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {} }
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
bool Class::is(void* object) { return object != nullptr; }
il2cpp::Class* Class::get() { return nullptr; }
il2cpp::Method* Method::get() { return nullptr; }
void* Method::ptr() { if (std::string(name) == "GetCurrentState") return reinterpret_cast<void*>(&native_state); ++fixture::scans; return nullptr; }
void call_failed_note(const char*) { std::abort(); }
}
namespace hooks { bool attach_raw(ref::Method&, void*, void**) { return false; } void note_fault(const char*) { std::abort(); } }
namespace log_ { void write(const char*, ...) {} }
namespace unity { void* find_object(ref::Class&) { ++fixture::scans; return nullptr; } }
namespace fixture { std::wstring scene = L"Title"; }
namespace field { const std::wstring& scene_name() { return fixture::scene; } }
namespace actions { bool defined(const char*) { return false; } bool run(const char*) { return false; } void define(const char*, Fn) {} }
namespace fixture {bool map_open=false,event_item_open=false;}
namespace queries { bool defined(const char*) { return false; } bool is(const char* name) {const std::string key=name;return key=="MapPanels.IsActive"?fixture::map_open:key=="EventItemSelectState.IsActive"?fixture::event_item_open:fixture::dialogue;} void define(const char*, std::function<bool()>) {} }
namespace ffpr::fieldnav::map_transition { bool is_screen_fading() { return fixture::fade; } bool is_state_changing() { return false; } }
namespace events {
void on(Kind, Handler) {}
void fire(Kind kind) { if (kind == Kind::BattleEntering) ++fixture::entering; if (kind == Kind::BattleStart) ++fixture::starts; if (kind == Kind::BattleEnd) ++fixture::ends; }
}
namespace menu_state {
void reset_all() {} void reset(const char*) {} void set_active(const char*, bool) {} bool is_active(const char*) { return false; }
void register_reset(const char*, std::function<void()>) {} std::vector<std::string> active_states() { return {}; }
}
namespace ffpr::battle { void pause_reset() {} void clear_cached_target_controller() {} void result_reset_state() { ++fixture::resets; } void removal_reset_state() {} }
namespace fixture {game::Id game_id=game::Id::FF2;}
namespace game { Id current() { return fixture::game_id; } }
void native_transition(void*, int state, const void*) {
    ++fixture::calls;
    if (state == 13) fixture::cleaned_inside = fixture::resets || fixture::starts;
    fixture::navigation_inside = ffpr::system::is_field_navigation_ready();
    if (!fixture::queue_only) fixture::state = state;
    fixture::input_blocked = ffpr::system::input_transition_blocked();
    fixture::prefix_ready = state == 13 ? ffpr::battle::in_battle() : !ffpr::battle::in_battle();
}
int main() {
    int failures = 0;
    auto expect = [&](bool ok, const char* message) { if (!ok) { ++failures; std::printf("FAIL %s\n", message); } };
    ffpr::system::o_ChangeState = &native_transition;
    void* manager = &failures;
    ffpr::system::d_ChangeState(manager, 13, nullptr);
    expect(fixture::prefix_ready && fixture::calls == 1 && fixture::entering == 1, "battle flag and worker epochs gate features before native transition continues");
    expect(!fixture::cleaned_inside && fixture::resets == 0 && fixture::starts == 0,
        "native loading and opening audio proceed before any result/navigation/lifecycle cleanup");
    expect(fixture::input_blocked, "mod input is blocked inside native battle entry even before fading begins");
    for (int i = 0; i < 100; ++i) ffpr::battle::main_game_transition(13);
    expect(fixture::entering == 1 && fixture::resets == 0 && fixture::starts == 0,
        "repeated pending transition never starts heavy cleanup");
    ffpr::battle::on_battle_start(); // native opening presentation postfix
    for (int i = 0; i < 100; ++i) ffpr::battle::on_battle_start();
    expect(fixture::starts == 1 && fixture::resets == 1, "steady battle detection performs no repeated cleanup");
    ffpr::battle::try_clear_on_battle_end();
    expect(ffpr::battle::in_battle() && fixture::ends == 0, "fade/results callbacks cannot clear the native battle context early");
    ffpr::system::d_ChangeState(manager, 1, nullptr);
    expect(fixture::prefix_ready && fixture::calls == 2 && fixture::ends == 1, "native return releases context exactly once");
    expect(fixture::input_blocked, "native map/return loading blocks mod input before its original body returns");
    expect(!fixture::navigation_inside,"capture readiness is false inside native transition even while the old field state is still reported");
    fixture::state=3; fixture::queue_only=true;
    ffpr::system::d_ChangeState(manager,1,nullptr);
    expect(!ffpr::system::is_field_navigation_ready() && ffpr::system::input_transition_blocked(),
        "queued map transition remains blocked after ChangeState returns with old field state");
    ffpr::system::d_ChangeState(manager,13,nullptr);
    expect(!ffpr::system::is_field_navigation_ready(),"queued battle transition cannot reopen capture from old GetCurrentState");
    fixture::queue_only=false;ffpr::system::d_ChangeState(manager,3,nullptr);
    expect(ffpr::system::is_field_navigation_ready(),"only a new native field-ready transition resumes navigation");
    for (int state : {0, 1, 2, 12, 14}) {
        fixture::state = state;
        ffpr::system::g_requested_state = state;
        expect(ffpr::system::input_transition_blocked(), "native loading/encounter/title transition state blocks hotkeys");
    }
    for (int state : {3, 4, 5, 8, 9, 10, 13}) {
        fixture::state = state;
        ffpr::system::g_requested_state = state;
        expect(!ffpr::system::input_transition_blocked(), "stable field/vehicle/menu/dialogue/battle keeps mod controls");
        fixture::fade = true;
        expect(ffpr::system::input_transition_blocked(), "fade blocks controls even if native state still reports the old ready screen");
        fixture::fade = false;
    }
    fixture::state = 12; fixture::dialogue = true; ffpr::system::g_requested_state = 12;
    expect(!ffpr::system::input_transition_blocked(), "visible scripted dialogue remains manually reviewable");
    fixture::dialogue=false;
    for(int game=1;game<=6;++game) {
        fixture::state=12;ffpr::system::g_requested_state=12;fixture::map_open=true;
        expect(!ffpr::system::input_transition_blocked(),"native event-owned map permits review/I controls in the shared all-six state contract");
        expect(!ffpr::system::is_field_navigation_ready()&&!ffpr::system::is_main_menu_state(),"event-owned map cannot activate field navigation or preloaded item menu readers");
        fixture::fade=true;expect(ffpr::system::input_transition_blocked(),"visible map never bypasses an actual fade");fixture::fade=false;
        for(int state:{0,1,2,14}) {fixture::state=state;ffpr::system::g_requested_state=state;expect(ffpr::system::input_transition_blocked(),"visible/stale map never bypasses loading or title transition");}
        fixture::state=12;ffpr::system::g_requested_state=12;fixture::map_open=false;
        expect(ffpr::system::input_transition_blocked(),"closing the map restores the unattended event guard immediately");
    }
    fixture::dialogue=true;
    fixture::dialogue=false;fixture::event_item_open=true;
    expect(!ffpr::system::input_transition_blocked(),"positive event-item ownership permits I during Event12");
    fixture::fade=true;expect(ffpr::system::input_transition_blocked(),"event item details never bypass fades");fixture::fade=false;fixture::event_item_open=false;
    expect(ffpr::system::input_transition_blocked(),"closing event item selection immediately restores the unattended-event guard");
    fixture::dialogue=true;
    for (int state=0; state<=20; ++state) {
        fixture::state=state;ffpr::system::g_requested_state=state;
        expect(ffpr::system::is_main_menu_state()==(state==5),"only actual Menu5 permits command-bar speech; Event12 intro preload is silent");
    }
    fixture::state=5;ffpr::system::g_requested_state=12;
    expect(!ffpr::system::is_main_menu_state(),"queued menu departure suppresses old command focus");
    ffpr::system::g_requested_state=5;fixture::state=12;
    expect(!ffpr::system::is_main_menu_state(),"queued menu entry cannot expose preloaded focus before native Menu5");
    fixture::state=5;++ffpr::system::g_state_change_depth;
    expect(!ffpr::system::is_main_menu_state(),"native state entry never reads partially initialized command bars");
    --ffpr::system::g_state_change_depth;
    ffpr::system::reset_main_game_state(); fixture::scene=L"Boot";
    expect(ffpr::system::input_transition_blocked(),"boot loading blocks input before the main-game manager exists");
    fixture::scene=L"Title";
    expect(!ffpr::system::input_transition_blocked(),"stable title keeps the direct settings exception");
    for(int game=1;game<=6;++game) {
        fixture::game_id=static_cast<game::Id>(game);
        ffpr::system::d_ChangeState(manager,13,nullptr);ffpr::battle::on_battle_start();
        expect(ffpr::battle::in_battle(),"all-six live native Battle13 owns battle readers");
        ffpr::system::d_ChangeState(manager,14,nullptr);
        expect(fixture::prefix_ready&&!ffpr::battle::in_battle()&&fixture::input_blocked,
            "all-six Return14 releases battle readers before the native teardown body");
        ffpr::battle::on_battle_start();
        expect(!ffpr::battle::in_battle(),"late battle-start presentation cannot reopen Return14");
    }
    ffpr::system::reset_main_game_state();fixture::scene=L"Title";
    expect(!ffpr::system::input_transition_blocked(),"title settings remain accessible after all-six return checks");
    expect(fixture::scans == 0, "automatic transition detection has no scene scan, native query or shutdown polling");
    std::printf("%s: native context gates, one cleanup per transition, no game queries/scans; %d failures\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
