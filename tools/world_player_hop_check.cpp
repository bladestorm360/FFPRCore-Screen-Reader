// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Every game reports a valid Player transportation record while walking.
// Prove its world-to-interior leg agrees with the former invalid-record fixture.
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
        const auto* player=baked::transport(1);if(!player||player->type!=1)return 2;
        bool found=false;int attempts=0;
        for(size_t index=0;index<baked::submap_count()&&!found&&attempts<512;++index) {
            const auto* world=baked::submap_at(index);if(!world||!world->world||world->first_map_id<1)continue;
            const auto& grid=baked::grids(*world);
            map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);
            flags.current_map_id=world->first_map_id;map_state::Resolver state(flags,fixture::records);
            for(const auto& door:grid.objects) {
                if(found||attempts>=512)break;
                if(door.type!=baked::ObjType::GotoMap||!state.active(*world,door))continue;
                const auto* room=baked::submap_by_map_id(door.a);if(!room||room->world)continue;
                const auto* arrival=point_in(*room,door.b,state);if(!arrival)continue;
                for(const auto side:std::array<Cell,4>{{{1,0,1},{1,1,0},{1,0,-1},{1,-1,0}}}) {
                    const Cell start{1,(door.tx+side.x+grid.width)%grid.width,(door.ty+side.y+grid.height)%grid.height};
                    baked_route::Request foot;foot.submap=world;
                    if(!baked_route::open(foot,grid,start)||baked_route::terminal_transition(foot,grid,start))continue;
                    ++attempts;MapGoal goal;goal.map_id=door.a;goal.cell={arrival->layer,arrival->tx,arrival->ty};
                    const auto plan=[&](bool native) {
                        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=world->first_map_id;
                        snapshot.submap=world;snapshot.layer=1;snapshot.position=world_of_cell(*world,start);snapshot.vehicle_shortcuts=false;
                        if(native)snapshot.vehicle={true,1,player->type,{}};
                        CrossLeg result;std::thread worker([&]{SnapshotScope scope(snapshot);result=find_across(nullptr,nullptr,goal,true);});worker.join();return result;
                    };
                    const auto reference=plan(false);if(!reference.found)continue;
                    const auto native=plan(true);found=true;++witnesses;
                    const bool pass=native.found&&native.next_map_id==reference.next_map_id&&native.steps_here==reference.steps_here&&
                        !native.phases.empty()&&std::none_of(native.phases.begin(),native.phases.end(),[](const auto& phase){return phase.transportation_id>0;});
                    if(!pass)++failures;
                    std::printf("FF%d native Player world%d -> room%d: reference%d native%d next%d steps%d pass%d\n",
                        number,world->first_map_id,door.a,reference.found,native.found,native.next_map_id,native.steps_here,pass);
                    break;
                }
            }
        }
        if(!found){++failures;std::printf("FAIL FF%d has no real world/interior witness within512 candidates\n",number);}
    }
    std::printf("All-six native Player first hops: %d witnesses, %d failures, owner%d\n",witnesses,failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
