// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace short_passages {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

bool solid(int map, int id, const map_state::Flags& state) {
    map_state::Resolver selected(state,fixture::records);
    const auto& room = *baked::submap_by_map_id(map);
    return std::any_of(baked::grids(room).objects.begin(),baked::grids(room).objects.end(),[&](const auto& object) {
        return object.type == baked::ObjType::Entity && object.entity_id() == id &&
               (object.d & 1) && selected.active(room,object,true);
    });
}

int verify() {
    struct Gate { int map, area, entity, scenario; const char* label; };
    const Gate persistent[] = {{173,45,6,275,"Bal basement door honors its inside opening receipt"},
                               {195,6,148,527,"the pirates' ordinary door honors its opening receipt"},
                               {646,34,78,438,"the Ronka passage clears only after the original switch"},
                               {553,85,31,440,"the Rift's exterior door uses its native opening receipt"},
                               {555,85,14,440,"the Rift's interior corridor shares the same opening receipt"}};
    for (const auto& gate : persistent) {
        auto state = exdeath_routes::flags(gate.map,0,0);
        state.current_area_id = gate.area;
        const bool closed = solid(gate.map,gate.entity,state);
        state.scenario[gate.scenario] = 1;
        check(closed && !solid(gate.map,gate.entity,state),gate.label);
    }
    auto bal = exdeath_routes::flags(168,0,0);
    bal.current_area_id = 45;
    const bool bal_closed = solid(168,46,bal);
    bal.area[0] = 1;
    check(bal_closed && !solid(168,46,bal),"Bal's single treasury control clears its own local blocker");
    auto phantom = exdeath_routes::flags(183,0,0);
    phantom.current_area_id = 73;
    const bool phantom_closed = solid(183,27,phantom) && solid(183,29,phantom);
    phantom.local[0] = 1;
    check(phantom_closed && !solid(183,27,phantom) && !solid(183,29,phantom),
          "the Phantom Village box clears both of its own physical obstructions");

    auto basement = exdeath_routes::flags(173,0,0);
    basement.current_area_id = 45;
    MapGoal door;
    door.map_id = 173;
    door.cell = {1,6,49};
    check(route(173,{1,6,54},basement,door,TargetKind::Interact).path.success,
          "the locked Bal door remains reachable from the castle side");
    MapGoal beyond;
    beyond.map_id = 173;
    beyond.cell = {1,6,46};
    const auto closed = route(173,{1,6,54},basement,beyond,TargetKind::Spot);
    check(!closed.path.success && closed.path.reason == L"Story gated",
          "the Bal basement cannot route through the closed door");
    basement.scenario[275] = 1;
    check(route(173,{1,6,54},basement,beyond,TargetKind::Spot).path.success,
          "the Bal basement route opens after the native inside interaction");

    std::printf("FFV short passages: %d assertions, %d route legs, %d failures\n",exdeath_routes::assertions,
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
    std::thread worker([&] { result = short_passages::verify(); });
    worker.join();
    return result;
}
