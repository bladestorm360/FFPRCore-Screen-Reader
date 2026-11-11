// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Listing-only reverse-door policy and exact Dwarven Hollows reproduction.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include <tuple>

namespace {
int failures=0;
void expect(bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}}
using namespace field;using namespace field::baked_nav;
remote_exits::Door door(int from,int to,int depth,int target_depth,int x,int y,int ax,int ay){
    remote_exits::Door out;out.source=from;out.destination=to;out.source_depth=depth;out.destination_depth=target_depth;
    out.known=true;out.footprint.type=baked::ObjType::GotoMap;out.footprint.layer=1;
    out.footprint.tx=static_cast<int16_t>(x);out.footprint.ty=static_cast<int16_t>(y);out.footprint.tw=out.footprint.th=1;
    out.arrival=baked_route::Cell{1,ax,ay};return out;
}
void policy_cases(){
    std::vector<remote_exits::Door> pair{door(0,1,0,1,10,10,20,21),door(1,0,1,0,20,20,10,11)};
    auto mask=remote_exits::return_mask(pair);
    expect(mask.size()==2&&!mask[0]&&mask[1],"only the reciprocal remote return is hidden; current forward door stays");
    auto current=std::vector{pair[1],door(1,0,1,0,23,20,10,11),door(2,0,1,0,30,30,18,11)};
    expect(remote_exits::return_mask(current)==std::vector<bool>({true,true,true}),
           "each source's same-arrival returns disappear independently without needing a reverse doorway");
    current[1].arrival->layer=0;
    expect(remote_exits::return_mask(current)==std::vector<bool>({false,false,true}),
           "different native layers retain all choices from that floor");
    current[1].arrival.reset();
    expect(remote_exits::return_mask(current)==std::vector<bool>({false,false,true}),
           "an unproved landing retains every possible return from its source");
    pair[0].source_depth=1;pair[0].destination_depth=2;pair[1].source_depth=2;pair[1].destination_depth=1;
    mask=remote_exits::return_mask(pair);expect(!mask[0]&&mask[1],"forward entrances from remote floors remain selectable");
    auto altered=pair;altered[1].destination_depth=2;expect(!remote_exits::return_mask(altered)[1],"same-depth loops remain visible");
    altered=pair;altered[1].destination_depth=-1;expect(!remote_exits::return_mask(altered)[1],"external/world or unvisited destinations remain visible");
    altered=pair;altered[1].known=false;expect(!remote_exits::return_mask(altered)[1],"unknown native state cannot certify suppression");
    altered=pair;altered[0].arrival.reset();expect(!remote_exits::return_mask(altered)[1],"missing or ambiguous native arrival metadata stays visible");
    altered=pair;altered[0].arrival->layer=0;expect(!remote_exits::return_mask(altered)[1],"different layers do not become a reciprocal doorway");
    altered=pair;altered[1].footprint.tx=30;expect(!remote_exits::return_mask(altered)[1],"another physical exit to an earlier floor remains selectable");
    altered=pair;altered[0].arrival=baked_route::Cell{1,20,18};altered[0].arrival_direction=1;
    expect(remote_exits::return_mask(altered)[1],"native straight two-cell inward arrival certifies the reciprocal return");
    altered[0].arrival_direction=4;expect(!remote_exits::return_mask(altered)[1],"outward facing two-cell arrival stays uncertain");
    altered[0].arrival_direction=0;expect(!remote_exits::return_mask(altered)[1],"missing facing never broadens the arrival radius");
    altered[0].arrival=baked_route::Cell{1,19,19};altered[0].arrival_direction=1;
    expect(!remote_exits::return_mask(altered)[1],"diagonal two-cell proximity is not an inward doorway");
    altered=pair;altered[0].arrival=baked_route::Cell{1,20,17};altered[0].arrival_direction=1;
    expect(remote_exits::return_mask(altered)[1],"native straight three-cell inward arrival certifies Kaipo-style interior returns");
    altered[0].arrival_direction=4;expect(!remote_exits::return_mask(altered)[1],"three-cell outward arrival never certifies a return");
    altered[0].arrival_direction=0;expect(!remote_exits::return_mask(altered)[1],"three-cell arrival without native facing remains uncertain");
    altered[0].arrival=baked_route::Cell{1,19,18};altered[0].arrival_direction=1;
    expect(!remote_exits::return_mask(altered)[1],"diagonal three-cell proximity stays selectable");
    altered[0].arrival=baked_route::Cell{1,20,16};
    expect(!remote_exits::return_mask(altered)[1],"larger unqualified insets are not inferred from distance");
    altered=pair;altered.erase(altered.begin());expect(!remote_exits::return_mask(altered)[0],"unpaired one-way returns and drops are preserved");
    altered=pair;altered.push_back(pair[0]);expect(!remote_exits::return_mask(altered)[1],"multiple possible reverse matches remain conservative");
    int cancelled=0;expect(remote_exits::return_mask(pair,[&]{++cancelled;return true;}).empty()&&cancelled==1,"cancelled policy work publishes no partial mask");
}
void animated_object_floors(){
    // The real game uses AnimEntity10 for both reported key-item objects.
    // Their original bakes, state predicates and routes are already present;
    // verify the actual cross-floor collector rather than a synthetic row.
    for(const auto& item:std::vector<std::tuple<int,int,int,const wchar_t*>>{
        {242,252,79,L"\u30ea\u30e5\u30fc\u30c8"},{280,290,84,L"\u571f\u306e\u7259"}}){
        const auto [first,destination,collected,raw]=item;
        const auto* sm=baked::submap_by_map_id(destination);const baked::Object* target=nullptr;
        for(const auto& o:baked::grids(*sm).objects)if(o.type==baked::ObjType::AnimEntity&&o.entity_id()==5&&baked::object_identifier(o)==raw)target=&o;
        expect(target&&(target->d&(4|8)),"reported object is a genuinely actionable animated source");if(!target)continue;
        for(int origin=first;origin<destination;++origin){
            map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.scenario[78]=1;
            flags.current_map_id=origin;flags.current_area_id=first==242?45:51;
            map_state::Resolver state(flags,fixture::records);std::vector<RemoteObject> objects;
            std::thread worker([&]{objects=connected_objects_snapshot(origin,state);});worker.join();
            int matching=0;for(const auto& row:objects)if(row.map_id==destination&&row.entity_id==5&&row.raw_name==raw){
                ++matching;expect(row.type==baked::ObjType::AnimEntity&&row.event_type==L"Interactive Object"&&
                    row.cell==Cell{target->layer,target->tx,target->ty},"remote object preserves actual native type, Objects category and exact layer/cell");
            }
            expect(matching==1,"Lute/Fang is listed exactly once from every other connected floor");
        }
        map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.scenario[78]=1;flags.scenario[collected]=1;
        map_state::Resolver consumed(flags,fixture::records);const auto absent=connected_objects_snapshot(first,consumed);
        expect(std::none_of(absent.begin(),absent.end(),[&](const auto& o){return o.map_id==destination&&o.entity_id==5&&o.raw_name==raw;}),"collected key item remains absent under its actual native flag");
        flags.scenario[collected]=0;map_state::Resolver local(flags,fixture::records);const auto native_floor=connected_objects_snapshot(destination,local);
        expect(std::none_of(native_floor.begin(),native_floor.end(),[&](const auto& o){return o.map_id==destination;}),"own floor remains native-owned without a duplicate animated object");
        if(destination==252){flags.scenario[78]=0;map_state::Resolver unavailable(flags,fixture::records);const auto before=connected_objects_snapshot(first,unavailable);
            expect(std::none_of(before.begin(),before.end(),[&](const auto& o){return o.map_id==destination&&o.entity_id==5&&o.raw_name==raw;}),"real Lute availability prerequisite is preserved");}
        std::printf("Animated object map%d: %d earlier floors, collected state, native-floor ownership checked\n",destination,destination-first);
    }
}
bool has(const std::vector<RemoteObject>& objects,int source,int destination){
    return std::any_of(objects.begin(),objects.end(),[&](const auto& row){return row.map_id==source&&row.type==baked::ObjType::GotoMap&&row.dest_map_id==destination;});
}
void dwarven(const map_state::Flags& flags){
    map_state::Resolver state(flags,fixture::records);std::vector<RemoteObject> remote;
    std::thread listing([&]{remote=connected_objects_snapshot(84,state);});listing.join();
    expect(has(remote,85,90),"Dwarven B1 still lists the forward B2-to-B3 entrance");
    expect(!has(remote,90,85),"Dwarven B1 omits the remote B3-to-B2 return from the user's log");
    expect(!has(remote,85,84),"the remote B2-to-current-B1 return is omitted");
    expect(has(remote,90,185)&&!has(remote,185,90),"Subterranean Lake entrance stays while its paired return is hidden remotely");
    expect(has(remote,85,88),"the Inn/Item Shop room entrance remains selectable remotely");
    // Its native two-cell inward PointIn now provides the same doorway proof
    // as the reported Amur and Replito shops, without changing the route graph.
    expect(!has(remote,88,85),"the Inn's verified inward arrival identifies its reciprocal return");
    int exits=0;for(const auto& row:remote)exits+=row.type==baked::ObjType::GotoMap;
    std::printf("Dwarven B1: %d remote exits (baseline17), %zu total remote objects\n",exits,remote.size());
    // The list mask never enters selected-route planning or native map data.
    for(int root:{84,90}){
        const auto* sm=baked::submap_by_map_id(root);expect(sm!=nullptr,"real Dwarven floor exists");if(!sm)continue;
        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.submap=sm;snapshot.map_id=root;snapshot.layer=1;snapshot.locale=L"en";
        snapshot.position=world_of_cell(*sm,root==84?baked_route::Cell{1,13,5}:baked_route::Cell{1,19,7});
        snapshot.kind=TargetKind::Transition;snapshot.goal.map_id=85;
        const auto* target=baked::submap_by_map_id(85);
        const auto found=std::find_if(baked::grids(*target).objects.begin(),baked::grids(*target).objects.end(),[&](const auto& row){return row.type==baked::ObjType::GotoMap&&row.a==90;});
        expect(found!=baked::grids(*target).objects.end(),"native B2-to-B3 doorway remains in the bake");if(found==baked::grids(*target).objects.end())continue;
        snapshot.goal.cell={found->layer,found->tx,found->ty};
        Planned planned;std::thread routing([&]{planned=plan_snapshot(snapshot);});routing.join();
        expect(planned.path.success&&planned.path.next_map_id==85,"both forward navigation and the physical B3 return still route through the complete graph");
    }
    map_state::Resolver from_b3(flags,fixture::records);std::vector<RemoteObject> elsewhere;
    std::thread listing_back([&]{elsewhere=connected_objects_snapshot(90,from_b3);});listing_back.join();
    expect(has(elsewhere,85,84),"changing current floor to B3 exposes the route onward toward B1");
}
void town_returns(const map_state::Flags& flags){
    for(const auto& pair:{std::pair{97,102},std::pair{138,139}}){
        const auto* town=baked::submap_by_map_id(pair.first);const auto* room=baked::submap_by_map_id(pair.second);
        expect(town&&room,"reported town/shop bakes exist");if(!town||!room)continue;
        map_state::Resolver state(flags,fixture::records);std::vector<RemoteObject> listed;
        std::thread worker([&]{listed=connected_objects_snapshot(pair.first,state);});worker.join();
        bool static_portals=true;int portals=0;
        for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::GotoMap||o.type==baked::ObjType::PointIn){
            ++portals;const auto group=baked::str(o.group);static_portals&=group.empty()||group==L"entity_default"||group==L"default";
        }
        std::printf("town%d room%d: map-known=%d/%d static-portals=%d count=%d remote-return=%d\n",pair.first,pair.second,state.known(*town),state.known(*room),static_portals,portals,has(listed,pair.second,pair.first));
        expect(!has(listed,pair.second,pair.first),"reported shop's reciprocal return is omitted before entering the room");
        bool native_return=false;for(const auto& o:baked::grids(*room).objects)native_return|=o.type==baked::ObjType::GotoMap&&o.a==pair.first;
        expect(native_return,"shop's actual return remains in the routing bake");
    }
}
void kaipo_returns(map_state::Flags flags){
    const auto* town=baked::submap_by_map_id(32);expect(town!=nullptr,"reported Kaipo32 exists");if(!town)return;
    flags.current_map_id=32;flags.local.fill(0);flags.area.fill(0);
    map_state::Resolver state(flags,fixture::records);std::vector<RemoteObject> listed;
    std::thread listing([&]{listed=connected_objects_snapshot(32,state);});listing.join();
    std::set<int> rooms;
    for(const auto& forward:baked::grids(*town).objects)if(forward.type==baked::ObjType::GotoMap) {
        const auto* room=baked::submap_by_map_id(forward.a);if(!room||room->world||room==town||!rooms.insert(forward.a).second)continue;
        std::printf("Kaipo32 -> room%d town-door%d atL%d %d,%d / arrival%d known%d/%d remote-return%d\n",forward.a,forward.entity_id(),forward.layer,forward.tx,forward.ty,forward.b,state.known(*town),state.known(*room),has(listed,forward.a,32));
        expect(!has(listed,forward.a,32),"Kaipo interior return is absent while still outside that interior");
        for(const auto& native:baked::grids(*room).objects)if(native.type==baked::ObjType::PointIn||native.type==baked::ObjType::GotoMap)
            std::printf("  portal type%d entity%d L%d %d,%d a%d b%d\n",int(native.type),native.entity_id(),native.layer,native.tx,native.ty,native.a,native.b);
        const auto arrival=std::find_if(baked::grids(*room).objects.begin(),baked::grids(*room).objects.end(),[&](const auto& o){return o.type==baked::ObjType::PointIn&&o.a==forward.b;});
        const auto exit=std::find_if(baked::grids(*room).objects.begin(),baked::grids(*room).objects.end(),[](const auto& o){return o.type==baked::ObjType::GotoMap&&o.a==32;});
        expect(arrival!=baked::grids(*room).objects.end()&&exit!=baked::grids(*room).objects.end(),"actual interior arrival and return remain in the routing bake");
        if(arrival==baked::grids(*room).objects.end()||exit==baked::grids(*room).objects.end())continue;
        flags.current_map_id=forward.a;RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=forward.a;s.submap=room;s.layer=arrival->layer;
        s.position=world_of_cell(*room,{arrival->layer,arrival->tx,arrival->ty});s.kind=TargetKind::Transition;s.locale=L"en";s.vehicle={true,1,1,{}};
        s.goal.map_id=forward.a;s.goal.cell={exit->layer,exit->tx,exit->ty};s.goal.entity_id=exit->entity_id();s.goal.raw_name=baked::object_identifier(*exit);
        Planned plan;std::thread routing([&]{plan=plan_snapshot(s);});routing.join();
        std::printf("  on-floor return success%d next%d reason[%ls]\n",plan.path.success,plan.path.next_map_id,plan.path.reason.c_str());
        // A local exit goal walks onto its native trigger; next_map_id is the
        // first cross-floor approach receipt only when the goal is remote.
        expect(plan.path.success&&!plan.path.walk_phases.empty()&&plan.path.walk_phases.back().cells.back()==Cell{exit->layer,exit->tx,exit->ty},"return remains immediately usable once the player is on its own interior floor");
    }
    expect(rooms.size()==4,"all four actual Kaipo interiors are covered");
}
void ff4_return_floors(map_state::Flags flags){
    flags.local.fill(0);flags.area.fill(0);
    for(const auto& [current,remote]:{std::pair{86,87},std::pair{101,102}}) {
        flags.current_map_id=current;
        map_state::Resolver state(flags,fixture::records);
        std::vector<RemoteObject> rows;
        std::thread listing([&]{rows=connected_objects_snapshot(current,state);});listing.join();
        expect(!has(rows,current,remote)&&!has(rows,remote,current),"Troia shop and B3 returns are absent from their exact parent floor");
        const auto* here=baked::submap_by_map_id(current);const auto* there=baked::submap_by_map_id(remote);
        const auto& origin=baked::grids(*here).objects;const auto& objects=baked::grids(*there).objects;
        const auto entrance=std::find_if(origin.begin(),origin.end(),[&](const auto& o){return o.type==baked::ObjType::GotoMap&&o.a==remote;});
        const auto exit=std::find_if(objects.begin(),objects.end(),[&](const auto& o){return o.type==baked::ObjType::GotoMap&&o.a==current;});
        expect(entrance!=origin.end()&&exit!=objects.end(),"both directions remain in the native graph");
        if(entrance==origin.end()||exit==objects.end())continue;
        const auto arrival=std::find_if(objects.begin(),objects.end(),[&](const auto& o){return o.type==baked::ObjType::PointIn&&o.a==entrance->b;});
        expect(arrival!=objects.end(),"the source floor has its real native entry point");
        if(arrival==objects.end())continue;
        flags.current_map_id=remote;
        RouteSnapshot route(map_state::Resolver(flags,fixture::records));route.map_id=remote;route.submap=there;route.layer=arrival->layer;
        route.position=world_of_cell(*there,{arrival->layer,arrival->tx,arrival->ty});route.locale=L"en";
        route.goal.map_id=remote;route.goal.cell={exit->layer,exit->tx,exit->ty};route.goal.entity_id=exit->entity_id();
        route.goal.raw_name=baked::object_identifier(*exit);route.kind=TargetKind::Transition;
        Planned plan;std::thread directions([&]{plan=plan_snapshot(route);});directions.join();
        expect(plan.path.success&&!plan.path.walk_phases.empty()&&plan.path.walk_phases.back().cells.back()==route.goal.cell,
               "the ordinary return remains directly routable from the floor that owns it");
    }
    using namespace field::filter_worker;
    flags.current_map_id=4;
    map_state::Resolver state(flags,fixture::records);
    FilterInput input;input.current_map=4;input.exits=true;
    std::thread distinct([&]{
        for(const auto& row:connected_objects_snapshot(4,state)) {
            if(row.map_id!=5||row.dest_map_id!=4||row.type!=baked::ObjType::GotoMap)continue;
            FilterRow exit;exit.index=input.rows.size();exit.kind=Kind::MapExit;exit.category=Category::MapExits;
            exit.remote_map=row.map_id;exit.destination=row.dest_map_id;input.rows.push_back(exit);
        }
        expect(input.rows.size()==3&&build(input).order.size()==3,
               "all three real landings from FFIV map5 back to map4 survive exit grouping");
    });distinct.join();
}
void ff4_world_entrances(map_state::Flags flags){
    flags.current_map_id=1;flags.scenario[43]=1;
    const auto* world=baked::submap_by_map_id(1);
    field::filter_worker::FilterInput list;list.current_map=1;list.world=true;list.reachable_only=false;
    for(const auto& [id,destination,at]:std::vector<std::tuple<int,int,Cell>>{
            {56,86,{1,40,71}}, {59,96,{1,39,69}}}) {
        RouteSnapshot route(map_state::Resolver(flags,fixture::records));route.map_id=1;route.submap=world;route.layer=1;
        route.position=world_of_cell(*world,{1,45,66});route.locale=L"en";route.vehicle={true,1,1,{},1};
        route.goal.map_id=1;route.goal.cell=at;route.goal.entity_id=id;route.kind=TargetKind::Transition;
        const auto& objects=baked::grids(*world).objects;
        const auto selected=std::find_if(objects.begin(),objects.end(),[&](const auto& o){return o.type==baked::ObjType::GotoMap&&o.entity_id()==id&&o.a==destination;});
        expect(selected!=objects.end(),"the reported world entrance is still a real native trigger");
        if(selected==objects.end())continue;
        route.goal.raw_name=baked::object_identifier(*selected);
        for(const auto& object:objects)if(object.type==baked::ObjType::GotoMap&&object.a==destination) {
            field::filter_worker::FilterRow row;row.index=list.rows.size();row.kind=Kind::MapExit;
            row.category=Category::MapExits;row.destination=destination;row.entity_id=object.entity_id();
            row.position=row.numbering_position=world_of_cell(*world,{1,object.tx,object.ty});
            list.rows.push_back(std::move(row));
        }
        Planned plan;BrowsePlanned browse;double ms=0;
        std::thread worker([&]{
            const auto start=std::chrono::steady_clock::now();plan=plan_snapshot(route);browse=plan_browse_snapshot(route);
            ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
        });worker.join();
        expect(plan.path.success&&plan.path.next_map_id==0&&browse.route.direct&&browse.route.via_map_id==0,
               "the logged Troia entrance no longer tells the player to enter Troia first");
        if(plan.path.success&&!plan.path.walk_phases.empty()) {
            const auto end=plan.path.walk_phases.back().cells.back();
            expect(end!=at&&std::any_of(objects.begin(),objects.end(),[&](const auto& o){
                return o.type==baked::ObjType::GotoMap&&o.a==selected->a&&o.b==selected->b&&o.contains(end.x,end.y);
            }),"directions terminate at an entrance with exactly the same native destination and PointIn");
            std::printf("FFIV world entrance %d: final (%d,%d), %d steps, browse via %d, %.3f ms for directions and browse\n",
                id,end.x,end.y,plan.path.step_count,browse.route.via_map_id,ms);
        }
    }
    expect(list.rows.size()>2&&field::filter_worker::build_categories(list).views[0].order.size()==list.rows.size(),
           "literal world discovery still lists every physical Troia and Troia Castle entrance");
}
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);policy_cases();
    // Every game's largest floor graph is covered, plus the exact FFIII log.
    std::vector<std::string> storage;storage.reserve(6);
    for(int number=1;number<=6;++number){
        fixture::game_id=static_cast<game::Id>(number);std::ifstream file("data/ff"+std::to_string(number)+"/maps.bin",std::ios::binary);
        storage.emplace_back(std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>());
        expect(baked::load(storage.back().data(),storage.back().size()),"real all-six bake loads");
        std::ifstream data("data/ff"+std::to_string(number)+"/mapstate.json");std::string text((std::istreambuf_iterator<char>(data)),{});
        expect(json::parse(text,fixture::records),"real all-six state programs load");
        for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));g_loaded=g_prepared=true;
        auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
        if(number==3){dwarven(*flags);town_returns(*flags);animated_object_floors();}
        if(number==4){kaipo_returns(*flags);ff4_return_floors(*flags);ff4_world_entrances(*flags);}
        int largest=0,root=0;
        for(int i=0;i<baked::submap_count();++i){
            const auto* sm=baked::submap_at(i);if(sm->world||sm->first_map_id<=0)continue;
            map_state::Resolver state(*flags,fixture::records);const auto floors=connected_floors(sm,sm->first_map_id,state);
            if(static_cast<int>(floors.size())>largest){largest=static_cast<int>(floors.size());root=sm->first_map_id;}
        }
        if(root){
            map_state::Resolver state(*flags,fixture::records);std::vector<RemoteObject> remote;
            const auto begin=std::chrono::steady_clock::now();std::thread projection([&]{remote=connected_objects_snapshot(root,state);});projection.join();
            const auto* current=baked::submap_by_map_id(root);
            expect(!remote.empty(),"largest connected graph retains real destinations");
            for(const auto& row:remote)expect(baked::submap_by_map_id(row.map_id)!=current,"current floor remains exclusively live-owned");
            std::printf("FF%d largest graph: root%d %d other floors, %zu rows, %.3f ms including worker startup/join (not native/audio timing)\n",number,root,largest,remote.size(),std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());
        }
    }
    expect(!fixture::wrong_owner,"all projection and route work uses copied data without native worker access");
    std::printf("Remote exit listing: %d failures\n",failures);return failures?1:0;
}
