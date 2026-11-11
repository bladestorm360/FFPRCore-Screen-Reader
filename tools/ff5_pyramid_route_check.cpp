// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Finite source-to-route investigation for the still-open Pyramid guide.
#define main retained_pyramid_floating_main
#include "floating_continent_route_check.cpp"
#undef main
#include "../src/games/ff5/pyramid_model.cpp"
namespace pyramid_fixture {
using namespace floating_fixture;
int errors=0,checks=0;
int chosen_id=-3,chosen_map=0;
std::array<std::optional<bool>,3> corridor_hidden{false,false,false};
void check(bool value,const char* why){++checks;if(!value){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
Planned route(map_state::Flags f,Cell at,int map,int id){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.goal.map_id=map;s.goal.entity_id=id;s.goal.avoid_other_events=true;s.goal.allow_scripted_crossings=true;
 const auto* target=baked::submap_by_map_id(map);for(const auto& o:baked::grids(*target).objects)if(o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn&&o.entity_id()==id){s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(o);s.kind=o.type==baked::ObjType::GotoMap?TargetKind::Transition:baked_route::step_event(o)?TargetKind::Spot:TargetKind::Interact;break;}
 obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();std::printf("Pyramid map%d from%d,%d,%d ->%d/%d success%d next%d trap%d reason%s error%s %s\n",f.current_map_id,at.layer,at.x,at.y,map,id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str(),text::to_utf8(p.path.full_description).c_str());return p;
}
ff5::library::Plan advice(map_state::Flags f,Cell at,bool fresh=true,std::optional<bool> hidden=false,bool frozen=false){context(f);ff5::pyramid::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.phase_fresh=fresh;s.last_sand_switch_hidden=hidden;s.corridor_hidden=corridor_hidden;s.timers_frozen=frozen;s.locale=L"en";return ff5::pyramid::plan(s,fixture::guide_recipes);}
void arrive(map_state::Flags& f,Cell& at,int map,int point=1){f.current_map_id=map;f.local.fill(0);if(map==416)f.scenario[436]=f.scenario[437]=0;context(f);at=entry(map,point);}
void live_obstacles(RouteSnapshot& s,const map_state::Flags& f){
 obstacles(s);
 if(s.map_id==413&&corridor_hidden[0]==true){
  s.obstacles.erase(std::remove_if(s.obstacles.begin(),s.obstacles.end(),[](const auto& o){return o.id==1||o.id==117||o.id==118;}),s.obstacles.end());
  for(const auto& o:baked::grids(*s.submap).objects)if(o.type==baked::ObjType::NPC&&(o.entity_id()==117||o.entity_id()==118)&&corridor_hidden[o.entity_id()==117?1:2]==false){NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(o);n.cell={1,o.entity_id()==117?8:9,o.entity_id()==117?8:7};n.npc=true;n.collision=o.a;s.obstacles.push_back(n);}
 }
 if(s.map_id==421&&f.scenario[391]==1&&f.scenario[392]==0){
  // Exact live operation0520_1 hides only the second fall/control. Its
  // reloaded initializer reuses391 for the third. Simulate the native loaded
  // actors here,rather than claiming the initializer is live-event evidence.
  for(const auto& o:baked::grids(*s.submap).objects)if((o.entity_id()==12&&o.type==baked::ObjType::Entity)||(o.entity_id()==9&&o.type==baked::ObjType::Event)){
   NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(o);n.cell={o.layer,o.tx,o.ty};n.collision=o.a;n.trigger=o.type==baked::ObjType::Event;s.obstacles.push_back(n);
  }
 }
}
bool journey(map_state::Flags& f,Cell& at,int expected_map,int expected_id){
 const auto plan=advice(f,at);if(plan.unknown||plan.finished||plan.targets.empty())return false;
 for(int leg=0;leg<24;++leg){context(f);
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.candidates=plan.targets;live_obstacles(s,f);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  std::printf("Pyramid guide map%d from%d,%d,%d ->%d/%d success%d next%d trap%d %s %s\n",f.current_map_id,at.layer,at.x,at.y,p.goal.map_id,p.goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
  if(!p.path.success||p.path.required_trap||(expected_map!=-2&&p.goal.map_id!=expected_map)||(expected_id!=-2&&p.goal.entity_id!=expected_id))return false;
  chosen_id=p.goal.entity_id;chosen_map=p.goal.map_id;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;
  if(receipt->map_id==411)for(int bit:{378,379,380,387})f.scenario[size_t(bit)]=0;
  if(receipt->map_id==415)for(int bit=381;bit<=386;++bit)f.scenario[size_t(bit)]=0;
  if(receipt->map_id==421){for(int bit:{390,391,392})f.scenario[size_t(bit)]=0;if(receipt->destination==422)f.scenario[388]=0;}
  f.current_map_id=receipt->destination;f.local.fill(0);at=receipt->arrival;context(f);
  if(f.current_map_id==416)f.scenario[436]=f.scenario[437]=0;
  if(expected_id<0&&f.current_map_id==p.goal.map_id&&at==p.goal.cell)return true;
 }return false;
}
int full_check(){auto f=floating_fixture::flags();f.scenario.fill(0);f.scenario[135]=1;f.current_map_id=410;Cell at=entry(410);context(f);
 auto p=advice(f,at);check(p.targets.size()==1&&p.targets[0].goal.entity_id==17,"native entrance begins at the real guardian door");check(journey(f,at,410,17),"actual exterior arrival reaches the physical guardian door");f.scenario[136]=1;check(journey(f,at,411,-1),"full opened entrance reaches the real first-floor arrival");
 check(journey(f,at,411,15),"first-floor entrance reaches the real left needle toggle");f.scenario[378]=1;check(journey(f,at,411,17),"left-toggle position reaches the real right toggle with middle left off");f.scenario[380]=1;check(journey(f,at,415,-1),"needle solution reaches the actual snake-room entry with native exit resets");
 check(journey(f,at,415,1),"fresh snake room reaches its real activation trigger");f.scenario[384]=1;
 // Defeat the two optional live snakes here to isolate the mandatory door
 // ordering. Separate fallback cases below exercise their actual positions.
 f.scenario[385]=f.scenario[386]=1;
 check(journey(f,at,415,117),"first reachable switch opens actual door115 before the northern control");f.scenario[382]=1;check(journey(f,at,415,119),"opened first door permits the next physical switch");f.scenario[381]=1;check(journey(f,at,415,120),"next actual switch reaches the final snake-room door control");f.scenario[383]=1;check(journey(f,at,416,-1),"all three opened doors permit the real next-floor transition");
 check(journey(f,at,416,4),"third-floor native hazards retain a safe route to the real fourth-floor exit");arrive(f,at,417);check(journey(f,at,417,23),"actual fourth-floor arrival reaches the passage coffin");f.scenario[436]=1;at={1,10,15};
 check(journey(f,at,417,129),"opened coffin reaches the actual moved enemy,not its off-map home");f.scenario[325]=1;check(journey(f,at,419,-1),"defeated passage enemy permits the full fourth/save/fifth-floor journey");
 check(journey(f,at,421,-1),"fifth-floor main passage reaches the actual first quicksand platform");check(journey(f,at,421,10),"first quicksand switch is reachable before its fall retires");f.scenario[390]=1;check(journey(f,at,421,11),"first retired fall opens the second actual quicksand switch");f.scenario[391]=1;
 check(journey(f,at,421,12),"loaded third switch stays physically available before392 despite the reload-only391 hide");f.scenario[392]=1;check(journey(f,at,421,5),"all real retired falls permit the native seventh-floor exit");arrive(f,at,423);for(int bit:{390,391,392})f.scenario[size_t(bit)]=0;
 check(journey(f,at,424,-1),"seventh-floor arrival reaches the actual timed eighth-floor entrance");f.area[0]=0;p=advice(f,at);check(p.targets.empty()&&p.note==L"Wait for the floor to change.","lower entry waits for native phase1");f.area[0]=1;check(journey(f,at,424,-1),"phase1 reaches a real safe hold");check(at==Cell{1,11,13},"first timing leg ends on the exact phase-invariant left hold");
 p=advice(f,at);check(p.targets.empty(),"safe hold waits for the opposite phase instead of walking onto its pits");f.area[0]=0;check(journey(f,at,424,-1),"phase0 reaches the real northern safe floor");check(at==Cell{1,10,10},"second timing leg ends beyond all changing pits");check(journey(f,at,425,-1),"upper safe floor reaches the actual no-slide room entrance");check(journey(f,at,426,3),"sliding-room entrance reaches the actual tablet with no trap or slide");
 f.scenario[137]=1;arrive(f,at,410,3);check(advice(f,at).finished,"only full137 finishes after the complete return to the exterior");
 f.scenario[137].reset();check(advice(f,at).unknown,"missing final receipt cannot certify completion");
 // Actual fall/re-entry points and the still-living snakes are independent
 // acceptance states,not inferred from the clean end-to-end journey above.
 for(int map:{412,413,414}){auto state=floating_fixture::flags();state.scenario.fill(0);state.scenario[135]=state.scenario[136]=1;Cell here;arrive(state,here,map);corridor_hidden={true,true,true};check(journey(state,here,411,-1),"first-floor side/fall recovery reaches its actual first-floor return before restarting reset needles");
  if(map==414){check(journey(state,here,411,9),"coffin alcove leaves through its actual middle corridor without inventing a direct needle-gallery passage");arrive(state,here,413,2);corridor_hidden={false,false,false};check(journey(state,here,413,1),"middle corridor first crosses its actual flagless snake activation");corridor_hidden={true,false,false};bool escaped=false;
   for(int step=0;step<3;++step){if(!journey(state,here,-2,-2))break;if(chosen_map==411){escaped=true;break;}if(chosen_id!=117&&chosen_id!=118)break;corridor_hidden[chosen_id==117?1:2]=true;}
   check(escaped,"activated middle corridor clears actual blocking snakes and uses its opposite stair");}
  check(journey(state,here,411,18),"recovered first-floor position reaches the actual quicksand-stop control");state.scenario[378]=state.scenario[380]=state.scenario[387]=1;state.scenario[379]=0;check(journey(state,here,415,-1),"native stop101/387 solution reaches the real snake-room entrance after every side/fall recovery");}
 {auto state=floating_fixture::flags();state.scenario.fill(0);state.scenario[135]=state.scenario[136]=state.scenario[384]=1;Cell here;arrive(state,here,415);here={1,7,11};bool advanced=false;
  for(int step=0;step<6;++step){if(state.scenario[381]==1&&state.scenario[382]==1&&state.scenario[383]==1){advanced=journey(state,here,416,-1);break;}if(!journey(state,here,415,-2))break;
   const int bit=chosen_id==117?382:chosen_id==119?381:chosen_id==120?383:chosen_id==113?385:chosen_id==114?386:-1;if(bit<0||state.scenario[size_t(bit)]==1)break;state.scenario[size_t(bit)]=1;
  }check(advanced,"actual living-snake fallback operations finish the door puzzle without a trap or dead end");}
 for(int point:{4,5,6}){auto state=floating_fixture::flags();state.scenario.fill(0);state.scenario[135]=state.scenario[136]=1;Cell here;arrive(state,here,419,point);if(point==4){check(journey(state,here,419,31),"northern fall reaches the physical door-opening trigger before the blocked passage");state.scenario[400]=1;check(journey(state,here,419,32),"opened northern door crosses its real closing trigger before returning to main routing");state.scenario[400]=0;}check(journey(state,here,421,-1),"each real sixth-floor fall returns safely to the onward sixth-floor entrance");}
 {auto state=floating_fixture::flags();state.scenario.fill(0);state.scenario[135]=state.scenario[136]=1;Cell here;arrive(state,here,422);check(journey(state,here,422,49),"side-room arrival reaches its exact temporary wall switch");state.scenario[388]=1;here.y-=3;check(journey(state,here,421,-1),"native three-step wall opening first reaches the exact eastern return arrival");check(journey(state,here,421,11),"side-room return reaches its second switch before any fall");state.scenario[391]=1;check(journey(state,here,421,12),"live side return still operates the visible third switch before392");state.scenario[392]=1;check(journey(state,here,421,5),"side-return third switch permits the actual onward exit without forcing the unreachable first switch");}
 {auto state=floating_fixture::flags();state.scenario.fill(0);state.scenario[135]=state.scenario[136]=1;state.current_map_id=412;Cell here{1,8,17};check(journey(state,here,411,-1),"full quicksand callback landing,not merely its nominal PointIn,reaches the first-floor stairs");}
 {auto state=floating_fixture::flags();state.scenario.fill(0);state.scenario[135]=state.scenario[136]=1;state.area[0]=1;Cell here{1,14,16};state.current_map_id=424;
  check(journey(state,here,424,-1)&&here==Cell{1,13,13},"right-side approach reaches the exact invariant hold in phase1");
  check(journey(state,here,424,-1)&&here==Cell{1,11,13},"phase1 crosses directly between the invariant holds before taking the western route");state.area[0]=0;
  check(journey(state,here,424,-1)&&here==Cell{1,10,10},"western phase0 leg reaches the safe northern floor");check(journey(state,here,425,-1),"western northern floor reaches the no-slide entrance");check(journey(state,here,426,3),"right-side lower approach finishes via the safe western return room");
  arrive(state,here,425,2);check(journey(state,here,424,-1),"wrong eastern return-room entrance recovers through its real eighth-floor exit");check(journey(state,here,424,-1)&&here==Cell{1,14,10},"eastern upper arrival first reaches its invariant northern safe floor");
  check(journey(state,here,424,-1)&&here==Cell{1,13,13},"phase0 reverses from eastern northern floor to its invariant hold");state.area[0]=1;check(journey(state,here,424,-1)&&here==Cell{1,11,13},"phase1 carries eastern recovery across the invariant holds");state.area[0]=0;check(journey(state,here,424,-1)&&here==Cell{1,10,10},"eastern recovery returns to the western northern safe floor");check(journey(state,here,425,-1)&&journey(state,here,426,3),"full wrong-entrance recovery reaches the tablet without any sliding event");
  arrive(state,here,425,3);check(journey(state,here,426,3),"central returned-room arrival also reaches the real tablet safely");
  state.current_map_id=424;here={1,12,16};state.area[0].reset();check(advice(state,here).unknown,"unknown timed-floor phase cannot publish a crossing");}
 {auto state=floating_fixture::flags();state.current_map_id=413;corridor_hidden={true,std::nullopt,false};const auto unknown=advice(state,{1,11,7});check(unknown.unknown&&unknown.targets.empty(),"partially unknown corridor observations cannot retain earlier target candidates");}
 check(fixture::wrong_owner==0,"all Pyramid policy and planner work uses owned state");std::printf("Pyramid:%d assertions/%d failures;full native phase timing remains a separate owner check\n",checks,errors);return errors||fixture::wrong_owner?1:0;
}
}
int main(int argc,char** argv){using namespace pyramid_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;auto f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=411;Cell at=entry(411);
 if(argc==1)return full_check();
 if(argc==2){json::Value cases;if(!json::parse(read(argv[1]),cases)||cases.arr.size()>128)return 2;for(const auto& row:cases.arr){auto state=f;state.current_map_id=int(row.get(L"map")->n);apply(state,row.get(L"flags"));Cell start=entry(state.current_map_id,row.get(L"point")?int(row.get(L"point")->n):1);if(const auto* p=row.get(L"start")){if(p->arr.size()!=3)return 2;start={int(p->arr[0].n),int(p->arr[1].n),int(p->arr[2].n)};}const auto* goal=row.get(L"goal");if(!goal||goal->arr.size()!=2)return 2;route(state,start,int(goal->arr[0].n),int(goal->arr[1].n));}std::printf("Pyramid finite route investigation;no guide completeness claim;owner%d\n",fixture::wrong_owner.load());return fixture::wrong_owner?1:0;}
 for(int mask=0;mask<8;++mask){for(int i=0;i<3;++i)f.scenario[size_t(378+i)]=(mask>>i)&1;std::printf("Pyramid needle mask%d\n",mask);for(int id:{15,17,18,5})route(f,at,411,id);}f.scenario.fill(0);
 route(f,at,411,18);route(f,at,426,3);f.scenario[387]=1;route(f,at,426,3);route(f,at,411,20);f.scenario[196]=1;route(f,at,426,3);
 for(int map:{415,416,417,419,421,422,423,424,425}){f.current_map_id=map;at=entry(map);route(f,at,426,3);}
 std::printf("Pyramid initial route investigation;no guide completeness claim;owner%d\n",fixture::wrong_owner.load());return fixture::wrong_owner?1:0;
}
