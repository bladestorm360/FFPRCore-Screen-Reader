// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real worker collector and router over the preserved and final FFIII bakes.
#define main inherited_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(int argc, char** argv) {
    using namespace field; using namespace field::baked_nav;
    fixture::game_id=game::Id::FF3;
    int failures=0;
    auto expect=[&](bool ok,const char* message){if(!ok){++failures;std::printf("FAIL %s\n",message);}};
    const bool baseline=argc>1;
    std::ifstream mf(baseline?argv[1]:"data/ff3/maps.bin",std::ios::binary),sf("data/ff3/mapstate.json",std::ios::binary);
    static std::string bytes((std::istreambuf_iterator<char>(mf)),{}),states((std::istreambuf_iterator<char>(sf)),{});
    json::Value records;expect(baked::load(bytes.data(),bytes.size())&&json::parse(states,records),"load FFIII data");
    for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));
    g_loaded=true;g_prepared=true;
    const auto* pub=baked::submap_by_map_id(80);if(!pub)return 1;
    const auto& grid=baked::grids(*pub);const baked::Object* elder=nullptr;const baked::Object* girl=nullptr;int exits=0;
    for(const auto& o:grid.objects){if(o.type==baked::ObjType::NPC&&o.entity_id()==11)elder=&o;if(o.type==baked::ObjType::NPC&&o.entity_id()==12)girl=&o;if(o.type==baked::ObjType::GotoMap)++exits;}
    expect(elder&&girl&&exits==2,"pub actors and both exits retained");if(!elder||!girl)return 1;
    expect((elder->d&1)&&bool(elder->d&8)==baseline&&(girl->d&8),"colliding Elder discovery matches baseline and girl retained");
    baked_route::Request req;req.submap=pub;req.passages=true;req.ignore_hazards=true;
    auto goals=baked_route::goals_for(req,{1,7,8},baked_route::GoalKind::Interact,elder);
    req.goals=goals.cells;expect(goals.cells.size()==2,"forced Elder keeps two interaction goals");
    for(auto start:{baked_route::Cell{1,10,8},baked_route::Cell{1,10,11}}){req.start=start;expect(!baked_route::find(req).found,"forced Elder remains unreachable");}
    map_state::Resolver state(map_state::Flags{},records);std::vector<RemoteObject> remote;
    std::thread worker([&]{remote=connected_objects_snapshot(77,state);});worker.join();
    bool remote_elder=false,remote_girl=false;int remote_exits=0;
    for(const auto& row:remote)if(row.map_id==80){if(row.type==baked::ObjType::NPC&&row.entity_id==11)remote_elder=true;if(row.type==baked::ObjType::NPC&&row.entity_id==12)remote_girl=true;if(row.type==baked::ObjType::GotoMap)++remote_exits;}
    expect(remote_elder==baseline&&remote_girl,"real remote collector applies Elder discovery and retains girl");
    expect(remote_exits==2,"pub exits retained in remote collector");
    expect(!fixture::wrong_owner,"collector worker uses no native reads");
    std::printf("pub remote Elder=%d girl=%d exits=%d; forced goals=%zu; %d failures\n",remote_elder,remote_girl,remote_exits,goals.cells.size(),failures);
    return failures?1:0;
}
