// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact factory variants/party selectors and the complete Vector guide.
#define main retained_imperial_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace imperial_fixture {
using namespace floating_fixture;
int cases=0,errors=0,legs=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
void context(map_state::Flags& f){const auto* m=baked::submap_by_map_id(f.current_map_id);const auto* row=fixture::records.get(baked::str(m->name));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
Cell entrance(int map,int id){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==id)return{o.layer,o.tx,o.ty};std::abort();}
ffpr::ff6::story::Plan model(const map_state::Flags& f,Cell at,field::puzzle_policy::Policy policy={}){ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
Planned guided(map_state::Flags& f,Cell at){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;obstacles(s);Planned p;std::thread worker([&]{auto a=model(f,at);if(a.supported&&!a.unknown&&!a.finished&&!a.targets.empty()){s.candidates=a.targets;p=plan_snapshot(s);}});worker.join();std::printf("Imperial map%d from%d,%d,%d goal%d/%d success%d next%d trap%d error%s reason%s\n",s.map_id,at.layer,at.x,at.y,p.goal.map_id,p.goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.error).c_str(),text::to_utf8(p.path.reason).c_str());return p;}
bool camp_walk(map_state::Flags f,Cell goal,bool gates=true){f.current_map_id=162;context(f);const auto* sm=baked::submap_by_map_id(162);baked_route::Request r;r.submap=sm;r.start=entrance(162,1);r.goals={goal};r.passages=true;r.ignore_hazards=true;map_state::Resolver state(f,fixture::records);map_state::add_obstacles(state,*sm,r,gates,false,false);baked_route::Route p;std::thread worker([&]{p=baked_route::find(r);});worker.join();return p.found;}
bool port_walk(map_state::Flags f,bool gates=true){
 f.current_map_id=125;context(f);const auto* sm=baked::submap_by_map_id(125);baked_route::Request r;r.submap=sm;r.start=entrance(125,1);r.goals={entrance(125,8)};r.passages=true;r.ignore_hazards=true;map_state::Resolver state(f,fixture::records);map_state::add_obstacles(state,*sm,r,gates,false,false);baked_route::Route p;std::thread worker([&]{p=baked_route::find(r);});worker.join();
 for(const auto& o:baked::grids(*sm).objects)if(o.type==baked::ObjType::Event&&o.entity_id()==74){const bool active=state.active(*sm,o);check(state.story_barrier(*sm,o),"Albrook local pushback has its exact native identity");check(active==(f.scenario[257]!=1&&f.scenario[284]!=1),"native257/284 own Albrook pushback visibility");if(gates&&active)for(const auto& c:p.cells)check(c.layer!=o.layer||!o.contains(c.x,c.y),"valid alternate port route never crosses the active pushback");}
 std::printf("Albrook257=%d/284=%d gates%d walk%d\n",f.scenario[257].value_or(-1),f.scenario[284].value_or(-1),gates,p.found);return p.found;
}
Planned factory(map_state::Flags f,Cell at){f.current_map_id=143;context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=143;s.submap=baked::submap_by_map_id(143);s.position=world_of_cell(*s.submap,at);s.layer=at.layer;s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.goal.map_id=396;s.goal.cell=entrance(396,1);s.kind=TargetKind::Spot;obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();std::printf("Factory local1=%d/523=%d success%d next%d reason%s\n",f.local[1].value_or(-1),f.scenario[523].value_or(-1),p.path.success,p.path.next_map_id,text::to_utf8(p.path.reason).c_str());return p;}
Planned fly(map_state::Flags f,Cell saved,bool have_save=true,bool cancel=false){
 f.current_map_id=119;context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=119;s.submap=baked::submap_by_map_id(119);const auto at=entrance(119,1);s.position=world_of_cell(*s.submap,at);s.layer=at.layer;s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=true;
 s.goal.map_id=1;s.goal.cell={1,138,203};s.kind=TargetKind::Transition;s.name=L"Albrook";if(have_save)s.saved_vehicles.push_back({4,1,{float(saved.x),float(saved.y),0}});if(cancel)s.cancelled=[] {return true;};obstacles(s);
 Planned p;std::thread worker([&]{
  p=plan_snapshot(s);
  if(!cancel){ExitSnapshot browse(true,s.state);auto& b=browse.route;b.map_id=s.map_id;b.submap=s.submap;b.position=s.position;b.layer=s.layer;b.vehicle=s.vehicle;b.vehicle_shortcuts=s.vehicle_shortcuts;b.saved_vehicles=s.saved_vehicles;b.obstacles=s.obstacles;b.locale=s.locale;
   ExitTarget target;target.position=world_of_cell(*baked::submap_by_map_id(1),s.goal.cell);target.map_id=1;target.destination=125;target.cell=s.goal.cell;
   const auto distances=exit_distances(browse,{target},{});
   check(distances.size()==1&&(distances[0]>=0)==p.path.success,"Blackjack exit browsing and selected directions agree on exact saved-flight availability");}
 });worker.join();std::printf("Blackjack saved%d,%d present%d success%d next%d error%s reason%s\n",saved.x,saved.y,have_save,p.path.success,p.path.next_map_id,text::to_utf8(p.path.error).c_str(),text::to_utf8(p.path.reason).c_str());return p;
}
}
int main(){
 using namespace imperial_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto f=floating_fixture::flags();f.scenario.fill(0);f.party_terra=false;f.active_party_members=2;f.current_map_id=143;context(f);Cell at=entrance(143,1);
 auto p=guided(f,at);check(p.path.success&&p.goal.entity_id==61&&!p.path.required_trap,"Vector begins with the real old man");check(model(f,at).targets[0].instruction==L"Choose the first option.","native Yes is the first option");
 check(model(f,at).targets[0].goal.entity_id==61,"refusal cannot invent the native distraction");
 const auto oldman_path=p;p=factory(f,at);check(!p.path.success&&p.path.reason==L"Story gated","ordinary directions cannot cross factory detection regions before the real distraction");
 for(const auto& phase:oldman_path.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();f.local[1]=1;
 p=factory(f,at);check(p.path.success&&p.path.next_map_id==143&&!p.path.required_trap,"after real Yes ordinary directions use the actual crate landing, not guard detection");
 p=guided(f,at);check(p.path.success&&p.goal.entity_id==60&&!p.path.required_trap,"actual distraction exposes a reachable crate trigger");
 f.scenario[523]=1;at={1,57,34};p=guided(f,at);check(p.path.success&&p.goal.map_id==143&&p.goal.entity_id>=28&&p.goal.entity_id<=30&&!p.path.required_trap,"actual final crate landing reaches a real factory entrance without a false trap");
 auto advice=model(f,at);check(advice.finish_on_exit==396,"entry guide finishes at the actual first factory arrival");
 f.scenario[523]=0;f.local[1]=0;advice=model(f,at);check(advice.targets[0].goal.entity_id==61,"guard aftermath's reset returns to real distraction instead of a stale crate step");
 const auto* vector=baked::submap_by_map_id(143);int doors=0;
 for(int phase=0;phase<3;++phase){if(phase<2)f.scenario[194]=phase;else f.scenario[194].reset();map_state::Resolver state(f,fixture::records);int early=0,late=0;
  for(const auto& o:baked::grids(*vector).objects)if(o.type==baked::ObjType::Warp&&o.entity_id()>=28&&o.entity_id()<=30){if(phase==0)++doors;if(state.active(*vector,o)&&!state.consumed(*vector,o)){early+=o.a==396;late+=o.a==643;}}
  check(phase==0?(early==3&&late==0):phase==1?(early==0&&late==3):(early==0&&late==0),"all three entrances select only the actual native194 variant;unknown cannot certify either");
 }
 check(doors==6,"three physical factory doors each retain both exact native destinations");
 f.scenario[194]=1;check(!model(f,at).supported,"completed first factory chapter does not reopen the infiltration guide");
 f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=162;context(f);at=entrance(162,1);f.party_terra=true;f.active_party_members=2;
 check(!model(f,at).supported,"camp party guide cannot appear before native207");check(!camp_walk(f,entrance(162,2)),"early imperial soldiers block onward camp travel even with NPC avoidance off");f.scenario[207]=1;
 const auto* camp=baked::submap_by_map_id(162);const Cell beyond=entrance(162,2);
 for(int mode=0;mode<4;++mode){f.party_terra=mode!=0;f.active_party_members=mode==1?1:2;if(mode==3){f.party_terra.reset();f.active_party_members.reset();}
  map_state::Resolver state(f,fixture::records);check(mode==3?!state.known(*camp):state.known(*camp),"exact party initializer is known only with owned native inputs");
  int absent=0,solo=0,allowed=0;for(const auto& o:baked::grids(*camp).objects)if(o.type==baked::ObjType::Event&&state.active(*camp,o)){const auto group=baked::str(o.group);absent+=group==L"ev_e_0440";solo+=group==L"ev_e_0441";allowed+=group==L"ev_e_0442";}
  if(mode<3)check(mode==0?(absent>0&&!solo&&!allowed):mode==1?(solo>0&&!absent&&!allowed):(allowed==5&&!absent&&!solo),"native missing-Terra/solo/allowed variants are mutually exclusive");
  const bool reachable=camp_walk(f,beyond);std::printf("Camp mode%d known%d absent%d solo%d allowed%d walk%d\n",mode,state.known(*camp),absent,solo,allowed,reachable);check(reachable==(mode==2),"NPC avoidance off cannot route onward through native party pushback");
  check(!model(f,at).supported,"the ordinary party requirement never creates a puzzle guide");
 }
 f.party_terra=true;f.active_party_members=2;
 f.scenario[217]=1;check(camp_walk(f,beyond),"qualified return after217 is physically open");f.party_terra=false;check(!camp_walk(f,beyond),"missing Terra still triggers native pushback on repeat visits before225");
 f.party_terra=true;f.active_party_members=2;f.scenario[225]=1;check(!model(f,at).supported,"later225 phase retires this party guide");
 f=floating_fixture::flags();f.scenario.fill(0);check(port_walk(f)&&port_walk(f,false),"Albrook preserves the real alternate port route around the local pushback");f.scenario[257]=1;check(port_walk(f),"native257 retires the Albrook local guard");f.scenario[257]=0;f.scenario[284]=1;check(port_walk(f),"native284's later phase also removes the local pushback");
 f=floating_fixture::flags();f.scenario.fill(0);f.scenario[178]=1;f.scenario[194]=1;f.scenario[226]=1;f.scenario[282]=1;f.treasure[903]=1;
 p=fly(f,{1,151,74});check(p.path.success&&p.path.next_map_id==1&&p.pending.goal.map_id==1&&p.pending.goal.cell==Cell{1,138,203},"Blackjack proves actual saved-pose continuation and retains final destination");
 check(p.pending.incoming_transition&&p.pending.incoming_transition->arrival==Cell{1,151,74},"flight receipt uses actual saved pose, never fallback Narshe PointIn1");
 check(!p.path.walk_phases.empty()&&p.path.walk_phases.back().manual_after&&p.path.walk_phases.back().action_after.find(L"Choose the first option.")!=std::wstring::npos,"ordinary flight menu uses first option before284");
 f.scenario[284]=1;p=fly(f,{1,76,45});check(p.path.success&&p.pending.incoming_transition&&p.pending.incoming_transition->arrival==Cell{1,76,45}&&p.path.walk_phases.back().action_after.find(L"Choose the second option.")!=std::wstring::npos,"moved aircraft and later second-option Fly are both respected");
 p=fly(f,{1,151,74},false);check(!p.path.success,"missing owned aircraft pose cannot invent a world departure");
 f.treasure[903].reset();check(!fly(f,{1,151,74}).path.success,"unknown flight ownership cannot invent a native branch");
 f.treasure[903]=0;p=fly(f,{1,151,74},false);check(p.path.success&&p.pending.incoming_transition&&p.pending.incoming_transition->arrival==Cell{1,22,93}&&p.path.walk_phases.back().action_after.find(L"Choose")==std::wstring::npos,"native guest branch uses actual SetVehicle22,93 and has no menu");
 check(!fly(f,{1,151,74},false,true).path.success,"cancellation stops flight expansion");
 int dynamic=0;for(const auto& o:baked::grids(*baked::submap_by_map_id(119)).objects)dynamic+=(o.type==baked::ObjType::Warp&&(o.d&baked::kBlackjackSavedFlight))!=0;check(dynamic==1,"only exact native Blackjack control owns dynamic flight metadata");
 for(const auto& e:edges_of(baked::grids(*baked::submap_by_map_id(119)),119))check(!(e.object->d&baked::kBlackjackSavedFlight),"ordinary graph cannot certify a false static Narshe landing");
 check(fixture::wrong_owner==0,"all planner/model work uses owned data without native worker access");
 std::printf("Imperial:%d assertions/%d failures;representative native-bake poses,not game/audible timing\n",cases,errors);return errors?1:0;
}
