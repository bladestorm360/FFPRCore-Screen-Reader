// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual authored objectives and production planner on the native map/state.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff6/story_model.cpp"
#include "../src/games/ff6/dream_train_maze.cpp"
#include "../src/games/ff6/zone_eater_roof_model.cpp"
#include "../src/games/ff6/sealed_gate_model.cpp"
// The planner fixture already uses owned raw names. Match the model fixture's
// read-only label facade; names do not substitute for any route/state behavior.
namespace field {std::wstring safe_event_label(const std::wstring& raw,const std::wstring&,int,int,int){return raw;}}
namespace floating_fixture {
using namespace field;using namespace baked_nav;
void obstacles(RouteSnapshot& s){
 for(const auto& source:baked::grids(*s.submap).objects){
  if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp||!s.state.active(*s.submap,source,true))continue;
  const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);
 }
}
map_state::Flags flags(){map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.local.fill(0);f.area.fill(0);f.global.fill(0);f.global[0]=2;f.global[5]=1;f.scenario[286]=1;f.current_map_id=431;f.current_area_id=58;return f;}
void probe(map_state::Flags f,Cell at,int id){
 RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.goal.map_id=s.map_id;s.goal.entity_id=id;s.goal.avoid_other_events=true;s.goal.allow_scripted_crossings=true;
 for(const auto& source:baked::grids(*s.submap).objects)if(source.type!=baked::ObjType::Warp&&source.type!=baked::ObjType::PointIn&&source.entity_id()==id){const auto o=s.state.effective(*s.submap,source);s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(source);s.kind=baked_route::step_event(o)?TargetKind::Spot:TargetKind::Interact;break;}
 obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
 const auto& g=baked::grids(*s.submap);
 std::printf("Floating map%d area1=%d/2=%d/3=%d/4=%d/5=%d story292=%d from%d,%d,%d walk%d to%d success%d next%d trap%d reason%s %s\n",s.map_id,f.area[1].value_or(-1),f.area[2].value_or(-1),f.area[3].value_or(-1),f.area[4].value_or(-1),f.area[5].value_or(-1),f.scenario[292].value_or(-1),at.layer,at.x,at.y,baked::walkable(g,at.layer,at.x,at.y,true),id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
}
void apply(map_state::Flags& f,const json::Value* values){
 if(!values)return;
 const auto bank=[&](const wchar_t* name,auto& into){if(const auto* rows=values->get(name))for(const auto& r:rows->arr){if(r.arr.size()!=2)std::abort();const int i=int(r.arr[0].n);if(i<0||i>=int(into.size()))std::abort();into[size_t(i)]=int(r.arr[1].n);}};
 bank(L"scenario",f.scenario);bank(L"area",f.area);bank(L"local",f.local);bank(L"treasure",f.treasure);bank(L"global",f.global);
}
bool follow(map_state::Flags& f,Cell& at,int id,bool guided=false){
 for(int leg=0;leg<16;++leg){
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.goal.map_id=s.map_id;s.goal.entity_id=id;s.goal.avoid_other_events=true;s.goal.allow_scripted_crossings=true;
  for(const auto& source:baked::grids(*s.submap).objects)if(source.type!=baked::ObjType::Warp&&source.type!=baked::ObjType::PointIn&&source.entity_id()==id){const auto o=s.state.effective(*s.submap,source);s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(source);s.kind=baked_route::step_event(o)?TargetKind::Spot:TargetKind::Interact;break;}
  obstacles(s);Planned p;std::thread worker([&]{
   if(guided){
    ffpr::ff6::story::Snapshot input;input.map=f.current_map_id;input.flags=f;input.player=at;input.locale=L"en";
    auto advice=ffpr::ff6::story::plan(input);
    if(!advice.supported||advice.unknown||advice.finished||advice.targets.empty())return;
    s.candidates=std::move(advice.targets);
   }
   p=plan_snapshot(s);
  });worker.join();
  std::printf("Floating journey map%d stage%d from%d,%d,%d to%d leg%d success%d next%d trap%d %s %s\n",f.current_map_id,f.area[1].value_or(-1),at.layer,at.x,at.y,id,leg,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
  if(!p.path.success||p.path.required_trap)return false;
  if(guided&&p.goal.entity_id!=id){std::printf("FAIL authored guide chose%d rather than expected%d\n",p.goal.entity_id,id);return false;}
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;
  f.current_map_id=receipt->destination;at=receipt->arrival;
  if(guided&&id<0&&f.current_map_id==p.goal.map_id){
   if(!(at==p.goal.cell)){std::printf("FAIL native waypoint arrival%d,%d,%d differs from%d,%d,%d\n",at.layer,at.x,at.y,p.goal.cell.layer,p.goal.cell.x,p.goal.cell.y);return false;}
   return true;
  }
  if(receipt->entity_id==52||receipt->entity_id==53)f.area[1]=5;
 }
 return false;
}
int scenario(const json::Value& cases){
 if(cases.arr.empty()||cases.arr.size()>64)return 2;int failures=0,steps=0;
 for(const auto& row:cases.arr){
  auto f=flags();if(const auto* m=row.get(L"map"))f.current_map_id=int(m->n);apply(f,row.get(L"initial"));
  const auto* start=row.get(L"start"),*list=row.get(L"steps");if(!start||start->arr.size()!=3||!list||list->arr.size()>128)return 2;
  Cell at{int(start->arr[0].n),int(start->arr[1].n),int(start->arr[2].n)};
  const auto* guide=row.get(L"guide");
  for(const auto& step:list->arr){++steps;const auto* e=step.get(L"entity");if(!e)return 2;const bool ok=follow(f,at,int(e->n),guide&&guide->b);const auto* wanted=step.get(L"reachable");const bool expected=!wanted||wanted->b;
   if(ok!=expected){++failures;std::printf("FAIL expected reachability%d\n",expected);break;}
   apply(f,step.get(L"after"));if(const auto* pos=step.get(L"landing")){if(pos->arr.size()!=3)return 2;at={int(pos->arr[0].n),int(pos->arr[1].n),int(pos->arr[2].n)};}
  }
 }
 std::printf("Floating sequential cases:%zu cases/%d steps/%d failures/owner%d\n",cases.arr.size(),steps,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
}
int main(int argc,char** argv){using namespace floating_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 if(argc==1||(argc==2&&std::string(argv[1])!="--diagnostic")){json::Value cases;if(!json::parse(read(argc==1?"tools/floating_continent_cases.json":argv[1]),cases))return 2;return scenario(cases);}
 auto f=flags();for(Cell at:{Cell{2,4,19},Cell{2,4,9}})for(int id:{157,151,135,96,87,80,71,160})probe(f,at,id);
 f.scenario[287]=f.scenario[288]=1;for(int area=0;area<=4;++area){f.area[1]=area;for(int i=2;i<=6;++i)f.area[i]=1;for(int id:{135,96,87,80,71,160})probe(f,{2,4,19},id);}
 auto escape=flags();escape.current_map_id=631;escape.scenario[292]=1;escape.scenario[291]=1;for(int area=4;area<=11;++area){escape.area[1]=area;for(int id:{52,43,40,35,32,31,29,21,17,11})probe(escape,{2,67,24},id);}
 std::printf("Floating initial physical diagnostic: owner%d; no guide-completeness claim\n",fixture::wrong_owner.load());return fixture::wrong_owner?1:0;
}
