// FFIII's circuit is a guide to native movement, not a quest-state writer.
// State and native reads stay on the field owner; route work uses copied snapshots.
#include "chocobo_circuit.h"
#include "../../core/field/baked_nav.h"
#include "../../core/field/field_api.h"
#include "../../core/field/map_state.h"
#include "../../core/field/scene_events.h"
#include "../../core/core.h"
#include "../../core/hooks.h"
#include "../../core/modtext.h"
#include "../../core/say.h"
#include "../../core/text.h"
#include "../../core/ui/preferences.h"

namespace ff3::chocobo_circuit {
namespace {
using field::baked_route::Cell;
bool active = false, have_cell = false;
int step = 0;
Cell last_cell{};
uint64_t epoch = 0;

// Native menus can remain active while a route worker finishes; never publish over one.
bool ready() {
    return !menu_state::any_active() && !queries::is("GameStatePatches.IsInputTransitionBlocked") &&
        queries::is("GameStatePatches.IsFieldNavigationReady") &&
        !queries::is("BattleStateHelper.IsInBattle");
}
bool mounted() { const auto v = field::current_vehicle(); return v.valid && v.id == 12; }
field::baked_nav::MapGoal goal() {
    field::baked_nav::MapGoal g; g.map_id=1; g.cell=points[step]; return g;
}
void retire(bool clear) {
    if (active && clear) field::baked_nav::clear_preview_destination(goal());
    ++epoch; active=false; step=0; have_cell=false;
}
void invalid_vehicle() { retire(true); ffpr::say(T(L"Ride a chocobo on this world map to use this guide"), true); }
void announce(bool interrupt) {
    if (!active || !ready() || field::current_map_id()!=1) return;
    if (!mounted()) { invalid_vehicle(); return; }
    const int requested_step=step; const auto requested_epoch=++epoch;
    const wchar_t* key = step==0 ? L"Go to the circuit starting point" :
        step==5 ? L"Cross the circuit checkpoint" : step==13 ? L"Return to the circuit starting point" : L"Follow the circuit";
    auto line=TF(L"Step {0} of {1}: {2}", {text::num(step+1),text::num(14),T(key)});
    field::baked_nav::request_route(goal(), field::TargetKind::Spot, L"",
        [requested_epoch,requested_step,line=std::move(line),interrupt](field::PathInfo info) mutable {
            if (!active || epoch!=requested_epoch || step!=requested_step || !ready() || field::current_map_id()!=1) return;
            if (!mounted()) { invalid_vehicle(); return; }
            if (info.success) {
                const auto& directions=info.full_description.empty()?info.description:info.full_description;
                if (!directions.empty() && directions!=T(L"No movement needed")) line+=L". "+directions;
            } else line+=L". "+(!info.reason.empty()?info.reason:T(L"No path to the next step"));
            if (ui::prefs::mod_warnings_enabled() && !info.warning.empty())
                ffpr::say_sequence({info.warning,std::move(line)},interrupt);
            else ffpr::say(line,interrupt);
        },false);
}
// The checkpoint sets only bit 192. Reaching the return cell is not proof it fired.
void finish() {
    const auto recorded=field::map_state::scenario_flag(192);
    retire(true);
    ffpr::say(T(!recorded ? L"Circuit completion could not be checked" :
        *recorded ? L"Chocobo circuit recorded" : L"Circuit route finished, but the game has not recorded it"),true);
}
void advance_at(const Cell& cell, bool interrupt) {
    if (!(cell==points[step])) return;
    if (step==13) { finish(); return; }
    field::baked_nav::clear_preview_destination(goal());
    ++step; ++epoch;
    ffpr::say(T(L"Step done"),interrupt);
    announce(false);
}
void next() {
    if (!ready()) return;
    if (field::current_map_id()!=1 || !mounted()) { if(active) invalid_vehicle(); return; }
    if (active) {
        // The last movement input can precede native arrival; repeat observes it again.
        Cell cell;
        if (field::baked_nav::player_cell(field::map_handle(),field::player(),cell)) {
            last_cell=cell; have_cell=true;
            if (cell==points[step]) { advance_at(cell,true); return; }
        }
        announce(true); return;
    }
    const auto recorded=field::map_state::scenario_flag(192);
    if (recorded && *recorded) { ffpr::say(T(L"Chocobo circuit recorded"),true); return; }
    active=true; step=0; have_cell=false; ++epoch;
    ffpr::say(T(L"Chocobo circuit guide"),true);
    Cell cell;
    if (field::baked_nav::player_cell(field::map_handle(),field::player(),cell)) {
        last_cell=cell; have_cell=true;
        if (cell==points[0]) { advance_at(cell,false); return; }
    }
    announce(false);
}
void move() {
    if (!active || !ready() || field::current_map_id()!=1) return;
    Cell cell;
    if (!field::baked_nav::player_cell(field::map_handle(),field::player(),cell)) return;
    if (have_cell && cell==last_cell) return;
    last_cell=cell; have_cell=true;
    if (!mounted()) { invalid_vehicle(); return; }
    advance_at(cell,true);
}
void stop() { if (!active || !ready()) return; retire(true); ffpr::say(T(L"Puzzle guide stopped"),true); }
int install() {
    actions::define("GameModule.PuzzleGuide",[]{hooks::guarded("FF3.Circuit.Next",[]{next();});});
    actions::define("GameModule.StopPuzzleGuide",[]{stop();});
    actions::define("GameModule.PuzzleGuideMove",[]{move();});
    queries::define("GameModule.HasPuzzleGuide",[]{return field::current_map_id()==1 && (active || mounted());});
    queries::define("GameModule.PuzzleGuideActive",[]{return active;});
    events::on(events::Kind::MapChanging,[]{retire(false);});
    events::on(events::Kind::BattleEntering,[]{retire(false);});
    events::on(events::Kind::TitleScreen,[]{retire(false);});
    events::on(events::Kind::SceneLoaded,[]{if(field::scene_name()==L"Title")retire(false);});
    return 0;
}
core::Install s_install{"FF3.ChocoboCircuit",&install,515,game::kFF3};
}
}
