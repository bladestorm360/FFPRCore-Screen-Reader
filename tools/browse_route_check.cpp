// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Production browse worker against real bakes; fake native boundaries flag worker reads.
#define main existing_route_worker_main
#include "route_worker_check.cpp"
#undef main

namespace {
template<class Expect> void check_manual_readout(Expect expect)
{
    using namespace field;
    using namespace baked_nav;
    Phase phase;
    phase.cells = {{1,0,0},{1,0,1},{1,0,2},{1,1,2},{1,2,2},{1,3,2},
                   {1,3,3},{1,3,4},{1,3,5},{1,3,6},{1,4,6}};
    phase.action_after = L"Operate the control";
    phase.manual_after = true;
    const std::vector<Phase> phases{phase};
    PathInfo chunk;
    expect(describe_phases(phases,true,nullptr,3,&chunk)==L"2 South, 3 East, 4 South",
           "Three direction runs are read exactly, without later movement or the final action");
    expect(chunk.readout_limited && chunk.readout_cells.size()==10 &&
           chunk.readout_cells.back()==Cell{1,3,6},"The announced route owns its exact final walking tile");
    PathInfo full;
    expect(describe_phases(phases,true,nullptr,4,&full)==describe_phases(phases,true,nullptr) &&
           !full.readout_limited && full.readout_cells.empty(),
           "The last chunk includes the reachable action and has no additional progress trigger");
    using route_progress::ChunkProgress;
    using route_progress::advance_chunk;
    size_t cursor=0;
    expect(advance_chunk(chunk.readout_cells,cursor,{1,0,0})==ChunkProgress::Unchanged &&
           advance_chunk(chunk.readout_cells,cursor,{1,1,2})==ChunkProgress::Advanced &&
           advance_chunk(chunk.readout_cells,cursor,{1,9,9})==ChunkProgress::Deviated &&
           advance_chunk(chunk.readout_cells,cursor,{1,3,4})==ChunkProgress::Advanced &&
           advance_chunk(chunk.readout_cells,cursor,{1,3,6})==ChunkProgress::Completed,
           "Only actual progress through the issued cells can complete the chunk");
    Phase first;
    first.cells={{1,0,0},{1,0,1}};
    first.action_after=L"Operate the control";
    first.manual_after=true;
    Phase second;
    second.cells={{1,0,1},{1,1,1}};
    PathInfo boundary;
    expect(describe_phases({first,second},true,nullptr,1,&boundary)==L"1 South, Operate the control" &&
           boundary.readout_limited && boundary.readout_cells.empty(),
           "An interaction at the chunk boundary is voiced and must complete before any continuation");
    Phase long_route;
    long_route.cells.push_back({1,0,0});
    for (int leg=0;leg<8;++leg) for (int step=0;step<5;++step) {
        auto at=long_route.cells.back();
        at.x+=leg%2; at.y+=1-leg%2;
        long_route.cells.push_back(at);
    }
    PathInfo legacy;
    legacy.success=true;
    legacy.walk_phases={long_route};
    const auto old_description=describe_phases(legacy.walk_phases,false,nullptr);
    legacy.description=old_description;
    apply_manual_readout(legacy,0,false,nullptr);
    expect(legacy.description==old_description && legacy.readout_cells.empty(),
           "Default full directions keep the original summary with no automatic tracking");
    apply_manual_readout(legacy,0,true,nullptr);
    expect(legacy.description==old_description && legacy.readout_limited &&
           legacy.readout_cells.size()==31 && legacy.readout_cells.back()==long_route.cells[30],
           "With auto on, the unchanged legacy summary can advance after its spoken prefix");
}

template<class Expect> void check_ff5_water_pushbacks(Expect expect)
{
    using namespace field;
    using baked_route::Cell;
    int contacts = 0;
    for (const int map_id : {48, 54, 112, 261, 268, 476, 478, 545}) {
        const auto* map = baked::submap_by_map_id(map_id);
        if (!map) { expect(false, "Missing FFV current map"); continue; }
        const auto& grid = baked::grids(*map);
        for (const auto& object : grid.objects) {
            if (object.type != baked::ObjType::Event ||
                !baked::current_pushback(grid, object.layer, object.tx, object.ty)) continue;
            ++contacts;
            const Cell contact{object.layer, object.tx, object.ty};
            baked_route::Request request;
            request.submap = map;
            request.goals = {contact};
            request.goals_open = request.ignore_hazards = true;
            for (const Cell offset : {Cell{0, 1, 0}, Cell{0, -1, 0}, Cell{0, 0, 1}, Cell{0, 0, -1}}) {
                const Cell neighbor{contact.layer, contact.x + offset.x, contact.y + offset.y};
                if (baked::in_bounds(grid, neighbor.x, neighbor.y))
                    expect(!baked_route::edge_allowed(request, grid, neighbor, contact),
                           "A real pushback cannot be entered even as a goal with event hazards ignored");
            }
        }
    }
    expect(contacts == 86, "All source-qualified FFV pushbacks retain their native layers");
}

template<class Expect> void check_ff4_lunar_subterrane_exits(Expect expect)
{
    using namespace field;
    using namespace baked_nav;
    const auto* map = baked::submap_by_map_id(296);
    if (!map) { expect(false, "Missing Lunar Subterrane B1"); return; }
    RouteSnapshot snapshot(map_state::Resolver(map_state::Flags{}, fixture::records));
    snapshot.submap = map;
    snapshot.map_id = 296;
    snapshot.layer = 0;
    snapshot.position = world_of_cell(*map, {0, 22, 19});
    snapshot.kind = TargetKind::Transition;
    snapshot.locale = L"en";
    std::thread worker([&] {
        int reachable_stairs = 0;
        for (const auto& exit : baked::grids(*map).objects) {
            if (exit.type != baked::ObjType::GotoMap || (exit.a != 154 && exit.a != 296)) continue;
            snapshot.goal = {296, {exit.layer, exit.tx, exit.ty}, false, {}, 0, exit.entity_id()};
            const auto directions = plan_snapshot(snapshot).path;
            if (exit.a == 154) expect(directions.success, "The nearby Crystal Palace exit is reachable");
            if (!directions.success) continue;
            if (exit.a == 296) ++reachable_stairs;
            const auto browse = plan_browse_snapshot(snapshot).route;
            expect(browse.direct || browse.via_map_id || !browse.reason.empty(),
                   "A reachable Lunar Subterrane exit always has a meaningful browse description");
        }
        expect(reachable_stairs > 0, "The fixture exercises the real same-map stairs on B1");
    });
    worker.join();
}

template<class Expect> void check_ff4_magnetic_cavern(Expect expect)
{
    using namespace field;
    using namespace baked_nav;
    const auto* world = baked::submap_by_map_id(1);
    const auto* forest = baked::submap_by_map_id(160);
    const auto& exits = baked::grids(*world).objects;
    const auto cave = std::find_if(exits.begin(), exits.end(), [](const auto& object) {
        return object.type == baked::ObjType::GotoMap && object.entity_id() == 63 && object.a == 192;
    });
    expect(forest && cave != exits.end(), "the real Magnetic Cavern and Black Chocobo forest are baked");
    if (!forest || cave == exits.end()) return;

    const MapGoal goal{1, {cave->layer, cave->tx, cave->ty}, false, {}, 0, 63};
    RouteSnapshot snapshot(map_state::Resolver(map_state::Flags{}, fixture::records));
    snapshot.submap = world;
    snapshot.map_id = snapshot.layer = 1;
    snapshot.position = world_of_cell(*world, {1, 158, 186});
    snapshot.vehicle = {true, 9, 8, {}};
    snapshot.goal = goal;
    snapshot.kind = TargetKind::Transition;
    snapshot.locale = L"en";
    std::thread worker([&] {
        const auto falcon = plan_snapshot(snapshot);
        expect(falcon.path.success && falcon.path.next_map_id == 160 && falcon.pending.goal.cell == goal.cell &&
               falcon.pending.incoming_transition && falcon.pending.incoming_transition->entity_id == 82,
               "the Falcon route leads to the forest without losing the cavern destination");
        expect(falcon.path.description.find(L"Black Chocobo") != std::wstring::npos,
               "the Falcon route names the black chocobo interaction after the directions");

        snapshot.vehicle = {};
        snapshot.position = world_of_cell(*world, {1, 45, 41});
        const auto on_foot = plan_snapshot(snapshot);
        expect(on_foot.path.success && on_foot.path.next_map_id == 160,
               "a party already walking near the village can reach the mount without a parked vehicle");

        snapshot.submap = forest;
        snapshot.map_id = 160;
        snapshot.vehicle = {};
        snapshot.goal = goal;
        snapshot.pending = {};
        snapshot.obstacles.clear();
        const auto& objects = baked::grids(*forest).objects;
        const auto bird = std::find_if(objects.begin(), objects.end(), [](const auto& object) {
            return object.type == baked::ObjType::NPC && object.entity_id() == 14;
        });
        if (bird == objects.end()) { expect(false, "the forest has the scripted mount NPC"); return; }
        snapshot.layer = bird->layer;
        snapshot.position = world_of_cell(*forest, {bird->layer, bird->tx - 1, bird->ty});
        NativeObstacle actor;
        actor.type = static_cast<int>(baked::ObjType::NPC);
        actor.id = bird->entity_id();
        actor.cell = {bird->layer, bird->tx, bird->ty};
        actor.raw_name = baked::object_identifier(*bird);
        actor.npc = true;
        snapshot.obstacles.push_back(actor);
        const auto inside = plan_snapshot(snapshot);
        expect(inside.path.success && inside.path.next_map_id == 1 && inside.pending.goal.cell == goal.cell &&
               inside.path.full_description.find(L"Black Chocobo") != std::wstring::npos,
               "the forest route approaches the live mount NPC and retains the cavern");

        const auto departure = std::find_if(exits.begin(), exits.end(), [](const auto& object) {
            return object.type == baked::ObjType::PointIn && object.a == 32;
        });
        if (departure == exits.end()) { expect(false, "the mount script has its surface arrival"); return; }
        snapshot.submap = world;
        snapshot.map_id = snapshot.layer = 1;
        snapshot.position = world_of_cell(*world, {departure->layer, departure->tx, departure->ty});
        snapshot.vehicle = {true, 7, 10, {}};
        snapshot.obstacles.clear();
        const auto mounted = plan_snapshot(snapshot);
        expect(mounted.path.success && !mounted.path.walk_phases.empty() &&
               mounted.path.walk_phases.back().vehicle_before == route_progress::VehicleAction::ConfirmLand &&
               cave->contains(mounted.path.walk_phases.back().cells.back().x, mounted.path.walk_phases.back().cells.back().y),
               "the ridden Black Chocobo leaves the village, lands legally and reaches the cavern");
    });
    worker.join();
}

template<class Expect> void check_ff4_hovercraft_journey(Expect expect)
{
    using namespace field;
    using namespace baked_nav;
    const auto* map=baked::submap_by_map_id(1);
    const auto& grid=baked::grids(*map);
    const Cell origin{1,158,186}, parked_hover{1,122,44}, destination{1,224,123}; // Adamant Grotto
    RouteSnapshot snapshot(map_state::Resolver(map_state::Flags{},fixture::records));
    snapshot.submap=map;
    snapshot.map_id=1;
    snapshot.layer=1;
    snapshot.position=world_of_cell(*map,origin);
    snapshot.vehicle={true,10,8,{}};
    snapshot.vehicle_shortcuts=true;
    snapshot.locale=L"en";
    snapshot.goal={1,destination,false};
    snapshot.kind=TargetKind::Transition;
    auto set_parked=[&](Cell at){
        VehicleOnMap model;
        model.transportation_id=2;
        model.type=7;
        model.pos=world_of_cell(*map,at);
        snapshot.vehicles={model};
    };
    std::thread worker([&] {
        SnapshotScope scope(snapshot);
        auto request=walk_request(*map,origin);
        world_mode(request);
        request.goals={destination}; request.goals_open=true;
        snapshot.hovercraft_carried=true;
        const auto started=frame::now();
        auto loaded=hovercraft_directions(request);
        std::printf("FFIV loaded hovercraft: %zu phases, %.2f ms offline\n",loaded.size(),(frame::now()-started)*1000);
        expect(loaded.size()>=5,"Attached hovercraft has a complete release, separate parking, boarding and travel route");
        if (loaded.size()<5) return;
        expect(loaded[0].manual_after && loaded[0].movement_transportation_id==10 &&
               loaded[0].action_after==L"Lower the hovercraft.","Lowering stops the first leg aboard the Enterprise");
        const auto released=loaded[0].cells.back();
        const auto park=loaded[1].cells.back();
        expect((std::abs(park.x-released.x)>1 || std::abs(park.y-released.y)>1) &&
               loaded[2].vehicle_before==route_progress::VehicleAction::ConfirmLand &&
               loaded[2].action_cell==std::optional<Cell>(park) && loaded[2].movement_transportation_id==0 &&
               loaded[3].vehicle_before==route_progress::VehicleAction::ConfirmBoard && loaded[3].transportation_id==2 &&
               baked_route::step_direction(loaded[2].cells.back(),released,grid.width,grid.height)>=0,
               "The Enterprise parks outside the pickup range and the party walks back to the hovercraft");
        expect(released!=destination && loaded[3].cells.size()>1 && loaded.back().cells.back()==destination,
               "Adamant Grotto requires a release away from the entrance and a real hovercraft ride before entry");
        const auto prefix=plan_snapshot(snapshot);
        std::printf("FFIV hovercraft main route: success %d next %d legs %zu pending %d, %s %s\n",
                    prefix.path.success,prefix.path.next_map_id,prefix.path.walk_phases.size(),prefix.pending.valid,
                    text::to_utf8(prefix.path.full_description).c_str(),text::to_utf8(prefix.path.reason).c_str());
        expect(prefix.path.success && prefix.path.next_map_id==1 && prefix.pending.goal.cell==destination &&
               !prefix.path.walk_phases.empty() && prefix.path.walk_phases.back().manual_after,
               "The production snapshot publishes only the safe leg up to release and retains the selected entrance");
        const auto entrance=std::find_if(grid.objects.begin(),grid.objects.end(),[&](const auto& object) {
            return object.type==baked::ObjType::GotoMap && object.contains(destination.x,destination.y);
        });
        if (entrance!=grid.objects.end()) {
            ExitSnapshot browse(true,snapshot.state);
            browse.route.submap=map;
            browse.route.map_id=1;
            browse.route.layer=1;
            browse.route.position=world_of_cell(*map,origin);
            browse.route.vehicle={true,10,8,{}};
            browse.route.vehicle_shortcuts=true;
            browse.route.hovercraft_carried=true;
            const auto distances=exit_distances(browse,{{world_of_cell(*map,destination),1,entrance->a,destination}},{});
            expect(distances.size()==1 && distances[0]>0,"The with-vehicles list keeps the hovercraft-only entrance");
        } else expect(false,"The replay uses the real surface entrance");

        snapshot.hovercraft_carried=false;
        set_parked(parked_hover);
        auto pickup=hovercraft_directions(request);
        expect(!pickup.empty() && pickup.front().manual_after && pickup.front().cells.back()==parked_hover &&
               pickup.front().action_after==L"Pick up the hovercraft.","A distant parked hovercraft is collected first");
        const Cell logged_release{1,227,127};
        set_parked(logged_release);
        snapshot.position=world_of_cell(*map,logged_release);
        request.start=logged_release;
        auto lowered=hovercraft_directions(request);
        expect(lowered.size()>=4 && std::none_of(lowered.begin(),lowered.end(),[](const Phase& phase){return phase.manual_after;}) &&
               (std::abs(lowered[0].cells.back().x-logged_release.x)>1 ||
                std::abs(lowered[0].cells.back().y-logged_release.y)>1) &&
               lowered[2].movement_transportation_id==2 && lowered[2].cells.size()>1 &&
               lowered.back().cells.back()==destination,
               "The logged release resumes outside the native pickup area and completes the hovercraft leg");

        snapshot.vehicle={true,9,8,{}};
        expect(!prepare_hovercraft_route(request),"The Falcon cannot manufacture an Enterprise carry action");

        set_parked(parked_hover);
        VehicleOnMap carrier;
        carrier.transportation_id=10; carrier.type=8; carrier.pos=world_of_cell(*map,origin);
        snapshot.vehicles.push_back(carrier);
        snapshot.vehicle={true,1,1,{}};
        snapshot.position=world_of_cell(*map,{1,156,186});
        request=walk_request(*map,{1,156,186});
        world_mode(request);
        request.goals={destination};
        auto ground_start=hovercraft_directions(request);
        expect(ground_start.size()>=6 && ground_start[0].movement_transportation_id==0 &&
               ground_start[1].vehicle_before==route_progress::VehicleAction::ConfirmBoard &&
               ground_start[1].transportation_id==10,
               "Starting on foot first walks to and boards the available Enterprise");

        auto no_parking=std::make_shared<baked_route::TransportRules>();
        for (int y=0;y<grid.height;++y) for (int x=0;x<grid.width;++x)
            if (baked::group_lands(9,baked::attribute(grid,x,y)))
                baked_route::add_transport_cell(*no_parking,baked_route::TransportRules::Gate,*map,{1,x,y},
                                                baked_route::transportation_bit(10));
        snapshot.vehicle={true,10,8,{}};
        snapshot.hovercraft_carried=true;
        snapshot.vehicles.clear();
        request.start=origin;
        request.transport_rules=no_parking;
        request.transport_gates=true;
        expect(!prepare_hovercraft_route(request),"Release without a legal separate Enterprise parking place is unavailable");
    });
    worker.join();
}

template<class Expect> void check_ff4_sealed_cavern(Expect expect)
{
    using namespace field;
    using namespace baked_nav;
    std::ifstream input("data/ff4/mapstate.json",std::ios::binary);
    const std::string encoded(std::istreambuf_iterator<char>(input),{});
    json::Value records;
    if (!json::parse(encoded,records)) {
        expect(false,"FFIV Sealed Cavern field state loads");
        return;
    }
    struct Destination { int map; int entity; baked::ObjType type; TargetKind kind; };
    const std::array goals{
        Destination{284,38,baked::ObjType::Event,TargetKind::Transition},
        Destination{286,3,baked::ObjType::TreasureBox,TargetKind::Interact},
        Destination{286,4,baked::ObjType::TreasureBox,TargetKind::Interact},
        Destination{287,5,baked::ObjType::SavePoint,TargetKind::Spot},
    };
    const Cell entry{0,14,8}, rope{0,26,10}, arrival{0,26,29};
    for (const auto& dest : goals) {
        const auto* target_map = baked::submap_by_map_id(dest.map);
        const auto& objects = baked::grids(*target_map).objects;
        const auto target = std::find_if(objects.begin(),objects.end(),[&](const auto& object) {
            return object.type == dest.type && object.entity_id() == dest.entity;
        });
        if (target == objects.end()) {
            expect(false,"Native far-side Sealed Cavern destination is present");
            continue;
        }
        const Cell target_cell{target->layer,target->tx,target->ty};
        for (const auto& [map,start] : std::array{
                 std::pair{271,Cell{0,23,11}},
                 std::pair{279,Cell{0,13,7}}, std::pair{282,Cell{0,15,11}},
                 std::pair{284,entry}, std::pair{284,arrival}}) {
            map_state::Flags flags;
            flags.current_map_id = map;
            flags.current_area_id = 46;
            flags.scenario.fill(0);
            // Entering the save room and descending to B2 need their earlier
            // opened Assault Doors. The B3 side room has its own receipt.
            flags.scenario[264]=flags.scenario[266]=flags.scenario[269]=1;
            const auto observation = [&] {
                RouteSnapshot snapshot(map_state::Resolver(flags,records));
                snapshot.map_id = map;
                snapshot.submap = baked::submap_by_map_id(map);
                snapshot.layer = start.layer;
                snapshot.position = world_of_cell(*snapshot.submap,start);
                snapshot.vehicle = {true,1,1,{}};
                snapshot.locale = L"en";
                snapshot.goal = {dest.map,target_cell,dest.kind==TargetKind::Interact,{},0,dest.entity,baked::object_identifier(*target)};
                snapshot.kind = dest.kind;
                for (const auto& object:baked::grids(*snapshot.submap).objects) {
                    if (object.type!=baked::ObjType::Event || !snapshot.state.active(*snapshot.submap,object)) continue;
                    NativeObstacle contact;
                    contact.cell={object.layer,object.tx,object.ty};
                    contact.type=int(object.type);
                    contact.id=object.entity_id();
                    contact.trigger=true;
                    contact.raw_name=baked::object_identifier(object);
                    snapshot.obstacles.push_back(std::move(contact));
                }
                return snapshot;
            };
            Planned result, blocked;
            std::thread worker([&]{
                auto snapshot=observation();
                result = plan_snapshot(snapshot);
                if (map==282 && dest.map==286) {
                    flags.scenario[269]=0;
                    auto locked=observation();
                    blocked=plan_snapshot(locked);
                }
            });
            worker.join();
            std::printf("Sealed Cavern %d (%d,%d) to %d/%d: success %d next %d, %s %s\n",
                        map,start.x,start.y,dest.map,dest.entity,result.path.success,
                        result.path.next_map_id,text::to_utf8(result.path.full_description).c_str(),
                        text::to_utf8(result.path.reason).c_str());
            expect(result.path.success,"Sealed Cavern far-side destination is reachable");
            if (map==282 && dest.map==286)
                expect(!blocked.path.success && blocked.path.reason==L"Story gated",
                       "The rope route keeps the unopened B3 Assault Door restriction");
            if (start == entry && result.path.success) {
                const auto& receipt = result.pending.incoming_transition;
                const auto& phases = result.path.walk_phases;
                expect(result.path.next_map_id==284 && receipt && receipt->arrival==arrival &&
                       !phases.empty() && !phases.back().cells.empty() && phases.back().cells.back()==rope &&
                       result.pending.goal.map_id==dest.map && result.pending.goal.cell==target_cell,
                       "The B3 rope is an intermediate leg and retains the selected destination");
                g_pending=&result.pending;
                expect(note_trap_start(284,rope),"The native script entry marks the same-map crossing");
                record_map_arrival(284,entry);
                expect(result.pending.incoming_transition.has_value(),"The earlier bank cannot complete the crossing");
                record_map_arrival(284,arrival);
                expect(!result.pending.incoming_transition && result.pending.valid &&
                       result.pending.completed_transitions.size()==1,
                       "The opposite bank completes the leg while preserving the destination");
                g_pending=nullptr;
            }
        }
    }
}

template<class Expect> void check_ff4_moon(Expect expect)
{
    using namespace field;
    using namespace baked_nav;
    const auto* moon = baked::submap_by_map_id(3);
    if (!moon) { expect(false, "FFIV Moon data loads"); return; }
    map_state::Flags flags;
    flags.current_map_id = 3;
    int entrances = 0;
    for (const auto& entrance : baked::grids(*moon).objects) {
        if (entrance.type != baked::ObjType::GotoMap) continue;
        ++entrances;
        const Cell target{1,entrance.tx,entrance.ty};
        const Cell north{1,target.x,target.y-1}, south{1,target.x,target.y+1};
        baked_route::Request walk;
        walk.submap = moon;
        walk.goals = {target};
        expect(!baked_route::direction_allowed(walk,north,target), "Moon entrance blocks the north-side approach");
        expect(baked_route::direction_allowed(walk,south,target), "Moon entrance accepts the south-side approach");

        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));
        snapshot.map_id = 3;
        snapshot.submap = moon;
        snapshot.layer = 1;
        snapshot.vehicle = {true,1,1,{}};
        snapshot.locale = L"en";
        snapshot.goal.map_id = 3;
        snapshot.goal.cell = target;
        snapshot.kind = TargetKind::Transition;
        Planned from_south, from_north, from_air;
        std::thread worker([&] {
            snapshot.position = world_of_cell(*moon,south);
            from_south = plan_snapshot(snapshot);
            snapshot.position = world_of_cell(*moon,north);
            from_north = plan_snapshot(snapshot);
            snapshot.position = world_of_cell(*moon,{1,59,11});
            snapshot.vehicle = {true,11,8,{}};
            from_air = plan_snapshot(snapshot);
        });
        worker.join();
        expect(from_south.path.success && from_south.path.step_count == 1,
               "The real Moon entrance is one step north from its open side");
        expect(!from_north.path.success || from_north.path.step_count > 1,
               "The Moon route never publishes the forbidden south step from the opposite side");
        const auto& phases = from_air.path.walk_phases;
        // The other four contacts are on two plateaus with no legal landing.
        // From the logged flight origin, their first leg uses the lower
        // Western Lunar Path contact and retains the original destination.
        const Cell west_entrance{1,12,32};
        const bool direct = target == west_entrance || target == Cell{1,27,51} || target == Cell{1,55,35};
        const Cell first_entrance = direct ? target : west_entrance;
        const bool legal_air_route = from_air.path.success && phases.size() >= 2 && phases.back().cells.size() >= 2 &&
            phases.back().cells.back() == first_entrance &&
            baked_route::direction_allowed(walk,phases.back().cells[phases.back().cells.size()-2],first_entrance);
        std::printf("Moon contact (%d,%d) -> %d: Lunar Whale success %d, next map %d, phases %zu\n",
                    target.x,target.y,entrance.a,from_air.path.success,from_air.path.next_map_id,phases.size());
        expect(legal_air_route && (direct || (from_air.path.next_map_id == 291 &&
               from_air.pending.goal.map_id == 3 && from_air.pending.goal.cell == target)),
               "Lunar Whale uses a legal direct landing or enters Western Lunar Path first");
    }
    expect(entrances == 7, "All seven native Moon entrances were checked");
}

