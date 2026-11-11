// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Finite before-change diagnostic,not release qualification.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
namespace tycoon_probe {
using namespace field;using namespace baked_nav;
int investigate(){
 const auto* room=baked::submap_by_map_id(70);if(!room)return 2;
 for(int open=0;open<2;++open){map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);f.scenario[36]=1;f.scenario[514]=open;f.current_map_id=70;map_state::Resolver state(f,fixture::records);
  std::printf("Tycoon native gate514=%d/known%d\n",open,state.known(*room));
  const auto* entry=point_in(*room,1,state);if(!entry)return 3;const Cell start{entry->layer,entry->tx,entry->ty};std::printf("Actual PointIn1 %d,%d,%d\n",start.layer,start.x,start.y);
  for(const auto& goal:baked::grids(*room).objects)if(goal.type==baked::ObjType::GotoMap&&goal.a>1){
   RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=70;s.submap=room;s.layer=start.layer;s.position=world_of_cell(*room,start);s.vehicle={true,1,baked::transport(1)->type,{}};s.locale=L"en";s.vehicle_shortcuts=false;s.kind=TargetKind::Transition;s.goal.map_id=70;s.goal.cell={goal.layer,goal.tx,goal.ty};s.goal.entity_id=goal.entity_id();s.goal.raw_name=baked::object_identifier(goal);
   for(const auto& original:baked::grids(*room).objects){if(original.type==baked::ObjType::PointIn||original.type==baked::ObjType::GotoMap||original.type==baked::ObjType::Warp||!s.state.active(*room,original,true))continue;const auto o=s.state.effective(*room,original);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(original);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);}
   const auto route=plan_snapshot(s);std::printf("target%d at%d,%d:success%d/event%d/npc%d/%ls/%ls\n",goal.a,goal.tx,goal.ty,route.path.success,route.path.event_on_route,route.path.npc_approach,route.path.reason.c_str(),route.path.full_description.c_str());
  }
 }
 return fixture::wrong_owner?4:0;
}
}
int main(){using namespace tycoon_probe;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;int result=2;std::thread worker([&]{result=investigate();});worker.join();return result;}
