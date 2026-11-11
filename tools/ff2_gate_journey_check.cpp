// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Complete FF2 physical-gate journeys through the actual route worker. Native manual
// operations remain manual; this fixture applies only their reviewed receipts.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "ordinary_interaction_fixtures.h"
namespace {
int failures=0,episodes=0,legs=0,patrol_refreshes=0,story_handoffs=0;
void episode(const char* name,const char* recipe_id,int map,std::initializer_list<int> completed,std::initializer_list<int> learned={},std::optional<field::baked_route::Cell> native_pose={}) {
    using namespace field;using namespace baked_nav;
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);
    for(const int bit:completed)flags.scenario[bit]=1;
    std::set<int> words(learned);
    std::map<std::pair<int,int>,Cell> patrol_poses;
    json::Value puzzles;std::ifstream input("data/ff2/puzzles.json");
    if(!json::parse(std::string(std::istreambuf_iterator<char>(input),{}),puzzles)||
       !append_ordinary_interactions(puzzles,2)){++failures;return;}
    const auto& definitions=puzzles.get(L"puzzles")->arr;
    const auto wanted=text::widen(recipe_id);
    const auto found=std::find_if(definitions.begin(),definitions.end(),[&](const auto& p){return p.get(L"id")->s==wanted;});
    if(found==definitions.end()){++failures;return;}
    const int final_receipt=static_cast<int>(found->get(L"done_scenario")->n);
    const auto* start=baked::submap_by_map_id(map);map_state::Resolver initial(flags,fixture::records);
    const auto* entry=point_in(*start,1,initial);if(!entry){std::printf("FAIL %s no native entrance\n",name);++failures;return;}
    Cell at=native_pose.value_or(Cell{entry->layer,entry->tx,entry->ty});bool finished=true;
    for(const auto& step:found->get(L"steps")->arr) {
        if(flags.scenario[final_receipt]==1)break;
        const auto* word=step.get(L"done_keyword");const auto* bit=step.get(L"done_scenario");
        const int receipt=bit?static_cast<int>(bit->n):-1;
        if((word&&words.count(static_cast<int>(word->n)))||(receipt>=0&&flags.scenario[receipt]==1))continue;
        MapGoal goal;goal.map_id=static_cast<int>(step.get(L"map")->n);goal.entity_id=static_cast<int>(step.get(L"entity")->n);
        goal.raw_name=step.get(L"raw")->s;goal.cell={static_cast<int>(step.get(L"layer")->n),static_cast<int>(step.get(L"x")->n),static_cast<int>(step.get(L"y")->n)};
        goal.examine=step.get(L"examine")->b;goal.allow_world_transit=step.get(L"allow_world_transit")&&step.get(L"allow_world_transit")->b;
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
                NativeObstacle seen;seen.id=actor.entity_id();seen.type=static_cast<int>(actor.type);seen.raw_name=baked::object_identifier(original);
                seen.cell={actor.layer,actor.tx,actor.ty};seen.enabled=seen.applies=true;seen.trigger=baked_route::step_event(actor);
                const auto pose=patrol_poses.find({map,seen.id});
                if(pose!=patrol_poses.end())seen.cell=pose->second;
                seen.chest=actor.type==baked::ObjType::TreasureBox;seen.npc=actor.type==baked::ObjType::NPC||actor.type==baked::ObjType::ShopNPC;
                seen.collider=actor.type==baked::ObjType::CollisionEntity;seen.collision=actor.a;
                seen.transportation_ids=snapshot.state.transportation_mask(*snapshot.submap,original);seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);
                snapshot.obstacles.push_back(std::move(seen));
            }
            Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();++legs;
            if(!route.path.success||route.path.walk_phases.empty()) {
                std::printf("FAIL %s receipt%d word%d from%d L%d %d,%d to%d/%d: %ls / %ls\n",name,receipt,word!=nullptr,map,at.layer,at.x,at.y,goal.map_id,goal.entity_id,route.path.reason.c_str(),route.path.error.c_str());break;
            }
            if(route.path.npc_approach) {
                // Area/step1 soldiers really wander; encounter0179 does not hide
                // them. Replay a legal one-tile capture update, retaining every
                // soldier as a solid actor. Never turn a wait prefix into arrival.
                const auto end=route.path.walk_phases.back().cells.back();bool refreshed=false;
                if(map==21&&!route.path.walk_phases.back().wait_for_npc&&flags.scenario[75]==1&&flags.scenario[76]!=1) {
                    const bool cid=std::any_of(snapshot.obstacles.begin(),snapshot.obstacles.end(),[&](const auto& o){return o.npc&&o.id==30&&std::abs(o.cell.x-end.x)+std::abs(o.cell.y-end.y)==1;});
                    if(cid) {
                        // The route explicitly hands back at Cid, not at the
                        // passage. Native0083 sets76 and places the party13,11;
                        // the ordinary story interaction remains manual.
                        flags.scenario[76]=1;at={0,13,11};pending={};++story_handoffs;continue;
                    }
                }
                if(map==164&&route.path.walk_phases.back().wait_for_npc) {
                    const auto& grids=baked::grids(*snapshot.submap);
                    for(const auto& seen:snapshot.obstacles) {
                        if(!seen.npc||std::abs(seen.cell.x-end.x)+std::abs(seen.cell.y-end.y)!=1)continue;
                        const auto home=std::find_if(grids.objects.begin(),grids.objects.end(),[&](const auto& o){return o.entity_id()==seen.id&&o.type==baked::ObjType::NPC;});
                        if(home==grids.objects.end()||!(home->d&kFlagWanders)||seen.id<22||seen.id>25)continue;
                        for(const auto delta:std::vector<Cell>{{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}) {
                            Cell next{seen.cell.layer,home->tx+delta.x,home->ty+delta.y};
                            if(next==seen.cell||next==end||!baked::walkable(grids,next.layer,next.x,next.y,false))continue;
                            if(std::any_of(snapshot.obstacles.begin(),snapshot.obstacles.end(),[&](const auto& o){return o.id!=seen.id&&o.cell==next;}))continue;
                            patrol_poses[{map,seen.id}]=next;at=end;pending={};++patrol_refreshes;refreshed=true;break;
                        }
                        if(refreshed)break;
                    }
                }
                if(refreshed)continue;
                std::printf("FAIL %s unhandled NPC prefix from%d end%d,%d wait%d\n",name,map,end.x,end.y,route.path.walk_phases.back().wait_for_npc);break;
            }
            if(map==goal.map_id){at=route.path.walk_phases.back().cells.back();reached=true;break;}
            const auto arrival=route.pending.incoming_transition?route.pending.incoming_transition:route.pending.incoming_trap;
            if(!arrival){std::printf("FAIL %s missing actual arrival from%d next%d\n",name,map,route.path.next_map_id);break;}
            pending=route.pending;
            {SnapshotScope scope(snapshot);snapshot.pending=pending;note_trap_start(arrival->map_id,arrival->entry);record_map_arrival(arrival->destination,arrival->arrival);pending=snapshot.pending;}
            map=arrival->destination;at=arrival->arrival;flags.local.fill(0);
        }
        if(!reached){finished=false;break;}
        if(word)words.insert(static_cast<int>(word->n));
        else flags.scenario[receipt]=1;
        if(receipt==30)flags.treasure[65]=1;
        if(receipt==31)flags.treasure[502]=1;
        if(receipt==35)map=1; // native automatic escape, verified separately by source contract
        if(receipt==48){map=192;at={0,12,26};} //0054_2 places the player in the cell
    }
    std::printf("FF2 gates %s: complete%d final%d receipt%d=%d\n",name,finished,map,final_receipt,flags.scenario[final_receipt].value_or(-1));
    if(finished&&flags.scenario[final_receipt]==1)++episodes;else ++failures;
}
}
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff2/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff2/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF2;g_loaded=g_prepared=true;
    episode("Fynn fresh","fynn_incantation",43,{51});
    episode("Fynn boss done","fynn_incantation",43,{51,52});
    episode("Fynn first word","fynn_incantation",43,{51,52},{924});
    episode("Fynn all words","fynn_incantation",43,{51,52},{924,930,931});
    episode("Fynn finished","fynn_incantation",43,{51,52,56});
    episode("Kashuan locked","kashuan_sunfire",149,{25,26});
    episode("Kashuan door opened","kashuan_sunfire",149,{25,26,27});
    episode("Kashuan ally joined","kashuan_sunfire",156,{25,26,27,28});
    episode("Kashuan guardian down","kashuan_sunfire",150,{25,26,27,28,29});
    episode("Kashuan torch","kashuan_sunfire",150,{25,26,27,28,29,30});
    episode("Kashuan finished","kashuan_sunfire",149,{31});
    episode("Masks fresh","mysidia_masks",98,{40,51,52,56,57,59,60});
    episode("Masks statue done","mysidia_masks",207,{40,51,52,56,57,58,59,60});
    episode("Masks finished","mysidia_masks",207,{61});
    episode("Dreadnought guard","dreadnought_escape",164,{31,32},{917});
    episode("Dreadnought entry","dreadnought_escape",169,{31,32,33},{917});
    episode("Dreadnought bars","dreadnought_escape",169,{31,32,33,100},{917});
    episode("Dreadnought captives","dreadnought_escape",169,{31,32,33,34,100},{917});
    episode("Dreadnought finished","dreadnought_escape",164,{35});
    episode("Fynn passage locked","fynn_treasure_passage",21,{52,72},{928});
    episode("Fynn passage clue","fynn_treasure_passage",21,{52,72,88},{928});
    episode("Fynn passage later clue","fynn_treasure_passage",21,{52,72,75,88},{928});
    episode("Fynn passage finished","fynn_treasure_passage",21,{114});
    episode("Semitt hatch","semitt_snowcraft",118,{21});
    episode("Semitt finished","semitt_snowcraft",118,{21,22});
    episode("Coliseum fresh","coliseum_escape",191,{47});
    // Native0054_2 places the party inside the locked cell. PointIn1 is
    // outside it and cannot represent the post-boss prison state.
    episode("Coliseum jailed","coliseum_escape",192,{47,48},{},field::baked_route::Cell{0,12,26});
    episode("Coliseum freed","coliseum_escape",192,{47,48,49});
    episode("Coliseum finished","coliseum_escape",195,{47,48,49,50});
    episode("Fynn mirror","fynn_mirror",41,{51,52,71,72});
    episode("Fynn mirror finished","fynn_mirror",41,{73});
    std::printf("FF2 gate journeys: %d complete episodes/%d production legs, %d legal patrol refreshes/%d native story handoffs, failures%d owner%d\n",episodes,legs,patrol_refreshes,story_handoffs,failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
