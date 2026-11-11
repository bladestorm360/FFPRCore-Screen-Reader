// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Published world objects retain their real identity through route/browse consumers.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/core/field/target_kind.h"
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff3/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
    auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(1);flags->global.fill(1);flags->treasure.fill(1);
    flags->current_map_id=1;flags->current_area_id=1;
    map_state::Resolver provider(*flags,fixture::records);
    const auto inventory=world_objects_snapshot(1,provider,{});
    int failures=0,cases=0,underwater=0,doga=0;
    const auto expect=[&](bool pass,const char* message){if(!pass){++failures;if(failures<=16)std::printf("FAIL %s\n",message);}};
    // A diagonal tile change during capture invalidated every old publication.
    // The late sample fixes capture-age drift without accepting a stale result
    // or a changed vehicle, and abstains entirely off the native owner.
    auto moving=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
    moving->map_id=1;moving->submap=baked::submap_by_map_id(1);moving->layer=1;
    moving->position=world_of_cell(*moving->submap,{1,64,64});moving->vehicle={true,11,baked::transport(11)->type,{}};
    fixture::capture_ready=true;fixture::current_map=1;fixture::observed_vehicle=moving->vehicle;
    fixture::current_position=world_of_cell(*moving->submap,{1,65,65});
    Planned old;old.map_id=1;old.start={1,64,64};old.vehicle=moving->vehicle;
    expect(!still_current(old),"pre-capture pose cannot publish after a diagonal tile move");
    refresh_capture_origin(*moving);
    Planned fresh;fresh.map_id=1;fresh.start=cell_of(*moving->submap,moving->position,1);fresh.vehicle=moving->vehicle;
    expect(fresh.start==Cell{1,65,65}&&still_current(fresh),"late capture pose can publish safely without a new route search");
    fixture::observed_vehicle.id=8;
    expect(!still_current(fresh),"late sampling preserves rejection of a changed vehicle");
    const int reads=fixture::engine_reads;
    std::thread foreign([&]{refresh_capture_origin(*moving);});foreign.join();
    expect(fixture::engine_reads==reads,"capture refresh makes no engine reads off its owner");
    fixture::capture_ready=false;fixture::observed_vehicle={};
    size_t checked_edges=0;
    for(const int map_id:{3,4})for(int profile=0;profile<2;++profile) {
        const auto* map=baked::submap_by_map_id(map_id);const auto* model=baked::transport(map_id==3?14:9);
        baked_route::Request request;request.submap=map;request.vehicle_group=model->group;
        request.vehicle_type=model->type;request.landing_group=model->landing_group;request.transportation_id=model->id;
        if(profile) {
            request.goals={{1,95,77}};request.goals_open=true;request.allow_trap_goal=true;
            request.blocked={{1,95,78}};request.live_hazards=true;request.hazards={{1,96,78}};
        }
        std::vector<uint8_t> forward(256*256);
        const auto direction=[](const Cell& from,const Cell& to){
            if(to.x==from.x&&to.y==(from.y+255)%256)return 0;
            if(to.y==from.y&&to.x==(from.x+1)%256)return 1;
            if(to.x==from.x&&to.y==(from.y+1)%256)return 2;
            return 3;
        };
        for(int y=0;y<256;++y)for(int x=0;x<256;++x) {
            const Cell at{1,x,y};std::array<Cell,8> next;
            const auto count=baked_route::successors(request,at,next);
            for(size_t i=0;i<count;++i)forward[y*256+x]|=1<<direction(at,next[i]);
        }
        for(int y=0;y<256;++y)for(int x=0;x<256;++x) {
            const Cell at{1,x,y};std::array<Cell,8> before;
            const auto count=baked_route::world_predecessors(request,at,before);
            uint8_t actual=0,expected=0;
            for(size_t i=0;i<count;++i)actual|=1<<direction(at,before[i]);
            const Cell neighbors[4]={{1,x,(y+255)%256},{1,(x+1)%256,y},{1,x,(y+1)%256},{1,(x+255)%256,y}};
            for(int d=0;d<4;++d)if(forward[neighbors[d].y*256+neighbors[d].x]&(1<<((d+2)%4)))expected|=1<<d;
            expect(actual==expected,"reverse world edges exactly transpose real forward movement, goals, blockers and hazards");
            ++checked_edges;
        }
    }
    std::printf("Reverse world adjacency: %zu real map/profile cells checked\n",checked_edges);
    for(const auto& object:inventory) {
        const bool water=object.map_id==3&&object.world_exit;
        const bool land=object.map_id==4&&object.type==baked::ObjType::GotoMap&&
            ((object.cell.x==128&&object.cell.y==184)||(object.cell.x==150&&object.cell.y==203));
        if(!water&&!land)continue;
        if(water)++underwater;else ++doga;
        for(const int origin:{4,1}) {
            auto context=std::make_unique<map_state::Flags>(*flags);context->current_map_id=origin;
            auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*context,fixture::records));
            snapshot->map_id=origin;snapshot->submap=baked::submap_by_map_id(origin);snapshot->layer=1;snapshot->locale=L"en";
            snapshot->vehicle_shortcuts=true;
            const int id=origin==1?11:9;const Cell start=origin==1?Cell{1,64,64}:Cell{1,113,118};
            snapshot->position=world_of_cell(*snapshot->submap,start);snapshot->vehicle={true,id,baked::transport(id)->type,{}};
            snapshot->saved_vehicles={{9,4,{151,74,0}}};snapshot->vehicle_names[9]=L"Nautilus";
            // Same native restriction observations as the established journey fixture.
            for(const auto& actor:baked::grids(*snapshot->submap).objects)
                if(const auto* rule=air_transitions::match(3,origin,actor)) {
                    NativeObstacle seen;seen.type=2;seen.id=actor.entity_id();seen.cell={1,actor.tx,actor.ty};
                    seen.raw_name=baked::object_identifier(actor);seen.trigger=true;
                    seen.transportation_restricted=seen.transportation_complete=true;seen.transportation_ids=rule->mask;
                    snapshot->obstacles.push_back(std::move(seen));
                }
            // These are the real fields copied by RemoteBuilder/build_remote and
            // entity_nav, including the identity omitted by earlier journey tests.
            snapshot->goal.map_id=object.map_id;snapshot->goal.cell=object.cell;
            snapshot->goal.entity_id=object.entity_id;snapshot->goal.raw_name=object.raw_name;
            snapshot->kind=navigation_target_kind(Kind::MapExit,Category::MapExits);
            Planned route;BrowsePlanned browse;double route_ms=0,browse_ms=0;
            baked_route::WorkProfile route_work,browse_work;
            std::thread worker([&] {
                baked_route::reset_work_profile();
                auto began=std::chrono::steady_clock::now();route=plan_snapshot(*snapshot);route_work=baked_route::work_profile();
                route_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                baked_route::reset_work_profile();
                began=std::chrono::steady_clock::now();browse=plan_browse_snapshot(*snapshot);browse_work=baked_route::work_profile();
                browse_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
            });worker.join();++cases;
            std::printf("Published goal %d:%d type%d from%d vehicle%d: success%d next%d steps%d route%.3fms browse%.3fms reason[%ls]\n",
                object.map_id,object.entity_id,int(object.type),origin,id,route.path.success,route.path.next_map_id,
                route.path.step_count,route_ms,browse_ms,route.path.reason.c_str());
            std::printf("  Work route: find%d/%.3fms flood%d/%.3fms; browse: find%d/%.3fms flood%d/%.3fms\n",
                route_work.find_calls,route_work.find_ms,route_work.flood_calls,route_work.flood_ms,
                browse_work.find_calls,browse_work.find_ms,browse_work.flood_calls,browse_work.flood_ms);
            for(const auto& item:route_work.searches)std::printf("    Find %d,%d -> %d,%d vehicle%d found%d expanded%d %.3fms\n",item.x,item.y,item.gx,item.gy,item.vehicle,item.found,item.expansions,item.ms);
            expect(route.path.success,"actual published world destination returns a route");
            if(origin==4&&land&&object.cell==nautilus_surface::village_cell&&route.path.success&&route.active) {
                auto repeated=std::make_unique<RouteSnapshot>(*snapshot);
                repeated->goal=route.goal;repeated->previous_active=route.active;repeated->pending=route.pending;
                repeated->active={};
                if(!route.path.walk_phases.empty()&&route.path.walk_phases.front().cells.size()>1)
                    repeated->position=world_of_cell(*repeated->submap,route.path.walk_phases.front().cells[1]);
                Planned again;std::thread repeat([&]{again=plan_snapshot(*repeated);});repeat.join();
                const auto& receipt=again.pending.incoming_transition;
                std::printf("Repeated Village directions while moving: success%d next%d manual-receipt%d\n",
                    again.path.success,again.path.next_map_id,receipt.has_value());
                expect(again.path.success&&again.path.next_map_id==3&&receipt&&receipt->entity_id==-300020&&receipt->destination==3,
                    "repeated directions preserve the real pending dive and its arrival receipt");
            }
            if(route.path.success&&route.path.next_map_id)
                expect(browse.route.via_map_id==route.path.next_map_id,"browse and directions preserve the same first world transition");
            if(origin==1&&water&&object.entity_id==4) {
                // The available Nautilus can be parked on this continent,
                // rather than already waiting on the destination world.
                const auto& grid=baked::grids(*snapshot->submap);
                baked_route::Request foot;foot.submap=snapshot->submap;
                std::optional<Cell> parked;
                for(int y=8;y<248&&!parked;++y)for(int x=8;x<248&&!parked;++x) {
                    Cell at{1,x,y};
                    if(baked_route::open(foot,grid,at)&&
                       baked::group_lands(baked::transport(11)->landing_group,baked::attribute(grid,x,y))&&
                       !baked_route::terminal_transition(foot,grid,at)&&
                       baked_route::open(foot,grid,{1,x+1,y}))parked=at;
                }
                expect(parked.has_value(),"source-continent craft has a real legal parking/boarding contact");
                if(parked) {
                    snapshot->saved_vehicles.clear();snapshot->vehicles.clear();snapshot->pending={};snapshot->active={};
                    VehicleOnMap boat;boat.transportation_id=8;boat.type=baked::transport(8)->type;
                    boat.pos=world_of_cell(*snapshot->submap,*parked);snapshot->vehicles.push_back(boat);
                    Planned before_border,already_boarded;
                    std::thread change([&]{before_border=plan_snapshot(*snapshot);});change.join();
                    auto boarded=std::make_unique<RouteSnapshot>(*snapshot);boarded->pending={};boarded->active={};
                    boarded->position=boat.pos;boarded->vehicle={true,8,boat.type,{}};boarded->vehicles.clear();
                    std::thread continue_trip([&]{already_boarded=plan_snapshot(*boarded);});continue_trip.join();
                    std::printf("Source-continent Nautilus8 at%d,%d: Invincible/parked%d already-boarded%d\n",
                        parked->x,parked->y,before_border.path.success,already_boarded.path.success);
                    expect(already_boarded.path.success,"native upgraded Nautilus continuation to the actual underwater entrance is legal");
                    expect(before_border.path.success,"cross-world routing can change craft before crossing to the next world");
                }
            }
        }
    }
    expect(underwater==4&&doga==2,"all four underwater entrances and both Doga destinations come from the actual provider");
    expect(fixture::wrong_owner==0,"worker route and browse use no thread-affine native APIs");
    std::printf("FFIII published world requests: %d route/browse pairs, %d failures, owner%d; throttled tool timings are not game latency\n",cases,failures,fixture::wrong_owner.load());
    return failures?1:0;
}
