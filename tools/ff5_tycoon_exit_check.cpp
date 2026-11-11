// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
namespace tycoon_exit_check {
using namespace field;using namespace baked_nav;
int assertions=0,errors=0;
void check(bool v,const char* why){++assertions;if(!v){++errors;std::printf("FAIL %s\n",why);}}
map_state::Flags flags(int joined,int memory){map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);f.scenario[36]=f.scenario[125]=f.scenario[126]=1;f.scenario[127]=joined;f.scenario[128]=memory;f.current_map_id=73;f.current_area_id=17;return f;}
Planned route(const map_state::Flags& f,const MapGoal& goal,TargetKind kind){RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=73;s.submap=baked::submap_by_map_id(73);s.layer=1;s.position=world_of_cell(*s.submap,{1,14,25});s.vehicle={true,1,baked::transport(1)->type,{}};s.locale=L"en";s.vehicle_shortcuts=false;s.kind=kind;s.goal=goal;
 for(const auto& original:baked::grids(*s.submap).objects){if(original.type==baked::ObjType::PointIn||original.type==baked::ObjType::GotoMap||original.type==baked::ObjType::Warp||!s.state.active(*s.submap,original,true))continue;const auto o=s.state.effective(*s.submap,original);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(original);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);}
 return plan_snapshot(s);}
int verify(){const auto* room=baked::submap_by_map_id(73);const baked::Object* warp=nullptr,*event=nullptr;
 for(const auto& o:baked::grids(*room).objects)if(o.entity_id()==34){if(o.type==baked::ObjType::Warp)warp=&o;if(o.type==baked::ObjType::Event)event=&o;}
 check(warp&&event,"exact original scene and existing derived exit both remain");if(!warp||!event)return 2;
 check(warp->a==72&&warp->b==4&&(warp->d&baked::kConditionalSceneExit)&&baked::world_exit(*warp)&&!baked::is_trap(*warp),"only the actual conditional floor exit is ordinary travel,never a required trap");
 for(int joined=0;joined<2;++joined)for(int memory=0;memory<2;++memory){const auto f=flags(joined,memory);map_state::Resolver state(f,fixture::records);const bool replaced=joined&&!memory;
  check(state.consumed(*room,*warp)==replaced,"native no-return branch cannot grant an immediate floor transfer");check(!state.consumed(*room,*event),"the original contact remains reachable while its story scene is pending");
  MapGoal door;door.map_id=73;door.entity_id=34;door.raw_name=baked::object_identifier(*event);door.cell={event->layer,event->tx,event->ty};const auto local=route(f,door,TargetKind::Transition);
  check(local.path.success&&!local.path.required_trap,"the door itself keeps a valid local path in every native story state");check(local.path.event_on_route==replaced,"pending scene is an event requirement;ordinary exit has no scene/trap warning");
  const auto* target=baked::submap_by_map_id(72);map_state::Resolver projected(f,fixture::records);const auto* point=point_in(*target,4,projected);check(point,"actual destination PointIn4");if(!point)return 3;
  MapGoal onward;onward.map_id=72;onward.cell={point->layer,point->tx,point->ty};const auto next=route(f,onward,TargetKind::Spot);
  if(replaced){check(!next.pending.incoming_transition||next.pending.incoming_transition->destination!=72,"pending scene never fabricates the blocked direct exit");check(next.path.success||next.path.reason==L"Story gated","the presently inaccessible destination is correctly story gated while its door remains usable");}
  else {check(next.path.success,"available native branch has a complete cross-floor route");check(next.pending.incoming_transition&&next.pending.incoming_transition->destination==72&&!next.path.required_trap,"available direct exit reaches72 without a false trap warning");}
 }
 for(int unknown:{127,128}){auto f=flags(1,0);f.scenario[unknown].reset();map_state::Resolver state(f,fixture::records);check(state.consumed(*room,*warp),"unknown branch cannot assert the floor change");}
 std::printf("Tycoon conditional exit:%d assertions/%d failures;native branches,valid door approach and story-gated destination verified\n",assertions,errors);return errors?4:fixture::wrong_owner?5:0;
}
}
int main(){using namespace tycoon_exit_check;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;int result=2;std::thread worker([&]{result=verify();});worker.join();return result;}
