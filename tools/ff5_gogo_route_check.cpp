// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual air/battle/return objectives through all native tower entries.
#define main retained_gogo_floating_main
#include "floating_continent_route_check.cpp"
#undef main
#include "../src/games/ff5/mechanics_model.cpp"
namespace gogo_fixture {
using namespace floating_fixture;
int cases=0,errors=0,legs=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
ff5::library::Plan advice(const map_state::Flags& f,Cell at,field::puzzle_policy::Policy policy={}){ff5::mechanics::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.policy=policy;return ff5::mechanics::plan(s,fixture::guide_recipes);}
bool journey(map_state::Flags& f,Cell& at,int map,int entity){
 for(int i=0;i<32;++i){const auto plan=advice(f,at);if(!plan.supported||plan.unknown||plan.targets.size()!=1||plan.targets[0].goal.map_id!=map||plan.targets[0].goal.entity_id!=entity)return false;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.candidates=plan.targets;obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();++legs;
  if(!p.path.success){std::printf("Gogo map%d from%d,%d,%d target%d/%d success%d reason%s error%s %s\n",s.map_id,at.layer,at.x,at.y,map,entity,p.path.success,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str(),text::to_utf8(p.path.full_description).c_str());
   SnapshotScope view(s);const auto* room=baked::submap_by_map_id(map);const auto goals=goals_for(*room,s.goal.cell,s.kind);std::printf("Exact goal count%zu at%d,%d,%d kind%d\n",goals.cells.size(),s.goal.cell.layer,s.goal.cell.x,s.goal.cell.y,int(s.kind));for(const auto c:goals.cells)std::printf("Goal %d,%d,%d\n",c.layer,c.x,c.y);
   return false;}
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return f.current_map_id==map;}
  if(!p.pending.incoming_transition&&!p.pending.incoming_trap)return false;const auto crossing=p.pending.incoming_transition?p.pending.incoming_transition:p.pending.incoming_trap;
  // required_trap describes the whole planned journey,including later
  // floors. Validate this leg only when its actual incoming receipt is a trap.
  if(p.pending.incoming_trap&&!((s.map_id==486&&crossing->destination==487)||(s.map_id==490&&crossing->destination==491)))return false;
  f.current_map_id=crossing->destination;at=crossing->arrival;
 }return false;
}
}
int main(){using namespace gogo_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
 std::vector<std::pair<int,Cell>> entries;for(int map=485;map<=494;++map)for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn)entries.push_back({map,{o.layer,o.tx,o.ty}});entries.push_back({485,{0,11,5}});
 for(const auto& [map,entry]:entries){auto f=flags();f.scenario.fill(0);f.current_map_id=map;f.current_area_id=83;f.scenario[346]=1;Cell at=entry;
  bool ok=journey(f,at,490,3);if(ok){f.scenario[347]=1;at={0,7,6};ok=journey(f,at,494,8);}if(ok){f.scenario[166]=1;ok=journey(f,at,485,2);}check(ok,"every actual tower entry reaches air,actual Gogo and the real final exit");if(ok)std::printf("PASS journey from%d/%d,%d,%d\n",map,entry.layer,entry.x,entry.y);
 }
 auto f=flags();f.scenario.fill(0);f.current_map_id=494;f.current_area_id=83;f.scenario[347]=1;
 check(advice(f,{0,10,9},{true,false}).targets.empty(),"Other disable independently blocks the combat-puzzle solution");check(advice(f,{0,10,9},{false,true}).targets.front().goal.entity_id==8,"Navigation disable preserves the Other solution");
 f.scenario[166].reset();check(advice(f,{0,10,9}).unknown,"unknown native job receipt cannot finish or expose an exit");f.scenario[166]=0;f.scenario[347].reset();check(advice(f,{0,10,9}).unknown,"unknown air receipt cannot skip the real reset control");
 f.scenario[166]=1;const auto leave=advice(f,{0,10,9});check(!leave.finished&&leave.finish_map==622&&leave.puzzle_types==2&&leave.targets.front().goal.map_id==485,"real Mime receipt retains the full Other-owned return instead of finishing in the tower");
 f.current_map_id=622;f.scenario[347]=0;check(!advice(f,{1,9,10}).supported,"safe outside-room air reset cannot restart a completed battle puzzle");
 check(fixture::wrong_owner==0,"all route and rule work uses owned data");std::printf("Gogo:%d assertions/%d route legs/%d failures\n",cases,legs,errors);return errors||fixture::wrong_owner?1:0;
}