template<class Expect> void check_ff4_lunar_whale_beds(Expect expect)
{
    using namespace field;
    using namespace baked_nav;
    const Cell bed1{0,4,20}, east{0,5,20};
    for (int map_id : {161,357}) {
        const auto* room = baked::submap_by_map_id(map_id);
        if (!room) { expect(false,"Lunar Whale interior loads"); continue; }
        map_state::Flags flags;
        flags.current_map_id = map_id;
        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));
        snapshot.submap = room;
        snapshot.map_id = map_id;
        snapshot.layer = 0;
        snapshot.vehicle = {true,1,1,{}};
        snapshot.locale = L"en";
        snapshot.kind = TargetKind::Interact;
        int beds = 0;
        std::thread worker([&] {
            for (const auto& object : baked::grids(*room).objects) {
                if (!baked::stand_on_facing_north(object)) continue;
                ++beds;
                const Cell bed{object.layer,object.tx,object.ty};
                snapshot.goal = {map_id,bed,true};
                snapshot.goal.entity_id = object.entity_id();
                snapshot.goal.raw_name = baked::object_identifier(object);
                snapshot.position = world_of_cell(*room,bed);
                const auto on_bed = plan_snapshot(snapshot);
                expect(on_bed.path.success && on_bed.path.step_count == 0 && on_bed.path.walk_phases.size() == 1 &&
                       on_bed.path.walk_phases.front().cells == std::vector<Cell>{bed} &&
                       on_bed.path.walk_phases.front().arrival_direction == 0 && on_bed.path.description == L"face North",
                       "Standing on each Lunar Whale bed gives only the north facing instruction");
                if (bed != bed1) continue;
                snapshot.position = world_of_cell(*room,east);
                expect(plan_browse_snapshot(snapshot).route.direct,"The Lunar Whale bed is directly reachable from the east");
                const auto from_east = plan_snapshot(snapshot);
                expect(from_east.path.success && from_east.path.step_count == 1 && from_east.path.walk_phases.size() == 1 &&
                       from_east.path.walk_phases.front().cells == std::vector<Cell>{east,bed} &&
                       from_east.path.walk_phases.front().arrival_direction == 0,
                       "Lunar Whale Bed 1 route walks West onto its interaction tile and ends facing North");
                std::printf("Lunar Whale map %d Bed 1 directions: %ls\n",map_id,from_east.path.description.c_str());
                expect(from_east.path.description == L"1 West, face North" &&
                       from_east.path.full_description == L"1 West, face North",
                       "Both direction reads include the walk and the turn for Lunar Whale Bed 1");
            }
        });
        worker.join();
        expect(beds == 6,"All six beds in each Lunar Whale interior use the same approach");
    }
    std::puts("Lunar Whale bed approaches replayed in both interiors.");
}

