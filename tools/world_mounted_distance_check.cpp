// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// A legal longer mounted entrance approach must also admit its interior hop.
// Source-shaped current-carrier packets; no game/runtime responsiveness claim.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);int failures=0,witnesses=0;
    const auto read=[](const std::string& name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const std::array<std::vector<int>,6> profiles{{{3,4},{3,4},{9,14},{3,7},{2,5,6},{4}}};
    for(int number=1;number<=6;++number) {
        const auto base="data/ff"+std::to_string(number)+"/";const auto bytes=read(base+"maps.bin");
        if(!baked::load(bytes.data(),bytes.size())||!json::parse(read(base+"mapstate.json"),fixture::records))return 2;
        fixture::game_id=static_cast<game::Id>(number);g_loaded=g_prepared=true;
        for(int id:profiles[number-1]) {
            const auto* model=baked::transport(id);if(!model)return 2;bool found=false;int attempts=0;
            for(size_t index=0;index<baked::submap_count()&&!found&&attempts<16;++index) {
                const auto* world=baked::submap_at(index);if(!world||!world->world||world->first_map_id<1)continue;
                if(number==3&&id==14&&world->first_map_id!=3)continue;
                const auto& grid=baked::grids(*world);
                map_state::Flags flags;flags.scenario.fill(number==3?1:0);flags.global.fill(number==3?1:0);flags.treasure.fill(number==3?1:0);
                flags.current_map_id=world->first_map_id;map_state::Resolver state(flags,fixture::records);
                for(const auto& door:grid.objects) {
                    if(found||attempts>=16)break;
                    if((door.type!=baked::ObjType::GotoMap&&!baked::world_exit(door))||!state.active(*world,door)||state.consumed(*world,door))continue;
                    const auto* room=baked::submap_by_map_id(door.a);if(!room||room->world)continue;
                    const auto* arrival=point_in(*room,door.b,state);if(!arrival)continue;
                    for(int distance:{4,12,32})for(const auto side:std::array<Cell,4>{{{1,0,1},{1,1,0},{1,0,-1},{1,-1,0}}}) {
                        if(found||attempts>=16)break;
                        const Cell start{1,(door.tx+side.x*distance+grid.width)%grid.width,(door.ty+side.y*distance+grid.height)%grid.height};
                        const bool underwater=number==3&&id==14&&world->first_map_id==3;
                        if(!baked::vehicle_open(grid,model->group,start.x,start.y)||(!underwater&&baked::vehicle_open(grid,1,start.x,start.y)))continue;
                        bool deep=true;
                        for(const auto neighbor:std::array<Cell,4>{{{1,0,1},{1,1,0},{1,0,-1},{1,-1,0}}})
                            if(baked::vehicle_open(grid,1,(start.x+neighbor.x+grid.width)%grid.width,(start.y+neighbor.y+grid.height)%grid.height))deep=false;
                        if(!deep&&!underwater)continue;++attempts;
                        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=world->first_map_id;
                        snapshot.submap=world;snapshot.layer=1;snapshot.position=world_of_cell(*world,start);snapshot.vehicle_shortcuts=false;
                        snapshot.vehicle={true,model->id,model->type,baked::str(model->message)};snapshot.locale=L"en";
                        if(baked::world_exit(door)) {
                            NativeObstacle observed;observed.type=(door.d&255)==1?2:5;observed.id=door.entity_id();
                            observed.cell={-1,door.tx,door.ty};observed.raw_name=baked::object_identifier(door);
                            observed.trigger=(door.d&255)==1;
                            observed.transportation_ids=state.transportation_mask(*world,door);
                            observed.transportation_restricted=observed.transportation_ids!=~uint64_t(0);
                            snapshot.obstacles.push_back(std::move(observed));
                        }
                        snapshot.goal.map_id=snapshot.map_id;snapshot.goal.cell={1,door.tx,door.ty};snapshot.kind=TargetKind::Transition;
                        Planned local;std::thread local_worker([&]{local=plan_snapshot(snapshot);});local_worker.join();
                        if(!local.path.success)continue;
                        MapGoal goal;goal.map_id=door.a;goal.cell={arrival->layer,arrival->tx,arrival->ty};
                        CrossLeg remote;std::thread remote_worker([&]{SnapshotScope scope(snapshot);remote=find_across(nullptr,nullptr,goal,true);});remote_worker.join();
                        found=true;++witnesses;const bool pass=remote.found&&remote.next_map_id==door.a;
                        if(!pass)++failures;
                        std::printf("FF%d world%d carrier%d type%d deep%d,%d -> door%d,%d room%d: local%d/%dsteps remote%d next%d pass%d\n",
                            number,world->first_map_id,id,model->type,start.x,start.y,door.tx,door.ty,door.a,
                            local.path.success,local.path.step_count,remote.found,remote.next_map_id,pass);
                    }
                }
            }
            if(!found)std::printf("FF%d carrier%d: no eligible longer-approach witness in%d attempts; unqualified\n",number,id,attempts);
        }
    }
    if(witnesses<6)++failures;
    std::printf("Mounted longer first hops: %d witnesses, %d failures, owner%d\n",witnesses,failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
