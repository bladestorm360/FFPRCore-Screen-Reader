// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace forest_routes {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

int verify() {
    auto state = exdeath_routes::flags(377,0,0);
    state.current_area_id = 79;
    state.scenario[117] = state.scenario[118] = 1;
    MapGoal fire;
    fire.map_id = 381;
    fire.cell = {1,36,11};
    const auto fire_scene = route(377,{1,36,11},state,fire,TargetKind::Spot);
    check(fire_scene.path.success && fire_scene.pending.incoming_transition &&
          fire_scene.pending.incoming_transition->destination == 381 &&
          fire_scene.pending.incoming_transition->arrival == Cell{1,36,11},
          "the fire scene resumes in the actual burning forest after the world cutscene");
    if (!fire_scene.path.success) std::printf("Fire route: %ls / %ls\n",fire_scene.path.error.c_str(),fire_scene.path.reason.c_str());

    state.current_map_id = 381;
    state.scenario[119] = 1;
    MapGoal refuge;
    refuge.map_id = 382;
    refuge.cell = {1,16,10};
    const auto before_help = route(381,{1,36,11},state,refuge,TargetKind::Spot);
    check(!before_help.path.success,"the forest's refuge is unavailable before the native rescue contact");
    state.scenario[339] = 1;
    const auto opened = route(381,{1,36,11},state,refuge,TargetKind::Spot);
    check(opened.path.success && !opened.path.required_trap && opened.pending.incoming_transition &&
          opened.pending.incoming_transition->destination == 382 &&
          opened.pending.incoming_transition->arrival == Cell{1,16,10},
          "the revealed refuge route resumes in the playable cave after the native scene");
    if (!opened.path.success) std::printf("Refuge route: %ls / %ls\n",opened.path.error.c_str(),opened.path.reason.c_str());

    state.current_map_id = 383;
    state.current_area_id = 86;
    state.scenario[120] = state.scenario[340] = 1;
    const auto return_cave = route(383,{1,47,48},state,refuge,TargetKind::Spot);
    check(return_cave.path.success && return_cave.pending.incoming_transition &&
          return_cave.pending.incoming_transition->arrival == Cell{1,16,10},
          "the post-rescue forest return uses the same playable cave landing");

    struct Tree { int map, destination, prerequisite, opened; Cell player, control; };
    const Tree trees[] = {{377,378,117,437,{1,33,32},{1,33,31}},
                          {379,380,438,438,{1,54,26},{1,54,25}}};
    for (const auto& tree : trees) {
        auto before = exdeath_routes::flags(tree.map,0,0);
        before.current_area_id = 79;
        MapGoal control;
        control.map_id = tree.map;
        control.cell = tree.control;
        check(route(tree.map,tree.player,before,control,TargetKind::Interact).path.success,
              "the unopened tree control is reachable as an ordinary interaction");
        MapGoal beyond;
        beyond.map_id = tree.destination;
        beyond.cell = {1,5,14};
        check(!route(tree.map,tree.player,before,beyond,TargetKind::Spot).path.success,
              "the tree does not grant the route beyond its native prerequisite");
        before.scenario[tree.prerequisite] = before.scenario[tree.opened] = 1;
        before.scenario[118] = 1;
        const auto through = route(tree.map,tree.player,before,beyond,TargetKind::Spot);
        check(through.path.success && through.path.next_map_id == tree.destination,
              "the native tree opening enables its original passage");
    }
    std::printf("Forest of Moore: %d assertions, %d route legs, %d failures\n",exdeath_routes::assertions,
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
    if (!field::baked::load(bytes.data(),bytes.size()) || !json::parse(read("data/ff5/mapstate.json"),fixture::records)) return 2;
    fixture::game_id = game::Id::FF5;
    field::baked_nav::g_loaded = field::baked_nav::g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = forest_routes::verify(); });
    worker.join();
    return result;
}
