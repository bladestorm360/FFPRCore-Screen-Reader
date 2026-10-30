// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main() {
    using namespace field;using namespace baked_nav;
    std::ifstream f("data/ff3/maps.bin",std::ios::binary);std::string bytes{std::istreambuf_iterator<char>(f),{}};
    if(!baked::load(bytes.data(),bytes.size()))return 2;
    std::ifstream j("data/ff3/mapstate.json",std::ios::binary);std::string state_bytes{std::istreambuf_iterator<char>(j),{}};
    if(!json::parse(state_bytes,fixture::records))return 3;
    fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
    const auto* sm=baked::submap_by_map_id(2);if(!sm)return 4;
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=2;
    map_state::Resolver state(flags,fixture::records);
    const baked::Object* object=nullptr;
    for(const auto& o:baked::grids(*sm).objects) { const auto* rule=air_transitions::match(3,2,o);if(rule&&rule->script==356){object=&o;break;} }
    if(!object)return 5;const Cell target{1,object->tx,object->ty};
    Cell start{1,target.x,target.y-1};
    RouteSnapshot snapshot(state);snapshot.map_id=2;snapshot.submap=sm;snapshot.layer=1;snapshot.locale=L"en";
    snapshot.position=world_of_cell(*sm,start);snapshot.goal.map_id=2;snapshot.goal.cell=target;
    snapshot.goal.entity_id=object->entity_id();snapshot.goal.raw_name=baked::object_identifier(*object);
    snapshot.kind=TargetKind::AirshipTransition;snapshot.vehicle={true,15,3,{}};
    Planned result,ordinary;const auto t=std::chrono::steady_clock::now();std::thread worker([&]{
        auto prior=snapshot;prior.kind=TargetKind::Transition;ordinary=plan_snapshot(prior);result=plan_snapshot(snapshot);
    });worker.join();
    int final_id=0;bool land=false;for(const auto& p:result.path.walk_phases){final_id=p.transportation_id;land|=p.vehicle_before==route_progress::VehicleAction::WalkOff || (p.vehicle_before==route_progress::VehicleAction::ConfirmBoard&&p.transportation_id==16);}
    std::printf("map2 baked target (%d,%d) from (%d,%d): ordinary=%d typed=%d steps=%d final=%d landing=%d %.3fms\n",target.x,target.y,start.x,start.y,ordinary.path.success,result.path.success,result.path.step_count,final_id,land,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count());
    int failures=(!result.path.success||final_id!=15||land)?1:0;
    if(failures)std::printf("FAIL typed reason=%ls error=%ls\n",result.path.reason.c_str(),result.path.error.c_str());
    ExitSnapshot capture(false,state);capture.route.map_id=2;capture.route.submap=sm;capture.route.layer=1;capture.route.position=snapshot.position;capture.route.locale=L"en";capture.route.vehicle=snapshot.vehicle;
    auto selected=air_transition_target(capture,{}, {},{});
    if(!selected||!(selected->goal.cell==target)) {++failures;std::puts("FAIL worker must publish reachable exact return");}
    else std::printf("worker return target (%d,%d) distance=%d\n",selected->goal.cell.x,selected->goal.cell.y,selected->distance);
    capture.route.vehicle={true,6,3,{}};if(air_transition_target(capture,{}, {},{})){++failures;std::puts("FAIL wrong aircraft exposes flooded return");}
    for(int map:{1,4}) {
        const auto* world=baked::submap_by_map_id(map);if(!world){++failures;continue;}
        auto mapflags=flags;mapflags.current_map_id=map;map_state::Resolver local(mapflags,fixture::records);
        ExitSnapshot capture(false,local);capture.route.map_id=map;capture.route.submap=world;capture.route.layer=1;
        capture.route.locale=L"en";capture.route.position=world_of_cell(*world,map==1?Cell{1,39,13}:Cell{1,33,215});
        const auto began=std::chrono::steady_clock::now();
        for(int id: map==1?std::vector<int>{6,8,9,11,13}:std::vector<int>{6,8,9,10,13}) {
            const auto* transport=baked::transport(id);if(!transport){++failures;continue;}
            capture.route.vehicle={true,id,transport->type,{}};
            auto found=air_transition_target(capture,{}, {},{});
            std::printf("worker map%d vehicle%d target=%d distance=%d\n",map,id,!!found,found?found->distance:-1);
            if(!found){++failures;continue;}
            if(map==4) {
                RouteSnapshot plan(local);plan.map_id=map;plan.submap=world;plan.layer=1;plan.locale=L"en";
                plan.position=capture.route.position;plan.vehicle=capture.route.vehicle;plan.goal=found->goal;plan.kind=TargetKind::AirshipTransition;
                auto route=plan_snapshot(plan);const int expected=id==13?6:id;
                if(id==13&&!route.path.success) { SnapshotScope scope(plan);auto r=walk_request(*world,cell_of(*world,plan.position,1));world_mode(r);
                    r.goals={found->goal.cell};r.goals_open=false;r.cancelled=&planning_cancelled;
                    auto raw=baked_route::enterprise_route(r,true,false,true);
                    const auto* rule=air_transitions::match(3,map,343,L"浮遊大陸へ移動：エンタープライズ");std::vector<Phase> phases;
                    bool board=boarding_route(*world,baked::grids(*world),r,r.goals,phases,rule);
                    std::printf("diagnostic direct enterprise=%d phases=%zu board=%d group=%d start=(%d,%d) goal=(%d,%d)\n",raw.steps,phases.size(),board,r.vehicle_group,r.start.x,r.start.y,r.goals[0].x,r.goals[0].y);
                }
                const int final=route.path.walk_phases.empty()?0:route.path.walk_phases.back().transportation_id;
                if(!route.path.success||final!=expected){++failures;std::printf("FAIL map%d id%d selected planning final=%d success=%d reason=%ls error=%ls\n",map,id,final,route.path.success,route.path.reason.c_str(),route.path.error.c_str());}
            }
            auto pinned=air_transition_target(capture,found->goal,{},{});
            if(!pinned||!(pinned->goal.cell==found->goal.cell)||pinned->goal.raw_name!=found->goal.raw_name)++failures;
        }
        std::printf("worker map%d five vehicles plus pin: %.3fms combined offline\n",map,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
    }
    {
        const auto* world=baked::submap_by_map_id(1);auto mapflags=flags;mapflags.current_map_id=1;
        map_state::Resolver local(mapflags,fixture::records);ExitSnapshot capture(true,local);
        capture.route.map_id=1;capture.route.submap=world;capture.route.layer=1;capture.route.locale=L"en";
        capture.route.position=world_of_cell(*world,{1,39,12});VehicleOnMap vehicle;
        vehicle.transportation_id=13;vehicle.type=2;vehicle.pos=world_of_cell(*world,{1,39,13});capture.route.vehicles={vehicle};
        auto found=air_transition_target(capture,{}, {},{});std::printf("parked13 target=%d distance=%d\n",!!found,found?found->distance:-1);
        if(!found)++failures;
        else {
            RouteSnapshot plan(local);plan.map_id=1;plan.submap=world;plan.layer=1;plan.locale=L"en";plan.position=capture.route.position;
            plan.vehicles=capture.route.vehicles;plan.vehicle_shortcuts=true;plan.goal=found->goal;plan.kind=TargetKind::AirshipTransition;
            auto route=plan_snapshot(plan);bool boarded=false;for(const auto& p:route.path.walk_phases)boarded|=p.vehicle_before==route_progress::VehicleAction::WalkOn;
            const int final=route.path.walk_phases.empty()?0:route.path.walk_phases.back().transportation_id;
            std::printf("parked13 plan=%d board=%d final=%d steps=%d reason=%ls error=%ls\n",route.path.success,boarded,final,route.path.step_count,route.path.reason.c_str(),route.path.error.c_str());
            if(!route.path.success||!boarded||final!=6)++failures;
        }
        capture.with_vehicles=false;if(air_transition_target(capture,{}, {},{})){++failures;std::puts("FAIL parked vehicle requires with-vehicles policy");}
        int checks=0;auto cancelled=air_transition_target(capture,{}, {},[&]{return ++checks>1;});
        if(cancelled){++failures;std::puts("FAIL obsolete target search must not publish");}
    }
    {
        const auto* world=baked::submap_by_map_id(1);auto mapflags=flags;mapflags.current_map_id=1;
        map_state::Resolver local(mapflags,fixture::records);ExitSnapshot capture(false,local);
        capture.route.map_id=1;capture.route.submap=world;capture.route.layer=1;capture.route.locale=L"en";
        capture.route.position=world_of_cell(*world,{1,39,13});capture.route.vehicle={true,6,3,{}};
        auto nearest=air_transition_target(capture,{}, {},{});
        if(!nearest||nearest->destination_map!=2){++failures;std::puts("FAIL known scenario false targets flooded surface");}
        auto find_scenario=[&](const map_state::Flags& copied) {
            map_state::Resolver state_copy(copied,fixture::records);ExitSnapshot another(false,state_copy);
            another.route.map_id=1;another.route.submap=world;another.route.layer=1;another.route.locale=L"en";
            another.route.position=capture.route.position;another.route.vehicle=capture.route.vehicle;
            return air_transition_target(another,{}, {},{});
        };
        mapflags.scenario[66]=1;auto normal=find_scenario(mapflags);
        if(!normal||normal->destination_map!=4){++failures;std::puts("FAIL known scenario true targets normal surface");}
        mapflags.scenario[66].reset();auto unknown=find_scenario(mapflags);
        if(!unknown||unknown->destination_map!=0){++failures;std::puts("FAIL unknown scenario must not invent a surface map");}
        int tested=0;for(const auto& o:baked::grids(*world).objects) {
            const auto* rule=air_transitions::match(3,1,o);if(!rule||rule->script!=341||!nearest||
                (o.tx==nearest->goal.cell.x&&o.ty==nearest->goal.cell.y))continue;
            MapGoal goal;goal.map_id=1;goal.cell={1,o.tx,o.ty};goal.entity_id=o.entity_id();goal.raw_name=baked::object_identifier(o);
            auto pinned=air_transition_target(capture,goal,{},{});++tested;
            if(pinned&&pinned->goal.cell==goal.cell)break;
            if(tested>=8){++failures;std::puts("FAIL eligible continued exit must be pinned");break;}
        }
        if(!tested){++failures;std::puts("FAIL no distinct candidate to test pin");}
        fixture::game_id=game::Id::FF2;
        if(air_transition_target(capture,{}, {},{})){++failures;std::puts("FAIL other games cannot expose FFIII trigger");}
        fixture::game_id=game::Id::FF3;
    }
    int count[7]={};const int maps[]={1,1,1,2,4,4,4},scripts[]={341,345,349,356,343,347,351};
    for(int map:{1,2,4})if(const auto* world=baked::submap_by_map_id(map))for(const auto& o:baked::grids(*world).objects)
        if(const auto* rule=air_transitions::match(3,map,o))for(int i=0;i<7;++i)if(maps[i]==map&&scripts[i]==rule->script)++count[i];
    for(int i=0;i<7;++i){std::printf("map%d script%d count=%d\n",maps[i],scripts[i],count[i]);if(count[i]!=(i<3?508:i==3?1:5))++failures;}
    std::printf("continent transition: %d failures\n",failures);return failures?1:0;
}