template<class Expect> void check_ff4_underworld(Expect expect)
{
    using namespace field;
    using namespace field::baked_nav;
    const auto* underworld = baked::submap_by_map_id(2);
    if (!underworld || !underworld->world) {
        expect(false, "FFIV Underworld bake loads as a world map");
        return;
    }
    const Cell wall{1,108,89}, west{1,107,89}, north{1,108,88};
    const auto& grid = baked::grids(*underworld);
    baked_route::Request walking;
    walking.submap = underworld;
    expect(baked_route::open(walking, grid, west), "the west tile passes the foot transportation test");
    expect(!baked_route::edge_allowed(walking, grid, wall, west) &&
           !baked_route::edge_allowed(walking, grid, west, wall),
           "the Underworld partial wall blocks the reported west edge in both directions");
    expect(baked_route::edge_allowed(walking, grid, wall, north), "the Underworld north step remains open");
    walking.start = wall;
    walking.goals = {north};
    const auto north_route = baked_route::find(walking);
    expect(north_route.cells == std::vector<Cell>{wall, north}, "the planner still routes one step north from the wall");
    {
        map_state::Flags flags;
        flags.current_map_id=2;flags.scenario.fill(1);flags.global.fill(0);
        std::ifstream source("data/ff4/mapstate.json",std::ios::binary);
        const std::string text(std::istreambuf_iterator<char>(source),{});
        json::Value records;
        expect(json::parse(text,records),"Underworld map-state data loads");
        RouteSnapshot snapshot(map_state::Resolver(flags,records));
        snapshot.submap=underworld;snapshot.map_id=2;snapshot.layer=1;
        snapshot.position=world_of_cell(*underworld,{1,112,88});
        snapshot.vehicle={true,9,8,{}};snapshot.locale=L"en";
        snapshot.kind=TargetKind::Transition;
        int entries=0;
        for (const auto& object:grid.objects) {
            if (object.type!=baked::ObjType::GotoMap||object.a!=118) continue;
            ++entries;
            snapshot.goal={2,{object.layer,object.tx,object.ty},false,{},0,object.entity_id(),baked::object_identifier(object)};
            const auto result=plan_snapshot(snapshot);
            std::printf("Dwarven Castle Falcon entry %d (%d,%d): success %d, steps %d, %ls\n",
                        object.entity_id(),object.tx,object.ty,result.path.success,result.path.step_count,result.path.reason.c_str());
            expect(result.path.success&&result.path.reason!=L"Story gated","each Dwarven Castle entrance can use the same legal Falcon arrival");
        }
        expect(entries==4,"the Underworld bake contains all four equivalent Dwarven Castle entrances");

        const Cell parked{1,114,128};
        VehicleOnMap falcon;
        falcon.transportation_id=9;falcon.type=8;falcon.pos=world_of_cell(*underworld,parked);
        snapshot.vehicle={true,1,1,{}};
        snapshot.vehicles={falcon};snapshot.kind=TargetKind::Spot;
        snapshot.goal={2,parked,false,{},9};
        for (const Cell start : {Cell{1,113,130},Cell{1,114,130},Cell{1,114,129}}) {
            snapshot.position=world_of_cell(*underworld,start);
            const auto result=plan_snapshot(snapshot);
            std::printf("Parked Falcon from (%d,%d): success %d, steps %d, %ls\n",start.x,start.y,
                        result.path.success,result.path.step_count,result.path.reason.c_str());
            expect(result.path.success&&result.path.reason!=L"Requires vehicle"&&
                   result.path.next_map_id==(start.y==130?149:0),
                   "The parked Falcon route uses the Smithy return when its entrance blocks the direct approach");
            if(start.x!=113) continue;
            const auto* smithy=baked::submap_by_map_id(149);
            if(!smithy||!result.pending.incoming_transition) {
                expect(false,"The first leg records its exact Smithy arrival");
                continue;
            }
            const auto hop=*result.pending.incoming_transition;
            auto inside_flags=flags;
            inside_flags.current_map_id=149;
            RouteSnapshot inside(map_state::Resolver(inside_flags,records));
            inside.submap=smithy;inside.map_id=149;inside.layer=hop.arrival.layer;
            inside.position=world_of_cell(*smithy,hop.arrival);
            inside.vehicle=snapshot.vehicle;inside.locale=L"en";
            inside.goal=result.pending.goal;inside.kind=result.pending.kind;inside.pending=result.pending;
            {
                SnapshotScope scope(inside);
                note_trap_start(2,hop.entry);
                record_map_arrival(149,hop.arrival);
            }
            const auto return_route=plan_snapshot(inside);
            std::printf("Falcon return from Smithy (%d,%d) L%d: success %d, next %d, steps %d, %ls %ls\n",
                        hop.arrival.x,hop.arrival.y,hop.arrival.layer,return_route.path.success,return_route.path.next_map_id,
                        return_route.path.step_count,return_route.path.reason.c_str(),return_route.path.error.c_str());
            expect(return_route.path.success&&return_route.path.next_map_id==2&&return_route.pending.incoming_transition,
                   "Inside the Smithy, the kept Falcon route leads back to the Underworld");
            inside.pending=return_route.pending;
            expect(plan_snapshot(inside).path.success,"The kept Falcon destination can be requested again from inside the Smithy");
            inside.pending={};
            expect(!plan_snapshot(inside).path.success,"An unrelated indoor request cannot assume an off-map Falcon is parked");
            if(return_route.pending.incoming_transition) {
                auto returned=snapshot;
                returned.pending=return_route.pending;
                returned.position=world_of_cell(*underworld,return_route.pending.incoming_transition->arrival);
                const auto final_leg=plan_snapshot(returned);
                expect(final_leg.path.success&&final_leg.path.step_count==1&&final_leg.path.next_map_id==0,
                       "On return, the live Falcon model is one legal step away");
                returned.vehicles.clear();
                expect(!plan_snapshot(returned).path.success,"On return, an absent Falcon cannot use its old position");
            }
        }
    }
    auto flying = walking;
    flying.vehicle_group = 4;
    flying.vehicle_type = 3;
    flying.transportation_id = 4;
    expect(baked_route::open(flying, grid, west) && baked_route::edge_allowed(flying, grid, wall, west),
           "flight does not inherit the ground wall");

    std::ifstream stream("data/ff4/mapstate.json", std::ios::binary);
    const std::string bytes(std::istreambuf_iterator<char>(stream), {});
    json::Value records;
    if (!json::parse(bytes, records)) {
        expect(false, "FFIV Underworld initializer data loads");
        return;
    }
    map_state::Flags flags;
    flags.current_map_id = 2;
    flags.current_area_id = 2;
    flags.scenario.fill(0);
    flags.global.fill(0);
    flags.local.fill(0);
    flags.scenario[44] = 1;
    flags.scenario[45] = 1;
    flags.scenario[46] = 1;
    std::thread worker([&] {
        for (const Cell start : {Cell{1,110,89}, wall, north}) {
            for (const auto& [goal, entity] : {std::pair{Cell{1,65,26}, 40},
                     std::pair{Cell{1,64,23}, 33}, std::pair{Cell{1,61,22}, 124}}) {
                RouteSnapshot snapshot(map_state::Resolver(flags, records));
                snapshot.submap = underworld;
                snapshot.map_id = 2;
                snapshot.position = world_of_cell(*underworld, start);
                snapshot.vehicle = {true,1,1,{},1};
                snapshot.goal.map_id = 2;
                snapshot.goal.cell = goal;
                snapshot.goal.entity_id = entity;
                snapshot.locale = L"en";
                const auto began = std::chrono::steady_clock::now();
                const auto path = plan_snapshot(snapshot).path;
                const auto elapsed = std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                bool illegal_step = false;
                for (const auto& phase : path.walk_phases) {
                    for (size_t i = 1; i < phase.cells.size(); ++i)
                        if (!baked_route::direction_allowed(walking, phase.cells[i-1], phase.cells[i])) illegal_step = true;
                }
                std::printf("Underworld from (%d,%d) to (%d,%d): success %d steps %d blocked step %d %.3f ms %ls %ls\n",
                            start.x, start.y, goal.x, goal.y, path.success, path.step_count, illegal_step, elapsed,
                            path.reason.c_str(), path.error.c_str());
                expect(path.reason != L"Destination is no longer available", "the logged scene trigger is active in the replay");
                expect(!illegal_step, "the Underworld route never directs the player through a partial wall");
            }
        }
    });
    worker.join();
}

