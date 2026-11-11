// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace trench {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

map_state::Flags flags(int map, int mask, bool skull, bool open = false) {
    auto state = exdeath_routes::flags(map,0,0);
    state.current_area_id = 46;
    state.area[0] = skull;
    for (int i = 0; i < 3; ++i) state.scenario[192+i] = (mask >> i) & 1;
    state.scenario[195] = open;
    return state;
}

ff5::library::Plan advice(int map, Cell at, const map_state::Flags& state,
                         puzzle_policy::Policy policy = {true,true}) {
    ff5::mechanics::Snapshot snapshot;
    snapshot.map = map;
    snapshot.player = at;
    snapshot.flags = state;
    snapshot.locale = L"en";
    snapshot.policy = policy;
    return ff5::mechanics::plan(snapshot,fixture::guide_recipes);
}

bool guided(int map, Cell at, const map_state::Flags& state, const GuideTarget& target) {
    const auto result = route(map,at,state,target.goal,target.kind);
    if (!result.path.success) std::printf("Map %d at %d,%d to %d/%d: %ls / %ls\n",map,at.x,at.y,
                                         target.goal.map_id,target.goal.entity_id,result.path.error.c_str(),result.path.reason.c_str());
    return result.path.success && !result.path.required_trap;
}

bool actor_active(int map, int entity, const map_state::Flags& state) {
    const auto& room = *baked::submap_by_map_id(map);
    map_state::Resolver resolver(state,fixture::records);
    const auto& objects = baked::grids(room).objects;
    return std::any_of(objects.begin(),objects.end(),[&](const auto& actor) {
        return actor.type == baked::ObjType::Entity && actor.entity_id() == entity && resolver.active(room,actor);
    });
}

int verify() {
    const Cell entry{2,3,11};
    const auto locked = flags(473,0,false);
    const auto skull = advice(473,entry,locked);
    check(skull.targets.size() == 1 && skull.targets.front().goal.entity_id == 1,
          "the first guided action is the real skull control");
    if (skull.targets.empty()) return 2;
    check(guided(473,entry,locked,skull.targets.front()),"the skull control stays reachable before opening the switch room");
    MapGoal tablet;
    tablet.map_id = 474;
    tablet.cell = {1,10,21};
    check(!route(473,entry,locked,tablet,TargetKind::Spot).path.success,
          "the tablet route does not open before the chest sequence");

    const Cell inside{2,31,12};
    const int switch_ids[] = {8,10,9};
    for (int mask = 0; mask < 8; ++mask) {
        const auto state = flags(473,mask,true);
        const auto plan = advice(473,inside,state);
        check(plan.supported && !plan.unknown && !plan.targets.empty(),"every switch subset has the next native objective");
        if (plan.targets.empty()) return 3;
        if (mask == 7) {
            check(plan.targets.size() == 1 && plan.targets.front().goal.entity_id == 11,
                  "only the complete switch set selects the chest");
        } else {
            for (int i = 0; i < 3; ++i) {
                const bool shown = std::any_of(plan.targets.begin(),plan.targets.end(),[&](const auto& target) {
                    return target.goal.entity_id == switch_ids[i];
                });
                check(shown == !(mask & (1<<i)),"the guide offers every unpressed switch and omits activated ones");
            }
        }
        for (const auto& target : plan.targets)
            check(guided(473,inside,state,target),"the selected target has a real safe approach");
    }
    auto full = flags(473,7,true);
    check(actor_active(473,11,full) && actor_active(473,21,full),
          "pressing the third switch in the loaded room preserves the unopened chest and its gate");
    full.current_map_id = 472;
    check(!actor_active(473,11,full) && !actor_active(473,21,full),
          "a later floor load still projects the original native initializer's auto-opening");
    const auto reset = flags(473,0,true);
    const Cell reset_landing{0,17,17};
    const auto reset_advice = advice(473,reset_landing,reset);
    check(reset_advice.targets.size() == 3,"the native chest reset restores all three switch choices");
    if (reset_advice.targets.empty()) return 4;
    check(guided(473,reset_landing,reset,reset_advice.targets.front()),"the guide recovers from the early chest's southward knockback");

    auto open = flags(473,7,true,true);
    const Cell chest_approach{2,17,11};
    const auto ready = advice(473,chest_approach,open);
    check(ready.targets.size() == 1 && ready.targets.front().goal.map_id == 474,
          "the actual chest receipt selects the tablet room");
    if (ready.targets.empty()) return 5;
    check(guided(473,chest_approach,open,ready.targets.front()),"the tablet route uses the actual opened passage");
    open.current_map_id = 474;
    open.area[0] = 0;
    const auto at_tablet = advice(474,{1,10,21},open);
    check(at_tablet.targets.size() == 1 && at_tablet.targets.front().goal.map_id == 474 &&
          guided(474,{1,10,21},open,at_tablet.targets.front()),"the tablet room's native local reset cannot restart the switch sequence");
    open.scenario[148] = 1;
    check(advice(474,{1,10,17},open).finished,"only the native reward completes the guide");
    check(advice(473,entry,locked,{false,true}).targets.empty(),"the Navigation toggle independently controls this guide");

    auto corridor = flags(465,0,false);
    check(actor_active(465,16,corridor) && !actor_active(465,18,corridor),
          "the first corridor keeps the original pre-control barrier");
    corridor.area[2] = 1;
    check(!actor_active(465,16,corridor) && !actor_active(465,18,corridor),
          "the first ordinary control opens its original path");
    corridor.area[3] = 1;
    check(!actor_active(465,16,corridor) && actor_active(465,18,corridor),
          "the second ordinary control preserves its distinct native corridor change");
    auto optional = flags(466,0,false);
    check(actor_active(466,20,optional),"the optional side chest retains its own barrier");
    optional.area[1] = 1;
    check(!actor_active(466,20,optional),"one native interaction opens the optional side chest route");
    std::printf("Great Sea Trench: %d assertions, %d route legs, %d failures\n",exdeath_routes::assertions,
                exdeath_routes::legs,exdeath_routes::failures);
    return fixture::wrong_owner || exdeath_routes::failures ? 3 : 0;
}
}

int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read = [](const char* path) {
        std::ifstream stream(path,std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(stream),{});
    };
    const auto bytes = read("data/ff5/maps.bin");
    if (!field::baked::load(bytes.data(),bytes.size()) || !json::parse(read("data/ff5/mapstate.json"),fixture::records) ||
        !json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes)) return 2;
    fixture::game_id = game::Id::FF5;
    field::baked_nav::g_loaded = field::baked_nav::g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = trench::verify(); });
    worker.join();
    return result;
}
