// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace island_vents {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

int verify() {
    struct Branch { int map, actor; Cell arrival; };
    const Branch branches[] = {{429,16,{1,10,10}},{433,15,{1,11,12}},
                               {432,19,{1,6,12}},{431,17,{1,7,11}}};
    const auto& room = *baked::submap_by_map_id(430);
    const auto& objects = baked::grids(room).objects;
    for (size_t mask = 0; mask < std::size(branches); ++mask) {
        const auto& branch = branches[mask];
        auto state = exdeath_routes::flags(430,0,0);
        state.current_area_id = 62;
        state.scenario[167] = mask & 1;
        state.scenario[168] = (mask >> 1) & 1;
        state.scenario[240] = 1;
        map_state::Resolver selector(state,fixture::records);
        int active = 0;
        for (const auto& actor : objects) {
            if (actor.type != baked::ObjType::Warp || !selector.active(room,actor)) continue;
            check(actor.entity_id() == branch.actor,"only the vent selected by the real lever state is active");
            ++active;
        }
        check(active == 1,"the switch room has one actual vent destination in each state");
        MapGoal outward;
        outward.map_id = branch.map;
        outward.cell = branch.arrival;
        const auto vent = route(430,{1,7,7},state,outward,TargetKind::Spot);
        const auto& exit = vent.pending.incoming_transition;
        check(vent.path.success && !vent.path.required_trap && exit && exit->entity_id == branch.actor &&
              exit->destination == branch.map && exit->arrival == branch.arrival,
              "the selected vent reaches the final playable branch landing");
        if (!vent.path.success) std::printf("Vent to %d: %ls / %ls\n",branch.map,vent.path.error.c_str(),vent.path.reason.c_str());

        state.current_map_id = branch.map;
        MapGoal back;
        back.map_id = 430;
        back.cell = {1,7,7};
        const auto returning = route(branch.map,branch.arrival,state,back,TargetKind::Spot);
        const auto& entry = returning.pending.incoming_transition;
        check(returning.path.success && !returning.path.required_trap && entry && entry->destination == 430 && entry->arrival == back.cell,
              "the return vent resumes in the playable switch room");
        if (!returning.path.success) std::printf("Vent return from %d: %ls / %ls\n",branch.map,returning.path.error.c_str(),returning.path.reason.c_str());
    }
    std::printf("Island Shrine: %d assertions, %d route legs, %d failures\n",exdeath_routes::assertions,
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
    std::thread worker([&] { result = island_vents::verify(); });
    worker.join();
    return result;
}