template<class Expect> void check_dwarven_castle(Expect expect)
{
    using namespace field;
    using namespace field::baked_nav;
    std::ifstream stream("data/ff4/mapstate.json",std::ios::binary);
    const std::string bytes(std::istreambuf_iterator<char>(stream),{});
    json::Value records;
    if (!json::parse(bytes,records)) {
        expect(false,"Dwarven Castle initializer data loads");
        return;
    }
    std::thread worker([&] {
        {
            const auto* base=baked::submap_by_map_id(128);
            const auto* throne=baked::submap_by_map_id(120);
            map_state::Flags flags;
            flags.current_map_id=128;flags.current_area_id=40;
            flags.scenario.fill(0);flags.local.fill(0);flags.global.fill(0);
            for (int i:{44,45,46,55}) flags.scenario[i]=1;
            RouteSnapshot snapshot(map_state::Resolver(flags,records));
            snapshot.submap=base;snapshot.map_id=128;snapshot.layer=0;
            snapshot.position=world_of_cell(*base,{0,4,15});
            snapshot.vehicle={true,1,1,{}};snapshot.locale=L"en";
            snapshot.kind=TargetKind::Transition;
            for(const auto& object:baked::grids(*throne).objects)
                if(object.type==baked::ObjType::Event&&object.entity_id()==16)
                    snapshot.goal={120,{object.layer,object.tx,object.ty},false,{},0,16,baked::object_identifier(object)};
            const auto result=plan_snapshot(snapshot);
            std::printf("Dwarven Base to throne scene: success %d next %d steps %d %ls %ls\n",result.path.success,
                        result.path.next_map_id,result.path.step_count,result.path.reason.c_str(),result.path.error.c_str());
            expect(result.path.success&&result.path.next_map_id==127,"Dwarven Base can route toward the active throne-room scene through B2");
        }
        for (int story : {0,1}) {
            for (const auto [floor,start] : {std::pair{119,Cell{0,4,7}},std::pair{119,Cell{0,4,9}},
                 std::pair{121,Cell{0,17,13}},std::pair{121,Cell{0,17,11}},std::pair{121,Cell{0,9,5}}}) {
                for (int target : {125,123}) {
                    map_state::Flags flags;
                    flags.current_map_id=floor;
                    flags.current_area_id=39;
                    flags.scenario.fill(0);
                    flags.global.fill(0);
                    flags.local.fill(0);
                    flags.scenario[46]=story;
                    RouteSnapshot snapshot(map_state::Resolver(flags,records));
                    snapshot.submap=baked::submap_by_map_id(floor);
                    snapshot.map_id=floor;
                    snapshot.layer=start.layer;
                    snapshot.position=world_of_cell(*snapshot.submap,start);
                    snapshot.vehicle={true,1,1,{}};
                    snapshot.kind=TargetKind::Interact;
                    snapshot.locale=L"en";
                    snapshot.goal={target,target==125?Cell{0,26,5}:Cell{0,9,11},true,{},0,target==125?12:5};
                    for (const auto& object : baked::grids(*snapshot.submap).objects) {
                        if (!snapshot.state.active(*snapshot.submap,object)) continue;
                        const bool npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;
                        const bool chest=object.type==baked::ObjType::TreasureBox;
                        if (!npc&&!chest) continue;
                        NativeObstacle actor;
                        actor.type=int(object.type);
                        actor.id=object.entity_id();
                        actor.cell={object.layer,object.tx,object.ty};
                        actor.npc=npc;
                        actor.chest=chest;
                        actor.solidity_known=true;
                        actor.solid=chest||(object.d&1);
                        actor.raw_name=baked::object_identifier(object);
                        snapshot.obstacles.push_back(std::move(actor));
                    }
                    auto route=plan_snapshot(snapshot);
                    std::printf("Dwarven story %d from %d (%d,%d) to %d: success %d next %d steps %d %ls %ls\n",
                                story,floor,start.x,start.y,target,route.path.success,route.path.next_map_id,route.path.step_count,
                                route.path.reason.c_str(),route.path.error.c_str());
                    if (story==0)
                        expect(!route.path.success&&route.path.reason==L"Story gated"&&!route.pending.valid,
                               "The guarded chest clears the destination without an inn/1F round trip");
                    else
                        expect(route.path.success&&route.path.next_map_id==(floor==119?121:target==123?122:125),
                               "The chest route uses the inn stairs after the native guard group is removed");
                }
            }
        }
    });
    worker.join();
}

