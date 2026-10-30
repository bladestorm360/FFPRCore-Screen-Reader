// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual browse/filter workers, real all-six bakes, no native engine access.
#define main existing_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include "../src/core/field/navigation_labels.h"
#include <set>

int main() {
    using namespace field;
    using namespace field::baked_nav;
    using namespace field::filter_worker;
    int failures=0,total=0,ff6_maps=0,ff6_events=0,deist=0;
    auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    const auto read=[](const std::string& path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    double worst=0;
    std::vector<std::string> storage;storage.reserve(6);
    for(int n=1;n<=6;++n) {
        fixture::game_id=static_cast<game::Id>(n);storage.push_back(read("data/ff"+std::to_string(n)+"/maps.bin"));
        expect(baked::load(storage.back().data(),storage.back().size()),"actual game bake loads");
        expect(json::parse(read("data/ff"+std::to_string(n)+"/mapstate.json"),fixture::records),"matching event guards load");
        for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));
        g_loaded=true;g_prepared=true;
        bool same_name_checked=false;
        int tested=0,eligible=0,skipped=0,doors=0;
        std::thread worker([&] {
            for(int i=0;i<baked::submap_count();++i) {
                const auto* sm=baked::submap_at(i);if(n==6)++ff6_maps;
                const auto& grid=baked::grids(*sm);
                // Every bake is inspected, including unwalkable/script-only maps.
                // A locally enterable selected trigger must agree with the full
                // planner, even when its native event identity is numbered/reused.
                if(sm->first_map_id>0)for(const auto& o:grid.objects) {
                    if(o.type!=baked::ObjType::Event && o.type!=baked::ObjType::RandomEvent)continue;
                    if(!baked_route::step_event(o)||baked::is_trap(o))continue;
                    ++eligible;if(n==6)++ff6_events;
                    baked_route::Request movement;movement.submap=sm;movement.live_hazards=true;
                    Cell target{sm->world?1:o.layer,o.tx,o.ty};bool checked=false;
                    for(const auto& delta:{Cell{0,0,-1},Cell{0,1,0},Cell{0,0,1},Cell{0,-1,0}}) {
                        Cell from{target.layer,target.x+delta.x,target.y+delta.y};
                        if(!baked_route::open(movement,grid,from))continue;
                        RouteSnapshot snapshot(map_state::Resolver(map_state::Flags{},fixture::records));
                        snapshot.submap=sm;snapshot.map_id=sm->first_map_id;snapshot.layer=target.layer;
                        snapshot.position=world_of_cell(*sm,from);snapshot.goal.map_id=snapshot.map_id;snapshot.goal.cell=target;
                        snapshot.kind=TargetKind::Infer;snapshot.locale=L"en";
                        NativeObstacle event;event.type=static_cast<int>(o.type);event.id=o.entity_id();event.cell=target;event.trigger=true;
                        event.raw_name=baked::object_identifier(o);snapshot.obstacles={event};
                        const auto began=std::chrono::steady_clock::now();
                        const auto full=plan_snapshot(snapshot);
                        if(!full.path.success||full.path.next_map_id||full.path.required_trap||full.path.event_on_route)continue;
                        const auto browse=plan_browse_snapshot(snapshot);
                        const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                        worst=std::max(worst,ms);
                        if(!browse.route.direct)std::printf("FF%d map %d event %d (%d,%d) full route succeeds but browse fails\n",n,sm->first_map_id,o.entity_id(),o.tx,o.ty);
                        expect(browse.route.direct,"selected active scene trigger retains usable route evidence");
                        checked=true;++tested;break;
                    }
                    if(!checked)++skipped;
                }
                if(sm->first_map_id<=0)continue;
                FilterInput entrances;entrances.current_map=sm->first_map_id;entrances.entrance_suffixes=true;
                for(const auto& o:grid.objects)if(o.type==baked::ObjType::GotoMap && o.a>0) {
                    FilterRow row;row.index=entrances.rows.size();row.kind=Kind::MapExit;row.category=Category::MapExits;
                    row.position=world_of_cell(*sm,{o.layer,o.tx,o.ty});row.name=L"Floor";row.destination=o.a;row.cell_layer=o.layer;entrances.rows.push_back(row);
                }
                const auto original=entrances.rows;
                if(!same_name_checked){
                    std::map<int,int> counts;for(const auto& row:original)++counts[row.destination];
                    FilterInput example;example.current_map=sm->first_map_id;example.entrance_suffixes=true;
                    for(const auto& row:original)if(counts[row.destination]==1){example.rows.push_back(row);if(example.rows.size()==2)break;}
                    if(example.rows.size()==2){
                        for(auto& row:example.rows)row.name=L"Exit to Coliseum";
                        auto unmapped=example;
                        expect(prepare_navigation_labels(example),"same-name exits to distinct native map IDs prepare");
                        expect(example.rows[0].name!=example.rows[1].name&&example.rows[0].name.find(L" (")!=std::wstring::npos&&example.rows[1].name.find(L" (")!=std::wstring::npos,
                            "distinct destination IDs with same displayed name receive different letters in each game");
                        for(auto& row:unmapped.rows){row.destination=0;row.name=L"Exit";}
                        expect(prepare_navigation_labels(unmapped)&&unmapped.rows[0].name!=unmapped.rows[1].name,
                            "observed door/stair exits without destination IDs also receive distinct letters");
                        auto aliases=example;for(auto& row:aliases.rows)row.name=L"First exit";
                        aliases.rows[1].name=L"Second exit";
                        expect(prepare_navigation_labels(aliases)&&aliases.rows[0].name==L"First exit"&&aliases.rows[1].name==L"Second exit",
                            "only confirmed exact displayed names can create a suffix");
                        auto unavailable=example;for(auto& row:unavailable.rows)row.name=L"Confirmed exit";
                        unavailable.rows[1].available=false;
                        expect(prepare_navigation_labels(unavailable)&&unavailable.rows[0].name==L"Confirmed exit",
                            "unavailable rows do not confirm another same-name doorway");
                        same_name_checked=true;
                    }
                }

                expect(prepare_navigation_labels(entrances),"real entrance label projection completes");
                std::map<std::tuple<int,int,int,int>,std::wstring> labels;
                for(const auto& row:entrances.rows) {
                    const auto x=static_cast<int>(std::floor(sm->width*.5+row.position.x/16)),y=static_cast<int>(std::floor(sm->height*.5-row.position.y/16));
                    labels[{row.destination,row.cell_layer,x,y}]=row.name;
                    if(row.name!=L"Floor"){++doors;if(n==2 && sm->first_map_id>=183 && sm->first_map_id<=187)++deist;}
                }
                entrances.rows=original;std::reverse(entrances.rows.begin(),entrances.rows.end());entrances.player={999,-888,0};
                expect(prepare_navigation_labels(entrances),"reverse-order entrance projection completes");
                for(const auto& row:entrances.rows) {
                    const auto x=static_cast<int>(std::floor(sm->width*.5+row.position.x/16)),y=static_cast<int>(std::floor(sm->height*.5-row.position.y/16));
                    expect(labels[{row.destination,row.cell_layer,x,y}]==row.name,"entrance letters are independent of list order/player position");
                }
            }
        });worker.join();
        total+=tested;expect(tested>0&&doors>0,"each game exercises selected events and multiple floor entrances");
        expect(same_name_checked,"all six games exercise general same-name and unknown-destination exit lettering");
        std::printf("FF%d: %d/%d event objects replayed; %d no direct local fixture; %d labelled entrance rows\n",n,tested,eligible,skipped,doors);
        if(n==3) {
            const auto* falgabard=baked::submap_by_map_id(144);expect(falgabard!=nullptr,"real Falgabard source exists");
            if(falgabard){
                FilterInput house;house.current_map=144;house.entrance_suffixes=true;
                for(const auto& o:baked::grids(*falgabard).objects)if(o.type==baked::ObjType::GotoMap&&o.a==150){
                    FilterRow row;row.index=house.rows.size();row.kind=Kind::MapExit;row.category=Category::MapExits;
                    row.position=world_of_cell(*falgabard,{o.layer,o.tx,o.ty});row.name=L"Exit to Falgabard House";
                    row.destination=o.a;row.cell_layer=o.layer;house.rows.push_back(row);
                }
                expect(house.rows.size()==2&&house.rows[0].cell_layer!=house.rows[1].cell_layer,"two native layer records reproduce the single Falgabard doorway");
                auto single=house;single.rows.resize(1);
                expect(prepare_navigation_labels(single)&&single.rows[0].name==L"Exit to Falgabard House","one confirmed House has no letter despite unobserved bake siblings");
                expect(prepare_navigation_labels(house)&&std::all_of(house.rows.begin(),house.rows.end(),[](const auto& row){return row.name==L"Exit to Falgabard House";}),
                    "overlapping source layers to the same point count as one doorway");
            }
            const auto* world=baked::submap_by_map_id(1);
            FilterInput events;events.current_map=1;events.world=true;events.category=Category::Events;
            for(const auto& o:baked::grids(*world).objects) {
                if(o.type!=baked::ObjType::Event)continue;
                const auto group=baked::str(o.group);
                if(group!=L"entity_default"&&group!=L"ev_e_0024")continue;
                FilterRow row;row.index=events.rows.size();row.kind=Kind::Event;row.category=Category::Events;
                row.raw_name=baked::object_identifier(o);row.entity_id=o.entity_id();row.event_type=L"Event";
                row.name=L"World map transition";row.position=world_of_cell(*world,{o.layer,o.tx,o.ty});row.cell_layer=o.layer;
                events.rows.push_back(std::move(row));
            }
            expect(events.rows.size()==1535,"real FFIII world snapshot reproduces the user's1535 event tiles");
            expect(build_categories(events).views[static_cast<size_t>(Category::Events)].order.size()==1535,"grouping off preserves all distinct native tiles");
            events.events=true;
            const auto grouped=build_categories(events);
            expect(grouped.views[static_cast<size_t>(Category::Events)].order.size()==8,"grouping collapses1524 border copies to3 families plus5 other native events");
            expect(grouped.audio_order.size()==1535,"soundscape physical grouping stays independent from the pathfinder toggle");
        }
        if(n!=6)continue;
        const auto* narshe=baked::submap_by_map_id(3);expect(narshe!=nullptr,"Narshe exterior exists");if(!narshe)continue;
        RouteSnapshot intro(map_state::Resolver(map_state::Flags{},fixture::records));
        intro.submap=narshe;intro.map_id=3;intro.layer=1;intro.position=world_of_cell(*narshe,{1,38,49});
        intro.goal.map_id=3;intro.goal.cell={1,38,50};intro.kind=TargetKind::Infer;intro.locale=L"en";
        FilterInput repeated;repeated.events=true;repeated.current_map=3;repeated.sort=true;repeated.player=intro.position;
        int gates=0;
        for(const auto& o:baked::grids(*narshe).objects) {
            if(o.type==baked::ObjType::CollisionEntity && (baked::str(o.group)==L"ev_e_0003" || baked::str(o.group)==L"entity_default")) {
                NativeObstacle gate;gate.type=19;gate.id=o.entity_id();gate.cell={o.layer,o.tx,o.ty};gate.collider=true;gate.collision=o.a;
                intro.obstacles.push_back(gate);++gates;
            }
            if(o.type==baked::ObjType::Event) {
                FilterRow row;row.index=repeated.rows.size();row.kind=Kind::Event;row.category=Category::Events;
                row.name=L"Scene trigger";row.raw_name=baked::object_identifier(o);row.entity_id=o.entity_id();row.event_type=L"Event";
                row.position=world_of_cell(*narshe,{o.layer,o.tx,o.ty});row.route_distance=static_cast<int>(row.index)+5;repeated.rows.push_back(row);
                if(o.tx==38&&o.ty==50) {
                    NativeObstacle event;event.type=2;event.id=o.entity_id();event.cell={o.layer,o.tx,o.ty};event.trigger=true;
                    intro.obstacles.push_back(event);
                }
            }
        }
        expect(gates==119,"real Narshe intro snapshot retains all119 native gates");
        std::thread intro_worker([&] {
            const auto browse=plan_browse_snapshot(intro);
            expect(browse.route.direct,"Narshe's one-step scene trigger is actionable with all119 gates intact");
            ExitSnapshot capture(false,map_state::Resolver(map_state::Flags{},fixture::records));
            capture.route.submap=narshe;capture.route.map_id=3;capture.route.layer=1;
            capture.route.position=intro.position;capture.route.obstacles=intro.obstacles;
            auto distances=exit_distances(capture,{{world_of_cell(*narshe,{1,38,50}),0,0,{},TargetKind::Infer}},{});
            expect(distances.size()==1&&distances[0]==1,"production grouping flood may enter the selected active event's goal edge");
            NativeObstacle barrier;barrier.type=2;barrier.cell={1,38,45};barrier.trigger=true;
            capture.route.obstacles.push_back(barrier);
            distances=exit_distances(capture,{{world_of_cell(*narshe,{1,38,17}),0,0,{},TargetKind::Infer}},{});
            expect(distances.size()==1&&distances[0]<0,"grouping cannot rank a trigger behind an earlier live event as directly reachable");
            intro.goal.cell={1,38,17};intro.obstacles.push_back(barrier);
            expect(!plan_browse_snapshot(intro).route.direct,"an earlier event still prevents event-free browsing distance");
            intro.obstacles.pop_back();
            intro.goal.cell={1,33,54};intro.kind=TargetKind::Transition;
            const auto blocked=plan_browse_snapshot(intro);
            std::printf("Narshe closed exit: direct%d reason%ls\n",blocked.route.direct,blocked.route.reason.c_str());
            expect(!blocked.route.direct&&blocked.route.reason==L"Story gated",
                "a real gate never recommends an unrelated nearby scene as progression");
            intro.obstacles.erase(std::remove_if(intro.obstacles.begin(),intro.obstacles.end(),[](const auto& o){return o.trigger;}),intro.obstacles.end());
            const auto no_event=plan_browse_snapshot(intro);
            std::printf("Narshe closed exit without scene: direct%d reason%ls\n",no_event.route.direct,no_event.route.reason.c_str());
            expect(!no_event.route.direct&&no_event.route.reason==L"Story gated","missing reachable trigger never invents a progression route");
            expect(prepare_navigation_labels(repeated),"Narshe native event grouping completes");
            std::vector<size_t> edge;
            for(size_t i=0;i<repeated.rows.size();++i)if(!repeated.rows[i].event_group.empty()) {
                const auto& o=repeated.rows[i];
                for(const auto& object:baked::grids(*narshe).objects)
                    if(object.type==baked::ObjType::Event && object.a==1180 && object.entity_id()==o.entity_id &&
                        baked::object_identifier(object)==o.raw_name) {edge.push_back(i);break;}
            }
            expect(edge.size()==29,"all29 Narshe edge-event tiles are present before grouping");
            if(edge.size()>1) {
                for(auto i:edge)repeated.rows[i].route_distance=-1;
                repeated.rows[edge.back()].route_distance=80;
                const auto grouped=build(repeated);
                int count=0;for(auto i:edge)count+=std::find(grouped.order.begin(),grouped.order.end(),i)!=grouped.order.end();
                expect(count==1&&std::find(grouped.order.begin(),grouped.order.end(),edge.back())!=grouped.order.end(),
                    "repeated native event keeps the reachable tile rather than the closest blocked sibling");
                expect(grouped.order.size()>1,"distinct native events with the same Scene trigger label stay separate");
                auto& kept=repeated.rows[edge.front()];kept.cell_valid=true;kept.cell_x=43;kept.cell_y=63;
                repeated.continued_goal=true;repeated.continued_map=3;repeated.continued_x=43;repeated.continued_y=63;
                repeated.continued_entity_id=kept.entity_id;repeated.continued_raw=kept.raw_name;
                const auto continued=build(repeated);
                expect(continued.continued>=0&&continued.order[continued.continued]==edge.front(),
                    "group refresh preserves an explicit remembered native objective instead of substituting its sibling");
                repeated.continued_goal=false;
                repeated.events=false;
                expect(build(repeated).order.size()==repeated.rows.size(),"event grouping off restores every native event row");
            }
        });intro_worker.join();
    }
    expect(deist>0,"real Deist multiple floor entrances receive suffixes");
    expect(fixture::wrong_owner==0,"all computation uses owned snapshots with zero native worker calls");
    std::printf("Navigation groups: %d event routes; all %d FF6 bakes inspected (%d event objects); %d Deist labels; worst paired compute %.3fms; %d failures\n",total,ff6_maps,ff6_events,deist,worst,failures);
    return failures?1:0;
}
