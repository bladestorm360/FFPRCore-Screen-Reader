// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Source-qualified current controller/model and production-route continuations.
#define main retained_dream_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace dream_fixture {
using namespace floating_fixture;
int failures=0,count=0;
Cell last_goal;
void joined_pose(Cell& at){if(at.x<last_goal.x)at.x+=2;else if(at.y<last_goal.y){++at.x;++at.y;}else if(at.y>last_goal.y){++at.x;--at.y;}}
void expect(bool ok,const char* why){++count;if(!ok){++failures;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
ffpr::ff6::story::Plan model(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){context(f);ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
Planned raw(map_state::Flags f,int map,int id,Cell at){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.position=world_of_cell(*s.submap,at);s.layer=at.layer;s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.goal.map_id=map;s.goal.entity_id=id;s.kind=TargetKind::Interact;for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp&&o.entity_id()==id){s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(o);break;}obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();return p;}
bool journey(map_state::Flags& f,Cell& at,int expected){
 for(int leg=0;leg<48;++leg){
  context(f);auto advice=model(f,at);if(!advice.supported||advice.unknown||advice.finished||advice.targets.empty()){std::printf("NO GUIDE map%d unknown%d finished%d\n",f.current_map_id,advice.unknown,advice.finished);return false;}
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.candidates=std::move(advice.targets);obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  std::printf("Dream map%d from%d,%d,%d goal%d map%d next%d success%d trap%d %s\n",f.current_map_id,at.layer,at.x,at.y,p.goal.entity_id,p.goal.map_id,p.path.next_map_id,p.path.success,p.path.required_trap,text::to_utf8(p.path.full_description).c_str());
  const bool same=p.goal.entity_id==expected||(expected==56&&p.goal.entity_id==57)||(expected==52&&p.goal.entity_id==53)||(expected==2&&p.goal.entity_id>=2&&p.goal.entity_id<=8)||(expected==1&&p.goal.entity_id==5);
  if(!p.path.success||p.path.required_trap||!same){std::printf("reason %s error %s\n",text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str());return false;}
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();last_goal=p.goal.cell;return true;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;
  f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
  if(receipt->map_id==p.goal.map_id&&receipt->entity_id==p.goal.entity_id)return true;
 }
 return false;
}
}
int main(){using namespace dream_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto f=floating_fixture::flags();f.scenario.fill(0);f.scenario[365]=1;f.current_map_id=300;f.party_cyan=false;f.active_party_members=4;context(f);auto at=entry(300);
 expect(model(f,at).note==L"Bring Cyan in a full four-character active party.","reserve Cyan cannot satisfy the actual entry requirement");
 expect(!raw(f,520,55,at).path.success,"ordinary routing cannot cross a missing-Cyan dream entrance");
 f.party_cyan=true;f.active_party_members=3;expect(model(f,at).note==L"Bring Cyan in a full four-character active party.","three active characters do not start the dream");
 expect(!raw(f,520,55,at).path.success,"ordinary routing cannot cross the incomplete-party dream entrance");
 f.active_party_members=4;auto plan=model(f,at);expect(plan.targets.size()==1&&plan.targets[0].goal.entity_id==-1&&plan.targets[0].instruction==L"Move one step north into the bed, then choose the first option to rest.","qualified entry uses the correct south-side bed waypoint");
 expect(model(f,at,{true,false}).targets.empty(),"Other off withholds the rest choice without skipping into the dream");
 expect(journey(f,at,-1),"guided approach reaches the south bed waypoint");
 auto normal=raw(f,520,55,at);std::printf("Ordinary dream entry success%d next%d reason%s error%s %s\n",normal.path.success,normal.path.next_map_id,text::to_utf8(normal.path.reason).c_str(),text::to_utf8(normal.path.error).c_str(),text::to_utf8(normal.path.full_description).c_str());expect(normal.path.success&&normal.pending.incoming_transition&&normal.pending.incoming_transition->arrival==Cell{1,22,21},"qualified ordinary entry retains the full actual dream arrival rather than door PointIn1");
 if(!normal.path.success){map_state::Resolver state(f,fixture::records);const auto* sm=baked::submap_by_map_id(300);for(const auto& o:baked::grids(*sm).objects)if(o.entity_id()==8)std::printf("Bed type%d %d,%d,%d a%d b%d d%d active%d consumed%d\n",int(o.type),o.layer,o.tx,o.ty,o.a,o.b,o.d,state.active(*sm,o),state.consumed(*sm,o));}
 bool north=false;if(!normal.path.walk_phases.empty()){const auto& cells=normal.path.walk_phases.back().cells;if(cells.size()>1)north=cells.back().x==cells[cells.size()-2].x&&cells.back().y==cells[cells.size()-2].y-1;}
 expect(north,"ordinary dream entry advances north into the bed from its only native accepted side");
 f.party_cyan.reset();expect(model(f,at).unknown,"unknown active Cyan is not assumed absent or present");
 expect(!raw(f,520,55,at).path.success,"unknown active Cyan cannot certify an ordinary route behind the dream entry");
 f.party_cyan=true;f.active_party_members.reset();expect(!raw(f,520,55,at).path.success,"unknown current member count cannot certify the dream entry");f.active_party_members=4;
 f.party_cyan=true;f.scenario[435]=1;f.scenario[961]=1;f.current_map_id=520;context(f);at={1,22,21};
 expect(journey(f,at,56),"first actual brother-sighting trigger is reachable through the native looping doors");f.scenario[436]=1;
 expect(journey(f,at,55),"first real lost-member interaction is reachable after the sighting");f.scenario[437]=1;joined_pose(at);
 expect(journey(f,at,52),"second actual sighting is reachable through the native maze");f.scenario[438]=1;
 expect(journey(f,at,51),"second real lost-member interaction is reachable after its sighting");f.scenario[439]=1;joined_pose(at);
 expect(journey(f,at,8),"both true reunion receipts route to the actual brothers encounter");f.scenario[440]=1;
 expect(journey(f,at,7),"actual brothers victory selects the real train passage");
 f.current_map_id=522;f.scenario[441]=1;context(f);plan=model(f,entry(522));expect(!plan.targets.empty()&&plan.targets[0].goal.map_id==522&&plan.targets[0].goal.entity_id==1,"the existing train guide owns its exact maps unchanged");
 f.current_map_id=528;f.scenario[449]=1;context(f);at=entry(528);expect(journey(f,at,0),"armor guide reaches the correct exit rather than a looping return");
 f.current_map_id=529;f.scenario[450]=1;context(f);at=entry(529,3);expect(journey(f,at,9),"second armor room reaches the actual dismount/Doma passage");
 f.current_map_id=530;f.scenario[451]=1;context(f);at=entry(530);expect(raw(f,537,2,at).path.success,"ordinary native route to the boss remains open without optional452/453 or family454");expect(journey(f,at,0),"actual Doma arrival routes through the full family-room episode");
 f.current_map_id=536;f.scenario[454]=1;context(f);at=entry(536);plan=model(f,at);expect(plan.puzzle_types==2&&plan.targets.size()==7,"possession battle instruction is a separate Other solution");
 expect(model(f,at,{true,false}).targets.empty(),"Other off stops at the boss mechanic without skipping to its reward");
 expect(journey(f,at,2),"family-room return reaches the actual Wrexsoul encounter");
 f.current_map_id=300;context(f);at=entry(300);expect(!raw(f,304,1,at).path.success,"final animated pickup is unavailable before the native waking receipt455");
 f.scenario[455]=1;plan=model(f,at);expect(plan.finish_on_scenario==456&&plan.puzzle_types==1,"waking is followed by the final pickup using Navigation guidance");
 expect(raw(f,304,1,at).path.success,"uncollected final animated pickup accepts an ordinary cross-floor path after waking");
 expect(journey(f,at,1),"post-waking route reaches the actual animated final pickup across Doma floors");
 f.scenario[456]=1;expect(model(f,at).finished,"only the native acquisition receipt finishes the complete episode");expect(!raw(f,304,1,at).path.success,"consumed native456 removes the animated alias normally");
 expect(ffpr::ff6::story::supported_map(300)&&ffpr::ff6::story::supported_map(520)&&ffpr::ff6::story::supported_map(529)&&ffpr::ff6::story::supported_map(537),"new native maps participate in the existing guide-entry lifecycle");
 expect(fixture::wrong_owner==0,"all production route/model work uses owned copied state");
 std::printf("Dream:%d assertions/%d failures;offline route fixture,not game response timing\n",count,failures);return failures||fixture::wrong_owner?1:0;
}
