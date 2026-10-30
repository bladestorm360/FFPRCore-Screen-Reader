// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
using namespace field; using namespace baked_nav;
int main() {
    std::ifstream f("data/ff3/maps.bin",std::ios::binary);
    std::string bytes{std::istreambuf_iterator<char>(f),{}};
    if(!baked::load(bytes.data(),bytes.size()))return 2;
    std::ifstream sf("data/ff3/mapstate.json",std::ios::binary);
    std::string state_bytes{std::istreambuf_iterator<char>(sf),{}};
    if(!json::parse(state_bytes,fixture::records))return 3;
    fixture::game_id=game::Id::FF3; g_loaded=g_prepared=true;
    const auto* sm=baked::submap_by_map_id(1);if(!sm)return 4;
    const auto& g=baked::grids(*sm);map_state::Flags flags;
    flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=1;
    map_state::Resolver state(flags,fixture::records);int failures=0;
    auto expect=[&](bool ok,const char* msg){if(!ok){++failures;std::printf("FAIL %s\n",msg);}};
    for(const Cell start: {Cell{1,26,61},Cell{1,25,39}}) {
        Cell goal{1,16,93}; baked_route::Request req;req.submap=sm;req.start=start;
        req.vehicle_group=6;req.vehicle_type=3;req.landing_group=5;req.transportation_id=6;
        req.goals={goal};req.goals_open=true;
        auto t=std::chrono::steady_clock::now();auto route=baked_route::enterprise_route(req,true);
        auto pure_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();
        int moved[4]={};Cell landing{};bool land=false;
        Cell previous=start;
        for(const auto& segment:route.segments) {
            for(const auto& c:segment.cells) { if(!(c==previous))++moved[segment.mode]; previous=c; }
            if(segment.mode==2&&!segment.cells.empty()){landing=segment.cells.front();land=true;}
        }
        // The original itinerary's last sailing cell is a concrete later landing
        // candidate. Verify a legal flight to it and a no-flight remaining route.
        const Cell later=start.x==26 ? Cell{1,26,72} : Cell{1,25,67};
        auto flight=req;flight.goals={later};flight.goals_open=false;auto airborne=baked_route::find(flight);
        auto ship=req;ship.start=later;ship.transportation_id=13;ship.vehicle_group=5;ship.vehicle_type=2;ship.landing_group=7;
        auto rest=baked_route::enterprise_route(ship,true);bool rest_flight=false;
        for(const auto& segment:rest.segments)rest_flight|=segment.mode==1;
        const bool witness=land && airborne.found && rest.steps>=0 && !rest_flight &&
            baked::group_lands(5,baked::attribute(g,later.x,later.y)) && airborne.steps()+rest.steps==route.steps;
        RouteSnapshot snapshot(state);snapshot.map_id=1;snapshot.submap=sm;snapshot.layer=1;snapshot.locale=L"en";
        snapshot.position=world_of_cell(*sm,start);snapshot.goal.map_id=1;snapshot.goal.cell=goal;snapshot.kind=TargetKind::Transition;
        snapshot.vehicle={true,6,3,{}};Planned plan;BrowsePlanned browse;t=std::chrono::steady_clock::now();
        std::thread worker([&]{plan=plan_snapshot(snapshot);});worker.join();
        const auto plan_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();
        std::thread browse_worker([&]{browse=plan_browse_snapshot(snapshot);});browse_worker.join();
        expect(browse.route.reason.starts_with(L"Fly ")&&!browse.route.reason.starts_with(L"Walk "),
            "actual airborne browse approach uses Fly before its native landing action");
        Cell plan_land{};bool plan_has=false;for(const auto& p:plan.path.walk_phases)
            if(p.vehicle_before==route_progress::VehicleAction::ConfirmBoard && p.transportation_id==13 && p.action_cell){plan_land=*p.action_cell;plan_has=true;break;}
        std::printf("start (%d,%d): steps=%d air=%d ship=%d foot=%d landing=(%d,%d), witness=(%d,%d) valid=%d pure=%.3fms expansions=%zu queue=%zu plan=%d steps=%d landing=(%d,%d) %.3fms\n",start.x,start.y,route.steps,moved[1],moved[0]+moved[2],moved[3],landing.x,landing.y,later.x,later.y,witness,pure_ms,route.expansions,route.peak_queue,plan.path.success,plan.path.step_count,plan_land.x,plan_land.y,plan_ms);
        expect(route.steps>=0&&plan.path.success&&plan.path.step_count==route.steps&&plan_has&&plan_land==landing,"pure and actual selected consumer agree");
        expect(witness,"independent equal-length later legal landing witness remains valid");
        if(witness)expect(moved[0]+moved[2]+moved[3]<=rest.steps,"selected route uses at least as much flight as the verified equal-length itinerary");
        auto flood=baked_route::enterprise_route(req,false);unsigned long long hash=1469598103934665603ull;
        int reachable=0;for(auto d:flood.foot.dist[0]) { hash=(hash^d)*1099511628211ull;reachable+=d!=0xffff; }
        std::printf("flood reachable=%d hash=%llu expansions=%zu queue=%zu\n",reachable,hash,flood.expansions,flood.peak_queue);
        auto cancelled=req;int calls=0;cancelled.cancelled=[&]{return ++calls>100;};
        auto stopped=baked_route::enterprise_route(cancelled,true);
        expect(stopped.steps<0&&!stopped.foot.submap&&calls==101,"selected search cancellation publishes no partial route");
        for(int id:{6,15}) {
            auto paired=req;paired.transportation_id=id;auto result=baked_route::enterprise_route(paired,true);
            expect(result.steps==route.steps,"both airship pairs preserve optimal distance");
        }
        auto sea=req;sea.goals={later};auto final=baked_route::enterprise_route(sea,true,true);
        expect(final.steps>=0&&!final.segments.empty()&&(final.segments.back().mode==0||final.segments.back().mode==2),"final ship goal stays aboard");
        auto blocked=req;blocked.blocked.push_back(later);auto detour=baked_route::enterprise_route(blocked,true);
        bool lands_blocked=false,legal_water=true;
        for(const auto& segment:detour.segments)if(segment.mode==2&&!segment.cells.empty()) {
            const auto c=segment.cells.front();lands_blocked|=c==later;
            legal_water&=baked::group_lands(5,baked::attribute(g,c.x,c.y));
        }
        expect(detour.steps>=0&&!lands_blocked&&legal_water,"blocked landing stays excluded and conversion uses native water terrain");
    }
    bool seam_checked=false; int seam_candidates=0;
    for(int map_id:{1,2}) {
        const auto* world=baked::submap_by_map_id(map_id);if(!world||!world->world)continue;
        const auto& grid=baked::grids(*world);
        for(int axis=0;axis<2&&!seam_checked;++axis) {
            const int count=axis?grid.width:grid.height;
            for(int i=0;i<count&&!seam_checked;++i) {
                baked_route::Request seam;seam.submap=world;
                seam.start=axis?Cell{1,i,0}:Cell{1,0,i};
                seam.goals={axis?Cell{1,i,grid.height-1}:Cell{1,grid.width-1,i}};
                seam.vehicle_group=6;seam.vehicle_type=3;seam.landing_group=5;seam.transportation_id=6;
                auto ship=seam;ship.vehicle_group=5;ship.vehicle_type=2;ship.landing_group=7;ship.transportation_id=13;
                if(!baked_route::open(seam,grid,seam.start)||!baked_route::open(seam,grid,seam.goals[0])||
                   !baked_route::open(ship,grid,seam.goals[0])||!baked::group_lands(5,baked::attribute(grid,seam.goals[0].x,seam.goals[0].y))||
                   !baked_route::edge_allowed(seam,grid,seam.start,seam.goals[0]))continue;
                ++seam_candidates;auto crossing=baked_route::enterprise_route(seam,true,true);
                if(crossing.steps==1) {
                    seam_checked=true;expect(crossing.segments.back().mode==2,"wrapped selected route lands aboard ship");
                    auto paired=seam;paired.transportation_id=15;auto other=baked_route::enterprise_route(paired,true,true);
                    expect(other.steps==1&&!other.segments.empty()&&other.segments.back().mode==2,"paired air15 wraps and lands aboard ship16");
                    std::printf("selected sea seam map=%d start=(%d,%d) goal=(%d,%d) steps=%d\n",map_id,seam.start.x,seam.start.y,seam.goals[0].x,seam.goals[0].y,crossing.steps);
                }
            }
        }
        if(seam_checked)break;
    }
    std::printf("selected wrapped sea seam candidates=%d checked=%d\n",seam_candidates,seam_checked);
    if(seam_candidates)expect(seam_checked,"selected enterprise route crosses a legal one-step world seam");
    std::printf("Enterprise landing: %d failures\n",failures);return failures?1:0;
}
