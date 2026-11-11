// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace ship_graveyard {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

int verify() {
    auto state = exdeath_routes::flags(211,0,0);
    state.current_area_id = 13;
    state.scenario[25] = 1; // The native arrival scene has returned control.
    const auto crossing = [&](Cell start, Cell end, const char* label) {
        MapGoal goal;
        goal.map_id = 211;
        goal.cell = end;
        const auto result = route(211,start,state,goal,TargetKind::Spot);
        check(result.path.success && !result.path.required_trap,label);
        if (!result.path.success) {
            std::printf("Ship Graveyard %d,%d to %d,%d: %ls / %ls\n",start.x,start.y,end.x,end.y,
                        result.path.error.c_str(),result.path.reason.c_str());
        }
    };
    crossing({1,32,33},{1,36,33},"the east jump connects its two real shoreline tiles");
    crossing({1,36,33},{1,32,33},"the west jump connects the same shorelines in reverse");
    crossing({1,37,49},{1,45,47},"the three-part jump reaches the southern side of the treasure ship");
    crossing({1,45,47},{1,37,49},"the reverse three-part jump reaches its native shore");
    const auto advice = [&](int map, Cell at) {
        ff5::mechanics::Snapshot snapshot;
        snapshot.map = map;
        snapshot.player = at;
        snapshot.flags = state;
        snapshot.locale = L"en";
        snapshot.policy = {true,true};
        return ff5::mechanics::plan(snapshot,fixture::guide_recipes);
    };
    MapGoal optional_chest;
    optional_chest.map_id = 211;
    optional_chest.cell = {1,13,54};
    const auto timed = route(211,{1,17,32},state,optional_chest,TargetKind::Interact);
    check(timed.path.success && !timed.path.required_trap && timed.pending.incoming_transition &&
          timed.pending.incoming_transition->entity_id == 102 && timed.pending.incoming_transition->arrival == Cell{1,13,51},
          "ordinary routing to the optional chest uses the full three-part timed jump");
    check(!timed.path.walk_phases.empty() && timed.path.walk_phases.back().action_after == L"Wait here for the jump.",
          "the timed approach ends by telling the player to wait");
    crossing({1,13,51},{1,17,32},"the optional chest island can return across the original jumps");

    int map = 211;
    Cell at{1,11,25};
    int stairs = 0, jumps = 0;
    struct Stage { int map, entity, receipt; Cell after; };
    const Stage stages[] = {{213,7,26,{1,7,16}},{220,9,27,{1,8,15}},
                            {211,17,28,{1,46,48}},{211,109,29,{1,47,19}}};
    for (const auto& stage : stages) {
        bool reached = false;
        Cell approach{};
        for (int leg = 0; leg < 24; ++leg) {
            state.current_map_id = map;
            const auto plan = advice(map,at);
            check(plan.supported && !plan.unknown && plan.targets.size() == 1 &&
                  plan.targets[0].goal.map_id == stage.map && plan.targets[0].goal.entity_id == stage.entity,
                  "the guide selects the next real contact from the native receipts");
            if (plan.targets.empty()) return 2;
            const auto& target = plan.targets.front();
            const auto result = route(map,at,state,target.goal,target.kind);
            check(result.path.success && !result.path.required_trap,"each guided leg finishes without a false trap");
            if (!result.path.success) {
                std::printf("Guide map %d at %d,%d to stage %d/%d: %ls / %ls\n",map,at.x,at.y,
                            stage.map,stage.entity,result.path.error.c_str(),result.path.reason.c_str());
                return 4;
            }
            const auto& incoming = result.pending.incoming_transition;
            if (!incoming) {
                reached = map == stage.map;
                if (!result.path.walk_phases.empty() && !result.path.walk_phases.back().cells.empty())
                    approach = result.path.walk_phases.back().cells.back();
                break;
            }
            if (incoming->destination == map) {
                if (incoming->arrival == at) return 5;
                ++jumps;
            } else {
                ++stairs;
            }
            map = incoming->destination;
            at = incoming->arrival;
        }
        if (!reached) return 6;
        state.scenario[stage.receipt] = 1;
        // The chest cutscene walks north and then returns to the same approach.
        at = stage.receipt == 28 ? approach : stage.after;
    }
    check(stairs >= 4 && jumps >= 2,"the full route includes the real interior crossings and manual jumps");
    state.scenario[29] = 0;
    MapGoal beyond_encounter;
    beyond_encounter.map_id = 211;
    beyond_encounter.cell = {1,47,23};
    const auto before_battle = route(211,{1,47,33},state,beyond_encounter,TargetKind::Spot);
    check(!before_battle.path.success,"ordinary routing cannot use the north jump before the native encounter finishes");
    state.scenario[29] = 1;
    check(advice(211,{1,47,19}).finished,"only the encounter callback completes the guide");
    crossing({1,47,33},{1,47,23},"after the encounter the native north jump returns to the exit side");
    std::printf("Ship Graveyard production: %d assertions, %d legs, %d stairs, %d jumps, %d failures\n",
                exdeath_routes::assertions,exdeath_routes::legs,stairs,jumps,exdeath_routes::failures);
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
    std::thread worker([&] { result = ship_graveyard::verify(); });
    worker.join();
    return result;
}
