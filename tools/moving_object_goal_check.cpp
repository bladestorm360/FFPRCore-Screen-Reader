// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// FFVI's moving chest has a real current instance, but an off-grid baked home.
// A native-shaped captured Entity must supply its actual interaction position.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);int failures=0;
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff6/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
    const auto* room=baked::submap_by_map_id(524);if(!room)return 2;
    const auto& objects=baked::grids(*room).objects;
    const auto source=std::find_if(objects.begin(),objects.end(),[](const auto& o){return o.type==baked::ObjType::Entity&&o.entity_id()==107;});
    if(source==objects.end()||source->ty!=-1)return 2;
    int cases=0;
    for(int x:{16,17,18})for(int mode=0;mode<8;++mode) {
        map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);flags.local.fill(0);flags.area.fill(0);
        flags.current_map_id=524;flags.scenario[444]=1;flags.local[0]=3;
        flags.area[2]=x==16?1:x==18?2:0;
        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=524;snapshot.submap=room;
        snapshot.layer=1;snapshot.position=world_of_cell(*room,{1,x,9});snapshot.vehicle={true,1,1,{}};
        snapshot.goal.map_id=524;snapshot.goal.entity_id=107;snapshot.goal.raw_name=baked::object_identifier(*source);
        snapshot.goal.cell={source->layer,source->tx,source->ty};snapshot.kind=TargetKind::Interact;snapshot.locale=L"en";
        NativeObstacle actor;actor.type=5;actor.id=107;actor.cell={1,x,8};actor.raw_name=snapshot.goal.raw_name;
        if(mode==1)actor.raw_name=L"another object";
        if(mode==2)actor.type=10;
        if(mode==3)actor.party=true;
        if(mode==4)actor.cell.layer=-1;
        if(mode==5)actor.cell.y=-1;
        if(mode!=6)snapshot.obstacles.push_back(actor);
        if(mode==7)snapshot.obstacles.push_back(actor);
        Planned result;std::thread worker([&]{result=plan_snapshot(snapshot);});worker.join();
        const bool pass=mode==0?(result.path.success&&snapshot.goal.cell==actor.cell&&result.path.step_count==0):
            (!result.path.success&&snapshot.goal.cell==Cell{source->layer,source->tx,source->ty});
        if(!pass)++failures;
        ++cases;std::printf("Moving chest captured%d,8 mode%d: success%d goal%d,%d steps%d pass%d\n",x,mode,result.path.success,snapshot.goal.cell.x,snapshot.goal.cell.y,result.path.step_count,pass);
    }
    // Opened ordinary chests remain navigable under the established policy.
    // Their treasure-opened bit is distinct from a retired script action.
    const auto chest=std::find_if(objects.begin(),objects.end(),[](const auto& o){return o.type==baked::ObjType::TreasureBox&&o.entity_id()==8;});
    if(chest==objects.end())return 2;
    {
        map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);flags.local.fill(0);flags.area.fill(0);
        flags.current_map_id=524;flags.treasure[199]=1;
        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=524;snapshot.submap=room;
        snapshot.layer=1;snapshot.position=world_of_cell(*room,{1,18,9});snapshot.vehicle={true,1,1,{}};
        snapshot.goal.map_id=524;snapshot.goal.entity_id=8;snapshot.goal.raw_name=baked::object_identifier(*chest);
        snapshot.goal.cell={chest->layer,chest->tx,chest->ty};snapshot.kind=TargetKind::Interact;snapshot.locale=L"en";
        NativeObstacle actor;actor.type=7;actor.chest=true;actor.id=8;actor.cell={1,18,8};actor.raw_name=snapshot.goal.raw_name;snapshot.obstacles.push_back(actor);
        Planned result;std::thread worker([&]{result=plan_snapshot(snapshot);});worker.join();
        const bool pass=snapshot.state.treasure_opened(*room,*chest)==true&&result.path.success&&snapshot.goal.cell==actor.cell;
        if(!pass)++failures;++cases;std::printf("Opened displaced ordinary chest remains navigable: pass%d\n",pass);
    }
    {
        map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);flags.local.fill(0);flags.area.fill(0);
        flags.current_map_id=524;flags.scenario[443]=flags.scenario[444]=1;
        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=524;snapshot.submap=room;
        snapshot.layer=1;snapshot.position=world_of_cell(*room,{1,17,9});snapshot.vehicle={true,1,1,{}};
        snapshot.goal.map_id=524;snapshot.goal.entity_id=107;snapshot.goal.raw_name=baked::object_identifier(*source);
        snapshot.goal.cell={source->layer,source->tx,source->ty};snapshot.kind=TargetKind::Interact;snapshot.locale=L"en";
        NativeObstacle actor;actor.type=5;actor.id=107;actor.cell={1,17,8};actor.raw_name=snapshot.goal.raw_name;snapshot.obstacles.push_back(actor);
        const bool consumed=snapshot.state.consumed(*room,*source);
        Planned result;std::thread worker([&]{result=plan_snapshot(snapshot);});worker.join();
        const bool pass=consumed&&!result.path.success&&snapshot.goal.cell==Cell{source->layer,source->tx,source->ty};
        if(!pass)++failures;++cases;std::printf("Retired moving-chest action: native-consumed%d pass%d\n",consumed,pass);
    }
    int games=0;
    for(int number=1;number<=6;++number) {
        const auto base="data/ff"+std::to_string(number)+"/";
        const auto maps=read((base+"maps.bin").c_str());
        if(!baked::load(maps.data(),maps.size())||!json::parse(read((base+"mapstate.json").c_str()),fixture::records))return 2;
        fixture::game_id=static_cast<game::Id>(number);bool found=false;
        for(size_t i=0;i<baked::submap_count()&&!found;++i) {
            const auto* map=baked::submap_at(i);if(!map||map->world||map->first_map_id<1)continue;
            for(const auto& o:baked::grids(*map).objects) {
                if(found)break;
                if(o.type!=baked::ObjType::Entity||o.tw!=1||o.th!=1||o.entity_id()<1||o.tx<0||o.ty<0)continue;
                const auto raw=baked::object_identifier(o);if(raw.empty())continue;
                const Cell start{o.layer,o.tx,o.ty+1};baked_route::Request request;request.submap=map;
                if(!baked_route::open(request,baked::grids(*map),start))continue;
                map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);flags.local.fill(0);flags.area.fill(0);
                flags.current_map_id=map->first_map_id;
                RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=map->first_map_id;snapshot.submap=map;
                snapshot.layer=start.layer;snapshot.position=world_of_cell(*map,start);snapshot.vehicle={true,1,1,{}};
                snapshot.goal.map_id=snapshot.map_id;snapshot.goal.entity_id=o.entity_id();snapshot.goal.raw_name=raw;
                snapshot.goal.cell={o.layer,o.tx,o.ty};snapshot.kind=TargetKind::Interact;snapshot.locale=L"en";
                NativeObstacle native;native.type=5;native.id=o.entity_id();native.cell=snapshot.goal.cell;native.raw_name=raw;snapshot.obstacles.push_back(native);
                Planned result;std::thread worker([&]{result=plan_snapshot(snapshot);});worker.join();
                if(!result.path.success)continue;
                found=true;++games;
                std::printf("FF%d stationary native object map%d entity%d: legal approach preserved\n",number,snapshot.map_id,o.entity_id());
            }
        }
        if(!found)++failures;
    }
    std::printf("Moving-object actual consumer: %d pose/identity cases, %d stationary game witnesses, %d failures, owner%d\n",cases,games,failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
