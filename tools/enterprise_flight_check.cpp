// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual production navigation with owned FFIII data; no game process.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include "fieldnav/autowalk_route.h"
namespace {
using namespace field; using namespace baked_nav;
int failures=0;
void expect(bool ok,const char* label) { if(!ok) { ++failures;std::printf("FAIL %s\n",label); } }
std::string read_bytes(const char* path) { std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}}; }
void replay(const baked::Submap& sm,Cell start) {
    const auto& g=baked::grids(sm);
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=1;
    map_state::Resolver state(flags,fixture::records);
    baked_route::Request request;request.submap=&sm;request.start=start;request.vehicle_group=6;request.vehicle_type=3;request.landing_group=5;request.transportation_id=6;
    const auto began=std::chrono::steady_clock::now();
    const auto coverage=baked_route::enterprise_route(request,false);
    const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
    std::printf("Enterprise coverage: %.3f ms, cells=%d states<=%d expansions=%zu peak-queue=%zu\n",elapsed,g.width*g.height,4*g.width*g.height,coverage.expansions,coverage.peak_queue);
    const auto& flood=coverage.foot;
    auto cancelled=request;int checks=0;cancelled.cancelled=[&]{return ++checks>100;};
    const auto cancel_start=std::chrono::steady_clock::now();const auto stopped=baked_route::enterprise_route(cancelled,false);
    std::printf("Cancellation: checks=%d, %.3f ms\n",checks,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-cancel_start).count());
    expect(!stopped.foot.submap&&checks==101,"obsolete search stops without publishing reachability");
    expect(!baked_route::enterprise_pair(5),"original locked Enterprise never receives flight");
    auto isolated=request;const int dx[4]={0,1,0,-1},dy[4]={-1,0,1,0};
    for(int i=0;i<4;++i)isolated.blocked.push_back({1,(start.x+dx[i]+g.width)%g.width,(start.y+dy[i]+g.height)%g.height});
    const auto surrounded=baked_route::enterprise_route(isolated,false);
    bool escaped=false;for(auto d:surrounded.foot.dist[0])escaped|=d!=0xffff;
    expect(!escaped,"live blockers prevent every exit from an isolated vehicle");
    auto directional=request;directional.directional.push_back({start,0,0});
    const auto walled=baked_route::enterprise_route(directional,false);escaped=false;
    for(auto d:walled.foot.dist[0])escaped|=d!=0xffff;
    expect(!escaped,"directional collision blocks departing the vehicle cell");
    auto trapped=request;trapped.live_traps=true;trapped.traps=isolated.blocked;
    const auto trap_result=baked_route::enterprise_route(trapped,false);escaped=false;
    for(auto d:trap_result.foot.dist[0])escaped|=d!=0xffff;
    expect(!escaped,"active traps cannot become movement or shore edges");
    int rapid_checks=0;const auto rapid_start=std::chrono::steady_clock::now();
    for(int i=0;i<50;++i) { auto obsolete=request;int count=0;obsolete.cancelled=[&]{++rapid_checks;return ++count>100;};baked_route::enterprise_route(obsolete,false); }
    std::printf("50 rapid cancellations: checks=%d, %.3f ms cumulative\n",rapid_checks,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-rapid_start).count());
    bool seam=false;
    for(int i=0;i<baked::submap_count()&&!seam;++i) {
        const auto* world=baked::submap_at(i);if(!world||!world->world)continue;
        const auto& grid=baked::grids(*world);
        for(int y=0;y<grid.height&&!seam;++y) {
            auto flight=request;flight.submap=world;flight.start={1,0,y};flight.goals={{1,grid.width-1,y}};flight.goals_open=false;
            if(!baked_route::open(flight,grid,flight.start)||!baked_route::open(flight,grid,flight.goals[0])||
               !baked_route::edge_allowed(flight,grid,flight.start,flight.goals[0]))continue;
            const auto across=baked_route::find(flight);
            if(across.steps()==1) { seam=true;expect(across.found&&across.cells.back()==flight.goals[0],"shared flight neighbours cross the real wrapped seam"); }
        }
        for(int x=0;x<grid.width&&!seam;++x) {
            auto flight=request;flight.submap=world;flight.start={1,x,0};flight.goals={{1,x,grid.height-1}};flight.goals_open=false;
            if(!baked_route::open(flight,grid,flight.start)||!baked_route::open(flight,grid,flight.goals[0])||
               !baked_route::edge_allowed(flight,grid,flight.start,flight.goals[0]))continue;
            const auto across=baked_route::find(flight);
            if(across.steps()==1)seam=across.found&&across.cells.back()==flight.goals[0];
        }
    }
    expect(seam,"real FFIII flight terrain permits a wrapped edge");
    auto foot=request;foot.vehicle_group=foot.vehicle_type=foot.landing_group=0;foot.goals_open=true;
    const baked::Object* town=nullptr;const baked::Object* nearby=nullptr;int farthest=-1,nearest=INT_MAX;
    for(const auto& o:g.objects) if(o.type==baked::ObjType::GotoMap&&o.a>0&&state.active(sm,o)) {
        foot.goals={{1,o.tx,o.ty}};
        const int distance=baked_route::transition_distance(foot,flood);
        if(distance>farthest) { town=&o;farthest=distance; }
        if(distance>=0&&distance<nearest) { nearby=&o;nearest=distance; }
    }
    expect(town!=nullptr,"real reachable town exists");if(!town)return;
    Cell target{1,town->tx,town->ty};
    // The unchanged generic landing branch reproduces the old list filter:
    // an unrecognized identity has no mode conversion and cannot reach this town.
    auto old=request;old.transportation_id=0;const auto old_ride=baked_route::flood(old);
    const auto old_land=land_after_ride(old,old_ride);foot.goals={target};
    const int old_distance=baked_route::transition_distance(foot,old_land);
    std::printf("Generic pre-fix landing/filter distance to town: %d\n",old_distance);
    expect(old_distance<0,"old mounted-air filter omits the reachable town");
    for(int id:{6,13,15,16}) {
        RouteSnapshot snapshot(state);snapshot.map_id=1;snapshot.submap=&sm;snapshot.layer=1;snapshot.locale=L"en";
        snapshot.position=world_of_cell(sm,start);snapshot.goal.map_id=1;snapshot.goal.cell=target;snapshot.kind=TargetKind::Transition;
        const auto* row=baked::transport(id);snapshot.vehicle={true,id,row->type,{}};
        Planned plan;BrowsePlanned browse;std::vector<int> distances[3];std::vector<bool> hidden;
        const auto plan_start=std::chrono::steady_clock::now();
        std::thread worker([&]{
            plan=plan_snapshot(snapshot);
            browse=plan_browse_snapshot(snapshot);
            for(int mode=0;mode<3;++mode) {
                ExitSnapshot capture(mode==1,state);capture.story_filter=mode==2;
                capture.route.submap=&sm;capture.route.map_id=1;capture.route.layer=1;
                capture.route.position=snapshot.position;capture.route.vehicle=snapshot.vehicle;
                distances[mode]=exit_distances(capture,{{world_of_cell(sm,target),1,town->a,target,TargetKind::Transition}},{});
                if(mode==2) { StoryTarget story;story.goal.map_id=1;story.goal.cell=target;story.kind=TargetKind::Transition;hidden=story_gate_mask(capture,{story},{}); }
            }
        });worker.join();
        const double plan_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-plan_start).count();
        bool walkoff=false,conversion=false;
        for(const auto& phase:plan.path.walk_phases) {
            walkoff|=phase.vehicle_before==route_progress::VehicleAction::WalkOff;
            if(phase.vehicle_before==route_progress::VehicleAction::ConfirmBoard) {
                conversion=true;
                expect(phase.action_cell&&*phase.action_cell==phase.cells.front(),"conversion action remains at the mode-change cell");
                expect(phase.transportation_id==(id==6||id==13?6:15)||phase.transportation_id==(id==6||id==13?13:16),"conversion waits for the paired observed identity");
            }
        }
        std::printf("id%d town (%d,%d): plan=%d steps=%d phases=%zu walkoff=%d convert=%d filters=%d/%d/%d, episode %.3f ms\n",id,target.x,target.y,plan.path.success,plan.path.step_count,plan.path.walk_phases.size(),walkoff,conversion,distances[0].empty()?-1:distances[0][0],distances[1].empty()?-1:distances[1][0],distances[2].empty()?-1:distances[2][0],plan_ms);
        expect(plan.path.success&&walkoff&&!browse.route.reason.empty(),"selected town and browse agree on valid shore route");
        for(const auto& distance:distances)expect(!distance.empty()&&distance[0]>=0,"each filter retains reachable town");
        expect(hidden.size()==1&&!hidden[0],"story filter retains the reachable town");
        if(id==6||id==15)expect(conversion,"air starts require water conversion");
        if(id==6&&plan.path.success&&plan.active) {
            for(const auto& phase:plan.path.walk_phases)if(phase.vehicle_before==route_progress::VehicleAction::ConfirmBoard) {
                RouteSnapshot repeat(state);repeat.map_id=1;repeat.submap=&sm;repeat.layer=1;repeat.locale=L"en";
                repeat.position=world_of_cell(sm,*phase.action_cell);repeat.goal.map_id=1;repeat.goal.cell=target;repeat.kind=TargetKind::Transition;
                repeat.vehicle={true,6,3,{}};repeat.previous_active=plan.active;
                for(int observed:{6,13}) { VehicleOnMap v;v.transportation_id=observed;v.type=baked::transport(observed)->type;v.pos=repeat.position;repeat.vehicles.push_back(v); }
                Planned again;const auto repeat_start=std::chrono::steady_clock::now();
                std::thread repeated([&]{again=plan_snapshot(repeat);});repeated.join();
                bool retained=false;for(const auto& p:again.path.walk_phases)retained|=p.vehicle_before==route_progress::VehicleAction::ConfirmBoard&&p.transportation_id==13;
                std::printf("Repeated request at conversion: retained=%d, %.3f ms\n",retained,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-repeat_start).count());
                expect(again.path.success&&retained,"repeated request before conversion retains native action");
                repeat.vehicle={true,13,2,{}};repeat.previous_active=plan.active;
                Planned after;std::thread converted([&]{after=plan_snapshot(repeat);});converted.join();
                bool wrong=false;for(const auto& p:after.path.walk_phases)wrong|=p.vehicle_before==route_progress::VehicleAction::ConfirmBoard&&p.transportation_id==13&&p.action_cell&&*p.action_cell==cell_of(sm,repeat.position,1);
                expect(after.path.success&&!wrong,"request after conversion uses observed ship identity");
                break;
            }
        }
        if(plan.path.success) {
            namespace a=ffpr::fieldnav::autowalk;
            a::Route follower;follower.set(plan.path.walk_phases,g.width,g.height,true);Cell at=start;
            bool arrived=false;int actions=0;
            for(int n=0;n<10000;++n) {
                const auto step=follower.next(at);
                if(step.result==a::Result::Move)at=step.next;
                else if(step.result==a::Result::Action) { ++actions;follower.complete_vehicle_action(); }
                else if(step.result==a::Result::Arrived) { arrived=true;break; }
                else break;
            }
            expect(arrived&&at==target,"production autowalk follower replays all phases to the town");
            if(id==6||id==15)expect(actions>0,"follower exposes native conversion action");
        }
    }
    if(nearby) {
        RouteSnapshot close(state);close.map_id=1;close.submap=&sm;close.layer=1;close.locale=L"en";
        close.position=world_of_cell(sm,start);close.goal.map_id=1;close.goal.cell={1,nearby->tx,nearby->ty};close.kind=TargetKind::Transition;
        close.vehicle={true,6,3,{}};Planned close_plan;std::thread close_worker([&]{close_plan=plan_snapshot(close);});close_worker.join();
        bool walkoff=false;for(const auto& p:close_plan.path.walk_phases)walkoff|=p.vehicle_before==route_progress::VehicleAction::WalkOff;
        expect(close_plan.path.success&&walkoff,"air6 reaches a nearby town through a legal shore");
        std::printf("Nearby town (%d,%d): steps=%d\n",nearby->tx,nearby->ty,close_plan.path.step_count);
        if(close_plan.active)for(const auto& phase:close_plan.path.walk_phases)if(phase.vehicle_before==route_progress::VehicleAction::ConfirmBoard) {
            close.position=world_of_cell(sm,*phase.action_cell);close.previous_active=close_plan.active;
            for(int observed:{6,13}) { VehicleOnMap v;v.transportation_id=observed;v.type=baked::transport(observed)->type;v.pos=close.position;close.vehicles.push_back(v); }
            Planned repeated;std::thread worker([&]{repeated=plan_snapshot(close);});worker.join();
            bool retained=false;for(const auto& p:repeated.path.walk_phases)retained|=p.vehicle_before==route_progress::VehicleAction::ConfirmBoard&&p.transportation_id==13;
            std::printf("Nearby conversion repeated: retained=%d, sea phase cells=%zu\n",retained,phase.cells.size());
            expect(repeated.path.success&&retained,"nearby same-cell conversion is retained on repeated request");break;
        }
    }
    auto direct=request;direct.transportation_id=13;direct.vehicle_group=5;direct.vehicle_type=2;direct.landing_group=7;
    direct.goals={{1,start.x,(start.y+g.height-1)%g.height}};direct.goals_open=true;
    const auto direct_shore=baked_route::enterprise_route(direct,true);
    expect(direct_shore.steps==1&&direct_shore.segments.size()==2&&direct_shore.segments[0].mode==0&&direct_shore.segments[1].mode==3,"ship13 may walk directly to nearby shore without taking off");
    RouteSnapshot boundary(state);boundary.map_id=1;boundary.submap=&sm;boundary.layer=1;boundary.locale=L"en";
    boundary.position=world_of_cell(sm,start);boundary.vehicle={true,6,3,{}};
    std::vector<Phase> constructed,remaining,recomputed;bool bypass=false;
    const auto boundary_start=std::chrono::steady_clock::now();
    std::thread boundary_worker([&]{SnapshotScope scope(boundary);auto movement=request;movement.goals=direct.goals;
        if(boarding_route(sm,g,movement,movement.goals,constructed)) {
            ActiveRoute active;active.phases=constructed;remaining_from(active,start,remaining);
            bypass=enterprise_conversion_plan(active);
            if(bypass)boarding_route(sm,g,movement,movement.goals,recomputed);
        }
    });boundary_worker.join();
    bool boundary_conversion=false;for(const auto& p:remaining)boundary_conversion|=p.vehicle_before==route_progress::VehicleAction::ConfirmBoard&&p.transportation_id==13;
    bool fresh_conversion=false;for(const auto& p:recomputed)fresh_conversion|=p.vehicle_before==route_progress::VehicleAction::ConfirmBoard&&p.transportation_id==13;
    std::printf("Actual one-cell sea conversion: phases=%zu, legacy retains=%d, bypass=%d, recomputed retains=%d, two searches %.3f ms\n",constructed.size(),boundary_conversion,bypass,fresh_conversion,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-boundary_start).count());
    expect(!constructed.empty()&&!boundary_conversion&&bypass&&fresh_conversion,"same-cell conversion bypasses unsafe reuse and is recomputed");
    int ship_scenes=0;bool scene_checked=false;
    for(const auto& o:g.objects)if(baked::ship_entry(o)&&state.active(sm,o)) {
        ++ship_scenes;if(scene_checked)continue;
        auto sea_goal=request;sea_goal.goals={{1,o.tx,o.ty}};sea_goal.goals_open=true;
        if(baked_route::enterprise_route(sea_goal,true,true).steps<0)continue;
        RouteSnapshot scene(state);scene.map_id=1;scene.submap=&sm;scene.layer=1;scene.locale=L"en";
        scene.position=world_of_cell(sm,start);scene.goal.map_id=1;scene.goal.cell=sea_goal.goals[0];scene.kind=TargetKind::Infer;scene.vehicle={true,6,3,{}};
        Planned scene_plan;std::thread scene_worker([&]{scene_plan=plan_snapshot(scene);});scene_worker.join();
        bool walkoff=false;for(const auto& p:scene_plan.path.walk_phases)walkoff|=p.vehicle_before==route_progress::VehicleAction::WalkOff;
        expect(scene_plan.path.success&&!walkoff,"reviewed ship scene finishes aboard rather than inventing shore");scene_checked=true;
    }
    std::printf("Active ship scenes: %d, production replayed: %d\n",ship_scenes,scene_checked);
    RouteSnapshot locked(state);locked.map_id=1;locked.submap=&sm;locked.layer=1;locked.locale=L"en";
    locked.position=world_of_cell(sm,start);locked.goal.map_id=1;locked.goal.cell=target;locked.kind=TargetKind::Transition;
    locked.vehicle={true,5,2,{}};Planned locked_plan;std::thread locked_worker([&]{locked_plan=plan_snapshot(locked);});locked_worker.join();
    bool locked_conversion=false;for(const auto& p:locked_plan.path.walk_phases)locked_conversion|=p.vehicle_before==route_progress::VehicleAction::ConfirmBoard;
    expect(!locked_conversion,"original ship5 does not acquire an air conversion in production planning");
    // A parked upgraded ship is approached on foot and boarded at its actual cell.
    Cell shore{1,start.x,(start.y+g.height-1)%g.height};
    RouteSnapshot parked(state);parked.map_id=1;parked.submap=&sm;parked.layer=1;parked.locale=L"en";
    parked.position=world_of_cell(sm,shore);parked.goal.map_id=1;parked.goal.cell=target;parked.kind=TargetKind::Transition;parked.vehicle_shortcuts=true;
    VehicleOnMap vehicle;vehicle.transportation_id=13;vehicle.type=2;vehicle.pos=world_of_cell(sm,start);parked.vehicles.push_back(vehicle);
    Planned parked_plan;std::thread worker([&]{parked_plan=plan_snapshot(parked);});worker.join();
    bool boarded=false;for(const auto& p:parked_plan.path.walk_phases)boarded|=p.vehicle_before==route_progress::VehicleAction::WalkOn&&p.transportation_id==13;
    std::printf("Parked13 town: plan=%d board=%d phases=%zu\n",parked_plan.path.success,boarded,parked_plan.path.walk_phases.size());
    expect(parked_plan.path.success&&boarded,"parked Enterprise uses its legal observed boarding edge");
}
}
int main() {
    using namespace field; using namespace baked_nav;
    std::ifstream f("data/ff3/maps.bin",std::ios::binary);
    std::string bytes{std::istreambuf_iterator<char>(f),{}};
    if(!baked::load(bytes.data(),bytes.size())) return 2;
    if(!json::parse(read_bytes("data/ff3/mapstate.json"),fixture::records))return 7;
    fixture::game_id=game::Id::FF3; g_loaded=g_prepared=true;
    const auto* sm=baked::submap_by_map_id(1); if(!sm) return 3;
    const auto& g=baked::grids(*sm);
    baked_route::Request req; req.submap=sm; req.vehicle_group=6; req.vehicle_type=3; req.landing_group=5;req.transportation_id=6;
    const auto* ship=baked::transport(13); if(!ship) return 4;
    for(int y=0;y<g.height;++y) for(int x=0;x<g.width;++x) {
        Cell water{1,x,y}; auto sailing=req;sailing.vehicle_group=ship->group;
        if(!baked_route::open(req,g,water)||!baked_route::open(sailing,g,water)) continue;
        for(const auto& d:std::vector<Cell>{{1,0,-1},{1,1,0},{1,0,1},{1,-1,0}}) {
            Cell shore{1,(x+d.x+g.width)%g.width,(y+d.y+g.height)%g.height};
            auto foot=req;foot.vehicle_group=0;foot.vehicle_type=foot.landing_group=0;
            if(!baked_route::open(foot,g,shore)||!baked::group_lands(ship->landing_group,baked::attribute(g,shore.x,shore.y))||baked_route::terminal_transition(foot,g,shore))continue;
            req.start=water;
            baked_route::Reach land; std::thread worker([&]{const auto ride=baked_route::flood(req);land=land_after_ride(req,ride);});worker.join();
            std::printf("Mounted air6 at (%d,%d), legal ship shore (%d,%d): landed reachable=%d; wrong-owner=%d\n",x,y,shore.x,shore.y,land.reachable(shore),fixture::wrong_owner.load());
            expect(land.reachable(shore),"air converts to ship and reaches shore");
            replay(*sm,water);
            std::printf("Enterprise flight: %d failures\n",failures);
            return fixture::wrong_owner||failures?5:0;
        }
    }
    return 6;
}
