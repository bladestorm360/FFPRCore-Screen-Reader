// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual Mist world entrance, both cave arrivals and shared browse/route owner.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff4/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff4/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF4;g_loaded=g_prepared=true;
    const auto* world=baked::submap_by_map_id(1);const baked::Object* mist=nullptr;
    for(const auto& object:baked::grids(*world).objects)if(object.type==baked::ObjType::GotoMap&&object.a==67){mist=&object;break;}
    if(!mist)return 2;
    int failures=0;
    for(bool native_player:{false,true})for(int phase=0;phase<8;++phase)for(const auto origin:std::vector<Cell>{{1,107,145},{1,81,120},{1,89,107}}) {
        map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);flags.area.fill(0);flags.local.fill(0);
        flags.scenario[6]=phase&1;flags.scenario[102]=(phase>>1)&1;flags.scenario[103]=(phase>>2)&1;
        flags.current_map_id=1;flags.current_area_id=1;
        auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
        snapshot->map_id=1;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";
        snapshot->position=world_of_cell(*world,origin);snapshot->vehicle_shortcuts=false;
        if(native_player)snapshot->vehicle={true,1,baked::transport(1)->type,{}};
        snapshot->goal.map_id=1;snapshot->goal.cell={1,mist->tx,mist->ty};snapshot->goal.entity_id=mist->entity_id();
        snapshot->goal.raw_name=baked::object_identifier(*mist);snapshot->kind=TargetKind::Transition;
        Planned route;BrowsePlanned browse;
        std::thread worker([&]{route=plan_snapshot(*snapshot);browse=plan_browse_snapshot(*snapshot);});worker.join();
        std::printf("Mist native-player%d flags%d origin%d,%d: route%d next%d steps%d reason[%ls]; browse direct%d via%d reason[%ls]\n",
            native_player,phase,origin.x,origin.y,route.path.success,route.path.next_map_id,route.path.step_count,route.path.reason.c_str(),
            browse.route.direct,browse.route.via_map_id,browse.route.reason.c_str());
        if((phase&1)&&(!route.path.success||(!browse.route.direct&&!browse.route.via_map_id)))++failures;
    }
    // Continue the retained world destination from both real cave arrivals.
    // The boss-cleared receipt permits crossing; unopened native chests remain
    // solid on this loaded floor, just as the owner's log observed.
    const auto* cave=baked::submap_by_map_id(162);
    for(int point:{1,2}) {
        map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);flags.local.fill(0);flags.area.fill(0);
        flags.scenario[6]=1;flags.current_map_id=162;flags.current_area_id=24;
        auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
        const auto* arrival=point_in(*cave,point,snapshot->state);if(!arrival){++failures;continue;}
        snapshot->map_id=162;snapshot->submap=cave;snapshot->layer=arrival->layer;snapshot->locale=L"en";
        snapshot->position=world_of_cell(*cave,{arrival->layer,arrival->tx,arrival->ty});snapshot->vehicle={true,1,baked::transport(1)->type,{}};
        snapshot->goal.map_id=1;snapshot->goal.cell={1,mist->tx,mist->ty};snapshot->goal.entity_id=mist->entity_id();
        snapshot->goal.raw_name=baked::object_identifier(*mist);snapshot->kind=TargetKind::Transition;
        for(const auto& chest:baked::grids(*cave).objects)if(chest.type==baked::ObjType::TreasureBox&&snapshot->state.active(*cave,chest)) {
            NativeObstacle actor;actor.type=7;actor.chest=true;actor.id=chest.entity_id();actor.raw_name=baked::object_identifier(chest);
            actor.cell={chest.layer,chest.tx,chest.ty};snapshot->obstacles.push_back(std::move(actor));
        }
        Planned route;std::thread worker([&]{route=plan_snapshot(*snapshot);});worker.join();
        const bool pass=route.path.success&&route.path.next_map_id==1&&route.pending.incoming_transition&&
            route.pending.incoming_transition->arrival==Cell{1,89,107};
        if(!pass)++failures;
        std::printf("Mist cave arrival%d: route%d next%d chests%zu northern-receipt%d reason[%ls]\n",point,route.path.success,
            route.path.next_map_id,snapshot->obstacles.size(),pass,route.path.reason.c_str());
    }
    std::printf("Mist foot approaches: %d failures, owner%d\n",failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
