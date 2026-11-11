// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual saved Ship pose/failed approach from the game-directory log. The
// preserved before run proves that choosing the nearest shore is insufficient.
#define FFPR_GIANT_ROUTE_FIXTURE_ONLY
#include "ff1_giant_route_check.cpp"
#include "../src/core/field/boarding_approach.h"
namespace board {
using namespace field;using namespace baked_nav;
int run(bool before) {
 const auto* world=baked::submap_by_map_id(1);const auto* ship=baked::transport(4);if(!world||!ship)return 2;
 const auto& grid=baked::grids(*world);const Cell boat{1,240,202},south{1,240,203},west{1,239,202};
 baked_route::Request foot;foot.submap=world;foot.start=south;foot.auto_groups={2};
 int cases=0,failures=0,reproduced=0;const auto expect=[&](bool yes,const char* why){++cases;if(!yes){++failures;std::printf("FAIL %s\n",why);}};
 const bool bad=boarding_approach::allowed(foot,*ship,south,boat),good=boarding_approach::allowed(foot,*ship,west,boat);
 std::printf("Logged Ship4 at240,202: south attribute%d native boarding%d; west attribute%d native boarding%d\n",baked::attribute(grid,south.x,south.y),bad,baked::attribute(grid,west.x,west.y),good);
 expect(!bad&&good,"native source-terrain rule reproduces North failure/East success");
 baked::Object boat_goal{};boat_goal.type=baked::ObjType::Entity;boat_goal.layer=1;boat_goal.tx=240;boat_goal.ty=202;boat_goal.tw=boat_goal.th=1;boat_goal.c=-1;
 std::vector<const baked::Object*> targets{&boat_goal};
 for(const auto& object:grid.objects)if(object.type==baked::ObjType::GotoMap&&object.tx==192&&object.ty==176)targets.push_back(&object);
 expect(targets.size()==2,"actual Ice world-map entrance exists");
 for(const auto* target:targets)for(const Cell start:std::vector<Cell>{south,west,{1,211,211}}) {
  map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=1;flags.current_area_id=1;
  const bool selected=target==&boat_goal;const auto kind=selected?TargetKind::Spot:TargetKind::Transition;
  auto s=giant::capture(*world,flags,start,boat,*target,true,true,kind);s.auto_groups={2};if(selected)s.goal.transportation_id=4;
  const auto planned=plan_snapshot(s);expect(planned.path.success,"actual destination has a route");
  bool boarded=false;
  for(const auto& phase:planned.path.walk_phases)if(phase.vehicle_before==route_progress::VehicleAction::WalkOn&&phase.transportation_id==4&&phase.action_cell) {
   boarded=true;const auto at=*phase.action_cell;auto request=foot;request.start=start;
   const bool valid=boarding_approach::allowed(request,*ship,at,boat);
   std::printf("Ship target%d start%d,%d: approach%d,%d direction%d physical%d steps%d\n",selected,start.x,start.y,at.x,at.y,phase.vehicle_direction,valid,planned.path.step_count);
   if(before){reproduced+=!valid;continue;}
   expect(valid,"every announced boarding direction is physically permitted");
   if(start==south)expect(at==west&&phase.vehicle_direction==1,"failed southern approach walks west/north before boarding East");
  }
  expect(boarded,"route actually boards the logged owned Ship");
 }
 if(before)expect(reproduced>=2,"preserved source reproduces invalid boarding in boat and onward routes");
 std::printf("FF1 boarding actual consumers: %d assertions, invalid before%d failures%d worker%d\n",cases,reproduced,failures,fixture::wrong_owner.load());
 return failures||fixture::wrong_owner?1:0;
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff1/maps.bin");if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF1;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;const bool before=std::getenv("FFPR_BOARDING_BEFORE")!=nullptr;std::thread worker([&]{result=board::run(before);});worker.join();return result;}
