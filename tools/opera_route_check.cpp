// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual story model/route consumer and real Opera geometry. Native scenes are
// represented by their reviewed phase receipts and player endpoint facts only.
#define main retained_opera_route_main
#include "route_worker_check.cpp"
#undef main
#include "../src/games/ff6/story_model.h"
namespace field {std::wstring safe_event_label(const std::wstring& s,const std::wstring&,int,int,int){return s;}}
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    json::Value puzzles;const auto bytes=read("data/ff6/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||
       !json::parse(read("data/ff6/puzzles.json"),puzzles))return 2;
    fixture::records.obj[L"story_guides"]=*puzzles.get(L"story_guides");
    fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
    int failures=0;
    const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
    const auto object=[](int map,int id,const wchar_t* group=nullptr)->const baked::Object* {
        const auto* room=baked::submap_by_map_id(map);if(!room)return nullptr;
        for(const auto& o:baked::grids(*room).objects)if(o.entity_id()==id&&
            o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn&&
            (!group||baked::str(o.group)==group))return &o;
        return nullptr;
    };
    const auto cell=[](const baked::Object& o){return Cell{o.layer,o.tx,o.ty};};
    std::thread worker([&] {
        for(int episode=0;episode<8;++episode) {
            map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);
            flags.scenario[156]=flags.scenario[157]=1;flags.global[5]=1;
            if(episode)for(int bit:{158,159,160})flags.scenario[static_cast<size_t>(bit)]=1;
            if(episode==2||episode==3||episode>=5)flags.scenario[161]=flags.scenario[162]=1;
            if(episode==3||episode>=6)flags.scenario[163]=1;
            if(episode>=4)flags.scenario[165]=flags.scenario[174]=1;
            if(episode==5)flags.scenario[164]=1; // prior lyric retry, then a timed failure
            if(episode==7){flags.area[6]=1;for(int local:{2,3,4,5})flags.area[static_cast<size_t>(local)]=1;}
            int map=episode==7?114:112,hops=0,actions=0,rats=0;
            map_state::Resolver initial(flags,fixture::records);
            // Area-local6 is cleared by the failure trip through the world.
            // A retained6=1 is a valid resume only after the director has
            // already moved the player inside at114/PointIn5.
            const auto* entrance=point_in(*baked::submap_by_map_id(map),episode==7?5:1,initial);
            expect(entrance!=nullptr,"Opera exterior native entrance exists");if(!entrance)continue;
            Cell at=cell(*entrance);std::optional<MapGoal> pending;
            std::optional<Cell> stagehand;
            std::set<std::tuple<int,int,int,int,uint32_t>> visited;
            for(int leg=0;leg<128&&flags.scenario[177]!=1;++leg) {
                uint32_t progress=0;for(int bit=156;bit<=177;++bit)progress|=static_cast<uint32_t>(flags.scenario[static_cast<size_t>(bit)].value_or(0))<<(bit-156);
                if(!visited.insert({map,at.layer,at.x,at.y,progress}).second){std::printf("Opera cycle case%d map%d at%d,%d\n",episode,map,at.x,at.y);break;}
                if(flags.current_map_id!=map){flags.local.fill(0);stagehand.reset();}
                flags.current_map_id=map;flags.current_area_id=39;
                ffpr::ff6::story::Snapshot state;state.map=map;state.player=at;state.flags=flags;state.locale=L"en";
                const auto guide=ffpr::ff6::story::plan(state);
                expect(guide.finish_on_scenario==177,"every playable Opera phase retains its actual completion receipt");
                if(guide.targets.empty()){std::printf("Opera no target case%d map%d note%s\n",episode,map,text::to_utf8(guide.note).c_str());break;}
                RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.submap=baked::submap_by_map_id(map);
                snapshot.map_id=map;snapshot.layer=at.layer;snapshot.position=world_of_cell(*snapshot.submap,at);snapshot.locale=L"en";snapshot.vehicle={true,1,1,{}};
                snapshot.candidates=guide.targets;
                if(pending) {
                    const auto same=std::find_if(snapshot.candidates.begin(),snapshot.candidates.end(),[&](const auto& target){return target.goal.map_id==pending->map_id&&target.goal.entity_id==pending->entity_id&&target.goal.raw_name==pending->raw_name;});
                    if(same!=snapshot.candidates.end())std::rotate(snapshot.candidates.begin(),same,std::next(same));
                }
                map_state::Resolver observed(flags,fixture::records);
                for(const auto& original:baked::grids(*snapshot.submap).objects) {
                    if(original.type==baked::ObjType::Warp||original.type==baked::ObjType::PointIn||original.type==baked::ObjType::TransitionHelper||!observed.active(*snapshot.submap,original,true))continue;
                    const auto o=observed.effective(*snapshot.submap,original);
                    if(map==117&&o.type==baked::ObjType::NPC&&o.entity_id()>=9&&o.entity_id()<=13&&flags.scenario[static_cast<size_t>(182-o.entity_id())]==1)continue;
                    NativeObstacle raw;raw.id=o.entity_id();raw.type=static_cast<int>(o.type);raw.raw_name=baked::object_identifier(o);raw.cell=cell(o);
                    if(map==115&&raw.id==9&&o.type==baked::ObjType::NPC&&stagehand)raw.cell=*stagehand;
                    raw.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;raw.chest=o.type==baked::ObjType::TreasureBox;
                    raw.trigger=baked_route::step_event(o)&&!observed.consumed(*snapshot.submap,original);
                    raw.collider=o.type==baked::ObjType::CollisionEntity;raw.collision=o.a;raw.enabled=true;raw.solidity_known=true;raw.solid=(o.d&1)!=0;
                    raw.transportation_ids=observed.transportation_mask(*snapshot.submap,original);raw.transportation_restricted=raw.transportation_ids!=~uint64_t(0);
                    raw.applies=(raw.transportation_ids&baked_route::transportation_bit(1))!=0;snapshot.obstacles.push_back(std::move(raw));
                }
                const auto out=plan_snapshot(snapshot);
                std::printf("Opera case%d map%d at%d,%d L%d target%d:%d success%d next%d npc%d reason%s\n",episode,map,at.x,at.y,at.layer,out.goal.map_id,out.goal.entity_id,out.path.success,out.path.next_map_id,out.path.npc_approach,text::to_utf8(out.path.reason+L" / "+out.path.error).c_str());
                if(!out.path.success)break;
                if(out.path.guidance_index>=0)pending=snapshot.candidates[static_cast<size_t>(out.path.guidance_index)].goal;
                if(out.path.next_map_id>0) {
                    const auto transfer=out.pending.incoming_trap?out.pending.incoming_trap:out.pending.incoming_transition;
                    expect(transfer.has_value(),"every Opera transfer retains its actual arrival");if(!transfer)break;
                    map=transfer->destination;at=transfer->arrival;++hops;continue;
                }
                if(!out.path.walk_phases.empty()&&!out.path.walk_phases.back().cells.empty())at=out.path.walk_phases.back().cells.back();
                int actor=out.goal.entity_id;
                if(actor<0){++actions;continue;} // reached the authored room-entry waypoint; recapture its actual locals
                if(out.path.npc_approach) {
                    const auto& phase=out.path.walk_phases.back();int matches=0;
                    const int dx[]={0,1,0,-1},dy[]={-1,0,1,0};
                    for(const auto& raw:snapshot.obstacles)if(raw.npc&&raw.cell.layer==at.layer) {
                        const bool aimed=phase.arrival_direction>=0&&phase.arrival_direction<4?
                            raw.cell.x==at.x+dx[phase.arrival_direction]&&raw.cell.y==at.y+dy[phase.arrival_direction]:raw.cell==at;
                        if(aimed){actor=raw.id;++matches;}
                    }
                    expect(matches==1,"a native NPC interaction prefix identifies one actual contact from its facing");if(matches!=1)break;
                }
                ++actions;
                if(map==112&&actor==21) {
                    flags.scenario[158]=1;map=114;const auto* manager=object(114,10,L"ev_e_0330");expect(manager!=nullptr,"Impresario anchor exists");if(!manager)break;
                    at=cell(*manager);++at.x;hops+=4; // sc_e_0315_4: Locke18,46/Ground
                } else if(map==112&&(actor==20||actor==19||actor==18)) {
                    if(actor==20)flags.scenario[161]=1;
                    if(actor==20)flags.area[7]=1;else flags.area[6]=1;
                    for(int local:{2,3,4,5})flags.area[static_cast<size_t>(local)]=1;
                    map=114;map_state::Resolver current(flags,fixture::records);const auto* entry=point_in(*baked::submap_by_map_id(map),5,current);
                    expect(entry!=nullptr,"retry scene ends at actual PointIn5");if(!entry)break;at=cell(*entry);hops+=2;
                } else if(map==113&&(actor==6||actor==12)) {
                    flags.scenario[static_cast<size_t>(actor==6?159:164)]=1;
                    const auto* celes=object(113,actor);expect(celes!=nullptr,"Celes's native rehearsal position exists");if(!celes)break;at=cell(*celes);
                } else if(map==113&&actor==7) {
                    // Actual lyric/action producer is checked separately. The
                    // successful scene returns98,10/Ground and sets165, not160.
                    flags.scenario[165]=1;const auto* stage=object(113,7);if(!stage)break;at=cell(*stage);++at.x;at.layer=1;hops+=2;
                } else if(map==113&&actor==13)flags.scenario[166]=1;
                else if(map==112&&actor==17)flags.scenario[175]=1;
                else if(map==114&&(actor==10||actor==9)) {
                    const bool reprise=flags.scenario[175]==1;
                    flags.scenario[167]=1;if(reprise)flags.scenario[176]=1;
                    flags.area[2]=flags.area[3]=flags.area[4]=flags.area[5]=0;
                    const auto* manager=object(114,actor);if(!manager)break;at=cell(*manager);--at.y;hops+=reprise?2:4;
                } else if(map==115&&actor==6)flags.scenario[168]=1;
                else if(map==115&&actor==9) {
                    // Native sc_e_0331 pauses dialogue's timer, moves the man
                    // east to X119 and sets map-local2. The player stays put.
                    const auto* man=object(115,9);if(!man)break;stagehand=cell(*man);++stagehand->x;flags.local[2]=1;
                }
                else if(map==117&&actor>=9&&actor<=13){flags.scenario[static_cast<size_t>(182-actor)]=1;++rats;}
                else if(map==117&&actor==5) {
                    // The winning native continuation visits118,114,123,120;
                    // sc_e_0344_5 alone sets the final completion flag177.
                    flags.scenario[177]=1;map=120;hops+=4;
                } else {expect(false,"every selected Opera action has its reviewed native effect");break;}
            }
            expect(flags.scenario[177]==1,"the complete Opera phase/route consumer reaches its actual final story receipt");
            std::printf("Opera episode%d actions%d hops%d rats%d final%d\n",episode,actions,hops,rats,flags.scenario[177].value_or(-1));
        }
    });worker.join();
    expect(!fixture::wrong_owner,"Opera phase and route consumers invoke no native API on workers");
    std::printf("Opera route check: %d failures\n",failures);return failures?1:0;
}
