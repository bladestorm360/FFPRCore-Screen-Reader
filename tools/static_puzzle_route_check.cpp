// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real authored controls/binary coordinates and complete Citadel maze routes.
// This data-only fixture never loads a game or claims live actor observations.
#define main retained_static_route_main
#include "route_worker_check.cpp"
#undef main
#include "ordinary_interaction_fixtures.h"
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    int failures=0,controls=0,maze_starts=0,control_entrances=0,reward_routes=0,guardian_stops=0;
    const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
    const auto read=[](const std::string& name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    for(int game=1;game<=6;++game) {
        const auto prefix="data/ff"+std::to_string(game)+"/";
        const auto bytes=read(prefix+"maps.bin");json::Value puzzles;
        expect(baked::load(bytes.data(),bytes.size())&&json::parse(read(prefix+"puzzles.json"),puzzles)&&json::parse(read(prefix+"mapstate.json"),fixture::records),"actual maps, puzzle recipes and gate records load");
        if(!append_ordinary_interactions(puzzles,game))return 2;
        fixture::game_id=static_cast<game::Id>(game);g_loaded=g_prepared=true;
        for(const auto& puzzle:puzzles.get(L"puzzles")->arr)for(const auto& step:puzzle.get(L"steps")->arr) {
            const auto* entity=step.get(L"entity");if(!entity)continue;
            const auto* room=baked::submap_by_map_id(int(step.get(L"map")->n));
            int matches=0;
            if(room)for(const auto& o:baked::grids(*room).objects)
                if(o.entity_id()==int(entity->n)&&baked::object_identifier(o)==step.get(L"raw")->s) {
                    // An authored interaction owns the real actor. A derived
                    // escape Warp can share its ID/name/cell without being a
                    // second physical control (Dreadnought engine12).
                    if(o.type==baked::ObjType::Warp)continue;
                    ++matches;expect(o.tx==step.get(L"x")->n&&o.ty==step.get(L"y")->n&&o.layer==step.get(L"layer")->n,"authored control matches the shipped binary's exact identity and layer/cell");
                }
            const bool wyvern_variants=game==2&&int(step.get(L"map")->n)==180&&int(entity->n)==6;
            // Three native groups share this logical identity: pre-pendant
            //0166 and two0050 variants. Only the latter can award the egg.
            if(matches!=(wyvern_variants?3:1))std::printf("Binary control identity FF%d map%d entity%d: matches%d\n",game,int(step.get(L"map")->n),int(entity->n),matches);
            expect(matches==(wyvern_variants?3:1),"authored control resolves every exact baked identity, including the three reviewed Wyvern variants");++controls;
            // Independently qualify the fourteen newly found passage controls
            // from native room arrivals with their actual closed-gate state.
            const int map=int(step.get(L"map")->n);
            const bool added=(game==3&&(map==43||map==69||map==73||map==110||map==133||map==141||map==162||map==324||map==305||map==306))||
                (game==4&&(map==25||map==42||map==47||map==84||map==91||map==94||map==95||map==104));
            if(added&&room) {
                map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.treasure.fill(0);
                if(game==3&&(map==305||map==306))flags.scenario[93]=1;
                flags.current_map_id=map;map_state::Resolver state(flags,fixture::records);
                bool reachable=false;int arrivals=0;
                for(const auto& arrival:baked::grids(*room).objects)if(arrival.type==baked::ObjType::PointIn&&state.active(*room,arrival)) {
                    baked_route::Request request;request.submap=room;request.transportation_id=1;
                    request.start={arrival.layer,arrival.tx,arrival.ty};request.passages=game::flags().has_hidden_passages;
                    map_state::add_obstacles(state,*room,request,true,false,false);
                    const Cell at{int(step.get(L"layer")->n),int(step.get(L"x")->n),int(step.get(L"y")->n)};
                    const auto goals=baked_route::goals_for(request,at,step.get(L"examine")->b?baked_route::GoalKind::Interact:baked_route::GoalKind::Spot);
                    request.goals=goals.cells;
                    const auto route=baked_route::find(request);reachable|=route.found;++arrivals;
                }
                if(!reachable)std::printf("FAIL FF%d control map%d entity%d: no closed-state arrival in%d points\n",game,map,int(entity->n),arrivals);
                expect(reachable,"each additional control has a legal real entrance approach before its passage opens");
                if(reachable)++control_entrances;
            }
        }
        if(game!=1)continue;
        const auto* maze=baked::submap_by_map_id(94);
        std::vector<Cell> starts;
        for(const auto& o:baked::grids(*maze).objects)if(o.type==baked::ObjType::PointIn)starts.push_back({o.layer,o.tx,o.ty});
        const auto& maze_steps=puzzles.get(L"puzzles")->arr[0].get(L"steps")->arr;
        const auto& goal=*std::find_if(maze_steps.begin(),maze_steps.end(),[](const auto& step){return step.get(L"map")->n==95;});
        const auto reward=std::find_if(maze_steps.begin(),maze_steps.end(),[](const auto& step){const auto* e=step.get(L"entity");return e&&e->n==9;});
        expect(reward!=maze_steps.end(),"trial includes its actual reward chest");
        for(const auto start:starts) {
            int map=94,hops=0;Cell at=start;
            map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.scenario[44]=1;
            while(map!=95&&hops<64) {
                flags.current_map_id=map;
                RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=map;snapshot.submap=baked::submap_by_map_id(map);snapshot.layer=at.layer;
                snapshot.position=world_of_cell(*snapshot.submap,at);snapshot.kind=TargetKind::Spot;snapshot.locale=L"en";snapshot.vehicle={true,1,1,{}};
                snapshot.goal.map_id=95;snapshot.goal.cell={int(goal.get(L"layer")->n),int(goal.get(L"x")->n),int(goal.get(L"y")->n)};
                const auto result=plan_snapshot(snapshot);
                expect(result.path.success,"production planner reaches the true maze endpoint from each real intermediate teleport arrival");if(!result.path.success)break;
                const auto transfer=result.pending.incoming_trap?result.pending.incoming_trap:result.pending.incoming_transition;
                expect(result.path.next_map_id>0&&transfer.has_value(),"each maze leg retains its actual native map/arrival receipt");if(!transfer||result.path.next_map_id<=0)break;
                map=transfer->destination;at=transfer->arrival;++hops;
            }
            expect(map==95&&hops<64,"complete replay ends on3F rather than the first-floor return pad");
            if(map==95&&reward!=maze_steps.end())for(unsigned cleared=0;cleared<4;++cleared) {
                // Native victory callbacks set locals0/1 and hide guardians
                //19/21. Capture them as present until each real handoff; do
                // not prove the reward route with its encounters erased.
                auto progress=flags;progress.current_map_id=95;progress.local.fill(0);
                progress.local[0]=cleared&1;progress.local[1]=(cleared>>1)&1;
                const auto* room=baked::submap_by_map_id(95);Cell pose=at;bool reached=false;
                const Cell chest{int(reward->get(L"layer")->n),int(reward->get(L"x")->n),int(reward->get(L"y")->n)};
                for(int leg=0;leg<4;++leg) {
                    RouteSnapshot s(map_state::Resolver(progress,fixture::records));s.map_id=95;s.submap=room;s.layer=pose.layer;
                    s.position=world_of_cell(*room,pose);s.kind=TargetKind::Interact;s.locale=L"en";s.vehicle={true,1,1,{}};
                    s.goal.map_id=95;s.goal.cell=chest;s.goal.entity_id=9;s.goal.raw_name=reward->get(L"raw")->s;s.goal.examine=true;
                    for(const auto& source:baked::grids(*room).objects) {
                        if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(*room,source))continue;
                        const auto object=s.state.effective(*room,source);
                        if((object.entity_id()==19&&progress.local[0]==1)||(object.entity_id()==21&&progress.local[1]==1))continue;
                        NativeObstacle n;n.type=int(object.type);n.id=object.entity_id();n.raw_name=baked::object_identifier(source);
                        n.cell={object.layer,object.tx,object.ty};n.enabled=n.applies=true;n.trigger=baked_route::step_event(object);
                        n.npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;n.chest=object.type==baked::ObjType::TreasureBox;
                        n.collider=object.type==baked::ObjType::CollisionEntity;n.collision=object.a;
                        if(object.type==baked::ObjType::Warp){if((object.d&255)!=1)continue;n.type=2;}
                        s.obstacles.push_back(std::move(n));
                    }
                    const auto plan=plan_snapshot(s);
                    expect(plan.path.success&&!plan.path.next_map_id&&!plan.path.required_trap,"reward route stays on3F with its real physics and no return warp");
                    if(!plan.path.success||plan.path.walk_phases.empty())break;
                    for(const auto& phase:plan.path.walk_phases)for(const auto cell:phase.cells)
                        expect(cell!=Cell{0,29,28},"reward route never steps onto the automatic1F return pad");
                    pose=plan.path.walk_phases.back().cells.back();
                    if(plan.path.npc_approach) {
                        int guardian=-1;
                        for(const auto& actor:s.obstacles)if(actor.npc&&(actor.id==19||actor.id==21)&&actor.cell.layer==pose.layer&&
                            std::abs(actor.cell.x-pose.x)+std::abs(actor.cell.y-pose.y)<=1)guardian=actor.id;
                        expect(guardian>=0&&plan.path.walk_phases.back().manual_after,"only a real native guardian can interrupt the chest approach");
                        if(guardian<0)break;
                        progress.local[guardian==19?0:1]=1;++guardian_stops;continue;
                    }
                    reached=pose.layer==chest.layer&&std::abs(pose.x-chest.x)+std::abs(pose.y-chest.y)==1;break;
                }
                expect(reached,"every complete maze resume continues to the reward's actual cardinal interaction side");
                if(reached)++reward_routes;
            }
            std::printf("Citadel maze arrival%d,%d L%d: %d native transfers, final%d\n",start.x,start.y,start.layer,hops,map);++maze_starts;
        }
    }
    expect(controls==89&&maze_starts>1&&control_entrances==18,"active and retired controls, eighteen entrance approaches and multiple real maze resumes are covered");
    expect(reward_routes==maze_starts*4&&guardian_stops>0,"all four actual guardian progress profiles reach the trial reward");
    expect(fixture::wrong_owner==0,"data-only route work performs no native worker read");
    std::printf("Static route check: %d controls, %d additional entrance approaches, %d maze resumes, %d reward routes, %d guardian handoffs, %d failures\n",controls,control_entrances,maze_starts,reward_routes,guardian_stops,failures);
    return failures?1:0;
}
