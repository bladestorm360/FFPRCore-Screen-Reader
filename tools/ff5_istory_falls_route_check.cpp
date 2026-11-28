// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace falls_routes {
using namespace field;
using namespace baked_nav;
using exdeath_routes::route;
using exdeath_routes::check;

map_state::Flags flags(int map, int mask, int timer = 0) {
    auto value = exdeath_routes::flags(map,0,0);
    value.current_area_id = 69;
    value.scenario[293] = 1;
    value.scenario[358] = timer;
    for (int i = 0; i < 3; ++i) value.scenario[402+i] = (mask >> i) & 1;
    return value;
}

MapGoal goal(int map, int entity, Cell cell, std::wstring raw, bool strict) {
    MapGoal result;
    result.map_id = map;
    result.entity_id = entity;
    result.cell = cell;
    result.raw_name = std::move(raw);
    result.avoid_other_events = strict;
    result.allow_scripted_crossings = true;
    return result;
}

int verify() {
    const auto exit = goal(478,7,{1,5,22},L"Event",false);
    const auto ring = goal(478,12,{1,10,8},L"まもりのゆびわ",false);
    for (int timer : {0,1}) for (const Cell start : {Cell{0,24,2},Cell{1,16,2},Cell{1,3,8}}) {
        const auto state = flags(478,0,timer);
        const auto onward = route(478,start,state,exit,TargetKind::Transition);
        if (!onward.path.success || onward.path.required_trap)
            std::printf("Istory exit timer%d from L%d (%d,%d): %ls / %ls\n",timer,start.layer,start.x,start.y,
                        onward.path.reason.c_str(),onward.path.error.c_str());
        check(onward.path.success && !onward.path.required_trap,"both native entrances and the switch approach can reach the normal exit");
        const auto chest = route(478,start,state,ring,TargetKind::Interact);
        if (timer == 0) {
            check(!chest.path.success && chest.path.reason == L"Event required","running water makes the exact chest event gated");
        } else {
            if (!chest.path.success || chest.path.required_trap)
                std::printf("Istory chest from L%d (%d,%d): %ls / %ls\n",start.layer,start.x,start.y,
                            chest.path.reason.c_str(),chest.path.error.c_str());
            check(chest.path.success && !chest.path.required_trap,"stopping the current opens the optional chest approach");
        }
    }
    const auto first = route(478,{1,16,2},flags(478,0),exit,TargetKind::Transition);
    std::printf("Istory first: success%d next%d %ls / %ls / %ls\n",first.path.success,first.path.next_map_id,
                first.path.reason.c_str(),first.path.error.c_str(),first.path.description.c_str());
    check(first.path.description.find(L"Ride the current") != std::wstring::npos,"ordinary directions stop their walking count at the real current");
    check(first.pending.incoming_transition && first.pending.incoming_transition->destination == 478 &&
          first.pending.incoming_transition->arrival == Cell{0,16,13},"ground current 37 uses the native Bottom landing");
    auto unavailable = flags(478,0);
    unavailable.scenario[358].reset();
    const auto unknown_chest = route(478,{1,3,8},unavailable,ring,TargetKind::Interact);
    check(!unknown_chest.path.success && unknown_chest.path.reason == L"Navigation data is loading",
          "unknown timer cannot permit directions through the waterfall");
    const auto* map = baked::submap_by_map_id(478);
    const std::pair<int,Cell> landings[] = {{37,{0,16,13}},{38,{0,9,10}},{41,{0,20,11}},
                                           {42,{0,16,13}},{43,{0,12,11}},{44,{0,9,10}}};
    int checked_arcs = 0;
    for (const auto& [id,cell] : landings) {
        for (const auto& object : baked::grids(*map).objects) {
            if (object.type != baked::ObjType::Warp || object.entity_id() != id) continue;
            ++checked_arcs;
            map_state::Resolver running(flags(478,0),fixture::records);
            const auto* arrival = point_in(*map,object.b,running);
            check((object.d & baked::kAutomaticCurrent) && baked::world_exit(object) && object.a == -1,
                  "exact current arc represents ordinary forced movement on the same floor");
            check(arrival && Cell{arrival->layer,arrival->tx,arrival->ty} == cell,"all six arc landings match the native motion and layer contacts");
            map_state::Resolver stopped(flags(478,0,1),fixture::records);
            auto incomplete = flags(478,0);
            incomplete.scenario[358].reset();
            map_state::Resolver unknown(incomplete,fixture::records);
            check(!running.consumed(*map,object) && stopped.consumed(*map,object) == (id==38) &&
                  unknown.consumed(*map,object) == (id==38),"only the exact timed current is guarded against a stopped or unknown timer");
            check(route(478,cell,flags(478,0),exit,TargetKind::Transition).path.success,"routing can resume from each actual waterfall landing");
        }
    }
    check(checked_arcs == 6,"six exact native actors have one scripted movement arc each");
    const auto control = goal(483,20,{1,4,18},L"sc_e_0320",true);
    const auto drop = goal(483,22,{1,8,22},L"sc_e_0321_1",true);
    for (int mask = 0; mask < 8; ++mask) {
        int control_routes = 0, drop_routes = 0;
        for (const Cell start : {Cell{1,13,15},Cell{1,19,13},Cell{1,6,18}}) {
            const auto state = flags(483,mask);
            control_routes += route(483,start,state,control,TargetKind::Interact).path.success;
            drop_routes += route(483,start,state,drop,TargetKind::Spot).path.success;
        }
        check(control_routes == 3,"ordinary exit switch is reachable from all three native entries in every layout");
        check(drop_routes == ((mask & 4) ? 3 : 0),"only the third switch gates the exit, independently of the other two");
    }
    std::printf("Istory Falls production: %d assertions, %d route legs, %d failures\n",
                exdeath_routes::assertions,exdeath_routes::legs,exdeath_routes::failures);
    return fixture::wrong_owner || exdeath_routes::failures ? 3 : 0;
}
}

int main() {
    using namespace falls_routes;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read = [](const char* path) {
        std::ifstream stream(path,std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(stream),{});
    };
    const auto bytes = read("data/ff5/maps.bin");
    if (!baked::load(bytes.data(),bytes.size()) || !json::parse(read("data/ff5/mapstate.json"),fixture::records)) return 2;
    fixture::game_id = game::Id::FF5;
    g_loaded = g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = verify(); });
    worker.join();
    return result;
}
