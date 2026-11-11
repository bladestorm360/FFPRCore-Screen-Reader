// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual training event on real geometry, with active/reserve/unknown Sabin.
#define main duncan_floating_main
#include "floating_continent_route_check.cpp"
#undef main
int main(){using namespace floating_fixture;int assertions=0,failures=0;auto expect=[&](bool ok,const char* why){++assertions;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 const auto* room=baked::submap_by_map_id(272);const baked::Object* arrival=nullptr;const baked::Object* trigger=nullptr;
 for(const auto& o:baked::grids(*room).objects){if(o.type==baked::ObjType::PointIn&&o.a==1)arrival=&o;if(o.type==baked::ObjType::Event&&o.entity_id()==18)trigger=&o;}
 if(!arrival||!trigger)return 2;
 for(int present:{-1,0,1})for(bool complete:{false,true}){
  auto f=flags();f.current_map_id=272;f.current_area_id=12;f.scenario[297]=1;f.scenario[464]=complete;
  if(present>=0)f.party_sabin=present!=0;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=272;s.submap=room;s.layer=arrival->layer;s.position=world_of_cell(*room,{arrival->layer,arrival->tx,arrival->ty});s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.goal.map_id=272;s.goal.entity_id=18;s.goal.raw_name=baked::object_identifier(*trigger);s.goal.cell={trigger->layer,trigger->tx,trigger->ty};s.goal.avoid_other_events=true;s.kind=TargetKind::Spot;obstacles(s);
  // The reused geometry helper includes possible unknown obstacles. Such a
  // projection is not a native observation authorizing this current target.
  // Test missing observation here; a real captured active event retains the
  // established authoritative-current-floor fallback in production.
  if(present<0)s.obstacles.erase(std::remove_if(s.obstacles.begin(),s.obstacles.end(),[](const auto& o){return o.id==18&&o.trigger;}),s.obstacles.end());
  Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  std::printf("Duncan present%d complete%d from%d,%d,%d goal%d,%d,%d result%d reason%s\n",present,complete,arrival->layer,arrival->tx,arrival->ty,trigger->layer,trigger->tx,trigger->ty,p.path.success,text::to_utf8(p.path.reason).c_str());
  expect(p.path.success==(present==1&&!complete),"actual native training event is reachable only for active Sabin before464");
  expect(!p.path.required_trap,"training path never grants an invented trap or jump");
 }
 expect(fixture::wrong_owner==0,"training routes use owned state on the worker");std::printf("Duncan route:%d assertions/%d failures/owner%d\n",assertions,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