template<class Expect> void check_tomra_shop(Expect expect)
{
    using namespace field;
    using namespace field::baked_nav;
    const auto* town=baked::submap_by_map_id(144);
    if(!town) { expect(false,"Tomra's town geometry loads");return; }
    json::Value records;
    RouteSnapshot snapshot(map_state::Resolver({},records));
    snapshot.submap=town;snapshot.map_id=144;snapshot.layer=0;
    snapshot.position=world_of_cell(*town,{0,17,11});
    snapshot.vehicle={true,1,1,{}};snapshot.locale=L"en";
    snapshot.kind=TargetKind::Transition;
    int doors=0;
    for(const auto& object:baked::grids(*town).objects) {
        if(object.type!=baked::ObjType::GotoMap||object.a!=147)continue;
        ++doors;
        snapshot.goal={144,{object.layer,object.tx,object.ty},false,{},0,object.entity_id(),baked::object_identifier(object)};
        const auto result=plan_snapshot(snapshot);
        std::printf("Tomra shop door %d (point %d): success %d, %d steps, via %d, %ls\n",object.entity_id(),object.b,
                    result.path.success,result.path.step_count,result.path.next_map_id,result.path.reason.c_str());
        expect(result.path.success&&result.path.next_map_id==0&&!result.path.walk_phases.empty()&&
               result.path.walk_phases.back().cells.back()==snapshot.goal.cell,
               "Both Tomra shop doors use their own town-side approach");
        if(object.b==2) {
            NativeObstacle dwarf;
            dwarf.type=int(baked::ObjType::NPC);dwarf.id=27;dwarf.cell={0,17,12};
            dwarf.npc=true;dwarf.solidity_known=true;dwarf.solid=true;dwarf.movement_known=true;dwarf.wanders=true;
            snapshot.obstacles.push_back(dwarf);
            const auto wait=plan_snapshot(snapshot);
            expect(wait.path.success&&wait.path.npc_approach&&wait.path.next_map_id==0,
                   "A dwarf in the alcove gives a wait at the local Tomra door route");
            snapshot.obstacles.clear();
        }
    }
    expect(doors==2,"Tomra has two separate shop arrivals");
}

