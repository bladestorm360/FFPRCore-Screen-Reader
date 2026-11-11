// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real factory grids and the production planner; no game loaded/native call.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
namespace magitek {
using namespace field;using namespace baked_nav;
RouteSnapshot capture(int map,Cell start,Cell goal,bool late=false){
 map_state::Flags f;f.scenario.fill(1);for(int bit=183;bit<=194;++bit)f.scenario[size_t(bit)]=0;f.scenario[297]=0;f.treasure.fill(0);f.global.fill(0);f.global[0]=2;f.global[5]=1;f.area.fill(0);f.local.fill(0);f.current_map_id=map;
 f.scenario[194]=late?1:0;
 const auto* room=baked::submap_by_map_id(map);const auto* meta=fixture::records.get(baked::str(room->name));f.current_area_id=int(meta->get(L"area_id")->n);
 RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=map;s.submap=room;s.layer=start.layer;s.position=world_of_cell(*room,start);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.goal.map_id=map;s.goal.cell=goal;s.kind=TargetKind::Spot;
 for(const auto& source:baked::grids(*room).objects){
  if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp||!s.state.active(*room,source,true))continue;
  const auto o=s.state.effective(*room,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);
 }return s;
}
}
int main(){using namespace magitek;int failures=0,cases=0,arcs=0,approaches=0;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char*p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto expect=[&](bool ok,const char* reason,int map,int id){++cases;if(!ok){++failures;std::printf("FAIL map%d control%d %s\n",map,id,reason);}};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 for(int map:{396,643,397}){const auto* room=baked::submap_by_map_id(map);const auto& grid=baked::grids(*room);
  for(const auto& arc:grid.objects){
   if(arc.type!=baked::ObjType::Warp)continue;const int id=arc.entity_id();
   if(!(map==396&&(id==46||id==57||id==33||id==31||id==29))&&!(map==643&&(id==57||id==33||id==29))&&!(map==397&&id==62))continue;
   ++arcs;const baked::Object* source=nullptr,*arrival=nullptr;
   for(const auto&o:grid.objects){if(o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn&&o.entity_id()==id&&baked::object_identifier(o)==baked::object_identifier(arc))source=&o;if(o.type==baked::ObjType::PointIn&&o.a==arc.b)arrival=&o;}
   expect(source&&arrival,"one actual native control and landing resolve",map,id);if(!source||!arrival)continue;
   const bool manual=source->type==baked::ObjType::Entity;const Cell goal{arrival->layer,arrival->tx,arrival->ty};
   for(int side=0;side<4;++side){if(manual&&!(((arc.d>>8)&15)&(1<<side)))continue;if(!manual&&side)continue;
    constexpr int dx[]{0,1,0,-1},dy[]{-1,0,1,0};Cell start{source->layer,source->tx+(manual?dx[side]:0),source->ty+(manual?dy[side]:0)};
    if(!baked::walkable(grid,start.layer,start.x,start.y,true))continue;++approaches;
    auto snapshot=capture(map,start,goal);Planned plan;std::thread worker([&]{plan=plan_snapshot(snapshot);});worker.join();
    // The existing manual-travel owner uses incoming_trap for a manual warp;
    // its name does not imply that the route requires an actual trap.
    const auto receipt=plan.pending.incoming_transition?plan.pending.incoming_transition:plan.pending.incoming_trap;
    std::printf("Factory arc map%d control%d side%d from%d,%d,%d to%d,%d,%d success%d via%d receipt%d\n",map,id,side,start.layer,start.x,start.y,goal.layer,goal.x,goal.y,plan.path.success,plan.path.next_map_id,receipt?receipt->entity_id:-1);
    expect(plan.path.success&&!plan.path.required_trap,"physical native passage is usable",map,id);
    if(plan.path.next_map_id){expect(receipt&&receipt->destination==map,"continuation retains current native map",map,id);if(receipt)expect(receipt->arrival==goal,"native landing matches destination",map,id);}
    if(manual&&plan.path.success&&plan.path.next_map_id)expect(!plan.path.walk_phases.empty()&&plan.path.walk_phases.back().manual_after,"native action remains manual",map,id);
    if(manual&&receipt&&plan.path.success&&plan.path.next_map_id){
     snapshot.pending=plan.pending;SnapshotScope scope(snapshot);const auto previous=snapshot.pending.completed_transitions.size();
     record_map_arrival(map,goal);expect(snapshot.pending.completed_transitions.size()==previous,"arrival cannot complete an unstarted native action",map,id);
     expect(note_trap_start(map,receipt->entry),"real manual operation marks same-floor crossing started",map,id);
     record_map_arrival(map,goal);expect(snapshot.pending.completed_transitions.size()==previous+1&&!snapshot.pending.incoming_transition,"exact native landing completes one receipt",map,id);
    }
   }
   if(map==396&&id==46){auto before=capture(map,{1,28,8},goal);auto after=capture(map,{1,28,8},goal,true);expect(before.state.active(*room,arc)&&!after.state.active(*room,arc),"native194 removal keeps old ladder unavailable",map,id);}
  }
  if(map==397)continue;
  for(int id:{46,57,33,31,29}){const baked::Object* control=nullptr;for(const auto&o:grid.objects)if(o.type==baked::ObjType::Entity&&o.entity_id()==id){control=&o;break;}if(!control)continue;
   std::printf("Factory map%d control%d at%d,%d,%d mask%d\n",map,id,control->layer,control->tx,control->ty,control->d);
  }
  const auto* meta=fixture::records.get(baked::str(room->name));std::printf("Factory map%d area%d\n",map,int(meta->get(L"area_id")->n));
  Cell entry{};for(const auto&o:grid.objects)if(o.type==baked::ObjType::PointIn&&o.a==1){entry={o.layer,o.tx,o.ty};break;}
  for(Cell goal:{Cell{1,10,45},Cell{1,11,13},Cell{1,11,18},Cell{2,14,51}}){auto s=capture(map,entry,goal);Planned result;std::thread worker([&]{result=plan_snapshot(s);});worker.join();
   std::printf("Factory map%d from%d,%d,%d to%d,%d,%d walk%d known%d success%d next%d reason%s\n",map,entry.layer,entry.x,entry.y,goal.layer,goal.x,goal.y,baked::walkable(grid,goal.layer,goal.x,goal.y,true),s.state.known(*room),result.path.success,result.path.next_map_id,text::to_utf8(result.path.reason).c_str());
  }
 }expect(arcs==11,"all authored native links are covered",0,0);std::printf("Factory:%d arcs/%d approaches/%d assertions/%d failures/owner%d\n",arcs,approaches,cases,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
