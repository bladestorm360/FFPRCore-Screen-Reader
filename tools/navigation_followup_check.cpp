// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real bakes and production worker planning. No game launch or native execution.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
namespace {
using namespace field;using namespace baked_nav;
int failures=0;
void expect(bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}}
std::string bytes(const std::string& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
std::unique_ptr<RouteSnapshot> room(int map,Cell start) {
    auto flags=std::make_unique<map_state::Flags>();
    flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
    flags->scenario[50]=flags->scenario[51]=flags->scenario[52]=1;
    flags->current_map_id=map;flags->current_area_id=9;
    auto out=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
    out->map_id=map;out->submap=baked::submap_by_map_id(map);out->layer=start.layer;out->locale=L"en";
    out->position=world_of_cell(*out->submap,start);
    for(const auto& original:baked::grids(*out->submap).objects) {
        if(!out->state.active(*out->submap,original))continue;
        const auto o=out->state.effective(*out->submap,original);
        const bool npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;
        const bool chest=o.type==baked::ObjType::TreasureBox;
        const bool gate=(o.type==baked::ObjType::Entity||o.type==baked::ObjType::AnimEntity)&&(o.d&1);
        const bool trigger=(o.type==baked::ObjType::Event||o.type==baked::ObjType::RandomEvent)&&(o.a||o.b);
        if(!npc&&!chest&&!gate&&!trigger)continue;
        NativeObstacle actor;actor.cell={o.layer,o.tx,o.ty};actor.id=o.entity_id();actor.type=static_cast<int>(o.type);
        actor.npc=npc;actor.chest=chest;actor.trigger=trigger;actor.raw_name=baked::object_identifier(o);
        out->obstacles.push_back(std::move(actor));
    }
    return out;
}
bool check_ship(const baked::Submap& world) {
    const auto& grid=baked::grids(world);
    const baked::TransportRow* ship=nullptr;
    for(const auto& row:baked::transports())if(row.type==2){ship=&row;break;}
    if(!ship)return false;
    auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(map_state::Flags{},fixture::records));
    snapshot->submap=&world;snapshot->map_id=world.first_map_id;snapshot->layer=1;snapshot->locale=L"en";
    auto foot=walk_request(world,{1,0,0});
    Cell boat,shore;bool found=false;
    for(int y=0;y<grid.height&&!found;++y)for(int x=0;x<grid.width&&!found;++x) {
        const Cell at{1,x,y};
        if(!baked::vehicle_open(grid,ship->group,x,y)||!baked::group_crosses(ship->group,baked::attribute(grid,x,y))||baked_route::open(foot,grid,at))continue;
        for(const auto side:std::vector<Cell>{{1,0,-1},{1,1,0},{1,0,1},{1,-1,0}}) {
            Cell adjacent{1,(x+side.x+grid.width)%grid.width,(y+side.y+grid.height)%grid.height};
            if(baked_route::open(foot,grid,adjacent)){boat=at;shore=adjacent;found=true;break;}
        }
    }
    if(!found)return false; // intro/alternate worlds can deliberately disable this transport
    snapshot->position=world_of_cell(world,shore);
    VehicleOnMap parked;parked.transportation_id=ship->id;parked.type=ship->type;parked.pos=world_of_cell(world,boat);
    snapshot->vehicles={parked};snapshot->vehicle_names[ship->id]=L"Ship";
    // Native ships can be NPCs, with all-layer collision and an offset sprite.
    // The model identity must bind the same physical boat used by transfer routes.
    NativeObstacle actor;actor.npc=true;actor.id=0;actor.raw_name=L"Ship";actor.cell={-1,boat.x+1,boat.y};snapshot->obstacles={actor};
    snapshot->goal.map_id=world.first_map_id;snapshot->goal.cell={1,boat.x+1,boat.y};snapshot->goal.entity_id=0;
    snapshot->goal.raw_name=L"Ship";snapshot->goal.transportation_id=ship->id;snapshot->kind=TargetKind::Spot;
    Planned plan;BrowsePlanned browse;
    std::thread worker([&]{plan=plan_snapshot(*snapshot);browse=plan_browse_snapshot(*snapshot);});worker.join();
    expect(plan.goal.cell==boat&&plan.path.success,"selected native NPC Ship binds to model tile/layer and uses the normal legal boarding route");
    expect(plan.path.full_description.find(L"board the Ship")!=std::wstring::npos&&browse.route.reason.find(L"board the Ship")!=std::wstring::npos,
        "directions and browse explain actual Ship boarding instead of Not directly reachable");
    snapshot->vehicles.clear();std::thread missing([&]{plan=plan_snapshot(*snapshot);});missing.join();
    expect(!plan.path.success&&plan.path.reason==L"Destination is no longer available","a missing vehicle never becomes an arbitrary walkable coordinate goal");
    std::printf("FF%d: water-only Ship target (%d,%d), native NPC/model identity and missing-model handback verified\n",baked::game_number(),boat.x,boat.y);
    return true;
}
void check_duster_boarding_overlap() {
    const auto* world=baked::submap_by_map_id(4);
    auto flags=std::make_unique<map_state::Flags>();
    flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
    flags->scenario[73]=flags->scenario[905]=1;
    auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
    snapshot->map_id=4;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";
    snapshot->position=world_of_cell(*world,{1,114,111});snapshot->vehicle={true,1,1,L""};
    snapshot->vehicle_shortcuts=true;snapshot->kind=TargetKind::Transition;
    snapshot->goal.map_id=4;snapshot->goal.cell={1,46,56};
    VehicleOnMap nautilus;nautilus.transportation_id=9;nautilus.type=3;
    nautilus.pos=world_of_cell(*world,{1,114,113});snapshot->vehicles={nautilus};
    snapshot->vehicle_names[9]=L"The Nautilus";
    Planned first,repeated,boarded;
    std::thread initial([&]{first=plan_snapshot(*snapshot);});initial.join();
    expect(first.path.success&&first.path.walk_phases.size()==3,"actual Duster coast to Ancient Ruins has a complete walking/boarding/flight/landing route");
    if(!first.path.success)return;
    const Cell start{1,114,111};
    const bool overlap=std::any_of(first.path.walk_phases.begin(),first.path.walk_phases.end(),[&](const auto& phase){
        return phase.transportation_id==9&&phase.movement_transportation_id==9&&phase.action_cell&&
               std::find(phase.cells.begin()+1,phase.cells.end(),start)!=phase.cells.end();});
    expect(overlap,"real Nautilus flight crosses the initial walkable tile after boarding to the south");
    snapshot->previous_active=first.active;
    std::thread again([&]{repeated=plan_snapshot(*snapshot);});again.join();
    auto boards=[](const Planned& plan){return std::any_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& phase){
        return phase.transportation_id==9&&phase.movement_transportation_id==9&&phase.action_cell&&!phase.action_before.empty();});};
    expect(repeated.path.success&&boards(repeated)&&repeated.path.step_count==first.path.step_count,
        "repeating directions while on foot cannot jump into the future flight through the same tile or drop Nautilus boarding");
    const int repeated_initial_steps=repeated.path.step_count;
    for(const auto& cell:first.path.walk_phases.front().cells){
        snapshot->position=world_of_cell(*world,cell);snapshot->previous_active=first.active;
        std::thread walk([&]{repeated=plan_snapshot(*snapshot);});walk.join();
        expect(repeated.path.success&&boards(repeated),"each foot approach step preserves the pending boarding even when flight overlaps the approach");
    }
    snapshot->vehicle={true,9,3,L""};snapshot->position=nautilus.pos;snapshot->previous_active=first.active;
    std::thread fly([&]{boarded=plan_snapshot(*snapshot);});fly.join();
    expect(boarded.path.success&&!boards(boarded)&&std::any_of(boarded.path.walk_phases.begin(),boarded.path.walk_phases.end(),[](const auto& phase){
        return phase.vehicle_before==route_progress::VehicleAction::ConfirmLand;}),"confirmed Nautilus boarding switches to flight and still lands before the Ruins entrance");
    std::printf("Duster/Ancient Ruins overlap: first%d/repeated%d/boarded%d steps; native riding state determines valid phase\n",
        first.path.step_count,repeated_initial_steps,boarded.path.step_count);
}
void check_transport_suffix_modes() {
    const auto found=std::find_if(baked::transports().begin(),baked::transports().end(),[](const auto& row){return row.type==3||row.type==7||row.type==8;});
    if(found==baked::transports().end())return;
    auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(map_state::Flags{},fixture::records));
    snapshot->vehicle={true,1,1,L""};
    ActiveRoute active;Phase walk,fly,land;
    walk.cells={{1,10,10},{1,10,11}};walk.movement_transportation_id=0;
    fly.cells={{1,10,11},{1,10,10},{1,10,9}};fly.transportation_id=found->id;fly.movement_transportation_id=found->id;
    fly.action_cell=fly.cells.front();fly.action_before=L"board";fly.vehicle_before=route_progress::VehicleAction::ConfirmBoard;
    land.cells={{1,10,9},{1,10,8}};land.transportation_id=found->id;land.movement_transportation_id=0;
    land.action_cell=land.cells.front();land.action_before=L"land";land.vehicle_before=route_progress::VehicleAction::ConfirmLand;
    active.phases={walk,fly,land};std::vector<Phase> rest;
    std::thread work([&]{SnapshotScope scope(*snapshot);
        expect(!remaining_from(active,{1,10,10},rest),"all-six on-foot overlap rejects a suffix in a future aircraft mode");
        expect(remaining_from(active,{1,10,11},rest)&&rest.front().vehicle_before==route_progress::VehicleAction::ConfirmBoard,
            "all-six exact boarding boundary retains its pending native action");
        active.phases[1].vehicle_before=route_progress::VehicleAction::None;
        expect(remaining_from(active,{1,10,11},rest)&&!rest.front().action_before.empty(),"manual deck boarding retains its action without an automated input enum");
        snapshot->vehicle={true,found->id,found->type,L""};
        expect(remaining_from(active,{1,10,10},rest)&&rest.front().movement_transportation_id==found->id&&rest.front().action_before.empty(),
            "all-six actual flight can retain its overlapping airborne suffix");
        expect(remaining_from(active,{1,10,9},rest)&&rest.front().vehicle_before==route_progress::VehicleAction::ConfirmLand,
            "all-six exact landing boundary retains its action before entering the walking phase");
    });work.join();
}
void check_object_vehicle_shortcuts() {
    const auto* world=baked::submap_by_map_id(1);const auto& grid=baked::grids(*world);
    const baked::Object* entrance=nullptr;
    for(const auto& object:grid.objects)if(object.type==baked::ObjType::Entity&&object.entity_id()==48)entrance=&object;
    expect(entrance!=nullptr,"logged cave entrance has a real baked object identity");if(!entrance)return;
    map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=1;
    auto make=[&] {auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
        snapshot->map_id=1;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";
        snapshot->auto_groups={2,5}; // actual FFII Canoe/Snowcraft, already owned at this story stage
        snapshot->position=world_of_cell(*world,{1,71,76});snapshot->goal.map_id=1;
        snapshot->goal.cell={1,entrance->tx,entrance->ty};snapshot->goal.entity_id=48;snapshot->goal.raw_name=baked::object_identifier(*entrance);
        snapshot->kind=TargetKind::Infer;return snapshot;};
    VehicleOnMap airship;airship.transportation_id=7;airship.type=3;airship.pos=world_of_cell(*world,{1,71,81});
    auto ground=make();ground->vehicles={airship};
    expect(ground->state.active(*world,*entrance),"logged post-Palamecia progress enables the actual cave object");
    Planned walking,assisted,cached,missing,mounted,nearby_result;BrowsePlanned browse;
    std::thread baseline([&]{walking=plan_snapshot(*ground);});baseline.join();
    expect(walking.path.success&&walking.path.step_count>200&&walking.path.walk_phases.size()==1,"walking-only baseline reproduces the long legal land approach");
    auto use_vehicle=make();use_vehicle->vehicles={airship};use_vehicle->vehicle_shortcuts=true;
    use_vehicle->previous_active=walking.active;
    const auto began=std::chrono::steady_clock::now();
    std::thread compare([&]{assisted=plan_snapshot(*use_vehicle);browse=plan_browse_snapshot(*use_vehicle);});compare.join();
    const double elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
    expect(assisted.path.success&&assisted.path.step_count<walking.path.step_count&&assisted.goal.entity_id==48&&assisted.goal.cell==ground->goal.cell,"vehicle assistance beats an existing walking route without changing the selected object");
    expect(std::any_of(assisted.path.walk_phases.begin(),assisted.path.walk_phases.end(),[](const auto& phase){return phase.transportation_id==7&&phase.vehicle_before==route_progress::VehicleAction::ConfirmBoard;})&&
           std::any_of(assisted.path.walk_phases.begin(),assisted.path.walk_phases.end(),[](const auto& phase){return phase.vehicle_before==route_progress::VehicleAction::ConfirmLand;})&&
           assisted.path.walk_phases.back().arrival_direction>=0,"object shortcut includes legal Airship boarding, landing and final interaction facing");
    auto absent=make();absent->vehicle_shortcuts=true;
    Planned old_walking;std::thread previous([&]{old_walking=plan_snapshot(*absent);});previous.join();
    auto refreshed=make();refreshed->vehicle_shortcuts=true;refreshed->vehicles={airship};refreshed->previous_active=old_walking.active;
    std::thread refresh([&]{cached=plan_snapshot(*refreshed);});refresh.join();
    expect(cached.path.success&&cached.path.step_count<old_walking.path.step_count,"a kept walking plan reevaluates a newly nearby parked Airship");
    absent->previous_active=assisted.active;
    std::thread lost([&]{missing=plan_snapshot(*absent);});lost.join();
    expect(missing.path.success&&missing.path.step_count==walking.path.step_count&&missing.path.walk_phases.size()==1,"missing shortcut vehicle retires the cached flight and restores the legal walking route");
    auto airborne=make();airborne->vehicle={true,7,3,L""};airborne->position=airship.pos;airborne->vehicle_shortcuts=true;
    std::thread landing([&]{mounted=plan_snapshot(*airborne);});landing.join();
    expect(mounted.path.success&&std::any_of(mounted.path.walk_phases.begin(),mounted.path.walk_phases.end(),[](const auto& phase){return phase.vehicle_before==route_progress::VehicleAction::ConfirmLand;}),"already-airborne object route lands before its ground interaction approach");
    auto nearby=make();nearby->vehicles={airship};nearby->vehicle_shortcuts=true;nearby->goal.entity_id=-1;nearby->goal.raw_name.clear();nearby->goal.cell={1,70,76};nearby->kind=TargetKind::Transition;
    std::thread close([&]{nearby_result=plan_snapshot(*nearby);});close.join();
    expect(nearby_result.path.success&&nearby_result.path.step_count==1&&nearby_result.path.walk_phases.size()==1,"one-step town entrance keeps walking when boarding the Airship is longer");
    for(int mode=0;mode<=2;++mode){fixture::filter_mode=mode;expect(capture_vehicle_shortcuts_policy()==(mode!=0),"captured filter permits shortcuts in vehicle/map modes and preserves walking-only preference");}
    fixture::filter_mode=-1;fixture::legacy_filter=true;expect(!capture_vehicle_shortcuts_policy(),"legacy enabled floor filter keeps walking-only preference");fixture::legacy_filter=false;
    expect(capture_vehicle_shortcuts_policy(),"legacy unfiltered map mode permits vehicle comparisons");
    std::printf("Logged cave: land %d steps, Airship-assisted %d, mounted %d; %.3f ms full+browse offline compute\n",walking.path.step_count,assisted.path.step_count,mounted.path.step_count,elapsed);
}
void check_saronia_short_walks() {
    const auto* world=baked::submap_by_map_id(4);
    map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=4;
    auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
    snapshot->map_id=4;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";
    snapshot->position=world_of_cell(*world,{1,83,62});snapshot->vehicle_shortcuts=true;snapshot->auto_groups={3};
    const auto* nautilus=baked::transport(9);
    expect(nautilus!=nullptr,"FFIII has its actual later aircraft model");if(!nautilus)return;
    VehicleOnMap model;model.transportation_id=nautilus->id;model.type=nautilus->type;model.pos=world_of_cell(*world,{1,83,63});
    snapshot->vehicles={model};snapshot->vehicle_names[model.transportation_id]=L"The Nautilus";
    for(const auto target:std::vector<Cell>{{1,91,65},{1,74,64}}) {
        snapshot->goal.map_id=4;snapshot->goal.cell=target;snapshot->kind=TargetKind::Transition;
        Planned walking,assisted,boarded;BrowsePlanned browse;
        snapshot->vehicle_shortcuts=false;
        std::thread base([&]{walking=plan_snapshot(*snapshot);});base.join();
        expect(walking.path.success,"logged nearby northern Saronia entrance has a legal walking route");
        snapshot->vehicle_shortcuts=true;
        std::thread short_route([&]{assisted=plan_snapshot(*snapshot);browse=plan_browse_snapshot(*snapshot);});short_route.join();
        expect(assisted.path.success&&assisted.path.step_count==walking.path.step_count&&assisted.path.walk_phases.size()==1&&
            assisted.path.walk_phases.front().transportation_id==0,"short Saronia directions retain walking despite an adjacent aircraft");
        expect(browse.route.reason.find(L"board")==std::wstring::npos,"short Saronia browse never advertises boarding");
        std::vector<Phase> prior_flight;
        std::thread old_plan([&]{SnapshotScope scope(*snapshot);auto request=walk_request(*world,{1,83,62});
            request.auto_groups={3};request.goals={target};request.live_hazards=request.live_traps=true;
            boarding_route(*world,baked::grids(*world),request,request.goals,prior_flight);});old_plan.join();
        expect(prior_flight.size()>1,"the former optional aircraft shortcut is reproducible on real Saronia terrain");
        auto old=std::make_shared<ActiveRoute>(*assisted.active);old->phases=prior_flight;snapshot->previous_active=old;
        Planned refreshed;std::thread reuse([&]{refreshed=plan_snapshot(*snapshot);});reuse.join();
        expect(refreshed.path.success&&refreshed.path.walk_phases.size()==1&&refreshed.path.step_count==walking.path.step_count,
            "a retained optional flight is replaced by the legal nearby walking route");
        snapshot->previous_active.reset();
        auto mounted=std::make_unique<RouteSnapshot>(*snapshot);mounted->position=model.pos;mounted->vehicle={true,model.transportation_id,3,L""};mounted->previous_active=assisted.active;
        std::thread riding([&]{boarded=plan_snapshot(*mounted);});riding.join();
        expect(boarded.path.success&&boarded.active&&boarded.active->transportation_id==model.transportation_id&&
            boarded.path.step_count!=walking.path.step_count&&boarded.path.full_description.find(L"board")==std::wstring::npos,
            "already-boarded aircraft replaces the prior short walking plan without a new boarding instruction");
        expect(std::any_of(boarded.path.walk_phases.begin(),boarded.path.walk_phases.end(),[](const auto& phase){return phase.vehicle_before==route_progress::VehicleAction::ConfirmLand;}),
            "already-boarded Nautilus must land before entering an ordinary Saronia town");
        std::printf("Saronia (%d,%d): walking%d, assisted%d tiles, %zu phases\n",target.x,target.y,walking.path.step_count,assisted.path.step_count,assisted.path.walk_phases.size());
    }
}
void check_palamecia() {
    const auto* world=baked::submap_by_map_id(1);const auto& grid=baked::grids(*world);
    const baked::Object* pad=nullptr;
    for(const auto& o:grid.objects)if(o.type==baked::ObjType::TransportationEventAction&&o.entity_id()==49)pad=&o;
    expect(pad&&pad->a==134,"Palamecia's actual landing pad identity/script exists");if(!pad)return;
    map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.scenario[79]=0;flags.current_map_id=1;
    auto make=[&] {auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
        snapshot->map_id=1;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";
        snapshot->goal.map_id=1;snapshot->goal.cell={1,pad->tx,pad->ty};snapshot->goal.entity_id=49;
        snapshot->goal.raw_name=baked::object_identifier(*pad);snapshot->kind=TargetKind::Infer;return snapshot;};
    auto mounted=make();expect(mounted->state.active(*world,*pad),"native world initializer makes Palamecia pad active before scenario79 completes");
    const auto* airship=baked::transport(3);expect(airship&&airship->type==3,"real FFII Airship model and movement group exist");if(!airship)return;
    auto foot=walk_request(*world,{1,0,0});Cell park,beside;bool found=false;
    for(int y=0;y<grid.height&&!found;++y)for(int x=0;x<grid.width&&!found;++x) {
        Cell cell{1,x,y},next{1,(x+1)%grid.width,y};
        if(baked_route::open(foot,grid,cell)&&baked_route::open(foot,grid,next)&&baked::vehicle_open(grid,airship->group,x,y)&&baked::group_lands(airship->landing_group,baked::attribute(grid,x,y))){park=cell;beside=next;found=true;}
    }
    expect(found,"real world contains a legal parked Airship with a reachable boarding side");if(!found)return;
    mounted->position=world_of_cell(*world,park);mounted->vehicle={true,3,3,L""};
    Planned flight;std::vector<int> distances;
    auto eligible=[&](RouteSnapshot& snapshot,bool vehicles) {auto list=std::make_unique<ExitSnapshot>(vehicles,snapshot.state);auto& copy=list->route;copy.submap=snapshot.submap;copy.map_id=snapshot.map_id;copy.layer=snapshot.layer;copy.position=snapshot.position;copy.vehicle=snapshot.vehicle;copy.vehicles=snapshot.vehicles;copy.goal=snapshot.goal;copy.kind=snapshot.kind;
        ExitTarget target{world_of_cell(*world,snapshot.goal.cell),0,0,snapshot.goal.cell};target.kind=TargetKind::Infer;
        return exit_distances(*list,{target},{});};
    std::thread flying([&]{flight=plan_snapshot(*mounted);distances=eligible(*mounted,true);});flying.join();
    expect(flight.path.success&&!flight.path.walk_phases.empty()&&flight.path.walk_phases.back().cells.back()==mounted->goal.cell&&flight.path.walk_phases.back().action_after==L"land","already-boarded Airship routes to the exact Palamecia pad with manual landing guidance");
    expect(distances.size()==1&&distances[0]>=0,"mounted event list uses Airship reachability instead of requiring walking/ordinary landing");
    auto parked=make();parked->position=world_of_cell(*world,beside);VehicleOnMap model;model.transportation_id=3;model.type=3;model.pos=world_of_cell(*world,park);parked->vehicles={model};
    Planned boarding;std::thread board([&]{boarding=plan_snapshot(*parked);distances=eligible(*parked,true);});board.join();
    expect(boarding.path.success&&std::any_of(boarding.path.walk_phases.begin(),boarding.path.walk_phases.end(),[](const auto& p){return p.transportation_id==3&&p.vehicle_before==route_progress::VehicleAction::ConfirmBoard;}),"on-foot Palamecia route boards the actual parked Airship before flying");
    expect(distances.size()==1&&distances[0]>=0,"Current map with vehicles ranks the landing event through a real reachable Airship");
    parked->vehicles.clear();Planned missing;std::thread absent([&]{missing=plan_snapshot(*parked);distances=eligible(*parked,true);});absent.join();
    expect(!missing.path.success&&distances[0]<0,"no Airship model cannot invent a walking route to the landing pad");
    std::printf("Palamecia: exact pad (%d,%d), Airship mounted/parked/missing routes and event eligibility checked\n",pad->tx,pad->ty);
}
// Preserve the user's same-map/vehicle distinction. These use full production
// snapshots and the real 256x256 terrain, not a distance-shaped fake graph.
void check_leviathan() {
    const auto* world=baked::submap_by_map_id(1);if(!world)return;
    const auto& grid=baked::grids(*world);const baked::Object* trigger=nullptr;
    for(const auto& object:grid.objects)if(baked::ship_entry(object)){expect(!trigger,"one reviewed FFII Ship-entry trigger");trigger=&object;}
    expect(trigger&&trigger->tx==57&&trigger->ty==132&&trigger->entity_id()==108&&trigger->a==228,
        "actual baked Leviathan trigger retains its identity, cropped tile and native destination");if(!trigger)return;
    auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
    // Initializer branches skip completed earlier scenes, then select this
    // group until its completion flag65 is set (native comparison jumps on1).
    for(int previous:{15,16,31,32,34,35,39,40,47,50,52,62})flags->scenario[previous]=1;
    flags->scenario[65]=0;flags->current_map_id=1;
    auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
    snapshot->submap=world;snapshot->map_id=1;snapshot->layer=1;snapshot->locale=L"en";
    snapshot->position=world_of_cell(*world,{1,30,194});snapshot->vehicle={true,4,2,L""};
    snapshot->goal.map_id=1;snapshot->goal.cell={1,57,132};snapshot->goal.entity_id=108;
    snapshot->goal.raw_name=baked::object_identifier(*trigger);snapshot->kind=TargetKind::Infer;
    expect(snapshot->state.active(*world,*trigger),"native progress selects Leviathan while completion flag65 remains unset");
    Planned full;BrowsePlanned browse;std::vector<int> eligible;const auto start=std::chrono::steady_clock::now();
    auto run=[&]{full=plan_snapshot(*snapshot);browse=plan_browse_snapshot(*snapshot);auto list=std::make_unique<ExitSnapshot>(true,snapshot->state);auto& copy=list->route;copy.submap=world;copy.map_id=1;copy.layer=1;copy.position=snapshot->position;copy.vehicle=snapshot->vehicle;copy.vehicles=snapshot->vehicles;copy.goal=snapshot->goal;copy.kind=snapshot->kind;ExitTarget row{world_of_cell(*world,snapshot->goal.cell),0,0,snapshot->goal.cell};row.kind=TargetKind::Infer;eligible=exit_distances(*list,{row},{});};
    std::thread worker(run);worker.join();
    expect(full.path.success&&!full.path.walk_phases.empty()&&full.path.walk_phases.back().cells.back()==snapshot->goal.cell,
        "logged boarded Ship reaches the exact Leviathan sea trigger");
    expect(browse.route.direct&&eligible.size()==1&&eligible[0]>=0,"mounted Event listing, eligibility and full route agree");
    bool sails=true;for(const auto& phase:full.path.walk_phases)sails&=phase.vehicle_before!=route_progress::VehicleAction::WalkOff&&phase.vehicle_before!=route_progress::VehicleAction::ConfirmLand&&phase.vehicle_before!=route_progress::VehicleAction::BackLand;
    expect(sails,"Leviathan scene starts aboard Ship, without a fictional pre-trigger landing");
    std::printf("Leviathan from logged Ship (30,194): full %d steps, eligibility %d, %.3f ms combined offline\n",full.path.step_count,eligible.empty()?-1:eligible[0],std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());
    auto movement=walk_request(*world,{1,30,194});const auto* ship=baked::transport(4);movement.vehicle_group=ship->group;movement.vehicle_type=2;
    expect(baked_route::transition_enterable(movement,grid,57,132),"reviewed native sea scene accepts current Ship terrain");
    movement.vehicle_type=3;expect(!baked_route::transition_enterable(movement,grid,57,132),"the Ship-entry flag cannot authorize an airship");movement.vehicle_type=2;
    for(const auto& object:grid.objects)if(object.type==baked::ObjType::GotoMap&&object.a>0)expect(!baked_route::transition_enterable(movement,grid,object.tx,object.ty),"native town doors still require leaving Ship");
    snapshot->position=world_of_cell(*world,{1,30,185});snapshot->vehicle={true,1,1,L""};
    VehicleOnMap parked;parked.transportation_id=4;parked.type=2;parked.pos=world_of_cell(*world,{1,30,194});snapshot->vehicles={parked};snapshot->vehicle_names[4]=L"Ship";
    std::thread board(run);board.join();expect(full.path.success&&eligible.size()==1&&eligible[0]>=0,"a legal walking/boarding/sailing route also keeps the Event visible with owned vehicles");
    snapshot->vehicles.clear();std::thread no_ship(run);no_ship.join();expect(!full.path.success&&eligible.size()==1&&eligible[0]<0,"foot-only party cannot acquire an unowned Ship from a sea-trigger flag");
    flags->scenario[65]=1;auto disabled=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
    disabled->submap=world;disabled->map_id=1;disabled->layer=1;disabled->position=snapshot->position;disabled->goal=snapshot->goal;disabled->kind=snapshot->kind;disabled->vehicle={true,4,2,L""};
    std::thread inactive([&]{full=plan_snapshot(*disabled);});inactive.join();expect(!full.path.success,"inactive Leviathan story group cannot become a valid route");
}
void check_mysidia_followup() {
    const auto* world=baked::submap_by_map_id(1);
    expect(world&&world->world,"FFII actual world map exists");if(!world)return;
    auto base=std::make_unique<RouteSnapshot>(map_state::Resolver(map_state::Flags{},fixture::records));
    base->submap=world;base->map_id=1;base->layer=1;base->locale=L"en";base->auto_groups={2,5};
    base->vehicle={true,2,5,L""}; // the current auto-ridden Canoe, same at both origins
    VehicleOnMap ship;ship.transportation_id=4;ship.type=2;ship.pos=world_of_cell(*world,{1,189,44});
    base->vehicles={ship};base->vehicle_names[4]=L"Ship";
    // Use the selected baked trigger, after the native MapRange crop. Raw
    // Tiled y232 becomes baked y231; never test the decorative neighbour.
    Cell island;bool found=false;
    for(const auto& object:baked::grids(*world).objects)if(object.type==baked::ObjType::GotoMap&&object.a==200) {
        island={1,object.tx,object.ty};found=true;break;
    }
    expect(found,"actual Tropical Island transition is present");if(!found)return;
    for(const auto origin:{Cell{1,40,168},Cell{1,177,14}}) {
        base->position=world_of_cell(*world,origin);base->goal.map_id=1;base->goal.cell=island;base->kind=TargetKind::Transition;
        auto walking=std::make_unique<ExitSnapshot>(false,base->state);
        auto vehicles=std::make_unique<ExitSnapshot>(true,base->state);
        for(auto* capture:{walking.get(),vehicles.get()}) {
            auto& route=capture->route;route.submap=world;route.map_id=1;route.layer=1;
            route.position=base->position;route.vehicle=base->vehicle;
            route.auto_groups=base->auto_groups;route.vehicles=base->vehicles;
        }
        std::vector<ExitTarget> exits{{world_of_cell(*world,island),0,200,island}};
        std::vector<int> foot_distance,vehicle_distance;Planned full;
        const auto began=std::chrono::steady_clock::now();
        std::thread worker([&] {
            foot_distance=exit_distances(*walking,exits,{});
            vehicle_distance=exit_distances(*vehicles,exits,{});
            full=plan_snapshot(*base);
        });worker.join();
        expect(vehicle_distance.size()==1&&vehicle_distance[0]>31&&full.path.success&&full.path.step_count>31,
            "Tropical Island remains listed and fully routable beyond speech brevity distance from both distant origins using the owned Ship");
        std::printf("Tropical Island from (%d,%d): walking filter %d, with owned Ship %d, full route %d steps; %.3f ms combined offline\n",
            origin.x,origin.y,foot_distance.empty()?-2:foot_distance[0],vehicle_distance.empty()?-2:vehicle_distance[0],full.path.step_count,
            std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
    }
    // Latest user Ship trace: a distant walk followed by a single eastward
    // boarding edge. The list must not imply that East is the current heading.
    base->position=world_of_cell(*world,{1,11,163});base->goal.cell={1,189,44};
    base->goal.transportation_id=4;base->kind=TargetKind::Spot;
    Planned full;BrowsePlanned browse;
    std::thread worker([&]{full=plan_snapshot(*base);browse=plan_browse_snapshot(*base);});worker.join();
    expect(full.path.success&&full.path.walk_phases.size()==2&&full.path.walk_phases[0].cells.size()>2,
        "logged Ship start has a genuine walking approach before boarding");
    if(full.path.walk_phases.size()==2) {
        const auto count=full.path.walk_phases[0].cells.size()-1;
        expect(browse.route.reason==L"Sail "+std::to_wstring(count)+L".0 steps, then "+full.path.walk_phases[1].action_before,
            "distant Ship listing uses the current Canoe's Sail approach before its later boarding heading");
        std::printf("Logged Ship: %zu approach steps before boarding; full route %d steps\n",count,full.path.step_count);
    }
    const auto* statue=baked::submap_by_map_id(98);expect(statue!=nullptr,"actual Statue Room exists");
    if(statue) {
        int staged=0;
        for(const auto& object:baked::grids(*statue).objects)if(object.type==baked::ObjType::NPC) {
            ++staged;expect((object.d&1)&&!(object.d&(4|8)),"real Masked Figure retains collision but no invented navigation action");
        }
        expect(staged==2,"both authored Masked Figure placements audited");
        std::vector<RemoteObject> remote;
        std::thread list([&]{map_state::Resolver state(map_state::Flags{},fixture::records);remote=connected_objects_snapshot(90,state);});list.join();
        expect(std::none_of(remote.begin(),remote.end(),[](const auto& object){return object.map_id==98&&object.type==baked::ObjType::NPC;}),
            "remote Current map discovery also excludes the scriptless Masked Figure");
    }
}
}
int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    for(int n=1;n<=6;++n) {
        const auto root="data/ff"+std::to_string(n)+"/";
        const auto data=bytes(root+"maps.bin");expect(baked::load(data.data(),data.size()),"matching real bake loads");
        expect(json::parse(bytes(root+"mapstate.json"),fixture::records),"matching initializer metadata loads");
        fixture::game_id=static_cast<game::Id>(n);g_loaded=g_prepared=true;
        int world_exits=0;
        for(int i=0;i<baked::submap_count();++i) {
            const auto& map=*baked::submap_at(i);const auto& grid=baked::grids(map);
            for(const auto& object:grid.objects)if(baked::world_exit(object)) {
                ++world_exits;const auto* dest=baked::submap_by_map_id(object.a);
                expect(dest!=nullptr,"every reviewed scripted transfer points at a real destination for that game");
                expect(!baked_route::step_event(object),"ordinary scripted exits are not story scene warnings");
                const auto* found=baked::world_exit_at(grid,object.entity_id(),baked::object_identifier(object),object.tx,object.ty);
                // The immutable native detector must not invent one destination
                // for a real conditional door. Its branch is selected later by
                // the owned state-aware planner (conditional_transfer_check).
                std::set<int> destinations;
                const auto indexed=grid.world_exits.find(object.entity_id());
                if(indexed!=grid.world_exits.end())for(const auto index:indexed->second) {
                    const auto& candidate=grid.objects[index];
                    if(candidate.contains(object.tx,object.ty)&&baked::object_identifier(candidate)==baked::object_identifier(object))
                        destinations.insert(candidate.a);
                }
                if(destinations.size()>1) {
                    expect(!found,"ambiguous native door does not advertise an arbitrary story destination");
                    std::printf("FF%d: conditional door map%d actor%d retains %zu branch destinations for owned planning\n",n,map.first_map_id,object.entity_id(),destinations.size());
                } else expect(found&&found->a==object.a,"native detector can use exact prepared identity/footprint without a map scan");
            }
        }
        std::printf("FF%d: %d script-backed world exits indexed; ordinary native doorways retained\n",n,world_exits);
        bool ship_checked=false;
        for(int i=0;i<baked::submap_count()&&!ship_checked;++i)if(baked::submap_at(i)->world)ship_checked=check_ship(*baked::submap_at(i));
        if(!ship_checked)std::printf("FF%d: no applicable Ship/shore pair in its world grids; native transport coverage retained elsewhere\n",n);
        check_transport_suffix_modes();
        if(n==3){check_saronia_short_walks();check_duster_boarding_overlap();}
        if(n==2) {
            {
                auto throne=room(43,{0,28,12});throne->goal.map_id=43;throne->goal.cell={0,26,30};throne->kind=TargetKind::Transition;
                const auto& grid=baked::grids(*throne->submap);
                const baked::Object* helper=nullptr;bool native_exit=false;
                for(const auto& o:grid.objects) {
                    if(o.type==baked::ObjType::TransitionHelper&&o.entity_id()==96)helper=&o;
                    if(o.type==baked::ObjType::GotoMap&&o.entity_id()==84&&o.a==40&&o.b==2)native_exit=true;
                }
                expect(helper&&native_exit,"Fynn throne-room transition companion preserves its real Castle Fynn 3F exit");
                if(helper) {
                    throne->goal.cell={helper->layer,helper->tx,helper->ty};
                    throne->position=world_of_cell(*throne->submap,{helper->layer,helper->tx+2,helper->ty-18});
                    expect(!baked_route::step_event(*helper)&&baked::transition_helper_at(grid,96,baked::object_identifier(*helper),helper->tx,helper->ty),"reviewed companion is indexed for live discovery and excluded from static event hazards");
                    NativeObstacle live;live.cell=throne->goal.cell;live.id=96;live.type=2;live.trigger=true;live.raw_name=baked::object_identifier(*helper);
                    throne->obstacles.push_back(live);
                    Planned exit;std::thread route([&]{exit=plan_snapshot(*throne);});route.join();
                    expect(exit.path.success&&!exit.path.event_on_route&&!exit.path.required_trap,"logged throne-room route remains valid without a false live event warning");
                    const auto identity=std::find_if(throne->obstacles.begin(),throne->obstacles.end(),[](const auto& actor){return actor.id==96;});
                    identity->id=999;identity->raw_name=L"Independent scene";
                    Planned scene;std::thread real([&]{scene=plan_snapshot(*throne);});real.join();
                    expect(scene.path.success&&scene.path.event_on_route,"independent live story trigger on the same exit remains a real hazard");
                }
            }
            check_leviathan();
            check_palamecia();
            check_object_vehicle_shortcuts();
            check_mysidia_followup();
            auto chest=room(202,{0,18,13});chest->goal.map_id=202;chest->goal.cell={0,26,16};chest->kind=TargetKind::Interact;
            for(const auto& object:baked::grids(*chest->submap).objects)if(object.type==baked::ObjType::TreasureBox&&object.tx==26&&object.ty==16)chest->goal.entity_id=object.entity_id();
            expect(chest->goal.entity_id>=0,"the logged B3 coordinate identifies an actual baked treasure chest");
            Planned detour;std::thread route([&]{detour=plan_snapshot(*chest);});route.join();
            expect(detour.path.success&&detour.path.next_map_id==203&&detour.goal.map_id==202&&detour.goal.cell==chest->goal.cell&&detour.goal.entity_id==chest->goal.entity_id,
                "logged B3 chest uses B4 as an intermediate floor without changing the final B3 destination");
            auto after_battle=std::make_unique<RouteSnapshot>(*chest);after_battle->previous_active.reset();
            Planned refreshed;std::thread repeat([&]{refreshed=plan_snapshot(*after_battle);});repeat.join();
            expect(refreshed.path.success&&refreshed.path.next_map_id==203&&refreshed.goal.map_id==202&&refreshed.goal.cell==chest->goal.cell&&refreshed.goal.entity_id==chest->goal.entity_id,
                "battle-style fresh planning retains the exact B3 chest through its B4 detour");
            std::printf("Logged chest: final B3/map%d (%d,%d), entity%d retained before/after battle; intermediate map%d\n",refreshed.goal.map_id,refreshed.goal.cell.x,refreshed.goal.cell.y,refreshed.goal.entity_id,refreshed.path.next_map_id);
            auto town=room(20,{0,20,37});town->goal.map_id=20;town->goal.cell={0,20,38};town->kind=TargetKind::Transition;
            Planned exit;std::thread work([&]{exit=plan_snapshot(*town);});work.join();
            expect(exit.path.success&&!exit.path.event_on_route&&!exit.path.required_trap,"Fynn's real southern script exit is reachable without a false scene/trap warning");
            for(int origin:{20,21}) {
                auto request=room(origin,origin==20?Cell{0,20,37}:Cell{0,17,12});
                request->goal.map_id=origin==20?22:21;request->goal.cell=origin==20?Cell{0,8,3}:Cell{0,19,12};
                request->kind=origin==20?TargetKind::Interact:TargetKind::Transition;
                CrossLeg leg;Planned blocked;const auto began=std::chrono::steady_clock::now();
                std::thread gate([&]{SnapshotScope scope(*request);leg=find_across(nullptr,nullptr,request->goal);blocked=plan_snapshot(*request);});gate.join();
                const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                expect(!blocked.path.success&&blocked.path.reason==L"Story gated","unfiltered Paul's chest/hidden passage keep their real gate reason");
                expect(!leg.incomplete&&leg.visited_maps<60,"indoor gate proof stays within the real connected floors instead of traversing the world graph");
                std::printf("FFII logged gated target origin%d: %d floor states, %.3f ms paired plan (offline, excludes native capture/speech)\n",origin,leg.visited_maps,ms);
            }
        }
    }
    expect(fixture::wrong_owner==0,"all route work stays on owned snapshots without native calls");
    std::printf("Navigation follow-up: %d failures\n",failures);return failures?1:0;
}
