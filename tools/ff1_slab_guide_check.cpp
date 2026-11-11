// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// A complete Rod guide needs a real walking journey through the world, while
// ordinary indoor requests retain their no-speculative-world-detour policy.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "ordinary_interaction_fixtures.h"
namespace {
int episodes=0,legs=0,failures=0;
void replay(int map,field::baked_route::Cell at,int complete) {
    using namespace field;using namespace baked_nav;
    map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);flags.treasure.fill(0);
    for(int i=16;i<=complete&&i<=20;++i)flags.scenario[i]=1;
    json::Value puzzles;std::ifstream f("data/ff1/puzzles.json");
    if(!json::parse(std::string(std::istreambuf_iterator<char>(f),{}),puzzles)||
       !append_ordinary_interactions(puzzles,1)){++failures;return;}
    const auto& recipes=puzzles.get(L"puzzles")->arr;
    const std::wstring_view wanted=map==93?L"citadel_of_trials":map==115?L"lute_slab":L"earth_rod_slab";
    const auto recipe=std::find_if(recipes.begin(),recipes.end(),[wanted](const auto& row){return row.get(L"id")->s==wanted;});
    if(recipe==recipes.end()){++failures;return;}
    bool done=true;
    for(const auto& step:recipe->get(L"steps")->arr) {
        const auto* done_flag=step.get(L"done_scenario");const int receipt=done_flag?static_cast<int>(done_flag->n):-1;
        if(receipt>=0&&flags.scenario[receipt]==1)continue;
        MapGoal goal;goal.map_id=static_cast<int>(step.get(L"map")->n);goal.entity_id=step.get(L"entity")?static_cast<int>(step.get(L"entity")->n):-1;
        goal.raw_name=step.get(L"raw")?step.get(L"raw")->s:L"";goal.cell={static_cast<int>(step.get(L"layer")->n),static_cast<int>(step.get(L"x")->n),static_cast<int>(step.get(L"y")->n)};
        goal.examine=step.get(L"examine")->b;goal.allow_world_transit=step.get(L"allow_world_transit")&&step.get(L"allow_world_transit")->b;
        Pending pending;bool reached=false;
        for(int hop=0;hop<64;++hop) {
            if(step.get(L"reach_map")&&step.get(L"reach_map")->b&&map==goal.map_id){reached=true;break;}
            flags.current_map_id=map;
            RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=map;snapshot.submap=baked::submap_by_map_id(map);
            snapshot.layer=at.layer;snapshot.position=world_of_cell(*snapshot.submap,at);snapshot.vehicle={true,1,1,{}};
            snapshot.locale=L"en";snapshot.goal=goal;snapshot.kind=goal.examine?TargetKind::Interact:TargetKind::Spot;snapshot.pending=pending;
            for(const auto& original:baked::grids(*snapshot.submap).objects) {
                if(original.type==baked::ObjType::PointIn||original.type==baked::ObjType::GotoMap||original.type==baked::ObjType::Warp||!snapshot.state.active(*snapshot.submap,original))continue;
                const auto actor=snapshot.state.effective(*snapshot.submap,original);
                NativeObstacle seen;seen.id=actor.entity_id();seen.type=static_cast<int>(actor.type);seen.raw_name=baked::object_identifier(original);
                seen.cell={actor.layer,actor.tx,actor.ty};seen.enabled=seen.applies=true;seen.trigger=baked_route::step_event(actor);
                seen.chest=actor.type==baked::ObjType::TreasureBox;seen.npc=actor.type==baked::ObjType::NPC||actor.type==baked::ObjType::ShopNPC;
                seen.collider=actor.type==baked::ObjType::CollisionEntity;seen.collision=actor.a;
                seen.transportation_ids=snapshot.state.transportation_mask(*snapshot.submap,original);seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);
                snapshot.obstacles.push_back(std::move(seen));
            }
            Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();++legs;
            if(!route.path.success||route.path.walk_phases.empty()) {
                std::printf("FAIL slab receipt%d from%d L%d %d,%d to%d/%d: %ls / %ls\n",receipt,map,at.layer,at.x,at.y,goal.map_id,goal.entity_id,route.path.reason.c_str(),route.path.error.c_str());break;
            }
            if(map==goal.map_id) {at=route.path.walk_phases.back().cells.back();reached=true;break;}
            const auto arrival=route.pending.incoming_transition?route.pending.incoming_transition:route.pending.incoming_trap;
            if(!arrival){std::printf("FAIL slab missing native arrival from%d next%d\n",map,route.path.next_map_id);break;}
            pending=route.pending;
            {SnapshotScope scope(snapshot);snapshot.pending=pending;note_trap_start(arrival->map_id,arrival->entry);record_map_arrival(arrival->destination,arrival->arrival);pending=snapshot.pending;}
            map=arrival->destination;at=arrival->arrival;flags.local.fill(0);
        }
        if(!reached){done=false;break;}
        if(receipt>=0)flags.scenario[receipt]=1;
        if(receipt==17)flags.treasure[508]=1;
        if(receipt==19)flags.treasure[253]=1;
        // The victory continuation relocates the player; the chest is a distinct
        // next operation. Never infer acquisition from the battle flag alone.
        // SetPos operand2 is a one-based layer request. Native SetPos AE4AC0
        // forwards it to MoveEntityLayerWithWorldPosition AE0C20, which sets
        // Unity layer request+8. Request1 is baked Bottom0, not Ground1.
        if(receipt==16)at={0,31,40};
    }
    std::printf("Slab episode start%d completed-through%d: complete%d\n",map,complete,done);
    if(done)++episodes;else ++failures;
}
}
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff1/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF1;g_loaded=g_prepared=true;
    map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);flags.treasure.fill(0);
    flags.current_map_id=78;flags.scenario[16]=flags.scenario[17]=1;
    const auto* earth=baked::submap_by_map_id(78);const auto* giant=baked::submap_by_map_id(81);const auto* world=baked::submap_by_map_id(1);
    map_state::Resolver state(flags,fixture::records);
    const baked::Object* actor=nullptr;
    for(const auto& object:baked::grids(*giant).objects)if(object.entity_id()==11&&object.type==baked::ObjType::NPC)actor=&object;
    if(!actor)return 2;
    MapGoal goal;goal.map_id=81;goal.cell={actor->layer,actor->tx,actor->ty};goal.entity_id=11;goal.raw_name=baked::object_identifier(*actor);goal.examine=true;
    RouteSnapshot source(map_state::Resolver(flags,fixture::records));source.map_id=78;source.submap=earth;
    source.layer=2;source.position=world_of_cell(*earth,{2,33,46});source.vehicle={true,1,1,{}};source.locale=L"en";
    baked_route::Request request;request.submap=earth;request.transportation_id=1;map_state::add_obstacles(state,*earth,request,true,false,true);
    for(const auto& cell:request.blocked){NativeObstacle seen;seen.type=19;seen.cell=cell;seen.collider=true;seen.collision=1;seen.enabled=true;source.obstacles.push_back(seen);}
    MapGoal outside;outside.map_id=1;
    const auto* first=baked::submap_by_map_id(76);
    for(const auto& object:baked::grids(*first).objects)if(object.type==baked::ObjType::GotoMap&&object.a==1)
        if(const auto* arrival=point_in(*world,object.b,state))outside.cell={1,arrival->tx,arrival->ty};
    CrossLeg whole,ordinary,exit,approach;
    std::thread worker([&]{
        SnapshotScope scope(source);ordinary=find_across(nullptr,nullptr,goal,true);
        goal.allow_world_transit=true;whole=find_across(nullptr,nullptr,goal,true);exit=find_across(nullptr,nullptr,outside,true);
    });worker.join();
    auto surface=source;surface.map_id=1;surface.submap=world;surface.layer=1;surface.position=world_of_cell(*world,outside.cell);surface.obstacles.clear();
    std::thread next([&]{SnapshotScope scope(surface);approach=find_across(nullptr,nullptr,goal,true);});next.join();
    std::printf("Rod prerequisite cave78 -> Giant81: authored%d ordinary%d; real cave exit%d, world approach%d; owner%d\n",whole.found,ordinary.found,exit.found,approach.found,fixture::wrong_owner.load());
    source.goal=goal;source.kind=TargetKind::Interact;
    Planned authored;std::thread first_request([&]{authored=plan_snapshot(source);});first_request.join();
    auto ordinary_after=source;ordinary_after.previous_active=authored.active;ordinary_after.pending=authored.pending;ordinary_after.goal.allow_world_transit=false;
    Planned ordinary_result;std::thread policy_change([&]{ordinary_result=plan_snapshot(ordinary_after);});policy_change.join();
    std::printf("Guide-to-ordinary same-target policy switch: authored%d ordinary%d\n",authored.path.success,ordinary_result.path.success);
    if(!authored.path.success||ordinary_result.path.success)++failures;
    for(int completed=15;completed<=19;++completed)replay(78,{2,33,46},completed);
    replay(81,{2,18,19},17);replay(40,{2,30,22},18);replay(115,{2,10,7},15);replay(115,{0,38,26},15);
    replay(93,{2,23,33},15);
    source.goal=goal;source.kind=TargetKind::Interact;source.cancelled=[]{return true;};
    Planned retired;std::thread obsolete([&]{retired=plan_snapshot(source);});obsolete.join();
    std::printf("Slab guides: %d episodes/%d production legs, failures%d, cancelled%d, owner%d\n",episodes,legs,failures,!retired.path.success,fixture::wrong_owner.load());
    return whole.found&&!ordinary.found&&exit.found&&approach.found&&!failures&&!retired.path.success&&fixture::wrong_owner==0?0:1;
}
