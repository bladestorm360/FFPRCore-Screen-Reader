// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace karnak_routes {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

map_state::Flags flags(int map, int encountered, bool timer = true, bool escaped = false, bool explosion = false) {
    auto value = exdeath_routes::flags(map,0,0);
    value.current_area_id = map == 1 ? 1 : map == 581 ? 101 : 24;
    value.scenario[359] = 0;
    value.scenario[353] = 1; // The wall blast precedes the timer and ordinary castle access.
    value.scenario[42] = value.scenario[43] = 1;
    value.scenario[46] = timer;
    value.scenario[245] = encountered;
    value.scenario[47] = escaped;
    value.scenario[992] = explosion;
    return value;
}

ff5::library::Plan advice(int map, Cell at, const map_state::Flags& state,
                         std::wstring guide = {}, puzzle_policy::Policy policy = {true,true}) {
    ff5::mechanics::Snapshot snapshot;
    snapshot.map = map;
    snapshot.player = at;
    snapshot.flags = state;
    snapshot.guide_id = std::move(guide);
    snapshot.locale = L"en";
    snapshot.policy = policy;
    return ff5::mechanics::plan(snapshot,fixture::guide_recipes);
}

int verify() {
    const auto& basement = *baked::submap_by_map_id(101);
    const auto& objects = baked::grids(basement).objects;
    const auto wall = std::find_if(objects.begin(),objects.end(),[](const auto& object) {
        return object.type == baked::ObjType::Entity && object.entity_id() == 34;
    });
    if (wall == objects.end()) return 2;
    auto before_blast = flags(101,0,false);
    before_blast.scenario[42] = before_blast.scenario[43] = before_blast.scenario[353] = 0;
    map_state::Resolver closed(before_blast,fixture::records);
    map_state::Resolver opened(flags(101,0),fixture::records);
    check((wall->d & 1) && closed.active(basement,*wall) && !opened.active(basement,*wall),
          "the original blast wall blocks the basement before the native receipt and clears after it");

    int map = 102;
    Cell at{1,4,7};
    int stair_legs = 0;
    for (; stair_legs < 12; ++stair_legs) {
        const auto state = flags(map,0);
        const auto plan = advice(map,at,state);
        check(plan.supported && !plan.unknown && plan.targets.size() == 1 && plan.targets[0].goal.entity_id == 29,
              "countdown uses the actual exit encounter as its next destination");
        if (plan.targets.empty()) return 2;
        const auto& target = plan.targets[0];
        const auto result = route(map,at,state,target.goal,target.kind);
        check(result.path.success && !result.path.required_trap,"each actual escape leg uses ordinary stairs and terrain");
        if (!result.path.success) {
            std::printf("Escape map %d at %d,%d,%d: %ls / %ls\n",map,at.layer,at.x,at.y,
                        result.path.error.c_str(),result.path.reason.c_str());
            return 3;
        }
        if (map == 95) break;
        const auto& incoming = result.pending.incoming_transition;
        if (!incoming) return 4;
        map = incoming->destination;
        at = incoming->arrival;
    }
    check(map == 95 && stair_legs > 1 && stair_legs < 12,"the actual timer start connects through every intervening native stair");
    const auto normal = advice(102,{1,4,7},flags(102,0,false));
    check(!normal.supported && normal.targets.empty(),"the ordinary castle visit never starts the timed escape recipe");
    const auto after_battle = advice(95,{1,23,35},flags(95,1));
    check(after_battle.targets.size() == 3,"native encounter return selects all three actual escape contacts");
    for (const auto& target : after_battle.targets) {
        const auto result = route(95,{1,23,35},flags(95,1),target.goal,target.kind);
        check(result.path.success && !result.path.required_trap,"actual exit contact is usable without a false trap warning");
    }
    MapGoal world;
    world.map_id = 1;
    world.cell = {1,83,78};
    const auto ordinary = route(95,{1,23,35},flags(95,1),world,TargetKind::Spot);
    check(ordinary.path.success && !ordinary.path.required_trap,"ordinary routing can use the actual scripted exit");
    const auto& arrival = ordinary.pending.incoming_transition;
    check(arrival && arrival->destination == 1 && arrival->arrival.x == 83 && arrival->arrival.y == 78,
          "the world route resumes at the full native landing after the explosion");
    const auto before = route(95,{1,23,35},flags(95,0,false),world,TargetKind::Spot);
    check(before.path.success && !before.path.required_trap,"the normal courtyard exit remains available before the countdown");
    check(!advice(1,{2,84,78},flags(1,1),L"karnak_escape").finished &&
          advice(1,{2,84,78},flags(1,1),L"karnak_escape").targets.empty(),"first world arrival cannot finish or route back through the castle");
    check(!advice(581,{2,19,8},flags(581,1,true,false,true),L"karnak_escape").finished,
          "the explosion's own receipt does not end guidance inside the cinematic");
    check(advice(1,{2,83,78},flags(1,1,true,true,true),L"karnak_escape").finished,
          "only the full native world callback completes the escape");
    check(!advice(1,{2,83,78},flags(1,1,true,true,true)).supported,"ordinary world entry never activates the castle recipe");
    check(advice(102,{1,4,7},flags(102,0),{}, {false,true}).targets.empty(),"Navigation toggle independently controls the timed route");
    std::printf("Karnak production: %d assertions, %d route legs, %d stairs, %d failures\n",
                exdeath_routes::assertions,exdeath_routes::legs,stair_legs,exdeath_routes::failures);
    return fixture::wrong_owner || exdeath_routes::failures ? 5 : 0;
}
}

int main() {
    using namespace karnak_routes;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read = [](const char* path) {
        std::ifstream stream(path,std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(stream),{});
    };
    const auto bytes = read("data/ff5/maps.bin");
    if (!baked::load(bytes.data(),bytes.size()) || !json::parse(read("data/ff5/mapstate.json"),fixture::records) ||
        !json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes)) return 2;
    fixture::game_id = game::Id::FF5;
    g_loaded = g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = verify(); });
    worker.join();
    return result;
}
