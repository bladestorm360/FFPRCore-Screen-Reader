// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real underwater bake, actual planner/list distances, native-free worker.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int check_manual_aircraft_landings(){
    using namespace field;using namespace baked_nav;
    int failures=0,cases=0;
    for(int n=1;n<=6;++n){
        std::ifstream file("data/ff"+std::to_string(n)+"/maps.bin",std::ios::binary);
        std::string blob{std::istreambuf_iterator<char>(file),{}};if(!baked::load(blob.data(),blob.size()))return 1;
        fixture::game_id=static_cast<game::Id>(n);int game_cases=0;
        for(int index=0;index<baked::submap_count();++index){
            const auto* map=baked::submap_at(index);if(!map||!map->world)continue;
            const auto& grid=baked::grids(*map);
            for(const auto& row:baked::transports()){
                if(row.type!=3&&row.type!=7&&row.type!=8)continue;
                bool witnessed=false;
                for(const auto& door:grid.objects){
                    if(witnessed)break;
                    if(door.type!=baked::ObjType::GotoMap||door.a<=0)continue;
                    const Cell target{1,door.tx,door.ty};
                    baked_route::Request movement;movement.submap=map;movement.vehicle_group=row.group;movement.vehicle_type=row.type;
                    for(const auto side:std::array<Cell,4>{{{1,0,-1},{1,1,0},{1,0,1},{1,-1,0}}}){
                        const Cell start{1,(target.x+side.x+grid.width)%grid.width,(target.y+side.y+grid.height)%grid.height};
                        if(!baked_route::open(movement,grid,start))continue;
                        auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(map_state::Flags{},fixture::records));
                        snapshot->map_id=map->first_map_id;snapshot->submap=map;snapshot->layer=1;snapshot->locale=L"en";
                        snapshot->position=world_of_cell(*map,start);snapshot->vehicle={true,row.id,row.type,L""};
                        snapshot->goal.map_id=snapshot->map_id;snapshot->goal.cell=target;snapshot->kind=TargetKind::Transition;
                        for(const auto& auto_row:baked::transports())if(auto_row.type==5)snapshot->auto_groups.push_back(auto_row.group);
                        Planned plan;std::thread worker([&]{plan=plan_snapshot(*snapshot);});worker.join();
                        if(!plan.path.success)continue;
                        witnessed=true;++cases;++game_cases;
                        const bool landed=std::any_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& phase){
                            const auto action=phase.vehicle_before;return action==route_progress::VehicleAction::ConfirmLand||action==route_progress::VehicleAction::BackLand||action==route_progress::VehicleAction::WalkOff;
                        });
                        if(!landed||plan.path.walk_phases.back().cells.back()!=target){++failures;std::printf("FAIL FF%d map%d aircraft%d ordinary entrance must land, then reach exact door\n",n,snapshot->map_id,row.id);}
                        break;
                    }
                }
            }
        }
        if(!game_cases){++failures;std::printf("FAIL no actual aircraft/ordinary-door witness for FF%d\n",n);}
    }
    std::printf("All-six manual aircraft landing: %d world/model witnesses, %d failures\n",cases,failures);
    return failures;
}
int main(){
    using namespace field;using namespace baked_nav;
    int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    std::ifstream file("data/ff3/maps.bin",std::ios::binary);std::string blob{std::istreambuf_iterator<char>(file),{}};
    if(!baked::load(blob.data(),blob.size()))return 2;
    fixture::game_id=game::Id::FF3;
    const auto* world=baked::submap_by_map_id(3);if(!world)return 3;
    expect(world->world&&world->width==256&&world->height==256,"underwater world has its real full-size baked terrain");
    auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(1);flags->current_map_id=3;
    auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
    snapshot->map_id=3;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";
    snapshot->position=world_of_cell(*world,{1,113,118});snapshot->vehicle={true,14,6,L""};
    for(const auto& object:baked::grids(*world).objects)if(object.type==baked::ObjType::Event){
        NativeObstacle native;native.type=2;native.id=object.entity_id();native.cell={-1,object.tx,object.ty};
        native.raw_name=baked::object_identifier(object);native.trigger=true;snapshot->obstacles.push_back(std::move(native));
    }
    int entrances=0;
    for(const auto& object:baked::grids(*world).objects)if(object.type==baked::ObjType::Warp&&object.a>0){
        ++entrances;snapshot->goal={};snapshot->goal.map_id=3;snapshot->goal.cell={1,object.tx,object.ty};
        snapshot->goal.entity_id=object.entity_id();snapshot->goal.raw_name=baked::object_identifier(object);
        snapshot->kind=TargetKind::Transition;
        Planned plan;std::vector<int> distances;
        std::thread worker([&]{
            plan=plan_snapshot(*snapshot);
            auto filter=std::make_unique<ExitSnapshot>(false,snapshot->state);
            auto& copy=filter->route;copy.submap=world;copy.map_id=3;copy.layer=1;
            copy.position=snapshot->position;copy.vehicle=snapshot->vehicle;copy.obstacles=snapshot->obstacles;
            distances=exit_distances(*filter,{{world_of_cell(*world,snapshot->goal.cell),3,object.a,snapshot->goal.cell,TargetKind::Transition}},{});
        });worker.join();
        expect(baked::world_exit(object),"ordinary underwater entrances are named Map Exits, not Scene triggers");
        expect(plan.path.success&&!plan.path.walk_phases.empty()&&plan.path.walk_phases.back().cells.back()==snapshot->goal.cell,
            "mounted Nautilus reaches the exact underwater entrance");
        expect(!distances.empty()&&distances[0]>=0,"underwater entrance survives current-map eligibility preparation");
        auto movement=walk_request(*world,{1,113,118});const auto* row=baked::transport(14);
        movement.vehicle_group=row->group;movement.vehicle_type=row->type;movement.transportation_id=row->id;
        expect(baked_route::transition_enterable(movement,baked::grids(*world),object.tx,object.ty),"submarine mode may enter the reviewed underwater transfer");
        movement.vehicle_type=3;
        expect(!baked_route::transition_enterable(movement,baked::grids(*world),object.tx,object.ty),"underwater transfer flag cannot authorize aircraft entry");
        for(const auto& phase:plan.path.walk_phases)expect(phase.vehicle_before!=route_progress::VehicleAction::WalkOff&&phase.vehicle_before!=route_progress::VehicleAction::ConfirmLand,
            "underwater entry stays aboard until the native entry script disembarks");
        std::printf("Underwater entrance%d -> map%d at%d,%d: success%d, list%d\n",object.entity_id(),object.a,object.tx,object.ty,plan.path.success,distances.empty()?-1:distances[0]);
    }
    expect(entrances==4,"all4 native underwater entrance tiles are covered");
    {
        const auto* surface=baked::submap_by_map_id(4);
        auto manor=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
        manor->map_id=4;manor->submap=surface;manor->layer=1;manor->locale=L"en";
        manor->position=world_of_cell(*surface,{1,128,185});manor->vehicle={true,8,3,L""};
        manor->auto_groups={3};manor->goal.map_id=4;manor->goal.cell={1,128,184};manor->goal.entity_id=158;
        manor->kind=TargetKind::Transition;Planned plan;
        std::thread worker([&]{plan=plan_snapshot(*manor);});worker.join();
        expect(plan.path.success&&std::any_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& p){return p.vehicle_before==route_progress::VehicleAction::ConfirmLand;}),
            "Doga's Manor ordinary entrance requires Nautilus landing before the final walk");
        if(plan.path.success)expect(plan.path.walk_phases.back().cells.back()==manor->goal.cell,"Doga's Manor retains the exact door tile after landing");
    }
    expect(fixture::wrong_owner==0,"underwater route/list work never calls native APIs on a worker");
    failures+=check_manual_aircraft_landings();
    std::printf("Nautilus underwater: %d failures\n",failures);return failures?1:0;
}
