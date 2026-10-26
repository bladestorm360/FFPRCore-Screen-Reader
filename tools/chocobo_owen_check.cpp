// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact logged locations and production owned-snapshot planning; no game process.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
namespace {
using namespace field;using namespace baked_nav;
int failures=0;
void expect(bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}}
std::string bytes(const std::string& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
std::unique_ptr<RouteSnapshot> make(int map,Cell start,Cell goal,TargetKind kind) {
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=map;
    auto out=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
    out->map_id=map;out->submap=baked::submap_by_map_id(map);out->layer=start.layer;out->locale=L"en";
    out->position=world_of_cell(*out->submap,start);out->goal.map_id=map;out->goal.cell=goal;out->kind=kind;return out;
}
Planned plan(RouteSnapshot& snapshot){Planned out;std::thread worker([&]{out=plan_snapshot(snapshot);});worker.join();return out;}
void world_event_vehicle_advice(){
    auto captured=make(1,{1,86,25},{1,87,91},TargetKind::Infer);
    // Exact reported world goal: an owned vehicle can fail to board/land, but
    // ownership is already satisfied and must not be blamed by the diagnostic.
    for(const auto& row:baked::transports()){
        if(row.group<=1)continue;
        VehicleOnMap vehicle;vehicle.transportation_id=row.id;vehicle.type=row.type;
        vehicle.pos=world_of_cell(*captured->submap,{1,86,30});captured->vehicles.push_back(vehicle);
        captured->vehicle_names[row.id]=L"Owned vehicle";
    }
    std::thread worker([&]{SnapshotScope scope(*captured);
        auto request=walk_request(*captured->submap,{1,86,25});request.goals={{1,87,91}};
        expect(vehicle_required(baked::grids(*captured->submap),request).empty(),"an inaccessible world event cannot require a vehicle group already present in the owned snapshot");
        // The mounted vehicle may not have a separately rendered parked entity.
        // Use a trivially reachable goal to isolate ownership from map geometry.
        const auto mounted=captured->vehicles.back();captured->vehicles.pop_back();
        request.vehicle_group=baked::transport(mounted.transportation_id)->group;request.goals={request.start};
        expect(vehicle_required(baked::grids(*captured->submap),request).empty(),"a mounted vehicle without a parked entity cannot be reported missing");
    });worker.join();
}
void collision_sides(const baked::Submap& map) {
    const Cell at{1,64,31};baked_route::Request movement;movement.submap=&map;
    baked_route::add_collision(movement.blocked,movement.directional,at,2);
    expect(movement.blocked.empty()&&movement.directional.size()==1,"HALF_TOP is partial geometry, not a full story wall");
    expect(baked_route::direction_allowed(movement,{1,63,31},at)&&baked_route::direction_allowed(movement,at,{1,63,31}),"half-top permits west entry and exit");
    expect(baked_route::direction_allowed(movement,{1,65,31},at)&&baked_route::direction_allowed(movement,{1,64,32},at),"half-top permits east and south entry");
    expect(!baked_route::direction_allowed(movement,{1,64,30},at)&&!baked_route::direction_allowed(movement,at,{1,64,30}),"half-top blocks its actual northern face in both directions");
    for(int id=2;id<=7;++id) {
        baked_route::Request test;test.submap=&map;baked_route::add_collision(test.blocked,test.directional,at,id);
        int count=0;for(const auto& d:std::vector<Cell>{{0,0,-1},{0,1,0},{0,0,1},{0,-1,0}}) {
            const Cell other{1,at.x+d.x,at.y+d.y};
            const bool enter=baked_route::direction_allowed(test,other,at),leave=baked_route::direction_allowed(test,at,other);
            expect(enter==leave,"each partial collider face is symmetric");count+=enter;
        }
        expect(test.blocked.empty()&&count==(id<=5?3:2),"half/through colliders retain their native open cardinal faces");
    }
    std::vector<Cell> blocked;std::vector<baked_route::Request::Directional> directional;
    baked_route::add_collision(blocked,directional,at,0);expect(blocked.empty()&&directional.empty(),"native None collision contributes no wall");
    baked_route::add_collision(blocked,directional,at,1);baked_route::add_collision(blocked,directional,at,-1);
    expect(blocked.size()==2,"full and unknown collision remain solid");
}
void owen() {
    auto riding=make(1,{1,32,40},{1,64,31},TargetKind::Transition);
    riding->vehicle={true,12,9,{}};riding->vehicle_names[12]=L"Chocobo";
    auto result=plan(*riding);
    expect(result.path.success&&result.path.walk_phases.size()==2,"logged chocobo approach retains its legal ride and final dismount");
    if(result.path.walk_phases.size()==2) {
        const auto& ride=result.path.walk_phases[0];const auto& foot=result.path.walk_phases[1];
        expect(ride.cells.back()==Cell{1,63,31}&&foot.cells.size()==2,"equal-cost choice keeps the rider mounted to the final usable approach, not fifteen cells early");
        expect(foot.action_before==L"dismount"&&foot.vehicle_before==route_progress::VehicleAction::ConfirmLand,"ground mount says dismount while retaining native confirm control");
        const auto* transport=baked::transport(12);const auto& grid=baked::grids(*riding->submap);
        expect(transport&&baked::group_lands(transport->landing_group,baked::attribute(grid,63,31)),"chosen dismount still obeys the actual native terrain rule");
    }
    std::printf("Logged chocobo: %d steps; %ls\n",result.path.step_count,result.path.full_description.c_str());
    auto foot=make(1,{1,63,31},{1,64,31},TargetKind::Transition);
    NativeObstacle collider;collider.type=19;collider.id=0;collider.cell={-1,64,31};collider.collider=true;collider.collision=2;foot->obstacles={collider};
    auto walk=plan(*foot);BrowsePlanned browse;std::vector<int> distances;
    std::thread filter([&]{browse=plan_browse_snapshot(*foot);ExitSnapshot capture(false,foot->state);
        capture.route.submap=foot->submap;capture.route.map_id=1;capture.route.layer=1;capture.route.position=foot->position;capture.route.obstacles=foot->obstacles;
        distances=exit_distances(capture,{{world_of_cell(*foot->submap,{1,64,31}),1,174,{1,64,31},TargetKind::Transition}},{});});filter.join();
    expect(walk.path.success&&walk.path.step_count==1&&browse.route.direct,"on-foot tower entrance is directly reachable from the exact logged adjacent cell");
    expect(distances.size()==1&&distances[0]==1,"current-map filter retains the adjacent Tower of Owen entrance without a chocobo");
    const auto* room=baked::submap_by_map_id(174);const auto& grid=baked::grids(*room);
    const baked::Object* hole=nullptr;for(const auto& object:grid.objects)if(object.type==baked::ObjType::Entity&&object.entity_id()==10)hole=&object;
    expect(hole!=nullptr,"real Toad-hole examined entity exists");if(!hole)return;
    for(auto kind:{TargetKind::Infer,TargetKind::Transition}) {
        auto request=make(174,{1,18,11},{1,19,19},kind);
        NativeObstacle actor;actor.type=5;actor.id=10;actor.cell={1,19,19};actor.raw_name=baked::object_identifier(*hole);request->obstacles={actor};
        const auto before=plan(*request);
        expect(before.path.success&&before.path.step_count>0&&before.path.reason.empty(),"hole object and its overlapping exit route to the examinable approach before native Toad check");
        if(before.path.success) {
            const auto& phase=before.path.walk_phases.back();
            expect(phase.cells.back()!=Cell{1,19,19}&&phase.arrival_direction>=0,"hole approach stops beside and retains structured facing of the solid examined object");
            request->position=world_of_cell(*room,phase.cells.back());const auto arrived=plan(*request);
            expect(arrived.path.success&&arrived.path.step_count==0&&arrived.path.full_description.find(L"face")!=std::wstring::npos,"hole facing is spoken on arrival through the existing phase-order contract");
        }
        const auto after=plan(*request);expect(after.path.success&&after.path.reason.empty(),"fresh planning never caches a fictional Story gated result after a party-form change");
    }
}
void native_mount_entry() {
    for(const auto& row:baked::transports())if(row.type==11||row.type==12) {
        bool checked=false;
        for(int i=0;i<baked::submap_count()&&!checked;++i) {
            const auto* world=baked::submap_at(i);if(!world->world)continue;const auto& grid=baked::grids(*world);
            for(const auto& door:grid.objects)if(door.type==baked::ObjType::GotoMap&&door.a>0) {
                baked_route::Request movement;movement.submap=world;movement.vehicle_group=row.group;movement.vehicle_type=row.type;
                if(!baked_route::transition_enterable(movement,grid,door.tx,door.ty))continue;
                for(const auto& d:std::vector<Cell>{{0,0,-1},{0,1,0},{0,0,1},{0,-1,0}}) {
                    const Cell at{1,door.tx+d.x,door.ty+d.y};if(!baked_route::open(movement,grid,at))continue;
                    auto snapshot=make(world->first_map_id,at,{1,door.tx,door.ty},TargetKind::Transition);
                    snapshot->vehicle={true,row.id,row.type,{}};auto result=plan(*snapshot);
                    if(!result.path.success)continue;
                    expect(result.path.step_count==1&&result.path.walk_phases.size()==1&&result.path.walk_phases.front().action_before.empty(),"Boko/Magitek native map entry keeps the mount and invents no voluntary dismount");checked=true;break;
                }
                if(checked)break;
            }
        }
        expect(checked,"every native ground-entry mount has a real one-step world entrance fixture");
    }
}
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    for(int n=1;n<=6;++n) {
        const auto root="data/ff"+std::to_string(n)+"/";const auto data=bytes(root+"maps.bin");
        expect(baked::load(data.data(),data.size()),"matching real bake loads");expect(json::parse(bytes(root+"mapstate.json"),fixture::records),"matching initializer loads");
        fixture::game_id=static_cast<game::Id>(n);g_loaded=g_prepared=true;
        collision_sides(*baked::submap_by_map_id(1));native_mount_entry();if(n==3){owen();world_event_vehicle_advice();}
        std::printf("FF%d partial-collision/native mount cases checked\n",n);
    }
    expect(!fixture::wrong_owner,"all route work uses owned data without native worker access");
    std::printf("Chocobo/Owen: %d failures\n",failures);return failures?1:0;
}
