// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual planner, browse and story-filter consumers; no game loaded.
#define main retained_key_route_main
#include "route_worker_check.cpp"
#undef main
namespace keyed {
using namespace field;using namespace baked_nav;
RouteSnapshot capture(const baked::Submap& sm,const map_state::Flags& flags,Cell at,const baked::Object& target){
 map_state::Resolver state(flags,fixture::records);RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=sm.first_map_id;s.submap=&sm;s.layer=at.layer;s.position=world_of_cell(sm,at);s.vehicle={true,1,1,{}};s.locale=L"en";
 s.goal.map_id=s.map_id;s.goal.entity_id=target.entity_id();s.goal.raw_name=baked::object_identifier(target);s.goal.cell={target.layer,target.tx,target.ty};s.goal.examine=true;s.kind=TargetKind::Interact;
 for(const auto& o:baked::grids(sm).objects){if(o.type==baked::ObjType::Warp||!state.active(sm,o))continue;NativeObstacle n;n.id=o.entity_id();n.type=int(o.type);n.raw_name=baked::object_identifier(o);n.cell={o.layer,o.tx,o.ty};n.enabled=n.applies=true;n.trigger=baked_route::step_event(o);n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(std::move(n));}
 return s;
}
int cornelia(bool before){int failures=0,cases=0,false_valid=0;const auto* sm=baked::submap_by_map_id(2);if(!sm)return 2;
 const auto expect=[&](bool okay,const char* mode,int profile,int id){++cases;if(!okay){++failures;std::printf("FAIL Cornelia %s profile%d entity%d\n",mode,profile,id);}};
 for(int profile=0;profile<(before?1:5);++profile){
  map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=2;
  // Key13 is awarded after the preceding castle story receipts. A key13-only
  // artificial save still selects the native earlier closed-door group.
  if(profile>0)flags.scenario[1]=1;if(profile>1)flags.scenario[5]=1;if(profile>2)flags.scenario[6]=1;if(profile>3)flags.scenario[13]=1;
  map_state::Resolver state(flags,fixture::records);const auto* arrival=point_in(*sm,1,state);if(!arrival)return 2;
  std::vector<Cell> starts{{arrival->layer,arrival->tx,arrival->ty}};if(!before)starts.push_back({2,26,24}); // exact owner log pose
  for(auto start:starts)for(const auto& target:baked::grids(*sm).objects){
   const bool chest=target.type==baked::ObjType::TreasureBox;
   const bool door=target.type==baked::ObjType::Entity&&baked::str(target.name).find(L"カギのかかった扉")!=std::wstring::npos&&state.active(*sm,target);
   if(!chest&&!door)continue;auto s=capture(*sm,flags,start,target);const auto route=plan_snapshot(s);
   if(before){if(chest&&route.path.success)++false_valid;continue;}
   const bool allowed=door||profile==4;
   expect(allowed?route.path.success:!route.path.success&&route.path.reason==L"Story gated","directions",profile,target.entity_id());
   auto browse=capture(*sm,flags,start,target);const auto result=plan_browse_snapshot(browse);
   expect(allowed?result.route.direct:!result.route.direct&&result.route.reason==L"Story gated","browse",profile,target.entity_id());
   ExitSnapshot filter(false,map_state::Resolver(flags,fixture::records));auto copied=capture(*sm,flags,start,target);
   filter.route.map_id=copied.map_id;filter.route.submap=copied.submap;filter.route.position=copied.position;filter.route.layer=copied.layer;
   filter.route.vehicle=copied.vehicle;filter.route.locale=copied.locale;filter.route.goal=copied.goal;filter.route.obstacles=std::move(copied.obstacles);filter.route.kind=TargetKind::Infer;
   const auto mask=story_gate_mask(filter,{{filter.route.goal,TargetKind::Interact}},[] {return false;});
   expect(mask.size()==1&&mask[0]==!allowed,"story-filter",profile,target.entity_id());
  }
 }
 if(before){std::printf("Cornelia preserved before-data: %d false-valid chest routes (expected6)\n",false_valid);return false_valid==6?0:1;}
 int declarations=0;for(const auto& [name,record]:fixture::records.obj){const auto* rows=record.get(L"collision_layers");if(!rows)continue;const auto* maps=record.get(L"map_ids");if(!maps||maps->arr.empty())return 2;const auto* room=baked::submap_by_map_id(int(maps->arr.front().n));if(!room)return 2;map_state::Resolver state({},fixture::records);
  for(const auto& row:rows->arr){const int id=int(row.get(L"id")->n);bool found=false;for(const auto& o:baked::grids(*room).objects)if(o.entity_id()==id&&baked::str(o.group)==row.get(L"group")->s&&baked::object_identifier(o)==row.get(L"raw")->s){unsigned expected=0;for(const auto& layer:row.get(L"layers")->arr)expected|=1u<<int(layer.n);expect(state.collision_layers(*room,o)==expected,"paired-native-layers",0,id);auto wrong=o;wrong.c=-9876;expect(state.collision_layers(*room,wrong)==(1u<<o.layer),"identity-mismatch",0,id);found=true;++declarations;break;}if(!found)return 2;}
 }
 expect(declarations==24,"all native key/story-door variants",0,0);
 std::printf("Cornelia key/door/chest consumers: %d cases/%d failures;24 exact declarations, worker-native reads%d\n",cases,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
}
#ifndef FFPR_KEY_DOOR_FIXTURE_ONLY
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff1/maps.bin");if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF1;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{const auto current=fixture::records;for(auto& [name,record]:fixture::records.obj)record.obj.erase(L"collision_layers");result=keyed::cornelia(true);fixture::records=current;if(!result)result=keyed::cornelia(false);});worker.join();return result;}
#endif
