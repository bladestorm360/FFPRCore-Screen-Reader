// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual logged 5F position and all three isolated chests, through the real worker.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "fieldnav/autowalk_route.h"
int main() {
 using namespace field; using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff1/maps.bin");
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF1;g_loaded=g_prepared=true;
 const auto* map=baked::submap_by_map_id(103);const auto& grid=baked::grids(*map);
 int failures=0,cases=0;const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
 expect(!map->world&&baked::wrap_width(*map)==54&&baked::wrap_height(*map)==0,"native room wraps horizontally while retaining collision layers");
 for(int x:{38,39,40}) {
  const baked::Object* chest=nullptr;
  for(const auto& object:grid.objects)if(object.type==baked::ObjType::TreasureBox&&object.tx==x&&object.ty==11)chest=&object;
  if(!chest)return 2;
  map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(1);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=103;
  RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=103;s.submap=map;s.layer=0;s.locale=L"en";s.position=world_of_cell(*map,{0,19,29});
  s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;
  s.goal.map_id=103;s.goal.cell={chest->layer,x,11};s.goal.entity_id=chest->entity_id();s.goal.raw_name=baked::object_identifier(*chest);s.kind=TargetKind::Interact;s.name=L"Treasure Chest";
  auto browse_snapshot=s;Planned plan;BrowsePlanned browse;
  std::thread worker([&]{plan=plan_snapshot(s);});worker.join();
  std::thread browse_worker([&]{browse=plan_browse_snapshot(browse_snapshot);});browse_worker.join();
  ++cases;expect(plan.path.success&&browse.route.direct,"Sunken Shrine secret-passage chest is reachable in directions and browsing");
  bool crossed=false;ffpr::fieldnav::autowalk::Route follower;
  follower.set(plan.path.walk_phases,plan.path.map_width,plan.path.map_height,plan.path.wraps,plan.path.loop_axes);
  const auto corners=build_beacon_corner_index(*map,plan.path.walk_phases,false,world_of_cell);
  baked_route::Request movement;movement.submap=map;
  for(const auto& phase:plan.path.walk_phases)for(size_t i=1;i<phase.cells.size();++i) {
   const auto a=phase.cells[i-1],b=phase.cells[i];
   const auto step=follower.next(a);
   expect(step.result==ffpr::fieldnav::autowalk::Result::Move&&step.next==b,"native follower retains layer/stair/wrapped steps");
   expect(baked_route::edge_allowed(movement,grid,a,b),"every secret-route edge preserves native collision masks");
   if(std::abs(a.x-b.x)>grid.width/2&&a.y==b.y) {
    crossed=true;const int direction=baked_route::step_direction(a,b,baked::wrap_width(*map),0);
    expect(direction==1||direction==3,"seam direction is one cardinal east/west step");
    expect(step.x==(direction==1?1:-1)&&step.y==0,"native follower walks the requested side of the seam");
    const auto* corner=corners.find(a);const auto here=world_of_cell(*map,a);
    expect(corner&&corner->found&&(corner->point.x-here.x)*step.x>0,"beacon points through the seam in the spoken direction");
   }
  }
  expect(!plan.path.success||crossed,"secret-room route uses the real horizontal edge connection");
  std::printf("FF1 Shrine103 chest%d/id%d: directions%d browse%d wrap%d phases%zu\n",x,chest->entity_id(),plan.path.success,browse.route.direct,crossed,plan.path.walk_phases.size());
 }
 baked_route::Request edge;edge.submap=map;edge.start={2,0,6};edge.goals={{2,53,6}};
 auto route=baked_route::find(edge);expect(route.found&&route.cells.size()==2,"upper-layer west seam is a single legal tile");
 edge.start={2,53,6};edge.goals={{2,0,6}};route=baked_route::find(edge);
 expect(route.found&&route.cells.size()==2,"upper-layer east seam is a single legal tile");
 edge.blocked={{2,0,6}};expect(!baked_route::find(edge).found,"copied blocker still vetoes the seam destination");
 expect(!baked_route::edge_allowed(edge,grid,{2,0,0},{2,0,50}),"horizontal native loop cannot invent a vertical edge");
 // Exercise the actual native-owner walk_edge, not just the pure follower.
 // Native queries remain fake/read-only and must receive one requested tile.
 map_state::Flags native_flags;native_flags.current_map_id=103;
 RouteSnapshot native_snapshot(map_state::Resolver(native_flags,fixture::records));
 native_snapshot.map_id=103;native_snapshot.submap=map;native_snapshot.layer=2;
 {
  SnapshotScope scope(native_snapshot);fixture::capture_ready=fixture::probe_walk_edge=true;
  for(int direction:{-1,1}) {
   const Cell from{2,direction<0?0:53,6},to{2,direction<0?53:0,6};
   fixture::current_position=world_of_cell(*map,from);fixture::walk_edge_queries=0;fixture::walk_edge_allowed=true;
   expect(walk_edge(nullptr,reinterpret_cast<void*>(1),from,to,false)&&fixture::walk_edge_queries==1,"native seam edge uses one owner collision query");
   expect(std::abs(fixture::walk_edge_position.x-fixture::current_position.x-direction*16.f)<.001f&&
          std::abs(fixture::walk_edge_position.y-fixture::current_position.y)<.001f,"native seam collision query is one requested tile");
   fixture::walk_edge_allowed=false;
   expect(!walk_edge(nullptr,reinterpret_cast<void*>(1),from,to,false),"native seam collision veto is never bypassed");
  }
  fixture::capture_ready=fixture::probe_walk_edge=false;
 }
 expect(fixture::wrong_owner==0,"secret-passage planning uses no native worker calls");
 std::printf("FF1 Sunken Shrine: %d chests, failures%d owner%d\n",cases,failures,fixture::wrong_owner.load());return failures?1:0;
}