template<class Expect> void check_baron_walk_open_doors(Expect expect)
{
    using namespace field;
    using namespace field::baked_nav;
    std::ifstream stream("data/ff4/mapstate.json",std::ios::binary);
    const std::string bytes(std::istreambuf_iterator<char>(stream),{});
    json::Value records;
    const auto* map=baked::submap_by_map_id(4);
    if(!map||!json::parse(bytes,records)) { expect(false,"Baron geometry and initializer load");return; }
    const auto& grid=baked::grids(*map);
    map_state::Flags flags;
    flags.current_map_id=4;flags.current_area_id=3;
    flags.scenario.fill(0);flags.local.fill(0);
    flags.scenario[280]=1; // the player is inside the front gate in the log
    // The native live actor list owns the engineers on the loaded courtyard.
    // Its presence can be newer than the initializer's saved story branch.
    std::thread worker([&] {
        auto populate_snapshot=[&](RouteSnapshot& snapshot,bool powered) {
            snapshot.map_id=4;snapshot.submap=map;snapshot.layer=0;
            snapshot.position=world_of_cell(*map,{0,20,30});
            snapshot.locale=L"en";snapshot.vehicle={true,1,1,{}};
            snapshot.kind=TargetKind::Interact;
            for(const auto& o:grid.objects) {
                if(!snapshot.state.active(*map,o)&&!(o.type==baked::ObjType::NPC&&baked::str(o.group)==L"ev_e_0055"))continue;
                const bool npc=o.type==baked::ObjType::NPC;
                const bool gate=o.type==baked::ObjType::Entity&&(o.d&1);
                const bool contact=baked_route::step_event(o);
                if(!npc&&!gate&&!contact)continue;
                NativeObstacle actor;
                actor.id=o.entity_id();actor.type=int(o.type);actor.cell={o.layer,o.tx,o.ty};
                actor.raw_name=baked::object_identifier(o);
                actor.npc=npc;actor.solidity_known=npc;actor.solid=(o.d&1)!=0;
                actor.trigger=contact&&powered;
                snapshot.obstacles.push_back(std::move(actor));
            }
        };
        std::vector<StoryTarget> targets;
        for(int id:{56,57}) {
            RouteSnapshot snapshot(map_state::Resolver(flags,records));
            populate_snapshot(snapshot,true);
            const auto o=std::find_if(grid.objects.begin(),grid.objects.end(),[&](const auto& object) {
                return object.type==baked::ObjType::NPC&&object.entity_id()==id;
            });
            if(o==grid.objects.end()) {expect(false,"the two real Baron engineers exist");return;}
            snapshot.goal={4,{o->layer,o->tx,o->ty},false,{},0,id,baked::object_identifier(*o)};
            targets.push_back({snapshot.goal, snapshot.kind});
            auto browse=plan_browse_snapshot(snapshot);
            expect(browse.route.reason!=L"Story gated"&&(browse.route.direct||browse.route.via_map_id==5),
                   "the engineer is listed as reachable through the ordinary Castle Baron door");
            const auto plan=plan_snapshot(snapshot);
            const auto& path=plan.path;
            bool used_opening=false;
            for(const auto& phase:path.walk_phases)for(size_t i=1;i<phase.cells.size();++i) {
                const auto from=phase.cells[i-1],to=phase.cells[i];
                if((to.x==19||to.x==20)&&to.layer==0&&to.y==19)
                    used_opening|=from==Cell{0,to.x,20};
            }
            std::printf("Baron engineer %d: %d steps, direct %d, next map %d, rear opening %d, %ls %ls\n",
                        id,path.step_count,browse.route.direct,path.next_map_id,used_opening,path.reason.c_str(),path.error.c_str());
            expect(path.success&&(path.next_map_id==0||path.next_map_id==5)&&!path.event_on_route&&used_opening,
                   "Castle Baron engineer directions walk onto the rear gate's native opening contact");
        }
        for(bool powered:{false,true}) {
            ExitSnapshot exits(false,map_state::Resolver(flags,records));
            populate_snapshot(exits.route,powered);
            const auto hidden=story_gate_mask(exits,targets,{});
            expect(hidden==std::vector<bool>(2,!powered),
                   "story filtering requires a powered native walk-open contact");
        }
        RouteSnapshot live(map_state::Resolver(flags,records));
        populate_snapshot(live,true);
        SnapshotScope scope(live);
        auto projected=walk_request(*map,{0,20,30});
        map_state::add_obstacles(live.state,*map,projected);
        expect(std::any_of(projected.walk_open_doors.begin(),projected.walk_open_doors.end(),[](const auto& door) {
            return door.cell==Cell{0,20,19}&&door.contact==Cell{0,20,20};
        })&&std::none_of(projected.walk_open_doors.begin(),projected.walk_open_doors.end(),[](const auto& door) {
            return door.cell==Cell{0,20,19}&&door.contact==Cell{0,20,18};
        }),"remote courtyard uses only the authored rear gate opening contacts");
    });
    worker.join();
}

template<class Expect> void check_baron_gate(Expect expect)
{
    using namespace field;
    using namespace field::baked_nav;
    std::ifstream stream("data/ff4/mapstate.json",std::ios::binary);
    const std::string bytes(std::istreambuf_iterator<char>(stream),{});
    json::Value records;
    if(!json::parse(bytes,records)) { expect(false,"Baron initializer data loads");return; }
    const auto* exterior=baked::submap_by_map_id(4);
    if(!exterior) { expect(false,"Baron courtyard bake loads");return; }
    const auto& grid=baked::grids(*exterior);
    const auto object=[&](int id)->const baked::Object*{
        for(const auto& o:grid.objects)if(o.entity_id()==id&&o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp)return &o;
        return nullptr;
    };
    std::thread worker([&]{
        for (int map:{4,9})for(int opened:{0,1}) {
            map_state::Flags flags;flags.current_map_id=map;flags.current_area_id=3;
            flags.scenario.fill(0);flags.global.fill(0);flags.local.fill(0);
            flags.scenario[280]=opened;flags.scenario[281]=opened;
            auto* gate=object(49);
            auto* exit=object(1);
            if(!gate||!exit) { expect(false,"Baron gate and exit identities load");return; }
            for (auto* target:{gate,exit}) {
                const bool to_gate=target==gate;
                RouteSnapshot snapshot(map_state::Resolver(flags,records));
                snapshot.map_id=map;snapshot.submap=baked::submap_by_map_id(map);snapshot.layer=0;
                snapshot.locale=L"en";snapshot.vehicle={true,1,1,{}};
                snapshot.position=world_of_cell(*snapshot.submap,map==4?Cell{0,16,33}:Cell{0,3,3});
                snapshot.goal={4,{target->layer,target->tx,target->ty},to_gate,{},0,target->entity_id(),baked::object_identifier(*target)};
                snapshot.kind=to_gate?TargetKind::Infer:TargetKind::Transition;
                if(map==4)for(const auto& o:grid.objects){
                    if(o.type!=baked::ObjType::Entity&&o.type!=baked::ObjType::Event)continue;
                    if((o.entity_id()<42||(o.entity_id()>50&&o.entity_id()!=142&&o.entity_id()!=143))||
                       !snapshot.state.active(*exterior,o))continue;
                    NativeObstacle seen;seen.id=o.entity_id();seen.type=int(o.type);seen.raw_name=baked::object_identifier(o);seen.cell={o.layer,o.tx,o.ty};
                    seen.trigger=o.type==baked::ObjType::Event;snapshot.obstacles.push_back(std::move(seen));
                }
                auto route=plan_snapshot(snapshot);
                if(to_gate) {
                    expect(snapshot.state.active(*exterior,*target)==!opened,
                           "Baron gate uses persistent receipt across the hall transition");
                    expect(route.path.success==!opened,"Baron gate remains available only until opened");
                } else {
                    expect(route.path.success,"Baron exit routes from both logged floors");
                    if(!opened&&route.path.next_map_id>0) {
                        expect(route.pending.valid && route.pending.goal.entity_id==1 &&
                               route.pending.goal.map_id==4 && route.pending.incoming_transition,
                               "Baron exit retains its destination through the gate approach floor change");
                    }
                }
            }
        }
    });worker.join();
}
template<class Expect> void check_baron_stair_chest(Expect expect)
{
    using namespace field;
    using namespace field::baked_nav;
    const Cell stair{0,2,6}, off_stair{0,2,5};
    const MapGoal goal{17,{0,2,5},false,{},0,9};
    auto make_snapshot = [&](int map, Cell position) {
        map_state::Flags flags;
        flags.current_map_id = map;
        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));
        snapshot.submap = baked::submap_by_map_id(map);
        snapshot.map_id = map;
        snapshot.layer = position.layer;
        snapshot.position = world_of_cell(*snapshot.submap, position);
        snapshot.locale = L"en";
        snapshot.kind = TargetKind::Interact;
        snapshot.goal = goal;
        for (const auto& object : baked::grids(*snapshot.submap).objects) {
            if (object.type != baked::ObjType::TreasureBox) continue;
            NativeObstacle actor;
            actor.cell = {object.layer,object.tx,object.ty};
            actor.type = int(object.type);
            actor.id = object.entity_id();
            actor.chest = true;
            actor.raw_name = baked::object_identifier(object);
            snapshot.obstacles.push_back(std::move(actor));
        }
        return snapshot;
    };
    std::thread worker([&] {
        for (Cell start : {Cell{0,6,7},Cell{0,4,5},Cell{0,8,5}}) {
            auto tower = make_snapshot(17,start);
            auto browse = plan_browse_snapshot(tower);
            expect(!browse.route.direct && browse.route.via_map_id == 20,
                   "Baron west chest browsing identifies the basement detour");
            auto route = plan_snapshot(tower);
            expect(route.path.success && route.path.next_map_id == 20 && !route.path.event_on_route &&
                   !route.path.required_trap && route.pending.incoming_transition &&
                   route.pending.incoming_transition->arrival == stair,
                   "Baron west chest from each logged start uses the real B1 arrival");
        }
        auto basement = make_snapshot(20,stair);
        const auto initial = plan_snapshot(basement);
        expect(initial.path.success && initial.path.next_map_id == 17 && initial.path.walk_phases.size() == 1 &&
               initial.path.walk_phases.front().cells == std::vector<Cell>{stair,off_stair,stair} &&
               initial.pending.incoming_transition && initial.pending.incoming_transition->arrival == stair,
               "Baron B1 return steps North then South to re-enter the stair");
        basement.previous_active = initial.active;
        basement.pending = initial.pending;
        const auto repeated = plan_snapshot(basement);
        expect(repeated.path.success && repeated.path.step_count == 2 && repeated.path.next_map_id == 17,
               "Baron B1 repeated directions retain the first step before leaving the stair");
        CornerSnapshot corner{initial.active,basement.submap,stair,goal.cell,20,17};
        const auto index = build_corner(corner);
        const auto* beacon = index.index.find(stair);
        expect(beacon && !beacon->arrived && beacon->steps_to_corner == 1 && beacon->steps_left == 2,
               "Baron B1 beacon points to the step off the stair");
        auto returned = make_snapshot(17,stair);
        const auto final = plan_snapshot(returned);
        expect(final.path.success && final.path.next_map_id == 0 && final.path.step_count == 0 &&
               final.path.walk_phases.size() == 1 && final.path.walk_phases.front().arrival_direction == 0,
               "Baron 1F return arrives at the chest and directs the player to face North");
        auto sibling = make_snapshot(17,{0,6,7});
        sibling.goal.cell = {0,3,5};
        sibling.goal.entity_id = 10;
        const auto sibling_route = plan_snapshot(sibling);
        expect(sibling_route.path.success && sibling_route.path.next_map_id == 0,
               "Baron neighboring chest still uses the direct same-floor route");
    });
    worker.join();
    std::puts("Baron East Tower chest, basement return, repeat and beacon replayed.");
}
}

