// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact current Jachol physical routes. Native timing is qualified separately;
// this capped fixture cannot certify audible/game response.
#define main retained_jachol_floating_main
#include "floating_continent_route_check.cpp"
#undef main
#include "../src/games/ff5/jachol_model.cpp"
namespace jachol_fixture {
using namespace floating_fixture;
int errors=0,checks=0;
bool door_open=false;
void check(bool ok,const char* why){++checks;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
Planned route(map_state::Flags f,Cell at,int map,int id,std::optional<Cell> moving={}){
 RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";
 s.goal.map_id=map;s.goal.entity_id=id;s.goal.avoid_other_events=true;s.goal.allow_scripted_crossings=true;
 for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn&&o.entity_id()==id){s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(o);s.kind=o.type==baked::ObjType::GotoMap?TargetKind::Transition:baked_route::step_event(o)?TargetKind::Spot:TargetKind::Interact;break;}
 obstacles(s);if(door_open&&s.map_id==284)s.obstacles.erase(std::remove_if(s.obstacles.begin(),s.obstacles.end(),[](const auto& o){return o.id==6||o.id==7;}),s.obstacles.end());
 if(moving){s.obstacles.erase(std::remove_if(s.obstacles.begin(),s.obstacles.end(),[](const auto& o){return o.id==8;}),s.obstacles.end());NativeObstacle o;o.type=int(baked::ObjType::Entity);o.id=8;o.cell=*moving;o.raw_name=s.goal.raw_name;o.collision=1;s.obstacles.push_back(o);}
 Planned p;std::thread t([&]{p=plan_snapshot(s);});t.join();std::printf("Jachol %d from%d,%d,%d ->%d/%d live%d success%d next%d %s %s\n",f.current_map_id,at.layer,at.x,at.y,map,id,moving.has_value(),p.path.success,p.path.next_map_id,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());return p;
}
ff5::library::Plan advice(const map_state::Flags& f,Cell at,bool fresh=true,bool hidden=false,bool frozen=false){ff5::jachol::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.fresh=fresh;s.skull_hidden=hidden;s.chest_door_hidden=door_open;s.timers_frozen=frozen;return ff5::jachol::plan(s,fixture::guide_recipes);}
bool journey(map_state::Flags& f,Cell& at,int expected,std::optional<Cell> skull={}){
 const auto p=advice(f,at);if(p.unknown||p.targets.empty())return false;
 for(int leg=0;leg<12;++leg){RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.locale=L"en";s.vehicle_shortcuts=false;s.candidates=p.targets;obstacles(s);
  if(door_open&&s.map_id==284)s.obstacles.erase(std::remove_if(s.obstacles.begin(),s.obstacles.end(),[](const auto& o){return o.id==6||o.id==7;}),s.obstacles.end());
  if(skull&&s.map_id==284){s.obstacles.erase(std::remove_if(s.obstacles.begin(),s.obstacles.end(),[](const auto& o){return o.id==8;}),s.obstacles.end());NativeObstacle o;o.type=int(baked::ObjType::Entity);o.id=8;o.cell=*skull;o.raw_name=p.targets.front().goal.raw_name;o.collision=1;s.obstacles.push_back(o);}
  Planned got;std::thread worker([&]{got=plan_snapshot(s);});worker.join();std::printf("Jachol guide map%d from%d,%d,%d selected%d success%d next%d %s %s\n",f.current_map_id,at.layer,at.x,at.y,got.goal.entity_id,got.path.success,got.path.next_map_id,text::to_utf8(got.path.reason).c_str(),text::to_utf8(got.path.full_description).c_str());
  if(!got.path.success||got.path.required_trap||got.goal.entity_id!=expected)return false;
  if(!got.path.next_map_id){for(const auto& phase:got.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  if(!got.pending.incoming_transition)return false;f.current_map_id=got.pending.incoming_transition->destination;at=got.pending.incoming_transition->arrival;f.local.fill(0);
 }return false;
}
}
int main(){using namespace jachol_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;auto f=flags();f.scenario.fill(0);f.area.fill(0);f.local.fill(0);f.current_map_id=284;f.current_area_id=47;
 for(const auto& o:baked::grids(*baked::submap_by_map_id(284)).objects)if(o.entity_id()>=6&&o.entity_id()<=26)std::printf("OBJECT %d type%d %d,%d,%d\n",o.entity_id(),int(o.type),o.layer,o.tx,o.ty);
 check(route(f,entry(284),284,25).path.success,"fresh entry reaches the physical first wall control");
 f.scenario[407]=1;f.area[1]=1;
 const auto* guide=fixture::guide_recipes.get(L"jachol_guide");const auto* hold=guide->get(L"targets")->get(L"staging");const Cell center{1,int(hold->get(L"x")->n),int(hold->get(L"y")->n)};
 // Do not predict a TileAnimation transform from its rectangle anchor. All
 // route goals below use the actual moved native-object snapshot. Exercise
 // each native random column and the three nearby captured row placements.
 for(int i=0;i<8;++i){f.area[1]=i+1;const int column=int(guide->get(L"skull_columns")->arr[size_t(i)].n);
  for(int y=center.y-1;y<=center.y+1;++y)check(route(f,center,284,8,Cell{1,column,y}).path.success,"loaded real switch uses its actual moved pose");
  Cell nearby{1,column,center.y};check(advice(f,nearby).targets.size()==1&&advice(f,nearby).targets[0].goal.entity_id==8,"fresh nearby column selects the actual real switch");
 }
 check(advice(f,center,false).targets.empty(),"expired phase cannot offer timed movement");check(advice(f,center,true,true).targets.empty(),"hidden real control waits");check(!advice(f,center,false,false,true).targets.empty(),"frozen visible control uses its actual native pose without a deadline");
 f.area[1]=0;check(advice(f,center,true,true).note==L"Wait for the skull switch to appear.","initial hidden phase can wait before a random column exists");check(advice(f,center).unknown,"visible control with unknown column cannot publish a route");f.area[1]=4;
 f.scenario[407]=0;Cell at=entry(284);check(journey(f,at,25),"full actual entrance reaches the first wall switch");f.scenario[407]=1;check(journey(f,at,-1)&&at==center,"first wall operation reaches real invariant staging floor");
 check(journey(f,at,8,Cell{1,29,center.y}),"real native phase reaches its moved manual control");f.scenario[407]=0;f.scenario[408]=1;check(journey(f,at,6),"successful skull operation reaches the chest-door control");door_open=true;check(journey(f,at,-1)&&f.current_map_id==285,"actual hidden door allows the full second-floor native arrival");
 check(journey(f,at,11),"second-floor arrival reaches its actual scripted chest");f.scenario[435]=1;check(journey(f,at,5),"scripted chest receipt leads to the next real chest");f.treasure[105]=1;check(journey(f,at,6),"first treasure receipt leads to the remaining chest");f.treasure[106]=1;check(journey(f,at,10)&&advice(f,at).finish_map==1,"all actual chest receipts lead to the native first-world climb");f.scenario[125]=1;check(advice(f,at).finish_map==173,"later-world climb retains the full Bal basement return");
 auto unknown=f;unknown.scenario[435].reset();check(advice(unknown,at).unknown,"missing chest receipt cannot skip its operation");check(fixture::wrong_owner==0,"Jachol model/routes stay on owned worker data");
 std::printf("Jachol %d checks/%d failures\n",checks,errors);return errors||fixture::wrong_owner?1:0;}
