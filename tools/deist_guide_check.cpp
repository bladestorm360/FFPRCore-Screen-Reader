// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Complete Deist guide journeys through the actual route worker. Native manual
// operations remain manual; this fixture applies only their reviewed receipts.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
namespace {
int failures=0,episodes=0,legs=0,pitfalls=0,moving=0;
void episode(const char* name,int map,field::baked_route::Cell at,int completed,bool learned) {
    using namespace field;using namespace baked_nav;
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);
    flags.scenario[40]=1; // actual Deist travel state: Leila ship event0044_3 already completed
    for(int i=41;i<=completed&&i<=46;++i)flags.scenario[i]=1;
    json::Value puzzles;std::ifstream input("data/ff2/puzzles.json");json::parse(std::string(std::istreambuf_iterator<char>(input),{}),puzzles);
    bool finished=true;
    for(const auto& step:puzzles.get(L"puzzles")->arr[0].get(L"steps")->arr) {
        const auto* word=step.get(L"done_keyword");const auto* bit=step.get(L"done_scenario");
        const int receipt=bit?static_cast<int>(bit->n):-1;
        if((word&&learned)||(receipt>=0&&flags.scenario[receipt]==1))continue;
        MapGoal goal;goal.map_id=static_cast<int>(step.get(L"map")->n);goal.entity_id=static_cast<int>(step.get(L"entity")->n);
        goal.raw_name=step.get(L"raw")->s;goal.cell={static_cast<int>(step.get(L"layer")->n),static_cast<int>(step.get(L"x")->n),static_cast<int>(step.get(L"y")->n)};
        goal.examine=true;goal.allow_world_transit=step.get(L"allow_world_transit")->b;
        Pending pending;bool reached=false;
        for(int hop=0;hop<64;++hop) {
            flags.current_map_id=map;
            RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=map;snapshot.submap=baked::submap_by_map_id(map);
            snapshot.layer=at.layer;snapshot.position=world_of_cell(*snapshot.submap,at);snapshot.vehicle={true,1,1,{}};
            snapshot.locale=L"en";snapshot.goal=goal;snapshot.kind=TargetKind::Interact;snapshot.pending=pending;
            for(const auto& original:baked::grids(*snapshot.submap).objects) {
                if(original.type==baked::ObjType::PointIn||original.type==baked::ObjType::GotoMap||original.type==baked::ObjType::Warp)continue;
                if(!snapshot.state.active(*snapshot.submap,original))continue;
                auto actor=snapshot.state.effective(*snapshot.submap,original);
                // Captured post-first-conversation live pose. The second talk
                // hides this actor; no pointer is transferred into this worker.
                if(map==176&&actor.entity_id()==32&&flags.scenario[41]==1&&flags.scenario[42]!=1){actor.ty=14;++moving;}
                NativeObstacle seen;seen.id=actor.entity_id();seen.type=static_cast<int>(actor.type);seen.raw_name=baked::object_identifier(original);
                seen.cell={actor.layer,actor.tx,actor.ty};seen.enabled=seen.applies=true;seen.trigger=baked_route::step_event(actor);
                seen.chest=actor.type==baked::ObjType::TreasureBox;seen.npc=actor.type==baked::ObjType::NPC||actor.type==baked::ObjType::ShopNPC;
                seen.collider=actor.type==baked::ObjType::CollisionEntity;seen.collision=actor.a;
                seen.transportation_ids=snapshot.state.transportation_mask(*snapshot.submap,original);seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);
                snapshot.obstacles.push_back(std::move(seen));
            }
            Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();++legs;
            if(!route.path.success||route.path.walk_phases.empty()) {
                std::printf("FAIL %s receipt%d word%d from%d L%d %d,%d to%d/%d: %ls / %ls\n",name,receipt,word!=nullptr,map,at.layer,at.x,at.y,goal.map_id,goal.entity_id,route.path.reason.c_str(),route.path.error.c_str());break;
            }
            if(map==goal.map_id){at=route.path.walk_phases.back().cells.back();reached=true;break;}
            if(route.pending.incoming_trap&&map==185){++pitfalls;std::printf("FAIL %s guide uses the avoidable B3 pitfall\n",name);break;}
            const auto arrival=route.pending.incoming_transition?route.pending.incoming_transition:route.pending.incoming_trap;
            if(!arrival){std::printf("FAIL %s missing actual arrival from%d next%d\n",name,map,route.path.next_map_id);break;}
            pending=route.pending;
            {SnapshotScope scope(snapshot);snapshot.pending=pending;note_trap_start(arrival->map_id,arrival->entry);record_map_arrival(arrival->destination,arrival->arrival);pending=snapshot.pending;}
            map=arrival->destination;at=arrival->arrival;flags.local.fill(0);
        }
        if(!reached){finished=false;break;}
        if(word)learned=true;
        else flags.scenario[receipt]=1;
        if(receipt==43)flags.treasure[235]=1;
        if(receipt==44)flags.treasure[503]=1;
    }
    std::printf("Deist %s: complete%d final%d eggPlaced%d\n",name,finished,map,flags.scenario[46].value_or(-1));
    if(finished&&flags.scenario[46]==1)++episodes;else ++failures;
}
}
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff2/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff2/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF2;g_loaded=g_prepared=true;
    episode("fresh castle",176,{0,16,30},40,false);
    episode("fresh cave",183,{0,12,6},40,false);
    episode("moving boy",176,{0,16,24},41,false);
    episode("word learned",179,{0,21,19},42,true);
    episode("pendant acquired",184,{0,33,16},43,true);
    episode("egg acquired",180,{0,9,19},44,true);
    episode("B3 resume",185,{0,15,27},44,true);
    episode("guardian defeated",187,{1,9,8},45,true);
    episode("already placed",187,{1,25,9},46,true);
    std::printf("Deist journey: %d complete episodes/%d production legs, pitfalls%d moving captures%d failures%d owner%d\n",episodes,legs,pitfalls,moving,failures,fixture::wrong_owner.load());
    return failures||pitfalls||fixture::wrong_owner?1:0;
}
