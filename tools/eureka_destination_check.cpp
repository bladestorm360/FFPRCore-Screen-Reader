// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact Eureka physical destinations through the actual provider and planner.
#define FFPR_ROUTE_FIXTURE_ONLY
#define main retained_eureka_route_main
#include "route_worker_check.cpp"
#undef main
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff3/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
    auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
    // Native sc_e_0090 and sc_e_0258 record the two used keys. A completely
    // zero bank is the closed-door case, not an entered endgame tower.
    flags->scenario[94]=1;flags->scenario[178]=1;
    std::vector<RemoteObject> weapons;
    map_state::Resolver provider(*flags,fixture::records);
    const auto objects=connected_objects_snapshot(305,provider);
    for(const auto& row:objects)if(row.type==baked::ObjType::AnimEntity&&row.map_id>=315&&row.map_id<=324)weapons.push_back(row);
    int failures=0,remote_paths=0,local_paths=0,used_guards=0;
    const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
    expect(weapons.size()==5,"actual Tower provider lists all five guarded Eureka weapon interactions");
    const auto plan=[&](int origin,const RemoteObject& weapon,const map_state::Flags& input,bool wrong_name=false) {
        const auto* room=baked::submap_by_map_id(origin);
        auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(input,fixture::records));
        snapshot->map_id=origin;snapshot->submap=room;snapshot->locale=L"en";snapshot->kind=TargetKind::Interact;
        snapshot->goal.map_id=weapon.map_id;snapshot->goal.cell=weapon.cell;snapshot->goal.entity_id=weapon.entity_id;
        snapshot->goal.raw_name=wrong_name?L"wrong identity":weapon.raw_name;snapshot->goal.examine=true;
        const auto& grid=baked::grids(*room);const baked::Object* start=nullptr;
        for(const auto& o:grid.objects)if(o.type==baked::ObjType::PointIn&&(!start||o.a==1)){start=&o;if(o.a==1)break;}
        if(!start){expect(false,"actual starting floor has a native arrival");return Planned{};}
        snapshot->layer=start->layer;snapshot->position=world_of_cell(*room,{start->layer,start->tx,start->ty});snapshot->vehicle={true,1,1,{}};
        if(origin==weapon.map_id) {
            NativeObstacle actor;actor.id=weapon.entity_id;actor.type=int(weapon.type);actor.raw_name=weapon.raw_name;
            actor.cell=weapon.cell;actor.enabled=true;actor.solidity_known=true;actor.solid=true;snapshot->obstacles.push_back(actor);
        }
        Planned output;std::thread worker([&]{output=plan_snapshot(*snapshot);});worker.join();return output;
    };
    for(const auto& weapon:weapons) {
        auto locked=*flags;locked.scenario[94]=0;locked.scenario[178]=0;
        expect(plan(305,weapon,locked).path.reason==L"Story gated","closed native key gates remain closed instead of granting a false through-route");
        for(int origin:{305,315,316,317,318,319,320,321,322,323,324}) {
            auto state=*flags;state.current_map_id=origin;
            if(const auto* record=fixture::records.get(baked::name_of(*baked::submap_by_map_id(origin))))
                if(const auto* area=record->get(L"area_id"))state.current_area_id=int(area->n);
            const auto route=plan(origin,weapon,state);
            std::printf("Eureka origin%d target%d:%d success%d next%d reason%s\n",origin,weapon.map_id,weapon.entity_id,route.path.success,route.path.next_map_id,text::to_utf8(route.path.reason).c_str());
            expect(route.path.success,"every real weapon remains routable from Tower entry and every Eureka floor");
            if(origin==weapon.map_id)++local_paths;else ++remote_paths;
        }
        expect(plan(weapon.map_id,weapon,*flags,true).path.reason==L"Destination is no longer available","a mismatched raw identity remains unavailable even beside a live weapon");
        const auto* room=baked::submap_by_map_id(weapon.map_id);
        const auto* record=fixture::records.get(baked::name_of(*room));const auto* events=record?record->get(L"events"):nullptr;
        std::set<std::pair<int,int>> bits;
        const auto collect=[&](const json::Value* code) {
            if(!code)return;
            for(const auto& instruction:code->arr)if(instruction.arr.size()>=3&&instruction.arr[0].n==2) {
                const int bank=int(instruction.arr[1].n),index=int(instruction.arr[2].n);
                if(bank>=0&&bank<8&&index>=0&&index<256)bits.insert({bank/4,(bank%4)*256+index});
            }
        };
        collect(record?record->get(L"code"):nullptr);
        if(events)for(const auto& event:events->arr) {
            const auto *id=event.get(L"id"),*code=event.get(L"code");if(!id||int(id->n)!=weapon.entity_id||!code)continue;
            collect(code);
        }
        bool guarded=false;
        for(const auto [bank,bit]:bits) {
            auto used=*flags;if(bank==0)used.scenario[size_t(bit)]=1;else used.treasure[size_t(bit)]=1;
            map_state::Resolver observed(used,fixture::records);
            for(const auto& object:baked::grids(*room).objects)if(object.type==weapon.type&&object.entity_id()==weapon.entity_id&&baked::object_identifier(object)==weapon.raw_name)
                if(!observed.active(*room,object)||observed.consumed(*room,object)) {
                    expect(plan(305,weapon,used).path.reason==L"Destination is no longer available","actual native defeated/collected state still retires the remote weapon");
                    std::printf("Native retirement target%d:%d bank%d bit%d\n",weapon.map_id,weapon.entity_id,bank,bit);guarded=true;
                }
        }
        expect(guarded,"each weapon has an actual native consumption/visibility guard witness");used_guards+=guarded;
    }
    // Independently reconcile actual binary types; never substitute a point or
    // warp identity for an interactive animated actor.
    int identities=0;
    for(int i=0;i<baked::submap_count();++i) {
        const auto* room=baked::submap_at(i);
        for(const auto& original:baked::grids(*room).objects) {
            const auto raw=baked::object_identifier(original);
            if(room->first_map_id<315||room->first_map_id>324||original.type!=baked::ObjType::AnimEntity||!(original.d&(4|8)))continue;
            ++identities;
        }
    }
    expect(identities==5&&fixture::wrong_owner==0,"all actual weapon identities and worker-only native ownership are preserved");
    std::printf("Eureka: %d remote paths,%d local paths,%d native collected guards,%d failures\n",remote_paths,local_paths,used_guards,failures);return failures?1:0;
}
