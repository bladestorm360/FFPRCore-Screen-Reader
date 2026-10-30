// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual owned-snapshot Castle Fynn/NPC routing; no native game execution.
#define main existing_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include "../src/core/fieldnav/autowalk_route.h"

namespace {
using namespace field;using namespace baked_nav;
int failures=0;
void expect(bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}}
std::string read(const char* path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
std::unique_ptr<RouteSnapshot> castle(int map,Cell start,bool opened) {
    auto flags=std::make_unique<map_state::Flags>();
    flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
    flags->scenario[50]=flags->scenario[51]=flags->scenario[52]=1;
    flags->scenario[56]=opened?1:0;flags->current_map_id=map;flags->current_area_id=16;
    auto out=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
    out->submap=baked::submap_by_map_id(map);out->map_id=map;out->layer=0;out->locale=L"en";
    out->position=world_of_cell(*out->submap,start);
    // Capture only physical objects actually present in this chosen native
    // initializer state. Future floors remain production Resolver projections.
    for(const auto& original:baked::grids(*out->submap).objects) {
        if(!out->state.active(*out->submap,original))continue;
        const auto object=out->state.effective(*out->submap,original);
        const bool npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;
        const bool chest=object.type==baked::ObjType::TreasureBox;
        const bool gate=(object.type==baked::ObjType::Entity||object.type==baked::ObjType::AnimEntity)&&(object.d&1);
        if(!npc&&!chest&&!gate)continue;
        NativeObstacle actor;actor.cell={object.layer,object.tx,object.ty};actor.id=object.entity_id();
        actor.type=static_cast<int>(object.type);actor.npc=npc;actor.chest=chest;actor.raw_name=baked::object_identifier(object);
        out->obstacles.push_back(std::move(actor));
    }
    return out;
}
MapGoal chest_goal(int map,int x,int y) {
    MapGoal goal;goal.map_id=map;goal.cell={0,x,y};
    for(const auto& object:baked::grids(*baked::submap_by_map_id(map)).objects)
        if(object.type==baked::ObjType::TreasureBox&&object.contains(x,y)){
            goal.entity_id=object.entity_id();goal.raw_name=baked::object_identifier(object);break;
        }
    return goal;
}
}
// FFIII's logged Molten Cave obstruction is a local rock-operated door, not
// missing story progression after the horns are stolen. Replay the exact floor
// through production routing, retaining both physical blockers until flag199.
void molten_cave() {
    using namespace field;using namespace baked_nav;
    const auto bytes=read("data/ff3/maps.bin");
    expect(baked::load(bytes.data(),bytes.size()),"real FFIII bake loads for Molten Cave review");
    expect(json::parse(read("data/ff3/mapstate.json"),fixture::records),"real Molten Cave initializer projection loads");
    fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
    for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));
    const auto* floor=baked::submap_by_map_id(191);expect(floor!=nullptr,"logged Molten Cave B3 exists");if(!floor)return;
    for(bool opened:{false,true}){
        map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);
        flags.current_map_id=191;flags.current_area_id=24;flags.scenario[199]=opened?1:0;
        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));
        snapshot.submap=floor;snapshot.map_id=191;snapshot.layer=1;snapshot.locale=L"en";
        snapshot.position=world_of_cell(*floor,{1,8,25});
        const baked::Object* rock=nullptr;int gates=0;
        for(const auto& original:baked::grids(*floor).objects){
            if(original.entity_id()==9&&original.type==baked::ObjType::Entity)rock=&original;
            if(!snapshot.state.active(*floor,original,true))continue;
            const auto object=snapshot.state.effective(*floor,original);
            const bool chest=object.type==baked::ObjType::TreasureBox;
            const bool gate=object.type==baked::ObjType::Entity&&(object.d&1);
            if(!chest&&!gate)continue;
            NativeObstacle obstacle;obstacle.cell={object.layer,object.tx,object.ty};obstacle.id=object.entity_id();
            obstacle.type=static_cast<int>(object.type);obstacle.chest=chest;obstacle.raw_name=baked::object_identifier(object);
            snapshot.obstacles.push_back(obstacle);gates+=gate;
        }
        expect(rock&&gates==(opened?0:2),"flag199 retires the rock control and hidden-door collider together");
        if(!opened&&rock){
            snapshot.goal.map_id=191;snapshot.goal.cell={rock->layer,rock->tx,rock->ty};
            snapshot.goal.entity_id=9;snapshot.goal.raw_name=baked::object_identifier(*rock);snapshot.kind=TargetKind::Infer;
            Planned result;std::thread worker([&]{result=plan_snapshot(snapshot);});worker.join();
            expect(result.path.success&&result.path.step_count>0,"the real Rock object is reachable from the exact logged player cell before opening the door");
            std::printf("Molten Cave closed door, Rock approach: success%d steps%d %ls\n",result.path.success,result.path.step_count,result.path.description.c_str());
        }
        snapshot.goal={};snapshot.goal.map_id=191;snapshot.goal.cell={1,20,6};snapshot.kind=TargetKind::Transition;
        Planned local;std::thread worker([&]{local=plan_snapshot(snapshot);});worker.join();
        expect(local.path.success==opened,"exact logged B3 passage is blocked only until the local rock switch is used");
        if(!opened)expect(local.path.reason==L"Story gated","logged generic gate wording is reproduced by the physical hidden-door collider");
        snapshot.goal={};snapshot.goal.map_id=192;snapshot.goal.cell={1,11,16};snapshot.kind=TargetKind::Transition;
        Planned onward;std::thread further([&]{onward=plan_snapshot(snapshot);});further.join();
        expect(onward.path.success==opened,"the onward Molten Cave route opens with the native rock flag, without another story requirement");
        std::printf("Molten Cave flag199=%d, local exit success%d, onward success%d reason=%ls\n",opened,local.path.success,onward.path.success,onward.path.reason.c_str());
    }
}

