// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// An eligible mounted local entrance must also be a usable first cross-map hop.
// Native game metadata supplies worlds, carriers, doors and actual PointIn goals.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);int failures=0,witnesses=0;
    const auto read=[](const std::string& name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    for(int number=1;number<=6;++number) {
        const auto base="data/ff"+std::to_string(number)+"/";const auto bytes=read(base+"maps.bin");
        if(!baked::load(bytes.data(),bytes.size())||!json::parse(read(base+"mapstate.json"),fixture::records))return 2;
        fixture::game_id=static_cast<game::Id>(number);g_loaded=g_prepared=true;
        int game_witnesses=0;
        for(const auto& model:baked::transports()) {
            if(model.type<=1||model.type==5||model.group<=0)continue;
            bool found=false;int attempts=0;
            for(size_t index=0;index<baked::submap_count()&&!found&&attempts<32;++index) {
                const auto* world=baked::submap_at(index);if(!world||!world->world||world->first_map_id<1)continue;
                const auto& grid=baked::grids(*world);
                map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);
                flags.current_map_id=world->first_map_id;map_state::Resolver state(flags,fixture::records);
                for(const auto& door:grid.objects) {
                    if(found||attempts>=32)break;
                    if(door.type!=baked::ObjType::GotoMap||!state.active(*world,door)||state.consumed(*world,door))continue;
                    const auto* room=baked::submap_by_map_id(door.a);if(!room||room->world)continue;
                    const auto* arrival=point_in(*room,door.b,state);if(!arrival)continue;
                    for(const auto side:std::array<Cell,4>{{{1,0,1},{1,1,0},{1,0,-1},{1,-1,0}}}) {
                        const Cell start{1,(door.tx+side.x+grid.width)%grid.width,(door.ty+side.y+grid.height)%grid.height};
                        if(!baked::vehicle_open(grid,model.group,start.x,start.y))continue;
                        ++attempts;
                        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=world->first_map_id;
                        snapshot.submap=world;snapshot.layer=1;snapshot.position=world_of_cell(*world,start);snapshot.vehicle_shortcuts=false;
                        snapshot.vehicle={true,model.id,model.type,baked::str(model.message)};snapshot.locale=L"en";
                        snapshot.goal.map_id=snapshot.map_id;snapshot.goal.cell={1,door.tx,door.ty};snapshot.kind=TargetKind::Transition;
                        Planned local;std::thread local_worker([&]{local=plan_snapshot(snapshot);});local_worker.join();
                        if(!local.path.success)continue;
                        MapGoal goal;goal.map_id=door.a;goal.cell={arrival->layer,arrival->tx,arrival->ty};
                        CrossLeg remote;std::thread remote_worker([&]{SnapshotScope scope(snapshot);remote=find_across(nullptr,nullptr,goal,true);});remote_worker.join();
                        found=true;++witnesses;++game_witnesses;
                        const bool pass=remote.found&&remote.next_map_id==door.a&&!remote.phases.empty();
                        if(!pass)++failures;
                        std::printf("FF%d world%d carrier%d type%d door%d at%d,%d: local%d remote%d next%d pass%d\n",
                            number,world->first_map_id,model.id,model.type,door.a,door.tx,door.ty,local.path.success,remote.found,remote.next_map_id,pass);
                        break;
                    }
                }
            }
            if(!found)std::printf("FF%d carrier%d type%d: no eligible local witness in%d attempts (not qualified)\n",number,model.id,model.type,attempts);
        }
        if(!game_witnesses){++failures;std::printf("FAIL FF%d has no mounted local witness\n",number);}
    }
    std::printf("All-six mounted first hops: %d witnesses, %d failures, owner%d\n",witnesses,failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
