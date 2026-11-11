// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual guide/planner on captured native quest receipts and physical rooms.
#define main floating_fixture_entry
#include "floating_continent_route_check.cpp"
#undef main
namespace south_figaro {
using namespace floating_fixture;
map_state::Flags initial(){auto f=flags();f.scenario.fill(0);for(int i=0;i<=51;++i)f.scenario[size_t(i)]=1;f.scenario[98]=1;f.global[0]=1;f.current_map_id=38;return f;}
void context(map_state::Flags& f){const auto* room=baked::submap_by_map_id(f.current_map_id);f.current_area_id=int(fixture::records.get(baked::str(room->name))->get(L"area_id")->n);}
Cell entrance(int map,int point){for(const auto&o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
ffpr::ff6::story::Plan model(map_state::Flags f,Cell at){ffpr::ff6::story::Snapshot state;state.map=f.current_map_id;state.flags=f;state.player=at;state.locale=L"en";return ffpr::ff6::story::plan(state);}
map_state::Resolver observed_state(const map_state::Flags& f){
 if(f.current_map_id!=1)return map_state::Resolver(f,fixture::records);
 // The native world initializer's51 branch emits this exact SetEntities.
 // Production captures that observation even when earlier native world calls
 // make pure initializer projection unknown. A Flags-only fake omits it.
 fixture::captured_map_state=f;fixture::observed_group_map=1;
 fixture::observed_map_groups={f.scenario[51]==1?L"ev_e_0066_1":L"ev_e_0066"};
 return map_state::Resolver();
}
void observed_obstacles(RouteSnapshot& s){
 if(!s.submap->world){obstacles(s);return;}
 for(const auto& source:baked::grids(*s.submap).objects){
  if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp)continue;
  const std::wstring group=baked::str(source.group);
  if(!group.empty()&&group!=L"entity_default"&&!fixture::observed_map_groups.count(group))continue;
  if(!s.state.active(*s.submap,source,true))continue;
  const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);
 }
}
bool follow(map_state::Flags& f,Cell& at,int expected,int& hops){
 for(int leg=0;leg<24;++leg){context(f);const auto* room=baked::submap_by_map_id(f.current_map_id);RouteSnapshot s(observed_state(f));s.map_id=f.current_map_id;s.submap=room;s.layer=room->world?1:at.layer;s.position=world_of_cell(*room,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;observed_obstacles(s);Planned p;ffpr::ff6::story::Plan advice;
  std::thread worker([&]{advice=model(f,at);if(advice.supported&&!advice.unknown&&!advice.finished){s.candidates=advice.targets;p=plan_snapshot(s);}});worker.join();
  std::printf("South guide map%d at%d,%d,%d walk%d expected%d chose%d targetmap%d success%d next%d npc%d unknown%d note%s reason%s\n",s.map_id,at.layer,at.x,at.y,baked::walkable(baked::grids(*room),at.layer,at.x,at.y,true),expected,p.goal.entity_id,p.goal.map_id,p.path.success,p.path.next_map_id,p.path.npc_approach,advice.unknown,text::to_utf8(advice.note).c_str(),text::to_utf8(p.path.reason).c_str());
  if(!advice.supported||advice.unknown||advice.finished||!p.path.success||p.path.required_trap)return false;
  if(!p.path.next_map_id){if(p.path.npc_approach||p.goal.entity_id!=expected)return false;for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;++hops;f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
  if(expected<0&&f.current_map_id==p.goal.map_id)return at==p.goal.cell;
 }return false;
}
}
int main(int argc,char**argv){using namespace south_figaro;int failures=0,cases=0,hops=0;std::setvbuf(stdout,nullptr,_IONBF,0);auto expect=[&](bool ok,const char*why){++cases;if(!ok){++failures;std::printf("FAIL %s\n",why);}};const auto read=[](const char*p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 if(argc==2&&std::string(argv[1])=="--geometry"){
  for(int map:{38,60,61}){const auto* room=baked::submap_by_map_id(map);const auto& grid=baked::grids(*room);std::printf("ROOM%d %dx%d\n",map,room->width,room->height);
   for(const auto&o:grid.objects)if(o.type==baked::ObjType::PointIn||o.type==baked::ObjType::GotoMap)std::printf("LINK%d type%d entity%d at%d,%d,%d a%d b%d\n",map,int(o.type),o.entity_id(),o.layer,o.tx,o.ty,o.a,o.b);
   if(map==38)continue;
   for(int layer:{1,2}){std::printf("LAYER%d\n",layer);for(int y=5;y<std::min<int>(room->height,28);++y){std::printf("%02d ",y);for(int x=0;x<room->width;++x){char ch=baked::walkable(grid,layer,x,y,true)?'.':'#';for(const auto&o:grid.objects)if(o.layer==layer&&o.tx==x&&o.ty==y){if(o.type==baked::ObjType::NPC)ch='N';else if(o.type==baked::ObjType::GotoMap)ch='E';else if(o.type==baked::ObjType::PointIn)ch='I';}std::putchar(ch);}std::putchar('\n');}}
  }return 0;
 }
 if(argc==2&&std::string(argv[1])=="--world"){
  auto f=initial();for(int i=54;i<=62;++i)f.scenario[size_t(i)]=1;f.current_map_id=1;context(f);const auto* room=baked::submap_by_map_id(1);map_state::Resolver state(f,fixture::records);const auto& grid=baked::grids(*room);auto at=entrance(1,5);
  for(const auto&o:grid.objects)if(o.type==baked::ObjType::GotoMap&&(o.a==333||o.a==630))std::printf("CAVE EDGE%d destination%d point%d active%d consumed%d at%d,%d native group%s\n",o.entity_id(),o.a,o.b,state.active(*room,o,true),state.consumed(*room,o),o.tx,o.ty,text::to_utf8(baked::str(o.group)).c_str());
  for(int y=at.y-1;y<=at.y+1;++y)for(int x=at.x-1;x<=at.x+1;++x)std::printf("WORLD cell%d,%d attr%d foot%d\n",x,y,baked::attribute(grid,x,y),baked::group_crosses(1,baked::attribute(grid,x,y)));
  std::printf("FOOT id1 type%d group%d\n",baked::transport(1)->type,baked::transport(1)->group);
  int hops=0;follow(f,at,7,hops);return 0;
 }
 if(argc==2){json::Value cases;if(!json::parse(read(argv[1]),cases))return 2;return floating_fixture::scenario(cases);}
 // Exact final player pose of sc_e_0066_1, rather than a generic town entry
 // whose arrival is outside this locked-down scenario's accessible pocket.
 auto f=initial();Cell at{1,47,43};
 struct Step {int target,receipt;};
 for(const auto step:{Step{8,52},Step{8,0},Step{103,53},Step{108,54},Step{7,55},Step{8,0},Step{1,56},Step{1,57},Step{7,0},Step{19,-1},Step{1,58},Step{11,59},Step{6,60},Step{1,61},Step{1,62},Step{7,582},Step{21,583},Step{2,63},Step{3,64}}){
  const bool ok=follow(f,at,step.target,hops);expect(ok,"full native sequence reaches the next real operation");if(!ok)break;
  if(step.receipt>0){f.scenario[size_t(step.receipt)]=1;if(step.receipt==52){f.scenario[53]=0;f.local[1]=1;}if(step.receipt==53)f.scenario[52]=0;if(step.receipt==59)f.scenario[52]=f.scenario[53]=0;}
  else if(step.receipt<0)f.scenario[52]=f.scenario[53]=0;
  else if(step.target==8)f.local[1]=1;
  else if(step.target==7)f.local[1]=1;
  // Merchant44's reviewed battle script independently gives costume52; the
  // cider callback then gives55 and leaves the player at native6,12/Upper.
  if(step.receipt==55){f.scenario[52]=1;f.scenario[53]=0;at={2,6,12};}
  // Uncostumed sc_e_0078_2 finishes its native jump24,15/layer2=Ground.
  if(step.receipt==58){f.current_map_id=48;at={1,24,15};f.local.fill(0);}
 }
 {auto unknown=initial();unknown.scenario[52].reset();expect(model(unknown,entrance(38,1)).unknown,"unknown disguise never certifies an outfit");auto retry=initial();retry.scenario[55]=retry.scenario[56]=1;retry.current_map_id=38;auto p=model(retry,entrance(38,13));expect(p.supported&&!p.unknown&&p.targets.size()==1&&p.targets[0].goal.map_id==60&&p.targets[0].goal.entity_id==1,"wrong-password return resumes second option without repeating completed cider/disguise steps");}
 {auto retry=initial();retry.current_map_id=62;retry.local[1]=1;auto pos=Cell{2,5,9};expect(follow(retry,pos,8,hops),"failed merchant Steal leaves and re-enters the native room before retrying");}
 {auto retry=initial();retry.scenario[52]=1;retry.current_map_id=38;retry.local[1]=retry.local[2]=1;auto pos=Cell{2,10,19};expect(follow(retry,pos,8,hops),"both defeated green soldiers lead through a real floor reset, never a missing NPC");retry.local[1]=1;expect(follow(retry,pos,103,hops),"green soldier is reachable again after actual town reload");}
 {auto retry=initial();retry.scenario[53]=retry.scenario[54]=retry.scenario[55]=1;retry.current_map_id=43;auto pos=Cell{2,6,12};expect(follow(retry,pos,12,hops),"saved cider without a stolen outfit reaches the alternative merchant");auto p=model(retry,pos);expect(p.targets.size()==1&&p.targets[0].instruction.find(L"second option")!=std::wstring::npos,"soldier recovery challenge gives second option before Steal");retry.scenario[52]=1;retry.scenario[53]=0;retry.local[1]=1;expect(follow(retry,pos,8,hops),"recovered merchant costume reaches the return boy");retry.local[1]=1;expect(follow(retry,pos,1,hops),"recovered cider state reaches the old man without repeating cider battle");}
 {auto retry=initial();retry.scenario[54]=retry.scenario[55]=retry.scenario[56]=1;retry.current_map_id=38;auto pos=entrance(38,13);expect(follow(retry,pos,1,hops),"actual wrong-password return with no outfit physically reaches password retry");}
 {auto saved=initial();for(int i=54;i<=63;++i)saved.scenario[size_t(i)]=1;saved.current_map_id=626;auto pos=entrance(626,2);expect(follow(saved,pos,3,hops),"saved final cave warning bypasses suppressed earlier noises");auto p=model(saved,pos);expect(p.finish_on_scenario==64,"only actual boss interaction arms final64");saved.scenario[63].reset();expect(model(saved,pos).unknown,"unknown cave warning cannot select a completed phase");}
 {auto unknown=initial();unknown.current_map_id=60;unknown.scenario[52]=1;unknown.local[1].reset();expect(model(unknown,entrance(60,1)).unknown,"unknown current boy pose never certifies an open passage");auto inactive=initial();inactive.current_map_id=1;inactive.scenario[50]=0;auto p=model(inactive,entrance(1,5));expect(!p.supported&&p.note==L"No puzzle guide for this map","ordinary world requests retain an explicit inactive response");}
 std::printf("South Figaro:%d assertions/%d native continuations/%d failures/owner%d\n",cases,hops,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
