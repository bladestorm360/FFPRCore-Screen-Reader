// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace death_valley {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

int verify() {
    auto state = exdeath_routes::flags(407,0,0);
    state.current_area_id = 63;
    MapGoal cave;
    cave.map_id = 408;
    cave.cell = {1,14,18};
    for (int x : {15,16,17}) {
        const auto fall = route(407,{1,x,31},state,cave,TargetKind::Spot);
        check(fall.path.success && fall.path.required_trap && fall.pending.incoming_trap &&
              fall.pending.incoming_trap->arrival == Cell{1,14,18},
              "every native fall resumes at the ready cave tile after the encounter");
        if (!fall.path.success) std::printf("Fall %d: %ls / %ls\n",x,fall.path.error.c_str(),fall.path.reason.c_str());
    }
    state.current_map_id = 408;
    state.scenario[131] = 1;
    MapGoal world;
    world.map_id = 19;
    world.cell = {1,121,89};
    const auto early = route(408,{1,14,18},state,world,TargetKind::Spot);
    check(!early.path.success,"the cave cannot leave before the native rescue contacts");
    const auto& room = *baked::submap_by_map_id(408);
    const auto& objects = baked::grids(room).objects;
    const int actors[] = {65,21,18,19,20};
    const Cell starts[] = {{1,14,18},{1,14,15},{1,15,13},{1,15,13},{1,16,12}};
    for (size_t i = 0; i < std::size(actors); ++i) {
        const auto actor = std::find_if(objects.begin(),objects.end(),[&](const auto& object) {
            return object.type == baked::ObjType::Event && object.entity_id() == actors[i];
        });
        if (actor == objects.end()) return 2;
        MapGoal contact;
        contact.map_id = 408;
        contact.cell = {actor->layer,actor->tx,actor->ty};
        contact.entity_id = actor->entity_id();
        contact.raw_name = baked::object_identifier(*actor);
        const auto step = route(408,starts[i],state,contact,TargetKind::Spot);
        check(step.path.success,"the current native rope contact stays reachable in its actual state");
        if (!step.path.success) std::printf("Rope %d: %ls / %ls\n",actors[i],step.path.error.c_str(),step.path.reason.c_str());
        if (i < 4) state.scenario[341+i] = 1;
    }
    const auto exit = route(408,{1,16,12},state,world,TargetKind::Spot);
    check(exit.path.success && !exit.path.required_trap && exit.pending.incoming_transition &&
          exit.pending.incoming_transition->destination == 19 && exit.pending.incoming_transition->arrival == Cell{1,121,89},
          "the actual rescue return bypasses the cutscene map and reaches the native world pose");
    if (!exit.path.success) std::printf("Return: %ls / %ls\n",exit.path.error.c_str(),exit.path.reason.c_str());
    std::printf("Death Valley: %d assertions, %d route legs, %d failures\n",exdeath_routes::assertions,
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
    std::thread worker([&] { result = death_valley::verify(); });
    worker.join();
    return result;
}
