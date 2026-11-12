// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Production browse worker against real bakes; fake native boundaries flag worker reads.
#define main existing_route_worker_main
#include "route_worker_check.cpp"
#undef main

namespace {
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
                    if(!opened) {
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
    std::vector<std::string> storage;storage.reserve(6);
    for(int number=1;number<=6;++number) {
        fixture::game_id=static_cast<game::Id>(number);
        std::ifstream input("data/ff"+std::to_string(number)+"/maps.bin",std::ios::binary);
        storage.emplace_back(std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>());
        expect(field::baked::load(storage.back().data(),storage.back().size()),"real game bake loads");
        for(int i=0;i<field::baked::submap_count();++i)field::baked::grids(*field::baked::submap_at(i));
        g_loaded=true;g_prepared=true;
        if(number==4) {
            check_dwarven_castle(expect);
            check_baron_gate(expect);
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
