// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual all-door geometry, directions, browsing, filters and manual-unlock
// journeys. Independent native-receipt floods are the consumable-budget oracle.
#define FFPR_KEY_TOPOLOGY_FIXTURE_ONLY
#include "ff3_key_topology_check.cpp"
namespace consumable {
using namespace field;using namespace baked_nav;
int failures=0,cases=0,journeys=0,unlocks=0,hops=0,chests=0,controls=0;
void expect(bool okay,const char* why,int map=0,int id=0) {
 ++cases;if(!okay){++failures;if(failures<35)std::printf("FAIL key %s map%d id%d\n",why,map,id);}
}
map_state::Flags initial(int root) {
 map_state::Flags f;f.scenario.fill(1);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);
 for(int bit=205;bit<=217;++bit)f.scenario[bit]=0;
 f.current_map_id=root;f.current_area_id=int(fixture::records.get(baked::name_of(*baked::submap_by_map_id(root)))->get(L"area_id")->n);
 f.party_mini=0;f.magic_keys=0;f.party_thief=false;return f;
}
RouteSnapshot capture(const baked::Submap& room,map_state::Flags f,Cell at,const baked::Submap& destination,const baked::Object& target) {
 f.current_map_id=room.first_map_id;map_state::Resolver state(f,fixture::records);RouteSnapshot s(map_state::Resolver(f,fixture::records));
 s.map_id=room.first_map_id;s.submap=&room;s.layer=at.layer;s.position=world_of_cell(room,at);s.vehicle={true,1,1,{}};s.locale=L"en";
 s.goal.map_id=destination.first_map_id;s.goal.cell={target.layer,target.tx,target.ty};s.goal.examine=true;
 s.goal.entity_id=target.entity_id();s.goal.raw_name=baked::object_identifier(target);s.kind=TargetKind::Interact;
 for(const auto& source:baked::grids(room).objects) {
  if(!state.active(room,source)||source.type==baked::ObjType::Warp)continue;
  const auto object=state.effective(room,source);NativeObstacle n;n.type=int(object.type);n.id=object.entity_id();n.raw_name=baked::object_identifier(object);
  n.cell={object.layer,object.tx,object.ty};n.enabled=n.applies=true;n.trigger=baked_route::step_event(object);
  n.npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;n.chest=object.type==baked::ObjType::TreasureBox;
  n.collider=object.type==baked::ObjType::CollisionEntity;n.collision=object.a;s.obstacles.push_back(std::move(n));
 }
 return s;
}
void merge(keys::Coverage& into,const keys::Coverage& other) {
 into.valid&=other.valid;for(const auto& [map,bits]:other.cells){auto& dst=into.cells[map];if(dst.empty())dst.assign(bits.size(),0);for(size_t i=0;i<bits.size();++i)dst[i]|=bits[i];}
}
bool contains(const keys::Coverage& coverage,const baked::Submap& room,Cell at) {
 const auto found=coverage.cells.find(room.first_map_id);const auto& grid=baked::grids(room);
 return found!=coverage.cells.end()&&at.layer>=0&&at.layer<3&&baked::in_bounds(grid,at.x,at.y)&&found->second[(size_t(at.layer)*grid.height+at.y)*grid.width+at.x];
}
bool approachable(const keys::Coverage& coverage,const baked::Submap& room,const baked::Object& target) {
 baked_route::Request movement;movement.submap=&room;movement.passages=true;
 const auto goals=baked_route::goals_for(movement,{target.layer,target.tx,target.ty},baked_route::GoalKind::Interact,&target);
 for(const auto& at:goals.cells)if(contains(coverage,room,at))return true;return false;
}
bool physical(const RouteSnapshot& snapshot,const PathInfo& path) {
 for(const auto& phase:path.walk_phases)for(size_t i=1;i<phase.cells.size();++i)for(const auto& actor:snapshot.obstacles)
  if(actor.type==kObjEntity&&actor.enabled&&actor.cell==phase.cells[i]) {
   const auto* object=baked_by_entity_id(baked::grids(*snapshot.submap),actor.id,baked::ObjType::Entity,actor.cell,actor.raw_name);
   if(object&&(object->d&1))return false;
  }
 return true;
}
void journey(int root,map_state::Flags flags,const baked::Submap& destination,const baked::Object& target,const json::Value& facts) {
 const auto* room=baked::submap_by_map_id(root);map_state::Resolver state(flags,fixture::records);const auto* entry=point_in(*room,1,state);if(!entry){expect(false,"journey entry");return;}
 Cell at{entry->layer,entry->tx,entry->ty};Pending pending;std::shared_ptr<const ActiveRoute> prior;bool done=false;
 for(int step=0;step<40&&!done;++step) {
  auto s=capture(*room,flags,at,destination,target);s.pending=pending;s.previous_active=prior;
  const auto plan=plan_snapshot(s);expect(plan.path.success,"journey valid",root,target.entity_id());if(!plan.path.success)return;
  expect(physical(s,plan.path),"journey crosses a physical closed lock",room->first_map_id,target.entity_id());
  expect(!plan.path.npc_approach&&plan.path.blocking_party<0,"journey stopped at unrelated actor",room->first_map_id,target.entity_id());
  if(plan.path.walk_phases.empty()||plan.path.walk_phases.back().cells.empty()) {expect(false,"journey missing phase");return;}
  at=plan.path.walk_phases.back().cells.back();pending=plan.pending;prior=plan.active;
  if(plan.active&&plan.active->key_approach) {
   const auto& phase=plan.path.walk_phases.back();expect(phase.manual_after&&phase.arrival_direction>=0,"unlock requires manual native interaction",room->first_map_id,target.entity_id());
   expect(pending.valid&&pending.goal.map_id==destination.first_map_id&&pending.goal.entity_id==target.entity_id(),"unlock preserves final objective",room->first_map_id,target.entity_id());
   int receipt=0;for(const auto& door:facts.get(L"doors")->arr)if(int(door.get(L"maps")->arr.front().n)==room->first_map_id){
    const auto& cell=door.get(L"cell")->arr;const Cell closed{int(cell[0].n),int(cell[1].n),int(cell[2].n)};
    if(closed.layer==at.layer&&std::abs(at.x-closed.x)+std::abs(at.y-closed.y)==1&&
       baked_route::direction_towards(at,closed,room->width,room->height)==phase.arrival_direction)receipt=int(door.get(L"receipt")->n);
   }
   expect(receipt>=205&&receipt<=217,"exact facing door receipt",room->first_map_id,target.entity_id());if(!receipt)return;
   expect(flags.scenario[receipt]==std::optional<int>(0),"never pay an opened door twice",room->first_map_id,receipt);
   if(flags.party_thief!=std::optional<bool>(true)){expect(flags.magic_keys&&*flags.magic_keys>0,"key budget not overspent",root,receipt);--*flags.magic_keys;}
   flags.scenario[receipt]=1;++unlocks;continue;
  }
  if(plan.path.next_map_id>0) {
   const auto receipt=pending.incoming_transition?pending.incoming_transition:pending.incoming_trap;
   expect(receipt&&receipt->destination==plan.path.next_map_id,"native transition receipt",room->first_map_id,target.entity_id());if(!receipt)return;
   room=baked::submap_by_map_id(receipt->destination);if(!room){expect(false,"journey missing map");return;}at=receipt->arrival;++hops;continue;
  }
  expect(room==&destination,"journey ends on destination floor",root,target.entity_id());
  baked_route::Request movement;movement.submap=room;movement.passages=true;
  const auto goals=baked_route::goals_for(movement,{target.layer,target.tx,target.ty},baked_route::GoalKind::Interact,&target);
  expect(std::find(goals.cells.begin(),goals.cells.end(),at)!=goals.cells.end(),"journey ends at legal final interaction",root,target.entity_id());done=true;
 }
 expect(done,"journey must terminate",root,target.entity_id());++journeys;
}
int run(const json::Value& facts) {
 int identities=0;for(const auto& door:facts.get(L"doors")->arr){
  const auto* room=baked::submap_by_map_id(int(door.get(L"maps")->arr.front().n));const int id=int(door.get(L"id")->n);bool found=false;
  for(const auto& object:baked::grids(*room).objects)if(object.type==baked::ObjType::Entity&&object.entity_id()==id){
   auto f=initial(room->first_map_id);f.magic_keys=1;map_state::Resolver one(f,fixture::records);const bool nested=int(door.get(L"receipt")->n)>=216;
   expect(one.can_unlock(*room,object)==!nested,"one-key exact identity",room->first_map_id,id);
   f.magic_keys=2;map_state::Resolver two(f,fixture::records);expect(two.can_unlock(*room,object),"two-key identity",room->first_map_id,id);
   for(int mismatch=0;mismatch<5;++mismatch){auto wrong=object;if(mismatch==0)wrong.c=-70;if(mismatch==1)++wrong.tx;if(mismatch==2)wrong.group=0;if(mismatch==3)wrong.name=0;if(mismatch==4)wrong.type=baked::ObjType::AnimEntity;expect(!two.can_unlock(*room,wrong),"identity mismatch cannot unlock",room->first_map_id,id);}
   const auto f0=initial(room->first_map_id);auto unknown=f0;unknown.magic_keys.reset();unknown.party_thief.reset();map_state::Resolver no(unknown,fixture::records);expect(!no.can_unlock(*room,object),"unknown capability",room->first_map_id,id);
   auto thief=unknown;thief.party_thief=true;map_state::Resolver yes(thief,fixture::records);expect(yes.can_unlock(*room,object),"active Thief without item storage",room->first_map_id,id);
   if(nested){f.magic_keys=1;f.scenario[214]=1;map_state::Resolver paid(f,fixture::records);expect(paid.can_unlock(*room,object),"paid ancestor costs no additional key",room->first_map_id,id);}
   expect(one.signature()!=two.signature()&&no.signature()!=yes.signature(),"capability changes invalidate route identity",room->first_map_id,id);
   // A selected lock remains a legal adjacent destination without a key.
   const Cell outside{object.layer,object.tx,object.ty+1};auto s=capture(*room,f0,outside,*room,object);const auto plan=plan_snapshot(s);
   expect(plan.path.success&&plan.path.step_count==0&&physical(s,plan.path),"door itself approachable",room->first_map_id,id);++identities;found=true;break;
  }expect(found,"declaration matches bake",room->first_map_id,id);
 }
 for(int root:{67,219,242}) {
  const auto* room=baked::submap_by_map_id(root);auto base=initial(root);map_state::Resolver state(base,fixture::records);const auto* entry=point_in(*room,1,state);if(!entry)return 2;
  const Cell start{entry->layer,entry->tx,entry->ty};std::vector<int> receipts;
  for(const auto& door:facts.get(L"doors")->arr){const auto* r=fixture::records.get(baked::name_of(*baked::submap_by_map_id(int(door.get(L"maps")->arr.front().n))));if(r&&r->get(L"area_id")&&int(r->get(L"area_id")->n)==base.current_area_id)receipts.push_back(int(door.get(L"receipt")->n));}
  std::array<keys::Coverage,3> oracle;oracle[0]=keys::reach_from(root,base);oracle[1]=oracle[2]=oracle[0];
  for(size_t i=0;i<receipts.size();++i){auto one=base;one.scenario[receipts[i]]=1;const auto coverage=keys::reach_from(root,one);merge(oracle[1],coverage);merge(oracle[2],coverage);
   for(size_t j=i+1;j<receipts.size();++j){auto two=one;two.scenario[receipts[j]]=1;merge(oracle[2],keys::reach_from(root,two));}}
  for(int profile=0;profile<5;++profile) {
   auto f=base;f.magic_keys=profile<=2?profile:0;f.party_thief=profile==3;if(profile==4){f.magic_keys.reset();f.party_thief.reset();}
   const auto& expected=oracle[profile==3?2:profile==4?0:profile];const auto actual=keys::reach_from(root,f);
   expect(expected.valid&&actual.valid,"complete floor flood",root,profile);
   for(const auto& [map,bits]:oracle[2].cells){const auto* destination=baked::submap_by_map_id(map);const auto& grid=baked::grids(*destination);
    for(int layer=0;layer<3;++layer)for(int y=0;y<grid.height;++y)for(int x=0;x<grid.width;++x){const Cell at{layer,x,y};expect(contains(actual,*destination,at)==contains(expected,*destination,at),"exact consumable-budget coverage",map,profile);}
    auto all=base;for(int receipt:receipts)all.scenario[receipt]=1;map_state::Resolver fully(all,fixture::records);
    for(const auto& target:grid.objects) {
     const bool chest=target.type==baked::ObjType::TreasureBox&&fully.active(*destination,target);
     const bool lock=target.type==baked::ObjType::Entity&&std::any_of(facts.get(L"doors")->arr.begin(),facts.get(L"doors")->arr.end(),[&](const auto& door){return int(door.get(L"maps")->arr.front().n)==map&&int(door.get(L"id")->n)==target.entity_id();});
     if((!chest&&!lock)||!approachable(oracle[2],*destination,target))continue;
     const bool allowed=approachable(expected,*destination,target);auto s=capture(*room,f,start,*destination,target);const auto plan=plan_snapshot(s);
     expect(allowed?plan.path.success:!plan.path.success&&plan.path.reason==L"Story gated","directions respects consumable budget",map,target.entity_id());
     expect(physical(s,plan.path),"published phase crosses physical lock",map,target.entity_id());
     auto browsing=capture(*room,f,start,*destination,target);const auto browse=plan_browse_snapshot(browsing);
     expect(allowed?browse.route.reason!=L"Story gated":!browse.route.direct&&browse.route.reason==L"Story gated","browse respects consumable budget",map,target.entity_id());
     ExitSnapshot filter(false,map_state::Resolver(f,fixture::records));auto copied=capture(*room,f,start,*destination,target);
     filter.route.map_id=copied.map_id;filter.route.submap=copied.submap;filter.route.position=copied.position;filter.route.layer=copied.layer;
     filter.route.vehicle=copied.vehicle;filter.route.locale=copied.locale;filter.route.goal=copied.goal;filter.route.kind=copied.kind;filter.route.obstacles=std::move(copied.obstacles);
     const auto mask=story_gate_mask(filter,{{filter.route.goal,TargetKind::Interact}},[]{return false;});
     expect(mask.size()==1&&mask[0]==!allowed,"filter respects consumable budget",map,target.entity_id());
     if(allowed&&profile!=4)journey(root,f,*destination,target,facts);if(chest)++chests;else ++controls;
    }
   }
  }
 }
 expect(identities==13&&journeys>0&&unlocks>0&&hops>0,"complete identities and journeys");
 std::printf("FFIII Magic Key/Thief: %d doors/%d assertions/%d chest profiles/%d door profiles/%d full journeys/%d manual unlocks/%d native hops; failures%d worker-native%d\n",identities,cases,chests,controls,journeys,unlocks,hops,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff3/maps.bin");json::Value facts;if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records)||!json::parse(read("docs/re/ff3_magic_key_gates_20261101.json"),facts))return 2;fixture::game_id=game::Id::FF3;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{result=consumable::run(facts);});worker.join();return result;}
