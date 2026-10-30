// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Owned offline snapshots and real bakes. Never loads or controls a game.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
namespace {
using namespace field;using namespace baked_nav;
int failures=0;
void expect(bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}}
std::string bytes(const std::string& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
bool crosses_town(const Planned& plan) {
    for(const auto& phase:plan.path.walk_phases)if(!phase.transportation_id)
        for(size_t i=1;i<phase.cells.size();++i)if(phase.cells[i].x==61&&phase.cells[i].y==83)return true;
    return false;
}
void all_doors() {
    int doors=0,warps=0,flight=0,selected=0,projected=0,conditional=0;
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);
    map_state::Resolver state(flags,fixture::records);
    for(int index=0;index<baked::submap_count();++index) {
        const auto* sm=baked::submap_at(index);const auto& grid=baked::grids(*sm);
        baked_route::Request movement;movement.submap=sm;movement.ignore_hazards=true;
        auto mask=state.transitions(*sm);expect(mask==state.transitions(*sm),"state mask is cached and shared within its owned resolver");
        auto live=movement;
        // Production combines the shared unconditional mask with owned per-leg
        // selector rules. A foot request must not inherit aircraft-only exits.
        map_state::add_obstacles(state,*sm,live,false,false,false);
        for(const auto& object:grid.objects)if(baked::transition_bits(object)) {
            const Cell target{object.layer,object.tx,object.ty};
            if(!baked::in_bounds(grid,target.x,target.y))continue;
            (object.type==baked::ObjType::GotoMap?doors:warps)++;
            expect(baked_route::terminal_transition(movement,grid,target),"real door/step warp has a prepared terminal mask on its own layer");
            if(state.active(*sm,object,true)&&!state.consumed(*sm,object)) {
                const auto current=state.effective(*sm,object);
                if(baked::in_bounds(grid,current.tx,current.ty)) {
                    const Cell cell{current.layer,current.tx,current.ty};
                    const auto ids=state.transportation_mask(*sm,object);
                    if(ids&baked_route::transportation_bit(baked_route::transportation_at(live,grid,cell)))
                        expect(baked_route::terminal_transition(live,grid,cell),"projected transition applies to the actual current walking mode");
                    if(ids!=~uint64_t(0))for(const auto& row:baked::transports()) {
                        auto modal=live;modal.transportation_id=row.id;modal.vehicle_group=row.id==1?0:row.group;modal.vehicle_type=row.type;
                        if(!(ids&baked_route::transportation_bit(row.id)))continue;
                        const bool terminates=(baked::transition_bits(current)&2)||!sm->world||modal.vehicle_group<=0||baked_route::native_ground_entry(row.type);
                        if(terminates)expect(baked_route::terminal_transition(modal,grid,cell),"conditional transition retains its terminal rule for each matching native vehicle");
                        ++conditional;
                    }
                    ++projected;
                }
            }
            for(const auto& d:std::vector<Cell>{{0,0,-1},{0,1,0},{0,0,1},{0,-1,0}}) {
                Cell from{target.layer,target.x+d.x,target.y+d.y};
                if(sm->world){from.x=(from.x+grid.width)%grid.width;from.y=(from.y+grid.height)%grid.height;}
                if(!baked::in_bounds(grid,from.x,from.y))continue;
                movement.goals.clear();movement.goals_open=true;
                expect(!baked_route::edge_allowed(movement,grid,from,target),"ignore-events fallback cannot enter an unselected transition");
                movement.goals={target};movement.goals_open=false;
                expect(!baked_route::edge_allowed(movement,grid,from,target),"boarding-side goal cannot substitute an adjacent entrance");
                movement.goals_open=true;
                if(sm->world&&object.type==baked::ObjType::GotoMap&&baked_route::transition_enterable(movement,grid,target.x,target.y)) {
                    expect(baked_route::edge_allowed(movement,grid,from,target),"selected native town remains a legal exact terminal goal");++selected;
                }
            }
            if(sm->world&&object.type==baked::ObjType::GotoMap)for(const auto& row:baked::transports())if(row.type==3) {
                auto ride=movement;ride.goals.clear();ride.vehicle_group=row.group;ride.vehicle_type=row.type;
                // Co-located script warps keep their separate activation rules.
                const auto& layer=grid.transitions[0];const int bits=layer[static_cast<size_t>(target.y)*grid.width+target.x];
                if(bits==1){expect(!baked_route::terminal_transition(ride,grid,target),"mounted flight may cross a native town without entering it");++flight;}
            }
        }
    }
    expect(doors>0&&projected>0,"each game audits real native doors and state-projected masks");
    std::printf("FF%d: %d native doors, %d step warps, %d projected, %d conditional mode witnesses, %d explicit town edges, %d flight crossings checked\n",baked::game_number(),doors,warps,projected,conditional,selected,flight);
}
void logged_enterprise() {
    const auto* world=baked::submap_by_map_id(1);const auto& grid=baked::grids(*world);
    for(const auto& object:grid.objects)if(object.contains(61,83))
        std::printf("Logged north cell: type%d destination%d point%d name%ls\n",int(object.type),object.a,object.b,baked::object_identifier(object).c_str());
    const baked::TransportRow* ship=nullptr;for(const auto& row:baked::transports())if(row.type==2){ship=&row;break;}
    expect(ship!=nullptr,"FFIII has native Ship transport");if(!ship)return;
    map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=1;
    auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
    snapshot->submap=world;snapshot->map_id=1;snapshot->layer=1;snapshot->locale=L"en";snapshot->vehicle_shortcuts=true;
    snapshot->position=world_of_cell(*world,{1,61,84});snapshot->goal.map_id=1;snapshot->goal.cell={1,87,65};snapshot->kind=TargetKind::Transition;
    VehicleOnMap model;model.transportation_id=ship->id;model.type=ship->type;model.pos=world_of_cell(*world,{1,61,79});
    snapshot->vehicles={model};snapshot->vehicle_names[ship->id]=L"The Enterprise";
    Planned plan;BrowsePlanned browse;std::thread worker([&]{plan=plan_snapshot(*snapshot);browse=plan_browse_snapshot(*snapshot);});worker.join();
    expect(plan.path.success,"logged Caanan goal still routes by the Enterprise");
    bool crossed=crosses_town(plan);
    expect(!crossed,"Enterprise walking leg avoids the town one tile north of the logged origin");
    std::printf("Logged Enterprise: %d steps, %zu phases, unsafe town crossing%d; %ls\n",plan.path.step_count,plan.path.walk_phases.size(),crossed,plan.path.full_description.c_str());
    expect(!browse.route.direct&&browse.route.reason.find(L"Enterprise")!=std::wstring::npos,"browse explains the corrected Enterprise boarding route");
    if(plan.active&&!plan.active->phases.empty()) {
        auto stale=std::make_shared<ActiveRoute>(*plan.active);
        stale->phases.front().cells={{1,61,84},{1,61,83},{1,61,82},{1,61,81},{1,61,80}};
        snapshot->previous_active=stale;
        Planned repaired;std::thread repeat([&]{repaired=plan_snapshot(*snapshot);});repeat.join();
        expect(repaired.path.success&&!crosses_town(repaired),"retained unsafe walking leg is rejected and rerouted around town");
    }
    for(int x:{60,62}) {
        snapshot->previous_active.reset();snapshot->position=world_of_cell(*world,{1,x,84});
        Planned side;std::thread repeat([&]{side=plan_snapshot(*snapshot);});repeat.join();
        expect(side.path.success&&!crosses_town(side),"both one-tile sideways origins retain safe boarding routes");
    }
    snapshot->position=world_of_cell(*world,{1,61,84});snapshot->goal.cell={1,61,79};snapshot->goal.transportation_id=ship->id;snapshot->kind=TargetKind::Spot;
    Planned boat;std::thread to_boat([&]{boat=plan_snapshot(*snapshot);});to_boat.join();
    expect(boat.path.success&&!crosses_town(boat),"selecting the Enterprise itself also avoids Tokkul's entrance");
    snapshot->goal.transportation_id=0;snapshot->goal.cell={1,61,83};snapshot->kind=TargetKind::Transition;
    Planned town;std::thread to_town([&]{town=plan_snapshot(*snapshot);});to_town.join();
    expect(town.path.success&&town.path.step_count==1,"selecting Tokkul still enters the town one tile north");
    snapshot->goal.cell={1,61,79};snapshot->goal.transportation_id=ship->id;snapshot->kind=TargetKind::Spot;
    snapshot->position=world_of_cell(*world,{1,61,83});
    Planned arrival;std::thread leave([&]{arrival=plan_snapshot(*snapshot);});leave.join();
    expect(arrival.path.success,"player already on a native arrival tile can leave it and reach the boat");
}
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    for(int n=1;n<=6;++n) {
        const auto root="data/ff"+std::to_string(n)+"/";const auto data=bytes(root+"maps.bin");
        expect(baked::load(data.data(),data.size()),"matching real bake loads");
        expect(json::parse(bytes(root+"mapstate.json"),fixture::records),"real initializer metadata loads");
        fixture::game_id=static_cast<game::Id>(n);g_loaded=g_prepared=true;
        all_doors();if(n==3)logged_enterprise();
    }
    expect(!fixture::wrong_owner,"no native calls from route worker");
    std::printf("Transition routes: %d failures\n",failures);return failures?1:0;
}
