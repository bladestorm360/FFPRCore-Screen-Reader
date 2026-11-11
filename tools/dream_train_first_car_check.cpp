// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Timed crate geometry prototype. Exact native control0904 moves the single
// physical crate28 from27,8 to27,9; no other collision is released by pull one.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff6/story_model.cpp"
#include "../src/games/ff6/dream_train_maze.cpp"
#include "../src/games/ff6/zone_eater_roof_model.cpp"
#include "../src/games/ff6/sealed_gate_model.cpp"
namespace field {std::wstring safe_event_label(const std::wstring& raw,const std::wstring&,int,int,int){return raw;}}
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff6/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
    const auto* room=baked::submap_by_map_id(524);if(!room)return 2;const auto& grid=baked::grids(*room);
    map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);flags.local.fill(0);flags.area.fill(0);
    flags.scenario[442]=1;flags.current_map_id=524;flags.current_area_id=23;
    map_state::Resolver state(flags,fixture::records);
    const baked::Object* lever=nullptr;
    for(const auto& actor:grid.objects)if(actor.type==baked::ObjType::Entity&&actor.entity_id()==26)lever=&actor;
    if(!lever)return 2;
    baked_route::Request initial;initial.submap=room;initial.start={1,32,10};initial.transportation_id=1;
    map_state::add_obstacles(state,*room,initial,true,false,false);
    auto timed=initial;
    const Cell before{1,27,8},after{1,27,9};
    if(std::count(timed.blocked.begin(),timed.blocked.end(),before)!=1)return 2;
    timed.blocked.erase(std::find(timed.blocked.begin(),timed.blocked.end(),before));timed.blocked.push_back(after);
    timed.goals={{1,26,8}};timed.allow_trap_goal=true;
    const auto approaches=baked_route::goals_for(initial,{lever->layer,lever->tx,lever->ty},baked_route::GoalKind::Interact,lever);
    int witnesses=0,best=INT_MAX;
    for(const auto& approach:approaches.cells) {
        auto prepare=initial;prepare.goals={approach};prepare.goals_open=false;
        const auto reachable=baked_route::find(prepare);if(!reachable.found)continue;
        timed.start=approach;const auto run=baked_route::find(timed);
        std::printf("Lever approach L%d %d,%d, prepare%dsteps: timed%d/%dsteps path",approach.layer,approach.x,approach.y,reachable.steps(),run.found,run.steps());
        for(const auto& cell:run.cells)std::printf(" %d,%d",cell.x,cell.y);std::printf("\n");
        if(run.found){++witnesses;best=std::min(best,run.steps());}
    }
    const baked::Object* chest=nullptr;
    for(const auto& actor:grid.objects)if(actor.type==baked::ObjType::TreasureBox&&actor.entity_id()==9)chest=&actor;
    if(!chest)return 2;
    auto locked=initial;locked.start={1,30,8};
    const Cell right_before{1,29,8},right_after{1,29,9};
    if(std::count(locked.blocked.begin(),locked.blocked.end(),right_before)!=1)return 2;
    locked.blocked.erase(std::find(locked.blocked.begin(),locked.blocked.end(),right_before));locked.blocked.push_back(right_after);
    locked.goals=baked_route::goals_for(locked,{chest->layer,chest->tx,chest->ty},baked_route::GoalKind::Interact,chest).cells;
    const auto opened=baked_route::find(locked);
    bool pass=opened.found&&!opened.cells.empty()&&opened.cells.back()==right_before;
    std::printf("Second-pull permanent layout: chest%d/%dsteps",opened.found,opened.steps());
    if(pass) {
        locked.start=opened.cells.back();locked.goals={{1,26,8}};locked.allow_trap_goal=true;
        const auto finish=baked_route::find(locked);pass=finish.found;
        std::printf(" -> native success%d/%dsteps (timer already stopped)",finish.found,finish.steps());
    }
    std::printf("\nTimed crate prototype: %d legal preparations, first-pull best%dsteps, permanent-layout%d, owner%d; native input timing not measured\n",witnesses,best,pass,fixture::wrong_owner.load());
    json::Value puzzles;if(!json::parse(read("data/ff6/puzzles.json"),puzzles))return 2;
    fixture::records.obj[L"story_guides"]=*puzzles.get(L"story_guides");
    int failures=(!witnesses||!pass),episodes=0,routes=0,scenes=0;
    struct Episode {const char* name;Cell start;int local=0,area=0;bool left_down=false,right_down=false,timeout=false,reward=false,metal=false,fed=false,centers=false;};
    const Episode cases[]={
        {"right arrival",{1,32,10}},
        {"left arrival",{1,5,10}},
        {"timeout retry",{1,30,8},0,0,false,false,true},
        {"timed success resume",{1,26,8},3,0,true},
        {"locked success resume",{1,26,8},3,0,false,true},
        {"reward survives reload",{1,32,10},0,0,false,false,false,true},
        {"moving left",{1,15,12},0,1,false,false,false,true,true,false,true},
        {"moving right",{1,19,12},0,2,false,false,false,true,true,false,true},
        {"moving center",{1,17,12},0,0,false,false,false,true,true,false,true},
        {"fed resume",{1,5,10},0,0,false,false,false,true,true,true}
    };
    // Real actor types/IDs/rectangles, plus the reviewed current native manual
    // mutations. This does not turn Entity gates into synthetic colliders.
    const auto capture=[](const ffpr::ff6::story::Snapshot& state,bool left_down,bool right_down,bool centers) {
        const auto* map=baked::submap_by_map_id(state.map);
        RouteSnapshot snapshot(map_state::Resolver(state.flags,fixture::records));snapshot.map_id=state.map;snapshot.submap=map;
        snapshot.layer=state.player.layer;snapshot.position=world_of_cell(*map,state.player);snapshot.vehicle={true,1,1,{}};snapshot.locale=L"en";
        for(const auto& original:baked::grids(*map).objects) {
            if(original.type==baked::ObjType::PointIn||original.type==baked::ObjType::GotoMap||original.type==baked::ObjType::Warp)continue;
            const int id=original.entity_id();const bool first_car=state.map==524;
            const bool acquired=state.flags.scenario[444]==1,fed=state.flags.scenario[443]==1;
            const bool central=first_car&&id>=100&&id<=105;
            const bool moving=first_car&&id==107;
            if(central&&!centers)continue;
            if(!snapshot.state.active(*map,original)&&!(central&&centers)&&!(moving&&acquired&&!fed))continue;
            if(first_car&&((id==26&&state.flags.local[0].value_or(0)>=2)||(id==106&&state.flags.local[0]==3)||
               (id==24&&acquired)||(moving&&fed)||(fed&&(id==22||id==23||central))))continue;
            auto actor=snapshot.state.effective(*map,original);
            if(first_car&&id==28){actor.tx=27;actor.ty=left_down?9:8;}
            if(first_car&&id==31){actor.tx=29;actor.ty=right_down?9:8;}
            if(first_car&&(id==24||(moving&&acquired))) {
                const int side=state.flags.area[2].value_or(0);actor.tx=side==1?16:side==2?18:17;actor.ty=10;
            }
            NativeObstacle seen;seen.id=id;seen.type=static_cast<int>(actor.type);
            seen.raw_name=baked::object_identifier(original);seen.cell={actor.layer,actor.tx,actor.ty};
            seen.enabled=seen.applies=true;seen.trigger=baked_route::step_event(actor);
            seen.chest=actor.type==baked::ObjType::TreasureBox;
            seen.npc=actor.type==baked::ObjType::NPC||actor.type==baked::ObjType::ShopNPC;
            seen.collider=actor.type==baked::ObjType::CollisionEntity;seen.collision=actor.a;
            seen.transportation_ids=snapshot.state.transportation_mask(*map,original);
            seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);
            snapshot.obstacles.push_back(std::move(seen));
        }
        return snapshot;
    };
    for(const auto& input:cases) {
        ffpr::ff6::story::Snapshot state;state.map=524;state.player=input.start;state.locale=L"en";
        state.flags=flags;state.flags.local[0]=input.local;state.flags.area[2]=input.area;
        state.flags.treasure[200]=int(input.reward);state.flags.treasure[198]=int(input.metal);
        state.flags.scenario[444]=int(input.metal);state.flags.scenario[443]=int(input.fed);
        bool left_down=input.left_down,right_down=input.right_down,centers=input.centers,retried=false,finished=false;
        for(int stage=0;stage<60;++stage) {
            const auto guide=ffpr::ff6::story::plan(state);
            if(guide.unknown||guide.targets.size()!=1){std::printf("FAIL %s stage%d model targets%zu unknown%d\n",input.name,stage,guide.targets.size(),guide.unknown);break;}
            const auto& target=guide.targets.front();auto snapshot=capture(state,left_down,right_down,centers);
            snapshot.goal=target.goal;snapshot.kind=target.kind;
            Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();++routes;
            if(!route.path.success||route.path.walk_phases.empty()) {
                std::printf("FAIL %s stage%d target%d map%d: %ls / %ls\n",input.name,stage,target.goal.entity_id,target.goal.map_id,route.path.reason.c_str(),route.path.error.c_str());break;
            }
            if(target.goal.map_id!=524) {
                finished=target.goal.map_id==525&&route.path.next_map_id==522;break;
            }
            bool changed=false;
            for(const auto& phase:route.path.walk_phases)for(size_t index=1;index<phase.cells.size()&&!changed;++index) {
                state.player=phase.cells[index];const auto at=state.player;
                if(state.flags.scenario[443]==1)continue;
                const bool side=at.y==11&&(at.x==16||at.x==18);
                const bool middle=centers&&((at.y==12&&at.x>=15&&at.x<=18)||(at.y==11&&(at.x==15||at.x==17)));
                if(side||middle) {
                    state.flags.area[2]=side?(at.x==16?1:2):0;centers=side;changed=true;++scenes;
                }
            }
            if(changed)continue; // native movement scene retires the old route before continuing
            state.player=route.path.walk_phases.back().cells.back();
            switch(target.goal.entity_id) {
            case 26:
                if(state.flags.local[0]==0){state.flags.local[0]=1;left_down=true;}
                else if(state.flags.local[0]==1){state.flags.local[0]=2;left_down=false;right_down=true;}
                else {stage=60;break;}
                if(input.timeout&&!retried&&state.flags.local[0]==1){retried=true;state.flags.local[0]=0;left_down=false;}
                break;
            case 9:state.flags.treasure[200]=1;break;
            case 7:state.flags.treasure[198]=1;state.flags.scenario[444]=1;break;
            case 107:state.flags.scenario[443]=1;centers=false;break;
            case 8:state.flags.treasure[199]=1;break;
            default:stage=60;break;
            }
        }
        std::printf("First car %s: complete%d\n",input.name,finished);
        if(finished)++episodes;else ++failures;
    }
    int typed_mazes=0;
    for(int bits=0;bits<8;++bits) {
        ffpr::ff6::story::Snapshot state;state.map=525;state.player={1,29,13};state.locale=L"en";state.flags=flags;
        state.flags.current_map_id=525;
        for(int i=0;i<3;++i)state.flags.scenario[445+i]=(bits>>i)&1;
        for(int i=0;i<6;++i)state.flags.local[i+1]=(28>>i)&1;
        bool finished=false;
        for(int stage=0;stage<24;++stage) {
            const auto guide=ffpr::ff6::story::plan(state);
            if(guide.unknown||guide.targets.size()!=1)break;
            const auto& target=guide.targets.front();auto snapshot=capture(state,false,false,false);
            snapshot.goal=target.goal;snapshot.kind=target.kind;
            Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();++routes;
            if(!route.path.success||route.path.walk_phases.empty()) {
                std::printf("FAIL actual-type maze bits%d stage%d target%d: %ls\n",bits,stage,target.goal.entity_id,route.path.reason.c_str());break;
            }
            state.player=route.path.walk_phases.back().cells.back();const int id=target.goal.entity_id;
            if(target.kind==TargetKind::Transition&&id==3){finished=true;break;}
            const int bit=id==41?445:id==32?446:id==26?447:-1;
            if(bit>=0)state.flags.scenario[bit]=1-state.flags.scenario[bit].value_or(0);
            else if(id==10)state.flags.scenario[448]=1;
            else break;
        }
        if(finished)++typed_mazes;else ++failures;
    }
    struct Handoff {int map;Cell start;bool intro,ready;int goal_map,entity;};
    const Handoff handoffs[]={
        {522,{0,108,5},false,false,522,1},
        {523,{1,32,10},false,false,522,1},
        {522,{0,88,5},true,false,524,-1},
        {523,{1,5,10},true,false,524,-1},
        {526,{1,7,11},true,true,527,-1},
        {522,{0,58,5},true,true,527,-1},
        {527,{2,8,12},true,true,527,8}
    };
    int handoff_passes=0;
    for(const auto& input:handoffs) {
        ffpr::ff6::story::Snapshot state;state.map=input.map;state.player=input.start;state.locale=L"en";state.flags=flags;
        state.flags.current_map_id=input.map;state.flags.scenario[442]=int(input.intro);
        for(int flag:{443,444,448})state.flags.scenario[flag]=int(input.ready);
        for(int chest:{198,199,200})state.flags.treasure[chest]=int(input.ready);
        const auto guide=ffpr::ff6::story::plan(state);bool valid=guide.targets.size()==1&&!guide.unknown;
        if(valid) {
            const auto& target=guide.targets.front();valid=target.goal.map_id==input.goal_map&&target.goal.entity_id==input.entity;
            auto snapshot=capture(state,false,false,false);snapshot.goal=target.goal;snapshot.kind=target.kind;
            Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();++routes;
            valid&=route.path.success;
            std::printf("Train handoff map%d %d,%d L%d -> map%d entity%d: route%d next%d pass%d %ls\n",input.map,input.start.x,input.start.y,
                input.start.layer,target.goal.map_id,target.goal.entity_id,route.path.success,route.path.next_map_id,valid,route.path.reason.c_str());
        }
        if(valid)++handoff_passes;else ++failures;
    }
    std::printf("Actual-type maze episodes: %d/8; train arrival/departure profiles: %d/%zu\n",typed_mazes,handoff_passes,std::size(handoffs));
    std::printf("First-car authored episodes: %d/%zu, %d production routes, %d native movement refreshes, %d failures, owner%d\n",
        episodes,std::size(cases),routes,scenes,failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