int main() {
    using namespace field::baked_nav;
    int failures=0, direct_games=0, detours=0;
    double worst_ms=0;
    auto expect=[&](bool ok,const char* reason){if(!ok){++failures;std::printf("FAIL %s\n",reason);}};
    check_manual_readout(expect);
    std::vector<std::string> storage;storage.reserve(6);
    for(int number=1;number<=6;++number) {
        fixture::game_id=static_cast<game::Id>(number);
        std::ifstream input("data/ff"+std::to_string(number)+"/maps.bin",std::ios::binary);
        storage.emplace_back(std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>());
        expect(field::baked::load(storage.back().data(),storage.back().size()),"real game bake loads");
        for(int i=0;i<field::baked::submap_count();++i)field::baked::grids(*field::baked::submap_at(i));
        g_loaded=true;g_prepared=true;
        if(number==5) check_ff5_water_pushbacks(expect);
        if(number==4) {
            check_ff4_lunar_subterrane_exits(expect);
            check_ff4_underworld(expect);
            check_ff4_moon(expect);
            check_ff4_lunar_whale_beds(expect);
            check_ff4_sealed_cavern(expect);
            check_ff4_hovercraft_journey(expect);
            check_ff4_magnetic_cavern(expect);
            check_dwarven_castle(expect);
            check_tomra_shop(expect);
            check_baron_gate(expect);
            check_baron_walk_open_doors(expect);
            check_baron_stair_chest(expect);
            const auto* jail=field::baked::submap_by_map_id(42);
            RouteSnapshot room(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
            room.submap=jail;room.map_id=42;room.layer=0;room.position=world_of_cell(*jail,{0,6,5});room.locale=L"en";
            room.goal.map_id=42;room.goal.cell={0,5,10};room.kind=field::TargetKind::Transition;
            Planned jail_route;std::thread worker([&]{jail_route=plan_snapshot(room);});worker.join();
            expect(jail_route.path.success&&!jail_route.path.walk_phases.empty()&&jail_route.path.walk_phases.front().cells.size()>1&&
                jail_route.path.walk_phases.front().cells[1]==field::baked_route::Cell{0,6,6},"actual Damcyan planner starts South around the reported wall");
            const auto* tower=field::baked::submap_by_map_id(55);
            const Cell landing{0,5,6}, upper_stair{0,5,5}, lower_stair{0,5,7};
            auto tower_route=[&](Cell start,Cell end,int entity,field::TargetKind kind) {
                RouteSnapshot snapshot(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                snapshot.submap=tower;snapshot.map_id=55;snapshot.layer=0;snapshot.locale=L"en";
                snapshot.position=world_of_cell(*tower,start);
                snapshot.goal.map_id=55;snapshot.goal.cell=end;snapshot.goal.entity_id=entity;snapshot.kind=kind;
                for(const auto& object:field::baked::grids(*tower).objects) {
                    const bool npc=object.type==field::baked::ObjType::NPC;
                    const bool chest=object.type==field::baked::ObjType::TreasureBox;
                    if((!npc&&!chest)||object.tx<0||field::baked::str(object.group)!=L"entity_default")continue;
                    NativeObstacle actor;
                    actor.cell={object.layer,object.tx,object.ty};actor.type=int(object.type);
                    actor.id=object.entity_id();actor.npc=npc;actor.chest=chest;
                    actor.raw_name=field::baked::object_identifier(object);
                    snapshot.obstacles.push_back(std::move(actor));
                }
                Planned result;std::thread worker([&]{result=plan_snapshot(snapshot);});worker.join();
                return result.path;
            };
            for(const auto& [target,entity]:{std::pair{Cell{0,11,6},6},std::pair{Cell{0,10,4},7}}) {
                for(const Cell start:{upper_stair,landing,lower_stair}) {
                    const auto path=tower_route(start,target,entity,field::TargetKind::Interact);
                    expect(path.success&&path.next_map_id==0&&path.walk_phases.size()==1,
                           "Fabul 2F NPC stays on its floor from the landing and both arrivals");
                    if(path.walk_phases.size()!=1)continue;
                    const auto& cells=path.walk_phases.front().cells;
                    const size_t turn=start==landing?0:1;
                    expect(cells.size()>turn+1&&cells[turn]==landing&&cells[turn+1]==Cell{0,6,6},
                           "the Fabul stairs are left through the central east turn");
                    expect(std::none_of(cells.begin()+std::min<size_t>(1,cells.size()),cells.end(),
                                       [&](Cell at){return at==upper_stair||at==lower_stair;}),
                           "the rest of the Fabul 2F NPC route avoids both stair triggers");
                }
            }
            for(const auto& [stair,entity]:{std::pair{upper_stair,4},std::pair{lower_stair,2}}) {
                const auto path=tower_route(landing,stair,entity,field::TargetKind::Transition);
                expect(path.success&&path.walk_phases.size()==1&&path.walk_phases.front().cells.size()==2&&
                       path.walk_phases.front().cells.back()==stair,"the selected Fabul stair remains an intentional exit");
            }
            // Native Hovercraft2/type7, the actual Antlion entrance173. Start
            // on the nearest hovercraft-only terrain so there is a real flight
            // before landing, rather than a zero-distance label-only fixture.
            const auto* craft=field::baked::transport(2);bool checked=false;
            for(int i=0;i<field::baked::submap_count()&&!checked;++i) {
                const auto* world=field::baked::submap_at(i);if(!world->world)continue;
                for(const auto& exit:field::baked::grids(*world).objects) {
                    if(exit.type!=field::baked::ObjType::GotoMap||exit.a!=173)continue;
                    field::baked_route::Cell start{1,-1,-1};
                    const auto& grid=field::baked::grids(*world);
                    for(int radius=1;radius<=16&&start.x<0;++radius)for(int dy=-radius;dy<=radius&&start.x<0;++dy)
                        for(int dx=-radius;dx<=radius;++dx) {
                            if(std::abs(dx)+std::abs(dy)!=radius)continue;
                            const int x=exit.tx+dx,y=exit.ty+dy;
                            if(field::baked::vehicle_open(grid,craft->group,x,y)&&!field::baked::vehicle_open(grid,1,x,y)) {start={1,x,y};break;}
                        }
                    if(start.x<0)continue;
                    RouteSnapshot hover(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                    hover.submap=world;hover.map_id=world->first_map_id;hover.layer=1;hover.locale=L"en";
                    hover.position=world_of_cell(*world,start);hover.vehicle={true,2,7,L""};
                    hover.goal.map_id=hover.map_id;hover.goal.cell={1,exit.tx,exit.ty};hover.goal.entity_id=exit.entity_id();hover.kind=field::TargetKind::Transition;
                    BrowsePlanned result;std::thread flight([&]{result=plan_browse_snapshot(hover);});flight.join();
                    expect(result.route.reason.rfind(L"Fly ",0)==0&&result.route.reason.find(L"land")!=std::wstring::npos,"Antlion hovercraft browse says Fly then land");
                    std::printf("FF4 Hovercraft Antlion from %d,%d: %ls\n",start.x,start.y,result.route.reason.c_str());checked=true;break;
                }
            }
            expect(checked,"actual hovercraft-only origin and Antlion landing exercised");
        }
        // Actual route worker: distinct selected town triggers must not collapse
        // to a sibling endpoint. Exercise a real entrance in every world bake.
        int explicit_exits=0;
        for(int i=0;i<field::baked::submap_count();++i) {
            const auto* map=field::baked::submap_at(i);if(!map->world)continue;
            const auto& grid=field::baked::grids(*map);
            field::baked_route::Request walking;walking.submap=map;
            for(const auto& object:grid.objects) {
                if(object.type!=field::baked::ObjType::GotoMap)continue;
                Cell target{1,object.tx,object.ty};
                if(field::baked_route::goals_for(walking,target,field::baked_route::GoalKind::Transition).cells.empty())continue;
                for(const auto& delta:{Cell{1,0,-1},Cell{1,1,0},Cell{1,0,1},Cell{1,-1,0}}) {
                    Cell start{1,(target.x+delta.x+grid.width)%grid.width,(target.y+delta.y+grid.height)%grid.height};
                    if(!field::baked_route::open(walking,grid,start))continue;
                    // This assertion covers the exact adjacent approach. A
                    // blocked world face instead belongs to the detour cases.
                    if(map->collision_off[1] && !field::baked_route::direction_allowed(walking,start,target))continue;
                    RouteSnapshot snapshot(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                    snapshot.submap=map;snapshot.map_id=map->first_map_id;snapshot.layer=1;snapshot.position=world_of_cell(*map,start);
                    snapshot.goal.map_id=snapshot.map_id;snapshot.goal.cell=target;snapshot.kind=field::TargetKind::Transition;snapshot.locale=L"en";
                    Planned planned;std::thread worker([&]{planned=plan_snapshot(snapshot);});worker.join();
                    const bool found=planned.path.success&&!planned.path.walk_phases.empty()&&!planned.path.walk_phases.back().cells.empty();
                    if(!found)continue;
                    const auto end=planned.path.walk_phases.back().cells.back();
                    expect(end.x==target.x&&end.y==target.y,"actual all-six explicit route ends at selected exit rather than sibling");
                    ++explicit_exits;break;
                }
            }
            if(number==1) {
                // Six actual Cornelia triggers flank x145, at x144/146,y154..156.
                // The central corridor is not itself an entrance.
                int cornelia_count=0;
                for(const auto& object:grid.objects) {
                    if(object.type!=field::baked::ObjType::GotoMap||object.a!=4||
                        (object.tx!=144&&object.tx!=146)||object.ty<154||object.ty>156)continue;
                    RouteSnapshot cornelia(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                    cornelia.submap=map;cornelia.map_id=map->first_map_id;cornelia.layer=1;
                    cornelia.position=world_of_cell(*map,{1,145,object.ty});cornelia.goal.map_id=cornelia.map_id;
                    cornelia.goal.cell={1,object.tx,object.ty};cornelia.kind=field::TargetKind::Transition;cornelia.locale=L"en";
                    Planned planned;std::thread worker([&]{planned=plan_snapshot(cornelia);});worker.join();
                    expect(planned.path.success&&planned.path.step_count==1&&planned.path.description==(object.tx==146?L"1 East":L"1 West"),
                        "each Cornelia entrance one tile east/west announces the selected direction and endpoint");
                    ++cornelia_count;
                }
                expect(cornelia_count==6,"all six real Cornelia entrance triggers are replayed");

            }
        }
        expect(explicit_exits>0,"each game exercises actual worker routes to real world exits");
        std::printf("FF%d: %d world exit objectives verified through actual route worker\n",number,explicit_exits);
        bool checked=false;
        for(int i=0;i<field::baked::submap_count()&&!checked;++i) {
            const auto* sm=field::baked::submap_at(i);if(sm->world||sm->first_map_id<=0)continue;
            field::baked_route::Request movement;movement.submap=sm;
            for(int layer=0;layer<3&&!checked;++layer)for(int y=1;y<sm->height-1&&!checked;++y)for(int x=1;x<sm->width-2&&!checked;++x) {
                const Cell from{layer,x,y},to{layer,x+1,y};
                if(!field::baked_route::open(movement,field::baked::grids(*sm),from)||!field::baked_route::open(movement,field::baked::grids(*sm),to))continue;
                RouteSnapshot snapshot(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                snapshot.submap=sm;snapshot.map_id=sm->first_map_id;snapshot.layer=layer;snapshot.position=world_of_cell(*sm,from);
                snapshot.goal.map_id=sm->first_map_id;snapshot.goal.cell=to;snapshot.kind=field::TargetKind::Spot;snapshot.locale=L"en";
                BrowsePlanned result;
                std::thread worker([&] {
                    ActiveRoute active;active.valid=true;active.map_id=987;
                    Pending pending;pending.valid=true;pending.goal.map_id=876;
                    g_active=&active;g_pending=&pending;
                    const auto began=std::chrono::steady_clock::now();result=plan_browse_snapshot(snapshot);
                    worst_ms=std::max(worst_ms,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
                    expect(g_active==&active&&active.map_id==987&&g_pending==&pending&&pending.goal.map_id==876,
                        "browsing preserves active route and remembered destination on its worker");
                    g_active=nullptr;g_pending=nullptr;
                });worker.join();
                if(result.route.direct){checked=true;++direct_games;}
            }
        }
        expect(checked,"each game supplies a direct reachable browsing fixture");
        if(number!=2)continue;
        const auto* shop=field::baked::submap_by_name(L"Map_20011_11");
        expect(shop!=nullptr,"Fynn shop exists");
        if(shop) {
            RouteSnapshot snapshot(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
            snapshot.submap=shop;snapshot.map_id=shop->first_map_id;snapshot.layer=0;snapshot.position=world_of_cell(*shop,{0,8,15});
            snapshot.goal.map_id=shop->first_map_id;snapshot.goal.cell={0,8,8};snapshot.kind=field::TargetKind::Spot;snapshot.locale=L"en";
            for(int x=0;x<shop->width;++x){NativeObstacle event;event.type=2;event.trigger=true;event.cell={-1,x,12};snapshot.obstacles.push_back(event);}
            BrowsePlanned result;std::thread worker([&]{result=plan_browse_snapshot(snapshot);});worker.join();
            expect(!result.route.direct&&!result.route.via_map_id,"live event barrier suppresses steps without inventing a floor");
            snapshot.obstacles.clear();snapshot.epoch=std::make_shared<std::atomic<uint64_t>>(1);
            std::thread cancelled([&]{result=plan_browse_snapshot(snapshot);});cancelled.join();
            expect(!result.route.direct,"cancelled browse cannot claim a direct route");
        }
        // Find a disconnected Deist section using its actual entry points and
        // objects, rather than assuming every same-floor target is reachable.
        for(int floor_id=183;floor_id<=187&&!detours;++floor_id) {
            const auto* floor=field::baked::submap_by_map_id(floor_id);if(!floor)continue;
            const auto& grid=field::baked::grids(*floor);
            for(const auto& start:grid.objects) {
                if(start.type!=field::baked::ObjType::PointIn||detours)continue;
                for(const auto& object:grid.objects) {
                    if(detours)break;
                    if(object.type!=field::baked::ObjType::TreasureBox&&object.type!=field::baked::ObjType::Entity&&object.type!=field::baked::ObjType::GotoMap)continue;
                    RouteSnapshot snapshot(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                    snapshot.submap=floor;snapshot.map_id=floor_id;snapshot.layer=start.layer;snapshot.position=world_of_cell(*floor,{start.layer,start.tx,start.ty});
                    snapshot.goal.map_id=floor_id;snapshot.goal.cell={object.layer,object.tx,object.ty};snapshot.locale=L"en";
                    snapshot.kind=object.type==field::baked::ObjType::TreasureBox?field::TargetKind::Interact:field::TargetKind::Infer;
                    BrowsePlanned result;std::thread worker([&]{result=plan_browse_snapshot(snapshot);});worker.join();
                    if(result.route.via_map_id>0){expect(!result.route.direct,"real same-floor detour cannot announce steps");++detours;
                        std::printf("Deist map %d object (%d,%d), entry (%d,%d): first required map %d\n",floor_id,object.tx,object.ty,start.tx,start.ty,result.route.via_map_id);}
                }
            }
        }
    }
    expect(direct_games==6&&detours>0,"all-six direct and real Deist cross-floor detour cases exercised");
    expect(fixture::wrong_owner==0,"browse compute never resolves or reads native APIs");
    std::printf("Browse routes: %d direct games, %d real detours; worst direct compute %.3f ms; %d failures\n",direct_games,detours,worst_ms,failures);
    return failures?1:0;
}
