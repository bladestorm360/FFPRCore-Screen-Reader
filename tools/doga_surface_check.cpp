// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual world grids, planner and category provider; no game/runtime probe.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include <chrono>
int main(){
 using namespace field;using namespace baked_nav;namespace r=field::baked_route;
 std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff3/maps.bin");if(!baked::load(bytes.data(),bytes.size()))return 2;
 if(!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF3;
 const auto* below=baked::submap_by_map_id(3);const auto* above=baked::submap_by_map_id(4);
 if(!below||!above)return 2;
 const auto& sea=baked::grids(*below);const auto& surface=baked::grids(*above);
 r::Request air;air.submap=above;air.start={1,150,203};air.vehicle_group=6;air.vehicle_type=3;air.transportation_id=9;air.ignore_hazards=true;
 const auto basin=r::flood(air);
 int candidates=0,nearest=INT_MAX;Cell best;
 for(int y=0;y<sea.height;++y)for(int x=0;x<sea.width;++x){
  const Cell at{1,x,y};if(baked::attribute(sea,x,y)!=30||baked::attribute(surface,x,y)!=16||basin.distance(at)<0)continue;
  ++candidates;const int d=std::abs(x-150)+std::abs(y-203);if(d<nearest){nearest=d;best=at;}
 }
 std::printf("Doga village air basin: %d paired surfaceable cells, nearest(%d,%d) distance%d\n",candidates,best.x,best.y,nearest);
 auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(1);flags->current_map_id=3;flags->current_area_id=1;
 auto capture=std::make_shared<ExitSnapshot>(true,map_state::Resolver(*flags,fixture::records));
 auto& snapshot=capture->route;snapshot.map_id=3;snapshot.submap=below;snapshot.layer=1;snapshot.locale=L"en";
 snapshot.position=world_of_cell(*below,{1,113,118});snapshot.vehicle={true,14,6,{}};
 snapshot.goal.map_id=3;snapshot.goal.cell=best;snapshot.kind=TargetKind::Spot;
 Planned underwater;std::thread worker([&]{underwater=plan_snapshot(snapshot);});worker.join();
 std::printf("Underwater native tunnel route: success%d steps%d end(%d,%d), error=%ls\n",underwater.path.success,underwater.path.step_count,best.x,best.y,underwater.path.error.c_str());
 auto post_flags=*flags;post_flags.current_map_id=4;
 auto post=std::make_unique<RouteSnapshot>(map_state::Resolver(post_flags,fixture::records));
 post->map_id=4;post->submap=above;post->layer=1;post->locale=L"en";post->position=world_of_cell(*above,best);post->vehicle={true,9,3,{}};
 post->goal.map_id=4;post->goal.cell={1,150,203};post->kind=TargetKind::Transition;
 Planned village;std::thread after([&]{village=plan_snapshot(*post);});after.join();
 bool lands=false;for(const auto& phase:village.path.walk_phases)lands|=phase.vehicle_before==route_progress::VehicleAction::ConfirmLand;
 std::printf("After native surface: village success%d steps%d land%d, error=%ls\n",village.path.success,village.path.step_count,lands,village.path.error.c_str());
 filter_worker::FilterInput input;input.navigation=capture;input.world=true;input.current_map=3;
 input.events=true;input.exits=true;input.sort=true;input.player=snapshot.position;input.reachable_only=true;
 filter_worker::Categories result;std::thread list_worker([&]{result=filter_worker::build_categories(input);});list_worker.join();
 const bool listed=!result.views[static_cast<size_t>(Category::MapExits)].order.empty();
 if(!listed)std::puts("FAIL underwater Map Exits must list a synthetic surfacing destination for Doga's Village");
 int failures=listed?0:1;
 const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 expect(candidates==4&&best==nautilus_surface::cell,"the authored point is one of exactly four paired native surface tiles in the village air basin");
 expect(underwater.path.success&&underwater.path.step_count==147&&village.path.success&&lands,"actual tunnel route and post-surface native landing remain possible");
 if(result.surface_target){
  const auto target=*result.surface_target;
  expect(result.views[static_cast<size_t>(Category::MapExits)].order.size()==1&&result.views[static_cast<size_t>(Category::Events)].order.empty(),"one marker is in Map Exits, without an Events duplicate");
  expect(target.goal.entity_id==nautilus_surface::entity&&target.goal.raw_name==nautilus_surface::identity&&target.distance==147,"owned category provider retains exact synthetic identity and real route distance");
  snapshot.goal=target.goal;snapshot.kind=TargetKind::Spot;
  Planned selected;std::thread selected_worker([&]{selected=plan_snapshot(snapshot);});selected_worker.join();
  std::printf("Selected marker: success%d steps%d manual%d action=%ls short=%ls full=%ls\n",selected.path.success,selected.path.step_count,
   selected.path.walk_phases.empty()?0:selected.path.walk_phases.back().manual_after,
   selected.path.walk_phases.empty()?L"":selected.path.walk_phases.back().action_after.c_str(),selected.path.description.c_str(),selected.path.full_description.c_str());
  expect(selected.path.success&&selected.path.step_count==147&&!selected.path.walk_phases.empty()&&
   selected.path.walk_phases.back().cells.back()==nautilus_surface::cell&&selected.path.walk_phases.back().manual_after&&
   selected.path.walk_phases.back().action_after==L"surface here"&&selected.path.full_description.find(L"surface here")!=std::wstring::npos,
   "selected marker routes while submerged and ends with a manual surface instruction");
  BrowsePlanned browse;std::thread browsing([&]{auto copy=std::make_unique<RouteSnapshot>(snapshot);browse=plan_browse_snapshot(*copy);});browsing.join();
  std::printf("Surface marker browse: direct%d via%d reason=%ls\n",browse.route.direct,browse.route.via_map_id,browse.route.reason.c_str());
  expect(browse.route.direct,"reachable synthetic surface endpoint shows its usable distance instead of Not directly reachable");
  bool underwater_only=true;int tunnel=0;
  for(const auto& phase:selected.path.walk_phases){underwater_only&=phase.vehicle_before==route_progress::VehicleAction::None;
   for(const auto& at:phase.cells)tunnel+=baked::attribute(sea,at.x,at.y)==29;}
  expect(underwater_only&&tunnel>0,"tunnel leg contains nonsurfaceable passage terrain and no fictional submarine landing");
  snapshot.position=target.position;Planned arrived;std::thread arrived_worker([&]{arrived=plan_snapshot(snapshot);});arrived_worker.join();
  expect(arrived.path.success&&arrived.path.step_count==0&&arrived.path.description.find(L"surface here")!=std::wstring::npos,"zero-step arrival retains the manual surface action");
  BrowsePlanned here;std::thread browse_here([&]{auto copy=std::make_unique<RouteSnapshot>(snapshot);here=plan_browse_snapshot(*copy);});browse_here.join();
  expect(here.route.direct,"zero-step surface destination remains a directly reachable browse endpoint");
  BrowsePlanned unavailable;std::thread wrong_vehicle([&]{auto copy=std::make_unique<RouteSnapshot>(snapshot);
   copy->vehicle={true,9,3,{}};unavailable=plan_browse_snapshot(*copy);});wrong_vehicle.join();
  expect(!unavailable.route.direct,"unfiltered marker browsing retains the wrong-vehicle access restriction");
  snapshot.position=world_of_cell(*below,{1,113,118});
 }
 auto get_marker=[&](bool reachable,std::function<bool()> cancel={}){
  std::optional<AirTransitionTarget> marker;std::thread worker_marker([&]{marker=nautilus_surface_target(*capture,cancel,reachable);});worker_marker.join();return marker;
 };
 capture->with_vehicles=false;expect(get_marker(true).has_value(),"mounted current-floor mode also lists the reachable marker");capture->with_vehicles=true;
 for(int id:{0,8,9,10}){snapshot.vehicle={id!=0,id,3,{}};expect(!get_marker(true),"other or missing vehicles cannot publish a reachable underwater surface action");
  const auto unfiltered=get_marker(false);expect(unfiltered&&unfiltered->distance<0,"Current map retains the real marker even without current access");}
 snapshot.vehicle={true,14,6,{}};
 int cancelled_calls=0;expect(!get_marker(true,[&]{return ++cancelled_calls>3;})&&cancelled_calls>3,"obsolete tunnel search cancels during owned routing and publishes no marker");
 for(int n:{1,2,4,5,6}){fixture::game_id=static_cast<game::Id>(n);expect(!get_marker(false),"every other game abstains from the FFIII marker");}
 fixture::game_id=game::Id::FF3;snapshot.map_id=4;snapshot.submap=above;expect(!get_marker(false),"surface map does not duplicate its village entrance as this underwater marker");
 snapshot.map_id=3;snapshot.submap=below;snapshot.cancelled={};
 const auto bench=[&](bool with_provider){
  double total=0,peak=0;std::thread timings([&]{
   for(int i=0;i<100;++i){const auto start=std::chrono::steady_clock::now();
    if(with_provider){auto marker=nautilus_surface_target(*capture,{},true);if(marker)snapshot.goal=marker->goal;}
    const auto plan=plan_snapshot(snapshot);expect(plan.path.success,"measured complete source-to-route episode remains usable");
    const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();total+=ms;peak=std::max(peak,ms);
   }});timings.join();std::printf("Doga owned %s:100 episodes total%.3fms peak%.3fms; native/audible latency unmeasured\n",with_provider?"marker+selected route":"selected route baseline",total,peak);
 };
 bench(false);bench(true);expect(fixture::wrong_owner==0,"all marker/list/path work uses owned data without worker native calls");
 double category_total=0,category_peak=0;
 std::thread complete_categories([&]{for(int i=0;i<20;++i){
  filter_worker::FilterInput complete;complete.navigation=capture;complete.current_map=3;complete.world=true;
  complete.reachable_only=true;complete.events=complete.exits=complete.sort=true;complete.player=snapshot.position;
  for(const auto& object:sea.objects)if(baked::world_exit(object)){
   filter_worker::FilterRow row;row.index=complete.rows.size();row.kind=Kind::MapExit;row.category=Category::MapExits;
   row.position=row.numbering_position=world_of_cell(*below,{1,object.tx,object.ty});row.cell_valid=true;
   row.cell_x=object.tx;row.cell_y=object.ty;row.cell_layer=1;row.entity_id=object.entity_id();row.destination=object.a;
   row.raw_name=baked::object_identifier(object);row.name=L"underwater entrance";complete.rows.push_back(std::move(row));
  }
  const auto start=std::chrono::steady_clock::now();const auto category=filter_worker::build_categories(complete);
  expect(category.surface_target&&category.views[static_cast<size_t>(Category::MapExits)].order.size()==4,"full real underwater exit list adds the one surfacing destination after native destination grouping");
  if(category.surface_target)snapshot.goal=category.surface_target->goal;
  const auto plan=plan_snapshot(snapshot);expect(plan.path.success,"complete category-to-selected-route episode succeeds");
  const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();category_total+=ms;category_peak=std::max(category_peak,ms);
 }});complete_categories.join();
 std::printf("Doga complete native-exit-category+selected-route worker:20 episodes total%.3fms peak%.3fms; game-frame/audible latency unmeasured\n",category_total,category_peak);
 expect(fixture::wrong_owner==0,"full native-exit list and selected route retain worker/native ownership");
 std::printf("Doga surfacing: %d failures\n",failures);return failures?1:0;
}
