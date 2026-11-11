// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Complete actual-consumer maze episodes from all eight native lever layouts.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff6/story_model.cpp"
#include "../src/games/ff6/dream_train_maze.cpp"
#include "../src/games/ff6/zone_eater_roof_model.cpp"
#include "../src/games/ff6/sealed_gate_model.cpp"
namespace field {std::wstring safe_event_label(const std::wstring& raw,const std::wstring&,int,int,int){return raw;}}
int main() {
    using namespace field;using namespace baked_nav;using ffpr::ff6::story::MazeAdvice;
    std::setvbuf(stdout,nullptr,_IONBF,0);int failures=0,episodes=0,routes=0;
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff6/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
    json::Value puzzles;if(!json::parse(read("data/ff6/puzzles.json"),puzzles))return 2;
    fixture::records.obj[L"story_guides"]=*puzzles.get(L"story_guides");
    const auto* room=baked::submap_by_map_id(525);if(!room)return 2;const auto& grid=baked::grids(*room);
    const auto actual_route=[&](const map_state::Flags& flags,const Cell& from,const MazeAdvice& advice,const GuideTarget* requested=nullptr) {
        const auto actor=std::find_if(grid.objects.begin(),grid.objects.end(),[&](const auto& o){return
            o.type==(advice.exit?baked::ObjType::GotoMap:baked::ObjType::Entity)&&o.entity_id()==advice.entity;});
        if(actor==grid.objects.end())return Planned{};
        RouteSnapshot route(map_state::Resolver(flags,fixture::records));route.submap=room;route.map_id=525;
        route.layer=from.layer;route.position=world_of_cell(*room,from);route.vehicle={true,1,1,{}};
        route.goal.map_id=525;route.goal.entity_id=actor->entity_id();route.goal.raw_name=baked::object_identifier(*actor);
        route.goal.cell={actor->layer,actor->tx,actor->ty};route.goal.cells={advice.approach};
        route.kind=advice.exit?TargetKind::Transition:TargetKind::Interact;route.locale=L"en";
        if(requested){route.goal=requested->goal;route.kind=requested->kind;}
        baked_route::Request request;request.submap=room;request.transportation_id=1;
        map_state::Resolver state(flags,fixture::records);map_state::add_obstacles(state,*room,request,true,false,false);
        route.transport_rules=request.transport_rules;
        for(const auto& cell:request.blocked) {
            NativeObstacle obstacle;obstacle.type=19;obstacle.cell=cell;obstacle.collider=true;obstacle.collision=1;
            route.obstacles.push_back(std::move(obstacle));
        }
        Planned result;std::thread worker([&]{result=plan_snapshot(route);});worker.join();++routes;return result;
    };
    const auto started=std::chrono::steady_clock::now();
    ffpr::ff6::story::Snapshot last;
    for(bool complete:{false,true})for(int bits=0;bits<8;++bits) {
        ffpr::ff6::story::Snapshot snapshot;snapshot.map=525;snapshot.player={1,29,13};
        snapshot.flags.scenario.fill(0);snapshot.flags.global.fill(0);snapshot.flags.treasure.fill(0);snapshot.flags.local.fill(0);
        snapshot.flags.current_map_id=525;snapshot.flags.scenario[448]=int(complete);
        for(int i=0;i<3;++i)snapshot.flags.scenario[445+i]=(bits>>i)&1;
        bool finished=false;int previous=INT_MAX;
        std::printf("episode complete%d bits%d controls",complete,bits);
        for(int stage=0;stage<10;++stage) {
            const auto advice=ffpr::ff6::story::dream_train_maze(snapshot,fixture::records);
            if(advice.entity<0||advice.unknown||advice.cancelled){std::printf(" unavailable");break;}
            const auto result=actual_route(snapshot.flags,snapshot.player,advice);
            if(!result.path.success||result.path.walk_phases.empty()||result.path.walk_phases.back().cells.empty()){std::printf(" route-failed%d",advice.entity);break;}
            snapshot.player=result.path.walk_phases.back().cells.back();
            if(!(snapshot.player==advice.approach)){std::printf(" different-approach");break;}
            if(advice.entity==10||advice.exit){finished=true;break;}
            if(advice.actions>=previous){std::printf(" nondecreasing");break;}previous=advice.actions;
            const int index=advice.entity==41?445:advice.entity==32?446:advice.entity==26?447:-1;
            if(index<0)break;
            snapshot.flags.scenario[index]=1-snapshot.flags.scenario[index].value_or(0);
            std::printf(" %d",advice.entity);
        }
        std::printf(" finished%d\n",finished);if(finished)++episodes;else ++failures;last=snapshot;
    }
    ffpr::ff6::story::Snapshot unknown;unknown.map=525;
    if(!ffpr::ff6::story::dream_train_maze(unknown,fixture::records).unknown)++failures;
    int cancel_calls=0;const auto cancelled=ffpr::ff6::story::dream_train_maze(last,fixture::records,[&]{++cancel_calls;return true;});
    if(!cancelled.cancelled||cancelled.entity>=0||cancel_calls!=1)++failures;
    int pattern_episodes=0;
    for(int mask=0;mask<64;++mask) {
        auto state=last;state.flags.scenario.fill(0);state.flags.scenario[442]=1;state.player={1,29,13};
        for(int i=0;i<6;++i)state.flags.local[i+1]=(mask>>i)&1;
        bool finished=false;
        for(int stage=0;stage<24;++stage) {
            const auto guide=ffpr::ff6::story::plan(state);
            if(guide.unknown||guide.targets.size()!=1)break;
            const auto& target=guide.targets.front();MazeAdvice step;step.entity=target.goal.entity_id;step.exit=target.kind==TargetKind::Transition;
            const auto result=actual_route(state.flags,state.player,step,&target);
            if(!result.path.success||result.path.walk_phases.empty()||result.path.walk_phases.back().cells.empty())break;
            state.player=result.path.walk_phases.back().cells.back();
            const int id=target.goal.entity_id;
            if(step.exit){finished=true;break;}
            if(id==41||id==32||id==26) {
                const int bit=id==41?445:id==32?446:447;
                state.flags.scenario[bit]=1-state.flags.scenario[bit].value_or(0);
            } else if(id>=61&&id<=66)state.flags.local[id-60]=(28>>(id-61))&1;
            else if(id==10) {
                int actual=0;for(int i=0;i<6;++i) {
                    const auto bit=state.flags.local[i+1];
                    if(!bit||(*bit!=0&&*bit!=1)){actual=-1;break;}actual|=*bit<<i;
                }
                if(actual!=28)break;state.flags.scenario[448]=1;
            } else break;
        }
        if(finished)++pattern_episodes;else {++failures;std::printf("FAIL complete authored chest episode mask%d\n",mask);}
    }
    std::printf("Authored maze/chest/reset/exit episodes: %d of64 passed\n",pattern_episodes);
    std::printf("Dream train: %d complete maze episodes, %d production routes, %.3f ms total, %d failures, owner%d\n",
        episodes,routes,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count(),failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