int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    static const auto bytes=read("data/ff2/maps.bin");expect(baked::load(bytes.data(),bytes.size()),"real FFII maps load");
    expect(json::parse(read("data/ff2/mapstate.json"),fixture::records),"real FFII state programs load");
    fixture::game_id=game::Id::FF2;g_loaded=g_prepared=true;
    for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));
    // The log gives seven actors' positions; the eighth remains at its authored
    // far-east patrol home. This is a bounded reconstruction, not a live capture.
    auto snapshot=castle(31,{0,19,19},false);snapshot->goal.map_id=31;
    snapshot->goal.cell={0,12,27};snapshot->kind=TargetKind::Transition;
    for(auto& actor:snapshot->obstacles) {
        if(actor.id==89)actor.cell={0,18,20};
        if(actor.id==90)actor.cell={0,24,23};
    }
    Planned planned;
    std::thread worker([&]{planned=plan_snapshot(*snapshot);});worker.join();
    std::printf("Fynn reconstructed soldiers: success%d steps%d reason=%ls\n",planned.path.success,planned.path.step_count,planned.path.reason.c_str());

    expect(planned.path.success&&!planned.path.npc_approach,"known soldier positions retain a complete legal detour");
    // A controlled live obstruction on the real floor. The same production
    // planner must prefer detours and offer interaction only after full failure.
    auto wall=castle(31,{0,19,19},false);wall->goal.map_id=31;wall->goal.cell={0,12,27};wall->kind=TargetKind::Transition;
    wall->obstacles.clear();
    for(int x=0;x<wall->submap->width;++x) {
        NativeObstacle actor;actor.cell={-1,x,24};actor.type=6;actor.npc=true;actor.id=300+x;
        actor.raw_name=L"Soldier";actor.solidity_known=actor.solid=true;wall->obstacles.push_back(actor);
    }
    const auto run=[&](RouteSnapshot& request){Planned out;std::thread work([&]{out=plan_snapshot(request);});work.join();return out;};
    // Real reported Mysidia geometry, not a second pathfinding implementation.
    // The log names patrol49 at the doorway approach. The actor's authored home
    // is west of it; occupancy must move rather than leave a second phantom wall.
    auto inn=castle(90,{0,21,6},true);inn->obstacles.clear();
    inn->goal.map_id=90;inn->goal.cell={0,29,13};inn->kind=TargetKind::Transition;
    NativeObstacle mage;
    for(const auto& object:baked::grids(*inn->submap).objects)if(object.type==baked::ObjType::NPC&&object.entity_id()==49) {
        mage.id=49;mage.type=6;mage.npc=true;mage.solidity_known=mage.solid=true;
        mage.raw_name=baked::object_identifier(object);break;
    }
    mage.cell={0,28,14};inn->obstacles={mage};
    const auto detour=run(*inn);
    expect(detour.path.success&&!detour.path.npc_approach,"Mysidia inn uses a complete detour around the patrol's occupied home");
    for(const auto& phase:detour.path.walk_phases)for(const auto& cell:phase.cells)
        expect(!(cell==mage.cell),"complete inn detour never crosses the solid mage");
    inn->obstacles.front().cell={0,29,14};
    const auto doorway=run(*inn);
    expect(doorway.path.success&&doorway.path.npc_approach&&!doorway.path.walk_phases.empty(),"logged doorway occupancy retains an honest usable prefix");
    if(!doorway.path.walk_phases.empty()) {
        const auto& end=doorway.path.walk_phases.back();
        expect(end.wait_for_npc&&!end.manual_after&&end.arrival_direction<0&&
            end.action_after.find(L"Wait for ")==0&&end.action_after.find(L"Interact")==std::wstring::npos,
            "open doorway approach waits for the patrol instead of claiming a gatekeeper interaction");
        ffpr::fieldnav::autowalk::Route walk;walk.set(doorway.path.walk_phases,45,41,false);
        for(const auto& cell:end.cells)expect(walk.next(cell).result==(cell==end.cells.back()?
            ffpr::fieldnav::autowalk::Result::Wait:ffpr::fieldnav::autowalk::Result::Move),"temporary approach terminates in Wait, never Action or Arrived");
        CornerSnapshot corner;corner.route=doorway.active;corner.submap=inn->submap;corner.at=end.cells.back();
        corner.map_id=corner.goal_map=90;corner.target=inn->goal.cell;
        const auto beacon=build_corner(corner);
        const auto* boundary=beacon.index.find(corner.at);
        expect(boundary&&!boundary->found&&!boundary->arrived,"beacon refreshes a temporary patrol boundary instead of latching onto false arrival");
    }
    inn->position=world_of_cell(*inn->submap,{0,29,15});
    expect(run(*inn).path.step_count==0,"adjacent doorway wait invents no movement");
    inn->obstacles.front().cell={0,28,14};const auto clears=run(*inn);
    expect(clears.path.success&&!clears.path.npc_approach&&clears.path.step_count==2,"fresh patrol movement restores the exact inn without a reload");
    // Authored movement, not corridor width, separates a patrol from a
    // stationary/conditionally moved gatekeeper after every detour fails.
    inn->obstacles.front().cell={0,29,13};const auto narrow=run(*inn);
    expect(narrow.path.npc_approach&&narrow.path.walk_phases.back().wait_for_npc&&!narrow.path.walk_phases.back().manual_after&&narrow.path.walk_phases.back().action_after.find(L"Wait for ")==0,
        "wandering NPC inside a one-tile channel still waits for its authored patrol instead of asking for interaction");
    inn->obstacles.front().cell={0,29,14};inn->obstacles.front().id=999;inn->obstacles.front().raw_name=L"Gatekeeper";
    const auto keeper=run(*inn);
    expect(keeper.path.npc_approach&&!keeper.path.walk_phases.back().wait_for_npc&&keeper.path.walk_phases.back().manual_after,
        "stationary area gatekeeper still requires manual interaction after every complete route fails");
    inn->obstacles.front().movement_known=true;inn->obstacles.front().wanders=true;
    const auto live_patrol=run(*inn);
    expect(live_patrol.path.npc_approach&&live_patrol.path.walk_phases.back().wait_for_npc,
        "a positively captured routine owns wait guidance even without an unambiguous baked name/id");
    inn->obstacles.front().id=mage.id;inn->obstacles.front().raw_name=mage.raw_name;inn->obstacles.front().wanders=false;
    const auto live_static=run(*inn);
    expect(live_static.path.npc_approach&&!live_static.path.walk_phases.back().wait_for_npc,
        "a native stationary routine overrides a stale wandering bake record");
    fixture::capture_ready=fixture::capture_movement=true;
    for(const auto& move:std::vector<std::pair<int,int>>{{0,1},{1,1},{2,1},{3,1},{3,0},{102,1}}) {
        fixture::movement_property.type=move.first;fixture::movement_property.steps=move.second;
        const auto captured=capture_obstacle(&fixture::movement_actor,*inn);
        expect(captured.movement_known&&captured.wanders==((move.first==2||move.first==3)&&move.second>0),
            "actual native capture distinguishes autonomous Area/Route from stationary, facing and scripted movement");
    }
    const auto prior_game=fixture::game_id;
    fixture::capture_routine=true;
    fixture::routine_controllers.values={&fixture::routine_controller};
    fixture::routine_instructions.values={&fixture::routine_move,&fixture::routine_animation};
    for(int game_number=1;game_number<=6;++game_number)for(int steps:{0,1}){
        inn->routine_actors=0;inn->routine_work=0;
        fixture::game_id=static_cast<game::Id>(game_number);
        fixture::movement_property.type=102;fixture::movement_property.steps=steps;
        const auto captured=capture_obstacle(&fixture::movement_actor,*inn);
        const bool native_patrol=game_number==3||game_number==5||game_number==6;
        expect(captured.movement_known&&captured.wanders==native_patrol,
            "active native RoutePuppet patrol follows each game's real controller, independently of authored step count");
        inn->obstacles.front().movement_known=captured.movement_known;inn->obstacles.front().wanders=captured.wanders;
        const auto assigned=run(*inn);
        expect(assigned.path.npc_approach&&assigned.path.walk_phases.back().wait_for_npc==native_patrol,
            "captured assigned patrol uses Wait while older scripted controllers retain manual interaction");
        fixture::routine_controller.current=nullptr;
        const auto completed=capture_obstacle(&fixture::movement_actor,*inn);
        expect(completed.movement_known&&!completed.wanders,"completed native route retains102 but cannot authorize waiting");
        fixture::routine_controller.current=&fixture::routine_animation;fixture::routine_controller.index=1;fixture::routine_controller.loop=false;
        expect(!capture_obstacle(&fixture::movement_actor,*inn).wanders,"one-shot animation tail cannot reuse a past move as future movement");
        fixture::routine_controller.loop=true;
        expect(capture_obstacle(&fixture::movement_actor,*inn).wanders==native_patrol,"looping delay/animation keeps its later movement while older versions remain manual");
        fixture::routine_instructions.values={&fixture::routine_animation};fixture::routine_controller.index=0;
        expect(!capture_obstacle(&fixture::movement_actor,*inn).wanders,"animation-only looping instructions cannot cause endless waiting");
        fixture::routine_instructions.values={&fixture::routine_move,&fixture::routine_animation};fixture::routine_controller.current=&fixture::routine_move;
        fixture::movement_property.type=0;
        expect(!capture_obstacle(&fixture::movement_actor,*inn).wanders,"destruction restoring the ordinary type remains stationary");
    }
    fixture::game_id=game::Id::FF6;fixture::movement_property.type=102;
    inn->routine_work=0;fixture::routine_controller.index=-1;
    expect(!capture_obstacle(&fixture::movement_actor,*inn).wanders,"invalid native instruction index stays manual");
    fixture::routine_controller.index=0;fixture::routine_controllers.count_override=1025;
    expect(!capture_obstacle(&fixture::movement_actor,*inn).wanders,"oversized controller registry stays manual without scanning");
    fixture::routine_controllers.count_override=INT_MIN;fixture::routine_instructions.count_override=257;
    expect(!capture_obstacle(&fixture::movement_actor,*inn).wanders,"oversized instruction list stays manual without scanning");
    fixture::routine_instructions.count_override=INT_MIN;
    fixture::routine_controllers.values.assign(1024,nullptr);fixture::routine_controllers.values.back()=&fixture::routine_controller;
    fixture::routine_instructions.values.assign(256,&fixture::routine_animation);
    std::vector<double> costs;costs.reserve(1000);
    for(int sample=0;sample<1000;++sample) {
        inn->routine_work=0;inn->routine_actors=0;const auto begun=std::chrono::steady_clock::now();
        for(int actor=0;actor<8;++actor)expect(!capture_obstacle(&fixture::movement_actor,*inn).wanders,"whole snapshot overload remains manual");
        costs.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begun).count());
        expect(inn->routine_work==4096,"all actors share one finite4096-work capture budget");
    }
    std::sort(costs.begin(),costs.end());
    std::printf("Instruction capture,1000 complete overloaded8-actor episodes: median%.3fus p99%.3fus max%.3fus (fake native metadata, no frame/audible claim)\n",costs[500],costs[990],costs.back());
    fixture::routine_controllers.values.clear();inn->routine_actors=0;inn->routine_work=0;
    for(int i=0;i<64;++i)expect(!instruction_will_move(&fixture::movement_actor,*inn),"empty native registry remains manual");
    const int capped_reads=fixture::native_reads;
    for(int i=0;i<64;++i)expect(!instruction_will_move(&fixture::movement_actor,*inn),"exhausted actor budget remains manual");
    expect(inn->routine_actors==64&&fixture::native_reads==capped_reads,"empty/invalid registries cannot repeat native accessor queries beyond the whole-capture actor limit");
    fixture::routine_controllers.values.clear();fixture::routine_instructions.values.clear();fixture::capture_routine=false;
    fixture::game_id=prior_game;
    fixture::capture_ready=fixture::capture_movement=false;
    std::printf("Mysidia inn: complete patrol detour, logged temporary doorway, fresh movement, narrow channel and gatekeeper checked\n");
    std::printf("Mysidia compute sample ms: detour %.3f, occupied doorway %.3f, cleared doorway %.3f (offline, not native/audible latency)\n",
        detour.compute_ms,doorway.compute_ms,clears.compute_ms);
    auto blocked=run(*wall);
    expect(blocked.path.success&&blocked.path.npc_approach&&!blocked.path.walk_phases.empty(),"true NPC choke returns a usable manual interaction prefix");
    if(blocked.path.npc_approach) {
        const auto& phase=blocked.path.walk_phases.back();
        expect(blocked.path.step_count>0,"distant true obstruction retains usable walking directions");
        expect(phase.manual_after&&phase.arrival_direction>=0&&phase.action_after==L"Interact: Soldier","NPC handback names and faces its actual interaction target");
        for(const auto& cell:phase.cells)expect(cell.y!=24,"interaction approach never walks into the blocking NPC wall");
        ffpr::fieldnav::autowalk::Route walk;walk.set(blocked.path.walk_phases,blocked.path.map_width,blocked.path.map_height,false);
        for(const auto& cell:phase.cells) {
            const auto next=walk.next(cell);
            expect(next.result==(cell==phase.cells.back()?ffpr::fieldnav::autowalk::Result::Action:ffpr::fieldnav::autowalk::Result::Move),
                "autowalk reaches a manual interaction boundary and never declares the original destination arrived");
        }
    }
    if(blocked.path.npc_approach) {
        const auto original=wall->position;wall->position=world_of_cell(*wall->submap,blocked.path.walk_phases.back().cells.back());
        const auto adjacent=run(*wall);
        expect(adjacent.path.success&&adjacent.path.npc_approach&&adjacent.path.step_count==0&&adjacent.path.description.find(L"face ")!=std::wstring::npos,
            "adjacent true blocker gives its interaction and facing without inventing movement");
        wall->position=original;
    }
    for(auto& actor:wall->obstacles){actor.solidity_known=true;actor.solid=false;}
    const auto passable=run(*wall);
    expect(passable.path.success&&!passable.path.npc_approach,"native trigger-only NPCs never create false solid blockers");
    for(auto& actor:wall->obstacles){actor.solid=true;actor.cell.y=18;}
    const auto moved=run(*wall);
    expect(moved.path.success&&!moved.path.npc_approach,"fresh actor positions clear an old choke without a map reload");
    wall->position=world_of_cell(*wall->submap,{0,19,19});wall->obstacles.clear();
    for(const auto offset:std::vector<Cell>{{0,0,-1},{0,1,0},{0,0,1},{0,-1,0}}) {
        NativeObstacle actor;actor.cell={-1,19+offset.x,19+offset.y};actor.type=6;actor.npc=true;
        actor.id=400+static_cast<int>(wall->obstacles.size());actor.raw_name=L"Soldier";actor.solidity_known=actor.solid=true;wall->obstacles.push_back(actor);
    }
    wall->goal=chest_goal(42,6,8);wall->kind=TargetKind::Interact;
    const auto cross=run(*wall);
    expect(cross.path.success&&cross.path.npc_approach&&cross.path.next_map_id==0&&cross.pending.valid,"cross-floor route keeps its original chest while handing back at the real NPC");
    wall->goal=chest_goal(75,9,15);
    const auto gate_and_npc=run(*wall);
    expect(!gate_and_npc.path.npc_approach,"removing NPCs cannot claim access through a separate closed story gate");

    for(int origin:{31,39,40})for(bool opened:{false,true}) {
        auto request=castle(origin,origin==31?Cell{0,22,16}:origin==39?Cell{0,35,29}:Cell{0,22,19},opened);
        for(const auto goal:{chest_goal(42,6,8),chest_goal(38,8,6),chest_goal(75,9,15)}) {
            request->goal=goal;request->kind=TargetKind::Interact;request->previous_active.reset();request->active={};request->pending={};
            std::thread route([&]{planned=plan_snapshot(*request);});route.join();
            std::printf("Fynn gate%d origin%d target%d: success%d next%d reason=%ls\n",opened,origin,goal.map_id,planned.path.success,planned.path.next_map_id,planned.path.reason.c_str());
            const bool gated=goal.map_id==38||goal.map_id==75;
            expect(planned.path.success==(!gated||opened),"only the open throne passage admits side-room/basement routes; ordinary wing stays reachable");
            if(gated&&!opened)expect(planned.path.reason==L"Story gated","remote floors explain the closed hidden passage");
            BrowsePlanned browse;std::thread browsing([&]{browse=plan_browse_snapshot(*request);});browsing.join();
            if(gated&&!opened)expect(!browse.route.direct&&browse.route.via_map_id==0&&browse.route.reason==L"Story gated","browse cannot promise a first floor for a globally gated chest");
        }
    }
    expect(planned.path.reason.find(L"Reachable scene trigger")==std::wstring::npos,"no unrelated return-stair event is proposed as an unlock");
    molten_cave();
    expect(fixture::wrong_owner==0,"NPC/gate planner never calls native APIs on workers");
    std::printf("NPC/gate check: %d failures\n",failures);return failures?1:0;
}
