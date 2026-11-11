// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual late-Narshe geometry, capability gates and manual door approaches.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
namespace narshe {
using namespace field;using namespace baked_nav;
RouteSnapshot capture(const baked::Submap& room,map_state::Flags f,baked_route::Cell start,const baked::Object& target){
 RouteSnapshot s(map_state::Resolver(f,fixture::records));s.submap=&room;s.map_id=3;s.layer=start.layer;s.position=world_of_cell(room,start);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;
 s.goal.map_id=3;s.goal.cell={target.layer,target.tx,target.ty};s.goal.entity_id=target.entity_id();s.goal.raw_name=baked::object_identifier(target);
 s.kind=target.type==baked::ObjType::GotoMap?TargetKind::Transition:baked_route::step_event(target)?TargetKind::Spot:TargetKind::Interact;
 s.goal.examine=s.kind==TargetKind::Interact;
 for(const auto& source:baked::grids(room).objects){
  if(source.type==baked::ObjType::Warp||source.type==baked::ObjType::PointIn||!s.state.active(room,source,true))continue;
  const auto o=s.state.effective(room,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.enabled=n.applies=true;
  n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(std::move(n));
 }
 return s;
}
}
int main(){using namespace narshe;int failures=0,cases=0,doors=0;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto expect=[&](bool okay,const char* text,int id){++cases;if(!okay){++failures;std::printf("FAIL Narshe%d %s\n",id,text);}};
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;const auto* room=baked::submap_by_map_id(3);if(!room)return 2;
 map_state::Flags f;f.scenario.fill(1);for(int bit=411;bit<=416;++bit)f.scenario[bit]=0;f.treasure.fill(0);f.global.fill(0);f.global[0]=3;f.global[5]=1;f.area.fill(0);f.local.fill(0);f.current_map_id=3;f.current_area_id=int(fixture::records.get(L"Map_20010")->get(L"area_id")->n);
 const auto& grid=baked::grids(*room);const baked::Object* entry=nullptr;for(const auto& o:grid.objects)if(o.type==baked::ObjType::PointIn&&o.a==1){entry=&o;break;}if(!entry)return 2;
 const baked_route::Cell start{entry->layer,entry->tx,entry->ty};std::printf("Native Narshe entry%d,%d,%d\n",start.layer,start.x,start.y);
 const auto* metadata=fixture::records.get(L"Map_20010")->get(L"party_gates");if(!metadata||metadata->arr.size()!=6)return 2;
 {auto flag=f;flag.party_locke.reset();const auto unknown=map_state::Resolver(flag,fixture::records).signature();flag.party_locke=false;const auto absent=map_state::Resolver(flag,fixture::records).signature();flag.party_locke=true;const auto present=map_state::Resolver(flag,fixture::records).signature();expect(unknown!=absent&&absent!=present&&unknown!=present,"membership changes invalidate cached reachability",0);}
 std::thread worker([&]{
  for(const auto& row:metadata->arr){
   const int id=int(row.get(L"id")->n),bit=int(row.get(L"receipt")->n);const baked::Object* door=nullptr;
   for(const auto& o:grid.objects)if(o.type==baked::ObjType::Entity&&o.entity_id()==id){door=&o;break;}if(!door){expect(false,"exact door resolves",id);continue;}++doors;
   const baked::Object* exit=nullptr;int closest=INT_MAX;
   for(const auto& o:grid.objects)if(o.type==baked::ObjType::GotoMap){const int distance=std::abs(o.tx-door->tx)+std::abs(o.ty-door->ty)+8*(o.layer!=door->layer);if(distance<closest){closest=distance;exit=&o;}}
   // The treasure-house entrance is native Event197, with its entry scene,
   // rather than an ordinary GotoMap. Its exact identity is source-established.
   if(id==114)for(const auto& o:grid.objects)if(o.type==baked::ObjType::Event&&o.entity_id()==197){exit=&o;closest=std::abs(o.tx-door->tx)+std::abs(o.ty-door->ty);}
   std::printf("Door%d L%d at%d,%d exitL%d at%d,%d destination%d distance%d\n",id,door->layer,door->tx,door->ty,exit?exit->layer:-1,exit?exit->tx:-1,exit?exit->ty:-1,exit?exit->a:-1,closest);
   expect(exit&&closest<=2,"real destination exit is immediately behind this door",id);if(!exit||closest>2)continue;
   for(int mode=0;mode<3;++mode){
    f.party_locke=mode==0?std::optional<bool>{}:std::optional<bool>(mode==2);
    if(mode==0){
     auto diagnostic=capture(*room,f,start,*door);SnapshotScope scope(diagnostic);
     const auto goals=goals_for(*room,{door->layer,door->tx,door->ty},TargetKind::Interact);
     std::printf("Door%d diagnostic known%d active%d possible%d startWalk%d goalCount%zu shape%u,%u d%d b%d\n",id,
      diagnostic.state.known(*room),diagnostic.state.active(*room,*door),diagnostic.state.active(*room,*door,true),
      baked::walkable(grid,start.layer,start.x,start.y,true),goals.cells.size(),door->tw,door->th,door->d,door->b);
    }
    auto front=capture(*room,f,start,*door);auto approach=plan_snapshot(front);expect(approach.path.success,"closed door front remains reachable regardless of ability",id);
    auto s=capture(*room,f,start,*exit);const auto p=plan_snapshot(s);
    std::printf("Door%d mode%d success%d manual%d reason%s error%s\n",id,mode,p.path.success,p.active&&p.active->key_approach,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str());
    expect(mode==2?p.path.success:!p.path.success&&p.path.reason==L"Story gated","behind-door route respects actual active Locke",id);
    if(mode==2)expect(p.active&&p.active->key_approach&&!p.path.walk_phases.empty()&&p.path.walk_phases.back().manual_after,"valid virtual journey stops for manual native unlocking",id);
    if(mode==2&&!p.path.walk_phases.empty())expect(p.path.walk_phases.back().action_after==L"Interact to unlock the door.","manual action uses the concise localized instruction",id);
    auto browsing=capture(*room,f,start,*exit);const auto b=plan_browse_snapshot(browsing);
    expect(mode==2?b.route.reason!=L"Story gated":!b.route.direct&&b.route.reason==L"Story gated","browse uses the same capability gate",id);
   }
   f.party_locke=false;f.magic_keys=99;f.party_thief=true;auto unrelated=capture(*room,f,start,*exit);expect(!plan_snapshot(unrelated).path.success,"FFIII key/Thief values cannot open an FFVI lock",id);f.magic_keys.reset();f.party_thief.reset();
   f.scenario[bit]=1;auto opened=capture(*room,f,start,*exit);const auto p=plan_snapshot(opened);expect(p.path.success&&(!p.active||!p.active->key_approach),"native opened receipt permits passage without Locke",id);f.scenario[bit]=0;
   // Native353 (the Falcon) enables ev_e_0703's locks. A real pre-collapse
   // profile cannot retain that later receipt while clearing297 alone.
   f.scenario[297]=f.scenario[353]=0;auto early=capture(*room,f,start,*exit);const auto unblocked=plan_snapshot(early);expect(unblocked.path.success&&(!unblocked.active||!unblocked.active->key_approach),"World of Balance houses do not acquire later Locke locks early",id);f.scenario[297]=f.scenario[353]=1;
  }
 });worker.join();expect(doors==6,"all six source locks exercised",0);expect(fixture::wrong_owner==0,"no worker native API/pointer access",0);
 std::printf("Narshe party gates:%d doors/%d cases/%d failures/owner%d\n",doors,cases,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
