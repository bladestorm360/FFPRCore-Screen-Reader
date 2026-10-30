// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Exact real FFVI initializer and production projection; no native game access.
#include "field/map_state.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>
int main(){
 using namespace field;
 const auto read=[](const char* path){std::ifstream file(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(file),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size()))return 2;
 json::Value records;if(!json::parse(read("data/ff6/mapstate.json"),records))return 2;
 const auto* camp=baked::submap_by_map_id(350);const auto* record=records.get(L"Map_30410");
 if(!camp||!record||baked::name_of(*camp)!=L"Map_30410")return 2;
 const baked::Object* dog=nullptr;
 for(const auto& object:baked::grids(*camp).objects)if(object.type==baked::ObjType::NPC&&object.entity_id()==21)dog=&object;
 if(!dog)return 2;
 int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 for(int mask:{0,1})for(bool hidden:{false,true}){
  map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);
  flags.current_map_id=350;flags.current_area_id=int(record->get(L"area_id")->n);
  flags.local[3]=mask;flags.scenario[81]=1;flags.scenario[82]=hidden?1:0;flags.scenario[90]=0;
  const auto selection=map_state::project(*record,flags);map_state::Resolver resolver(flags,records);
  const auto actor=selection.entities.find(21);
  const bool explicitly_hidden=actor!=selection.entities.end()&&actor->second.visible==std::optional<bool>(false);
  const bool active=resolver.active(*camp,*dog);
  std::printf("Imperial Camp mask%d hide-branch%d: known%d hidden%d active%d\n",mask,hidden,selection.known,explicitly_hidden,active);
  expect(selection.known,"render-only mask calls cannot stop the native initializer projection");
  expect(active!=hidden,"actual NPC eligibility follows the native Hide21 branch");
  if(hidden)expect(explicitly_hidden,"native Hide21 remains an explicit visibility fact");
 }
 // An unloaded floor has unknown map-local flags. The foreground variants
 // may differ, but both branches still guarantee this persistent Hide fact.
 for(int current_map:{1,350}){
  map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);
  flags.current_map_id=current_map;flags.scenario[81]=flags.scenario[82]=1;
  const auto selection=map_state::project(*record,flags);map_state::Resolver resolver(flags,records);
  const auto actor=selection.entities.find(21);
  const bool hidden=actor!=selection.entities.end()&&actor->second.visible==std::optional<bool>(false);
  std::printf("Imperial Camp unknown local/current%d: known%d hidden%d active%d\n",current_map,selection.known,hidden,resolver.active(*camp,*dog));
  expect(hidden&&!resolver.active(*camp,*dog),"cross-floor projection retains Hide21 shared by both mask variants");
 }
 // The authored home is a scene position. sc_e_0111's two consecutively
 // called dog tracks move west2/north3, then east1/north1. Check their actual
 // final geometry rather than invent a counter relay at the old home.
 map_state::Flags pose_flags;pose_flags.scenario.fill(0);pose_flags.treasure.fill(0);pose_flags.global.fill(0);pose_flags.local.fill(0);pose_flags.area.fill(0);
 pose_flags.current_map_id=350;pose_flags.current_area_id=int(record->get(L"area_id")->n);pose_flags.scenario[81]=1;
 map_state::Resolver pose_state(pose_flags,records);
 baked_route::Request geometry;geometry.submap=camp;geometry.passages=true;geometry.ignore_hazards=true;
 map_state::add_relays(pose_state,*camp,geometry);map_state::add_obstacles(pose_state,*camp,geometry,true,false,false);
 std::vector<baked_route::Cell> entries;
 for(const auto& object:baked::grids(*camp).objects){
  if(object.type==baked::ObjType::PointIn)entries.push_back({object.layer,object.tx,object.ty});
 }
 const auto reached=baked_route::flood(geometry,entries);
 for(const auto pose:{baked_route::Cell{1,46,11},baked_route::Cell{1,44,8},baked_route::Cell{1,45,7}}){
  auto current=*dog;current.tx=pose.x;current.ty=pose.y;
  const auto goals=baked_route::goals_for(geometry,pose,baked_route::GoalKind::Interact,&current);
  const bool accessible=std::any_of(goals.cells.begin(),goals.cells.end(),[&](const auto& cell){return reached.reachable(cell);});
  std::printf("Imperial dog scene pose%d,%d: approaches%zu reachable%d\n",pose.x,pose.y,goals.cells.size(),accessible);
  expect(accessible,"actual known visible-stage geometry admits the dog home and scripted positions");
 }
 std::printf("Imperial Camp mask projection: %d failures\n",failures);return failures?1:0;
}
