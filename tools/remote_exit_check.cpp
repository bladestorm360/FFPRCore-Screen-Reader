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
    pair[0].source_depth=1;pair[0].destination_depth=2;pair[1].source_depth=2;pair[1].destination_depth=1;
    mask=remote_exits::return_mask(pair);expect(!mask[0]&&mask[1],"forward entrances from remote floors remain selectable");
    auto altered=pair;altered[1].destination_depth=2;expect(!remote_exits::return_mask(altered)[1],"same-depth loops remain visible");
    altered=pair;altered[1].destination_depth=-1;expect(!remote_exits::return_mask(altered)[1],"external/world or unvisited destinations remain visible");
    altered=pair;altered[1].known=false;expect(!remote_exits::return_mask(altered)[1],"unknown native state cannot certify suppression");
    altered=pair;altered[0].arrival.reset();expect(!remote_exits::return_mask(altered)[1],"missing or ambiguous native arrival metadata stays visible");
    altered=pair;altered[0].arrival->layer=0;expect(!remote_exits::return_mask(altered)[1],"different layers do not become a reciprocal doorway");
    altered=pair;altered[1].footprint.tx=30;expect(!remote_exits::return_mask(altered)[1],"another physical exit to an earlier floor remains selectable");
    altered=pair;altered.erase(altered.begin());expect(!remote_exits::return_mask(altered)[0],"unpaired one-way returns and drops are preserved");
    altered=pair;altered.push_back(pair[0]);expect(!remote_exits::return_mask(altered)[1],"multiple possible reverse matches remain conservative");
    int cancelled=0;expect(remote_exits::return_mask(pair,[&]{++cancelled;return true;}).empty()&&cancelled==1,"cancelled policy work publishes no partial mask");
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
    // This actual room spawns two cells away from its return footprint. That
    // exceeds the policy's direct doorway witness, so keep the uncertain row
    // instead of weakening the preservation rule to make the list shorter.
    expect(has(remote,88,85),"the Inn's non-adjacent arrival remains conservatively listed");
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
        if(number==3)dwarven(*flags);
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
