// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Ice Cavern diagnostic: actual native arrivals and current-floor event capture,
// ordinary versus event-safe journeys to the unclaimed Levistone. No game runs.
#define FFPR_KEY_DOOR_FIXTURE_ONLY
#include "key_door_route_check.cpp"
#include "../src/core/fieldnav/autowalk_route.h"
#include <type_traits>
namespace ice {
using namespace field;using namespace baked_nav;
// Isolated FF1 delivery intentionally excludes the pending all-six authored
// guide policy. Exercise it only when that actual producer exists; ordinary
// precision navigation must qualify against both maintained source revisions.
template<class Goal,class=void>struct SafePolicy:std::false_type{};
template<class Goal>struct SafePolicy<Goal,std::void_t<decltype(std::declval<Goal>().avoid_other_events),decltype(std::declval<Goal>().allow_scripted_crossings)>>:std::true_type{};
template<class Goal>void safe_policy(Goal& goal,bool enabled) {
 if constexpr(SafePolicy<Goal>::value){goal.avoid_other_events=enabled;goal.allow_scripted_crossings=enabled;}
}
RouteSnapshot capture(const baked::Submap& room,const map_state::Flags& flags,Cell at,const baked::Object& target) {
 RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=room.first_map_id;s.submap=&room;s.layer=at.layer;
 s.position=world_of_cell(room,at);s.vehicle={true,1,1,{}};s.locale=L"en";
 s.goal.map_id=91;s.goal.cell={target.layer,target.tx,target.ty};s.goal.entity_id=target.entity_id();s.goal.raw_name=baked::object_identifier(target);s.goal.examine=true;s.kind=TargetKind::Interact;
 std::set<std::pair<int,std::wstring>> seen;
 for(const auto& source:baked::grids(room).objects) {
  if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(room,source))continue;
  const auto object=s.state.effective(room,source);const auto raw=baked::object_identifier(source);
  if(!seen.insert({object.entity_id(),raw}).second)continue;
  NativeObstacle n;n.type=int(object.type);n.id=object.entity_id();n.raw_name=raw;n.cell={object.layer,object.tx,object.ty};n.enabled=n.applies=true;
  n.trigger=baked_route::step_event(object);n.npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;n.chest=object.type==baked::ObjType::TreasureBox;
  n.collider=object.type==baked::ObjType::CollisionEntity;n.collision=object.a;
  if(object.type==baked::ObjType::Warp){if((object.d&255)!=1)continue;n.type=2;}
  s.obstacles.push_back(std::move(n));
 }return s;
}
void describe(const char* policy,const RouteSnapshot& snapshot,const Planned& result) {
 std::printf("Ice %s map%d startL%d %d,%d: success%d next%d trap%d event%d steps%d reason[%ls]",policy,snapshot.map_id,snapshot.layer,
  cell_of(*snapshot.submap,snapshot.position,snapshot.layer).x,cell_of(*snapshot.submap,snapshot.position,snapshot.layer).y,result.path.success,result.path.next_map_id,result.path.required_trap,result.path.event_on_route,result.path.step_count,result.path.reason.c_str());
 const auto receipt=result.pending.incoming_transition?result.pending.incoming_transition:result.pending.incoming_trap;
 if(receipt)for(const auto& object:baked::grids(*snapshot.submap).objects)if(object.entity_id()==receipt->entity_id&&object.contains(receipt->entry.x,receipt->entry.y))
  std::printf(" edgeType%d/id%d/d%d/trap%d at%d,%d",int(object.type),object.entity_id(),object.d,baked::is_trap(object),object.tx,object.ty);
 std::printf("\n");
}
int run() {
 const auto* goal_room=baked::submap_by_map_id(91);if(!goal_room)return 2;
 const baked::Object* target=nullptr;for(const auto& object:baked::grids(*goal_room).objects)if(object.type==baked::ObjType::AnimEntity&&object.entity_id()==8)target=&object;
 const auto* record=fixture::records.get(baked::name_of(*goal_room));if(!target||!record)return 2;
 std::set<int> bits;const auto collect=[&](const json::Value* code){if(code)for(const auto& op:code->arr){const auto& v=op.arr;if(v.size()>=3&&v[0].n==2&&v[1].n>=0&&v[1].n<4&&v[2].n>=0&&v[2].n<256)bits.insert(int(v[1].n)*256+int(v[2].n));}};
 collect(record->get(L"code"));if(const auto* events=record->get(L"events"))for(const auto& event:events->arr)if(const auto* id=event.get(L"id");id&&int(id->n)==8)collect(event.get(L"code"));
 if(bits.size()>10)return 2;const std::vector<int> indexes(bits.begin(),bits.end());map_state::Flags flags;bool found=false;
 for(unsigned pattern=0;pattern<(1u<<indexes.size());++pattern) {
  flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=91;flags.current_area_id=int(record->get(L"area_id")->n);
  for(size_t bit=0;bit<indexes.size();++bit)flags.scenario[indexes[bit]]=(pattern>>bit)&1;
  map_state::Resolver state(flags,fixture::records);if(state.known(*goal_room)&&state.active(*goal_room,*target)&&!state.consumed(*goal_room,*target)){found=true;break;}
 }
 if(!found)return 2;std::printf("Levistone active context:");for(int bit:indexes)std::printf(" %d=%d",bit,flags.scenario[bit].value_or(-1));std::printf("; targetL%d %d,%d\n",target->layer,target->tx,target->ty);

 int failures=0,contexts=0,episodes=0,hops=0,drops=0,steps=0,pit_count=0,encounters=0;
 const auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 expect(!precision_passages::read(*goal_room),"no unverified half-step bypass of the native pit island");
 for(const auto& object:baked::grids(*goal_room).objects)if(baked::is_trap(object)&&object.type==baked::ObjType::Warp) {
  ++pit_count;expect(object.type==baked::ObjType::Warp&&object.a==92&&object.b==99&&(object.d&255)==1,
    "every one of the 19 native B2 pits retains its B3/point99 step transition");
 }
 expect(pit_count==19,"all19 B2 pits preserved");
 const auto& first=baked::grids(*baked::submap_by_map_id(88));
 int b1_holes=0;for(const auto& o:first.objects)if(baked::is_trap(o)&&o.type==baked::ObjType::Warp) {
  ++b1_holes;expect(o.entity_id()==38&&o.type==baked::ObjType::Warp&&o.a==91&&o.b==2&&o.layer==0&&o.tx==30&&o.ty==29,
    "B1's real hole leads to the separate B2 island arrival");
 }expect(b1_holes==1,"one B1 island-entry hole");
 const auto phase_check=[&](const RouteSnapshot& snapshot,const Planned& planned) {
  const auto* room=snapshot.submap;
  for(size_t pi=0;pi<planned.path.walk_phases.size();++pi) {
   const auto& phase=planned.path.walk_phases[pi];
   expect(phase.precision.empty(),"normal keyboard route never requests half steps");
   for(size_t i=1;i<phase.cells.size();++i) {
    const auto a=phase.cells[i-1],b=phase.cells[i];++steps;
    const bool cardinal=(a.layer==b.layer&&std::abs(a.x-b.x)+std::abs(a.y-b.y)==1);
    const bool layer_change=a.layer!=b.layer&&std::abs(a.x-b.x)<=1&&std::abs(a.y-b.y)<=1;
    if(!cardinal&&!layer_change)std::printf("Invalid Ice edge L%d %d,%d -> L%d %d,%d\n",a.layer,a.x,a.y,b.layer,b.x,b.y);
    expect(cardinal||layer_change,"all Ice directions use native cardinal movement or a native layer change");
    for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::Warp&&baked::is_trap(o)&&o.layer==b.layer&&o.contains(b.x,b.y)) {
     const bool last=pi+1==planned.path.walk_phases.size()&&i+1==phase.cells.size();
     const auto receipt=planned.pending.incoming_trap;
     expect(last&&receipt&&receipt->entity_id==o.entity_id()&&planned.path.next_map_id==o.a&&planned.path.required_trap,
       "a real pit may be entered only as the explicit final transition of this leg");
    }
   }
  }
  for(const auto* word:{L"Northeast",L"Northwest",L"Southeast",L"Southwest",L"0.5"})
   expect(planned.path.description.find(word)==std::wstring::npos,"directions stay whole-step and cardinal");
 };
 // Real arrivals on every floor, both sides of the room, the exact failed
 // shortcut start, and the separate native point2 inside the pit island.
 for(int index=0;index<baked::submap_count();++index) {
  const auto* origin=baked::submap_at(index);if(baked::name_of(*origin).rfind(L"Map_30061_",0)!=0)continue;
  flags.current_map_id=origin->first_map_id;map_state::Resolver state(flags,fixture::records);
  std::vector<Cell> starts;for(const auto& o:baked::grids(*origin).objects)
   if(o.type==baked::ObjType::PointIn&&state.active(*origin,o))starts.push_back({o.layer,o.tx,o.ty});
  if(origin->first_map_id==88)starts.push_back({2,12,7});
  if(origin==goal_room)for(const Cell at:std::vector<Cell>{{2,18,25},{0,23,18},{0,18,13},{0,18,18},{0,20,17}})starts.push_back(at);
  std::sort(starts.begin(),starts.end(),[](const auto& a,const auto& b){return std::tie(a.layer,a.y,a.x)<std::tie(b.layer,b.y,b.x);});starts.erase(std::unique(starts.begin(),starts.end()),starts.end());
  for(const auto start:starts) {
   ++contexts;
   for(bool strict:SafePolicy<MapGoal>::value?std::vector<bool>{false,true}:std::vector<bool>{false}) {
    const auto* room=origin;Cell at=start;Pending pending;bool arrived=false,encounter_cleared=false;int local_drops=0;
    std::vector<int> visited_maps{room->first_map_id};
    for(int leg=0;leg<12;++leg) {
     flags.current_map_id=room->first_map_id;
     if(const auto* r=fixture::records.get(baked::name_of(*room)))flags.current_area_id=int(r->get(L"area_id")->n);
     auto s=capture(*room,flags,at,*target);s.pending=pending;safe_policy(s.goal,strict);
     // The real battle123 callback sc_enc_30061_1_2 runs puppet9 Hide on
     // actor26. Model that observed current-floor result only after the actual
     // interaction stop; never pretend it was absent from the initial route.
     if(room->first_map_id==92&&encounter_cleared)std::erase_if(s.obstacles,[](const auto& o){return o.id==26;});
     const auto next=plan_snapshot(s);describe(strict?"guided":"ordinary",s,next);
     expect(next.path.success,"every native arrival and reported side has a complete normal route");
     if(!next.path.success)break;
     phase_check(s,next);
     if(next.path.npc_approach) {
      expect(room->first_map_id==92&&!encounter_cleared&&at==Cell{0,21,6},"only the real B3 forced encounter interrupts this route");
      expect(!next.path.walk_phases.empty()&&next.path.walk_phases.back().cells.back()==Cell{0,21,6}&&next.path.walk_phases.back().manual_after,
        "stop safely before actor26 and hand native encounter control back");
      if(room->first_map_id!=92||encounter_cleared)break;
      encounter_cleared=true;++encounters;pending=next.pending;continue;
     }
     if(!next.path.next_map_id) {
      arrived=room==goal_room&&!next.path.walk_phases.empty()&&next.path.walk_phases.back().cells.back()==Cell{0,19,16};
      expect(!next.path.required_trap,"island arrival reaches the Levistone without another pit");break;
     }
     const auto receipt=next.pending.incoming_trap?next.pending.incoming_trap:next.pending.incoming_transition;
     expect(receipt.has_value(),"each stair/drop has an exact continuation receipt");if(!receipt)break;
     if(next.pending.incoming_trap) {
      ++local_drops;++drops;
      expect(next.path.required_trap,"necessary drop is explicitly warned");
      if(room==goal_room)expect(receipt->destination==92&&receipt->arrival==Cell{0,21,6},"outer B2 drop arrives at the real B3 point99");
      if(room->first_map_id==88)expect(receipt->destination==91&&receipt->arrival==Cell{0,20,17},"B1 hole arrives inside the Levistone island");
     }
     pending=next.pending;room=baked::submap_by_map_id(receipt->destination);at=receipt->arrival;++hops;
     if(!room)break;visited_maps.push_back(room->first_map_id);
    }
    expect(arrived,"full physical journey ends at the Levistone interaction approach");
    const bool island=origin==goal_room&&start==Cell{0,20,17};
    if(origin==goal_room) {
     expect(local_drops==(island?0:2),"outer room requires B3 drop then B1 island-entry drop; island origin needs none");
     if(!island)expect(visited_maps==std::vector<int>({91,92,88,91}),"both sides use B2 to B3 to B1 to B2 island");
    }
    if(origin->first_map_id==88&&start==Cell{2,12,7})expect(local_drops==2,"dungeon entrance cannot shortcut the pit island");
    ++episodes;
   }
  }
 }
 // Exact post-pickup B3 report: the remote list retained the northern B1 world
 // door13 instead of the reachable southern door19. The route graph can reach
 // the latter without revisiting a pit; the former is a different component.
 flags.scenario[24]=1;flags.current_map_id=92;flags.treasure[107]=1;
 const auto* b3=baked::submap_by_map_id(92);const auto* b1=baked::submap_by_map_id(88);
 filter_worker::FilterInput grouped;grouped.current_map=92;grouped.exits=true;
 for(int id:{13,19}) {
  const auto& objects=baked::grids(*b1).objects;const auto at=std::find_if(objects.begin(),objects.end(),[&](const auto& o){return o.type==baked::ObjType::GotoMap&&o.entity_id()==id;});
  expect(at!=objects.end(),"both native B1 world doors exist");if(at==objects.end())continue;
  auto request=capture(*b3,flags,{2,30,42},*target);request.goal.map_id=88;request.goal.cell={at->layer,at->tx,at->ty};
  request.goal.entity_id=id;request.goal.raw_name=baked::object_identifier(*at);request.goal.examine=false;request.kind=TargetKind::Transition;
  const auto route=plan_snapshot(request);const bool southern=id==19;
  expect(route.path.success==southern,"post-fall B3 reaches southern B1 exit; northern component stays unreachable");
  if(southern)expect(route.path.next_map_id==88&&!route.path.required_trap,"B3-to-world route uses the real B1 stair without another pit");
  filter_worker::FilterRow row;row.index=grouped.rows.size();row.remote_map=88;row.kind=Kind::MapExit;row.category=Category::MapExits;
  row.cell_x=at->tx;row.cell_y=at->ty;row.cell_layer=at->layer;row.entity_id=id;row.destination=1;row.name=L"Exit to World Map";
  row.route_distance=route.path.success?route.path.step_count:-1;grouped.rows.push_back(row);
  std::printf("Ice B3 explicit world door%d %d,%d success%d next%d\n",id,at->tx,at->ty,route.path.success,route.path.next_map_id);
 }
 const auto category=filter_worker::build_categories(grouped);
 expect(category.views[static_cast<size_t>(Category::MapExits)].order==std::vector<size_t>({1}),"existing reachability grouping chooses the reachable physical world door when both are retained");
 std::printf("Ice complete physical route: contexts%d episodes%d transitions%d deliberate-drops%d native-encounters%d cardinal-steps%d pits%d failures%d worker-native%d\n",contexts,episodes,hops,drops,encounters,steps,pit_count,failures,fixture::wrong_owner.load());
 return failures||fixture::wrong_owner?1:0;
}
}
int main(int argc,char**argv){std::setvbuf(stdout,nullptr,_IONBF,0);if(argc>2)return 2;const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read(argc==2?argv[1]:"data/ff1/maps.bin");if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF1;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{result=ice::run();});worker.join();return result;}
