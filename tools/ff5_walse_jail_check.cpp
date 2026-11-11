// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace walse_jail {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

int verify() {
    auto state = exdeath_routes::flags(63,0,0);
    state.current_area_id = 19;
    state.scenario[169] = state.scenario[355] = 1;
    const auto& room = *baked::submap_by_map_id(63);
    const auto& objects = baked::grids(room).objects;
    const auto collider = std::find_if(objects.begin(),objects.end(),[](const auto& object) {
        return object.type == baked::ObjType::Entity && object.entity_id() == 24;
    });
    if (collider == objects.end()) return 2;
    map_state::Resolver locked(state,fixture::records);
    const auto placed = locked.effective(room,*collider);
    check(locked.active(room,*collider) && placed.layer == 1 && placed.tx == 8 && placed.ty == 11,
          "the closed jail door uses the native moved collider rather than its off-map template");
    MapGoal inside;
    inside.map_id = 63;
    inside.cell = {1,8,9};
    const auto before = route(63,{1,10,6},state,inside,TargetKind::Spot);
    check(!before.path.success && before.path.reason == L"Story gated",
          "a route from outside cannot cross the jail while its native lock is closed");
    state.current_map_id = 62;
    const auto remote_locked = route(62,{1,14,20},state,inside,TargetKind::Spot);
    check(!remote_locked.path.success && remote_locked.path.reason == L"Story gated",
          "the remote route preview also respects the closed jail entrance");
    state.current_map_id = 63;
    MapGoal reopen;
    reopen.map_id = 63;
    reopen.cell = {1,8,10};
    const auto toward_opener = route(63,{1,8,9},state,reopen,TargetKind::Spot);
    check(toward_opener.path.success,"the native opening contact remains reachable from inside the jail");
    state.scenario[355] = 0;
    const auto after = route(63,{1,10,6},state,inside,TargetKind::Spot);
    check(after.path.success,"the route into the cell opens after the native receipt clears");
    state.current_map_id = 62;
    const auto remote_open = route(62,{1,14,20},state,inside,TargetKind::Spot);
    check(remote_open.path.success && remote_open.path.next_map_id == 63,
          "remote routing uses the original stair after the jail opens");
    std::printf("Walse jail: %d assertions, %d route legs, %d failures\n",exdeath_routes::assertions,
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
    std::thread worker([&] { result = walse_jail::verify(); });
    worker.join();
    return result;
}
