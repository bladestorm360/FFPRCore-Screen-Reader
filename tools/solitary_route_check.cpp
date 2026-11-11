// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Current source/route baseline for the complete fishing and raft episode.
#define main retained_solitary_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace solitary_fixture {
using namespace floating_fixture;
int errors=0,cases=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
ffpr::ff6::story::Plan model(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){context(f);ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
bool journey(map_state::Flags& f,Cell& at,int expected,bool native_bird=false){
 for(int leg=0;leg<16;++leg){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.position=world_of_cell(*s.submap,at);s.layer=at.layer;s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};obstacles(s);
  // Native0627_2 explicitly selects0628 after landing. This positive current
  // scene actor is absent from the empty cliff initializer, so model the
  // actual owner capture rather than inventing a permanent group projection.
  if(native_bird&&s.map_id==193){std::erase_if(s.obstacles,[](const auto& n){return n.npc&&n.id==1;});for(const auto& o:baked::grids(*s.submap).objects)if(o.type==baked::ObjType::NPC&&o.entity_id()==1){NativeObstacle n;n.type=int(o.type);n.id=1;n.raw_name=baked::object_identifier(o);n.cell={o.layer,o.tx,o.ty};n.npc=true;n.collision=1;s.obstacles.push_back(n);}}
  Planned p;std::thread worker([&]{const auto a=model(f,at);if(a.supported&&!a.unknown&&!a.finished&&!a.targets.empty()){s.candidates=a.targets;p=plan_snapshot(s);}});worker.join();
  std::printf("Island leg%d map%d from%d,%d,%d goal%d/%d success%d next%d trap%d npc%d reason%s error%s\n",leg,f.current_map_id,at.layer,at.x,at.y,p.goal.map_id,p.goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,p.path.npc_approach,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str());
  if(!p.path.success||p.path.required_trap||p.goal.entity_id!=expected)return false;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  if(!p.pending.incoming_transition)return false;f.current_map_id=p.pending.incoming_transition->destination;at=p.pending.incoming_transition->arrival;
 }
 return false;
}
Planned raw(map_state::Flags f,int map,int id,Cell at){
 context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.position=world_of_cell(*s.submap,at);s.layer=at.layer;s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.goal.map_id=map;s.goal.entity_id=id;s.kind=TargetKind::Interact;
 for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp&&o.entity_id()==id){s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(o);break;}
 obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();std::printf("Solitary map%d ->%d/%d life%d death%d bird%d success%d next%d reason%s\n",f.current_map_id,map,id,f.scenario[305].value_or(-1),f.scenario[525].value_or(-1),f.scenario[308].value_or(-1),p.path.success,p.path.next_map_id,text::to_utf8(p.path.reason).c_str());return p;
}
Planned world_route(map_state::Flags f,Cell at,const MapGoal& goal){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.position=world_of_cell(*s.submap,at);s.layer=at.layer;s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.goal=goal;s.kind=TargetKind::Transition;obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();std::printf("Raft regular map%d success%d next%d trap%d reason%s\n",s.map_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str());return p;}
}
int main(){using namespace solitary_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=191;f.scenario[297]=1;f.scenario[300]=1;
 auto p=raw(f,597,3,entry(191));check(!p.path.success&&p.path.reason==L"Story gated","existing raft gate remains closed before either real Cid outcome");f.scenario[305]=1;check(raw(f,597,3,entry(191)).path.success,"native recovery opens the real raft approach");f.scenario[305]=0;f.scenario[525]=1;f.scenario[307]=1;f.scenario[308]=1;check(raw(f,597,3,entry(191)).path.success,"full native bird outcome also opens the real raft approach");
 f.scenario[525]=0;f.scenario[307]=0;f.scenario[308]=0;f.current_map_id=192;for(int i=549;i<=552;++i)f.scenario[i]=1;
 check(raw(f,192,16,entry(192)).path.success,"actual tasty fish is reachable from the native beach entry");check(raw(f,192,17,entry(192)).path.success,"actual ordinary fish is reachable from the native beach entry");
 auto advice=model(f,entry(192));check(advice.targets.size()==1&&advice.targets[0].goal.entity_id==16&&advice.puzzle_types==2,"guide prefers actual available tasty fish as Other guidance");
 check(model(f,entry(192),{true,false}).targets.empty(),"Other off stops the current fishing operation without skipping to the raft");check(!model(f,entry(192),{false,true}).targets.empty(),"beneficial-fish solution is independent of Navigation preference");
 f.scenario[549]=0;advice=model(f,entry(192));check(advice.targets.size()==1&&advice.targets[0].goal.entity_id==17,"actual ordinary fish is the positive fallback when tasty fish is absent");
 f.scenario[550]=0;advice=model(f,entry(192));check(advice.targets.size()==1&&advice.targets[0].goal.entity_id==7&&advice.targets[0].instruction==L"Talk to Cid to refresh the fish.","no beneficial spawn routes to the actual native reroll producer");
 f.scenario[303]=f.scenario[304]=1;advice=model(f,entry(192));check(advice.targets[0].goal.entity_id!=18&&advice.targets[0].goal.entity_id!=19,"guide never recommends either harmful fish");
 f.scenario[301]=1;advice=model(f,entry(192));check(advice.targets[0].goal.entity_id==7&&advice.targets[0].instruction==L"Give Cid every fish you are carrying.","feeding instruction reflects actual all-held-fish consumption");
 f.scenario[301]=0;f.scenario[549].reset();check(model(f,entry(192)).unknown,"unknown spawn state cannot invent a beneficial fish");
 f=floating_fixture::flags();f.scenario.fill(0);f.scenario[297]=f.scenario[300]=1;f.scenario[549]=f.scenario[550]=1;f.current_map_id=191;context(f);Cell at=entry(191);
 check(journey(f,at,16),"complete guided first tasty-fish journey uses actual cabin/upper-island/beach transfers");f.scenario[301]=1;
 check(journey(f,at,7),"held tasty fish returns to the actual Cid control through all real floors");f.scenario[301]=0;f.scenario[549]=0;
 check(journey(f,at,17),"guided ordinary-fish fallback is reachable through the same native map family");f.scenario[302]=1;
 check(journey(f,at,7),"second positive catch returns to Cid without a false trap");f.scenario[302]=0;f.scenario[305]=1;at={1,9,15};
 advice=model(f,at);check(advice.targets[0].goal.map_id==597&&advice.targets[0].goal.entity_id==3&&advice.finish_on_scenario==306&&advice.puzzle_types==1,"actual recovery advances to real raft and full voyage306 as Navigation");
 check(model(f,at,{false,true}).targets.empty(),"Navigation off stops raft guidance even after fishing is complete");check(!model(f,at,{true,false}).targets.empty(),"Navigation remains available after the actual fishing outcome with Other off");
 check(journey(f,at,3),"recovery branch follows the actual hidden passage through raft room597");f.current_map_id=2;f.scenario[306]=1;check(model(f,{1,146,212}).finish_on_scenario!=306,"finished island episode yields World2 back to the accepted next episode");
 f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=191;f.scenario[297]=f.scenario[300]=f.scenario[525]=1;context(f);at=entry(191);
 check(model(f,at).targets[0].goal.entity_id==8&&journey(f,at,8),"native death outcome chooses the correct Cid variant rather than an unavailable living actor");
 // Exact native34,25 minus this room's actual16,0 crop, not a guessed origin.
 f.scenario[307]=1;f.current_map_id=193;context(f);at={1,18,25};check(model(f,at).targets[0].goal.entity_id==1&&journey(f,at,1,true),"actual full cliff arrival reaches the real current scene bird control");
 f.scenario[308]=1;f.current_map_id=192;context(f);at={1,14,9};check(journey(f,at,3),"actual final bird landing follows all native maps back to hidden passage and raft");
 f.current_map_id=2;f.scenario[306]=1;check(model(f,{1,146,212}).finish_on_scenario!=306,"completed death branch also releases shared World2 ownership");
 f.current_map_id=191;f.scenario[297]=0;check(!model(f,entry(191)).supported,"guide is not selected before the actual opening scene297");
 MapGoal town;town.map_id=2;for(const auto& o:baked::grids(*baked::submap_by_map_id(2)).objects)if(o.type==baked::ObjType::GotoMap&&o.a==194){town.cell={1,o.tx,o.ty};break;}
 f=floating_fixture::flags();f.scenario.fill(0);f.scenario[297]=f.scenario[300]=1;f.current_map_id=191;p=world_route(f,entry(191),town);check(!p.path.success,"regular world directions cannot bypass the actual unresolved island gate");
 f.scenario[305]=1;f.current_map_id=597;p=world_route(f,entry(597),town);check(p.path.success&&p.path.next_map_id==2&&!p.path.required_trap,"ordinary raft route uses the full World2 crossing without a false trap");
 check(p.pending.incoming_transition&&p.pending.incoming_transition->arrival==Cell{1,146,212},"regular raft directions retain the actual final player cell rather than beach192/World PointIn1");
 check(!p.path.walk_phases.empty()&&p.path.walk_phases.back().manual_after,"the native animated raft remains a manual interaction");
 f.current_map_id=2;f.scenario[306]=1;check(world_route(f,{1,146,212},town).path.success,"the actual final raft pose has a valid ordinary continuation on foot");
 f.current_map_id=191;f.scenario[353]=0;check(!raw(f,192,12,entry(191)).path.success,"later magicite cannot become a destination before its native353 condition");
 f.scenario[353]=1;f.scenario[471]=0;check(raw(f,192,12,entry(191)).path.success,"uncollected animated magicite remains a valid cross-floor destination in its actual later phase");
 f.scenario[471]=1;check(!raw(f,192,12,entry(191)).path.success,"native collected471 removes the animated pickup normally");
 check(fixture::wrong_owner==0,"all route work uses owned observations");std::printf("Solitary:%d assertions/%d failures;source/owned-route fixture,not game or audible timing\n",cases,errors);return errors?1:0;
}
