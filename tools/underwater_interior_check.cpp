// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Follow real FFIII underwater entry arcs to their interior, through the same
// selected-route and browse consumers as the published destination request.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff3/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
    map_state::Flags flags;flags.scenario.fill(1);flags.global.fill(1);flags.treasure.fill(1);
    flags.current_map_id=3;flags.current_area_id=1;
    map_state::Resolver state(flags,fixture::records);
    const auto* water=baked::submap_by_map_id(3);
    int entries=0,cases=0,failures=0;
    const auto expect=[&](bool pass,const char* message){if(!pass){++failures;std::printf("FAIL %s\n",message);}};
    const auto observe=[](RouteSnapshot& snapshot) {
        snapshot.obstacles.clear();
        for(const auto& actor:baked::grids(*snapshot.submap).objects) {
            const auto* air=air_transitions::match(3,snapshot.map_id,actor);
            if(!air&&!baked::world_exit(actor))continue;
            NativeObstacle seen;seen.type=air||(actor.d&255)==1?2:5;
            seen.id=actor.entity_id();seen.cell={-1,actor.tx,actor.ty};seen.raw_name=baked::object_identifier(actor);
            seen.trigger=air||(actor.d&255)==1;
            seen.transportation_ids=air?air->mask:snapshot.state.transportation_mask(*snapshot.submap,actor);
            seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);
            seen.transportation_complete=true;snapshot.obstacles.push_back(std::move(seen));
        }
    };
    // A continued interior destination may survive a filter change. Capture
    // the ready native departure values rather than silently lose the craft
    // needed on the next world because the final goal is not itself a world.
    {
        RouteSnapshot capture(map_state::Resolver(flags,fixture::records));
        capture.map_id=1;capture.submap=baked::submap_by_map_id(1);capture.goal.map_id=242;capture.vehicle_shortcuts=true;
        fixture::filter_mode=ui::prefs::kFilterCurrentMap;
        fixture::saved_models={{9,4,{151,74,0}}};
        const int before=fixture::saved_reads;capture_vehicle_names(capture);
        expect(fixture::saved_reads==before+1&&capture.saved_vehicles.size()==1,
            "remote interior capture retains real off-map craft even outside World map filter");
        capture.saved_vehicles.clear();capture.goal.map_id=1;
        const int same=fixture::saved_reads;capture_vehicle_names(capture);
        expect(fixture::saved_reads==same,"same-map ordinary capture does not add departure-save reads");
        fixture::filter_mode=-1;fixture::saved_models.clear();
    }
    std::vector<std::pair<const baked::Submap*,const baked::Object*>> doors;
    for(const auto& object:baked::grids(*water).objects)
        if(baked::world_exit(object))doors.push_back({water,&object});
    const auto* surface=baked::submap_by_map_id(4);
    for(const auto& object:baked::grids(*surface).objects)
        if(object.type==baked::ObjType::GotoMap&&((object.tx==128&&object.ty==184)||(object.tx==150&&object.ty==203)))
            doors.push_back({surface,&object});
    for(const auto& entry:doors) {
        const auto& door=*entry.second;
        if(!state.active(*entry.first,door)||state.consumed(*entry.first,door))continue;
        const auto* room=baked::submap_by_map_id(door.a);if(!room||room->world)continue;
        const auto* arrival=point_in(*room,door.b,state);if(!arrival)return 2;
        ++entries;
        for(int origin:{3,4,1}) {
            auto context=flags;context.current_map_id=origin;
            RouteSnapshot snapshot(map_state::Resolver(context,fixture::records));
            snapshot.map_id=origin;snapshot.submap=baked::submap_by_map_id(origin);snapshot.layer=1;snapshot.locale=L"en";
            snapshot.vehicle_shortcuts=true;
            const int carrier=origin==3?14:origin==4?9:11;
            snapshot.vehicle={true,carrier,baked::transport(carrier)->type,{}};
            snapshot.position=world_of_cell(*snapshot.submap,origin==1?Cell{1,64,64}:Cell{1,113,118});
            snapshot.saved_vehicles={{9,4,{151,74,0}}};snapshot.vehicle_names[9]=L"Nautilus";
            observe(snapshot);
            snapshot.goal.map_id=door.a;snapshot.goal.cell={arrival->layer,arrival->tx,arrival->ty};
            snapshot.kind=TargetKind::Transition;
            Planned route;BrowsePlanned browse;
            std::thread worker([&]{route=plan_snapshot(snapshot);browse=plan_browse_snapshot(snapshot);});worker.join();++cases;
            const bool pass=route.path.success&&(!(origin==3&&entry.first==water)||route.path.next_map_id==door.a)&&
                browse.route.via_map_id==route.path.next_map_id;
            std::printf("Entry%d,%d -> interior%d/%d,%d L%d from%d carrier%d: route%d next%d browse%d pass%d reason[%ls]\n",
                door.tx,door.ty,door.a,arrival->tx,arrival->ty,arrival->layer,origin,carrier,
                route.path.success,route.path.next_map_id,browse.route.via_map_id,pass,route.path.reason.c_str());
            expect(pass,"native underwater entrance admits its interior from every qualified world origin");
            const bool village=entry.first==surface&&Cell{1,door.tx,door.ty}==nautilus_surface::village_cell;
            if(origin==4&&(entry.first==water||village)&&route.path.success) {
                auto repeated=snapshot;repeated.goal=route.goal;repeated.previous_active=route.active;
                repeated.pending=route.pending;repeated.active={};
                if(!route.path.walk_phases.empty()&&route.path.walk_phases.front().cells.size()>1)
                    repeated.position=world_of_cell(*repeated.submap,route.path.walk_phases.front().cells[1]);
                Planned again;std::thread moving([&]{again=plan_snapshot(repeated);});moving.join();
                const auto& receipt=again.pending.incoming_transition;
                expect(again.path.success&&again.goal.map_id==door.a&&again.path.next_map_id==3&&receipt&&
                    receipt->entity_id==-300020&&receipt->destination==3,
                    "moving repeated interior request retains its final goal and native dive receipt");
                Planned continuation=again;
                for(int hop=0;hop<(village?2:1);++hop) {
                    if(!continuation.pending.incoming_transition) {expect(false,"continued world leg retains an arrival receipt");break;}
                    const auto arrival=*continuation.pending.incoming_transition;
                    auto arrived=snapshot;arrived.pending=continuation.pending;arrived.active={};arrived.previous_active.reset();
                    {
                        SnapshotScope scope(arrived);
                        note_trap_start(arrival.map_id,arrival.entry);
                        record_map_arrival(arrival.destination,arrival.arrival);
                    }
                    arrived.map_id=arrival.destination;arrived.submap=baked::submap_by_map_id(arrived.map_id);arrived.layer=1;
                    arrived.position=world_of_cell(*arrived.submap,arrival.arrival);
                    const int id=arrived.map_id==3?14:9;arrived.vehicle={true,id,baked::transport(id)->type,{}};
                    observe(arrived);
                    std::thread next([&]{continuation=plan_snapshot(arrived);});next.join();
                    const int expected=village&&hop==0?4:door.a;
                    std::printf("  Continued native world arrival%d: route%d next%d expected%d\n",arrived.map_id,
                        continuation.path.success,continuation.path.next_map_id,expected);
                    expect(continuation.path.success&&continuation.path.next_map_id==expected&&continuation.goal.map_id==door.a,
                        "dive/surface arrival continues to the final interior without another world loop");
                }
            }
            if(origin==1) {
                auto missing=snapshot;missing.saved_vehicles.clear();missing.pending={};missing.previous_active.reset();missing.active={};
                Planned unavailable;std::thread locked([&]{unavailable=plan_snapshot(missing);});locked.join();
                expect(!unavailable.path.success,"Invincible cannot invent the upgraded Nautilus on the next world");
            }
            auto cancelled=snapshot;cancelled.cancelled=[] {return true;};
            Planned retired;std::thread obsolete([&]{retired=plan_snapshot(cancelled);});obsolete.join();
            expect(!retired.path.success,"cancelled interior request never publishes a usable first leg");
        }
    }
    expect(entries==6&&cases==18,"all four real underwater arcs and both Doga interiors tested from all three world origins");
    expect(fixture::wrong_owner==0,"interior route/browse makes no native calls on workers");
    std::printf("Underwater interiors: %d arcs, %d route/browse pairs, %d failures, owner%d\n",entries,cases,failures,fixture::wrong_owner.load());
    return failures?1:0;
}
