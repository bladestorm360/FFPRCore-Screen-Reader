// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Full native-shaped Library episodes. Source-bearing inputs remain external.
#define main retained_library_layout_main
#include "ff5_library_layout_check.cpp"
#undef main
namespace journey {
using namespace field;using namespace baked_nav;
struct Node {State state;int map=275;std::vector<std::pair<int,int>> actions;};
struct Operation {int id=0;const baked::Object* object=nullptr;bool portal=false,waypoint=false;Cell cell;};
constexpr std::array<int,8> bits{48,49,50,163,371,395,396,397};
std::vector<int> controls(int map){switch(map){case 275:return {9,10,14,15,16,17,21,27,20};case 277:return {31};case 278:return {12,13,17};case 279:return {4};case 281:return {13,16,15,14};case 283:return {7,1};default:return {};}}
const wchar_t* script(int map,int id){
 if(map==275)return id==20?L"sc_e_0220":id==17?L"sc_e_0220_1":id==9?L"sc_e_0220_2":id==10?L"sc_e_0220_3":id==14?L"sc_e_0220_4":id==15?L"sc_e_0220_5":id==16?L"sc_e_0220_6":id==27?L"sc_e_0220_7":L"sc_e_0220_8";
 if(map==277)return L"sc_e_0221";
 if(map==278)return id==12?L"sc_e_0222":id==13?L"sc_e_0222_1":L"sc_e_0430";
 if(map==281)return id==13?L"sc_e_0223":id==16?L"sc_e_0223_1":id==15?L"sc_e_0223_2":L"sc_e_0431";
 return L"";
}
map_state::Selection base(const Node& n){const auto* sm=baked::submap_by_map_id(n.map);const auto* r=fixture::records.get(baked::name_of(*sm));return r?map_state::project(*r,n.state.flags):map_state::Selection{};}
bool enter(Node& n,int map,int point){
 if(map==272){n.state.flags.scenario[396]=0;if(n.state.flags.scenario[50]==1){n.map=272;return true;}map=275;point=1;}
 if(map==276||map==278)n.state.flags.scenario[397]=0;
 n.map=map;n.state.flags.current_map_id=map;n.state.flags.current_area_id=28;n.state.flags.local.fill(0);
 const auto selection=base(n);if(!selection.known)return false;n.state.entities=selection.entities;
 map_state::Resolver resolver(n.state.flags,fixture::records);const auto* sm=baked::submap_by_map_id(map);const auto* arrival=point_in(*sm,point,resolver);
 if(!arrival)return false;n.state.at={arrival->layer,arrival->tx,arrival->ty};return true;
}
std::wstring identity(const Node& n){std::wstringstream out;out<<n.map<<L':'<<key(n.state);for(int bit:bits)out<<L':'<<n.state.flags.scenario[bit].value_or(-1);return out.str();}
bool active(const Node& n,const baked::Object& o){const auto b=base(n);const auto group=baked::str(o.group);return (group==L"entity_default"||b.groups.count(group))&&shown(n.state,o.entity_id());}
std::vector<Operation> operations(const Node& n){
 std::vector<Operation> out;const auto* sm=baked::submap_by_map_id(n.map);const auto& objects=baked::grids(*sm).objects;
 for(int id:controls(n.map))for(const auto& o:objects)if(o.entity_id()==id&&o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::GotoMap&&active(n,o)){
  if(n.map==283&&((id==7&&n.state.flags.scenario[49]==1)||(id==1&&n.state.flags.scenario[49]!=1)))continue;
  if(n.map==279&&n.state.flags.scenario[48]==1)continue;
  out.push_back({id,&o,false,false,{o.layer,o.tx,o.ty}});break;
 }
 for(const auto& o:objects)if((o.type==baked::ObjType::GotoMap||(o.type==baked::ObjType::Warp&&(o.d&255)==1))&&active(n,o)&&o.a>=272&&o.a<=283)
  out.push_back({o.entity_id(),&o,true,false,{o.layer,o.tx,o.ty}});
 bool on_trigger=false;for(const auto& op:out)if(op.portal||baked_route::step_event(*op.object))on_trigger|=op.object->layer==n.state.at.layer&&op.object->contains(n.state.at.x,n.state.at.y);
 if(on_trigger)for(const auto d:std::array<Cell,4>{{{0,0,-1},{0,1,0},{0,0,1},{0,-1,0}}})out.push_back({0,nullptr,false,true,{n.state.at.layer,n.state.at.x+d.x,n.state.at.y+d.y}});
 return out;
}
RouteSnapshot snapshot(const Node& n,const Operation& op){
 const auto* sm=baked::submap_by_map_id(n.map);RouteSnapshot s(map_state::Resolver(n.state.flags,fixture::records));s.map_id=n.map;s.submap=sm;s.layer=n.state.at.layer;s.position=world_of_cell(*sm,n.state.at);s.vehicle={true,1,1,{}};s.locale=L"en";
 s.goal.map_id=n.map;s.goal.cell=op.cell;s.goal.avoid_other_events=true;
 if(op.object){s.goal.entity_id=op.id;s.goal.raw_name=baked::object_identifier(*op.object);s.goal.examine=!op.portal&&!baked_route::step_event(*op.object);}
 s.kind=s.goal.examine?TargetKind::Interact:TargetKind::Spot;
 std::set<std::pair<int,std::wstring>> captured;
 for(const auto& o:baked::grids(*sm).objects){
  if(o.type==baked::ObjType::PointIn||o.type==baked::ObjType::GotoMap||!active(n,o))continue;
  const auto raw=baked::object_identifier(o);if(!captured.insert({o.entity_id(),raw}).second)continue;
  NativeObstacle seen;seen.id=o.entity_id();seen.type=int(o.type);seen.raw_name=raw;seen.cell={o.layer,o.tx,o.ty};seen.enabled=seen.applies=true;
  // Automatic Warp rows retain their exact raw Event identity/type. They are
  // not a second native object or a generic Entity stand-in.
  if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;seen.type=2;}
  seen.trigger=baked_route::step_event(o);seen.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;seen.chest=o.type==baked::ObjType::TreasureBox;seen.collider=o.type==baked::ObjType::CollisionEntity;seen.collision=o.a;
  seen.transportation_ids=s.state.transportation_mask(*sm,o);seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);s.obstacles.push_back(std::move(seen));
 }
 return s;
}
bool effect(Node& n,const wchar_t* name,bool prefix=false){
 const auto* p=program(name);if(!p)return false;const auto value=map_state::project(*p->get(prefix?L"prefix_record":L"record"),n.state.flags);if(!value.known)return false;
 for(const auto& [id,e]:value.entities){auto& into=n.state.entities[id];if(e.visible)into.visible=e.visible;if(e.x)into.x=e.x;if(e.y)into.y=e.y;}
 if(!prefix)for(const auto& f:p->get(L"flag_updates")->arr)n.state.flags.scenario[int(f.get(L"index")->n)]=int(f.get(L"value")->n);
 return true;
}
bool return_receipt(Node& n,const wchar_t* name,int bit,const wchar_t* digest){
 // These two independently reviewed four-command scripts contain only a
 // constant reset and ChangeMap. The conservative generic projector rejects
 // ChangeMap by design; it must not reject the actual physical transfer.
 const auto* p=program(name);if(!p||p->get(L"sha256")->s!=digest)return false;
 const auto* updates=p->get(L"flag_updates");if(!updates||updates->arr.size()!=1)return false;
 const auto& reset=updates->arr.front();
 if(reset.get(L"index")->n!=bit||reset.get(L"value")->n!=0)return false;
 n.state.flags.scenario[bit]=0;return true;
}
bool apply(Node& n,const Operation& op,Cell end){
 const int map=n.map;n.state.at=end;n.actions.push_back({map,op.waypoint?0:op.id});
 if(op.waypoint)return true;
 if(op.portal){
  // Exact native scripted doors, independently resolved through the actual
  // asset script IDs2634/1242. Their receipts precede ChangeMap: the upper
  // return resets395; the lower return resets371. Ordinary GotoMap has no
  // such effect. Keep the native programs external to every Git tree.
  if(map==276&&op.id==2&&!return_receipt(n,L"sc_e_0220_9",395,L"671b8b883ade86e00bf0ce545b29bb2d35de96e9183799add0a6016bf4df411a"))return false;
  if(map==278&&op.id==2&&!return_receipt(n,L"sc_20221_7_to_20221_6",371,L"f95c2f277a3a0007b92881f1d4b51c16e7698f8c67370345506857c6dec383ab"))return false;
  return enter(n,op.object->a,op.object->b);
 }
 if(map==279){n.state.flags.scenario[48]=1;const auto keep=Cell{1,5,6};if(!enter(n,279,1))return false;n.state.at=keep;return true;}
 if(map==283&&op.id==7){n.state.flags.scenario[49]=1;if(!enter(n,283,1))return false;n.state.at={1,5,10};return true;}
 if(map==283&&op.id==1){n.state.flags.scenario[50]=1;return enter(n,272,1);}
 if(map==281&&op.id==14)return effect(n,L"sc_e_0431",true)&&effect(n,L"sc_e_0431_1");
 if(!effect(n,script(map,op.id)))return false;
 if(map==278&&op.id==17)n.state.at={1,7,13};
 return true;
}
bool solve(const char* label,int map,int point,std::initializer_list<int> completed){
 Node initial;initial.state.flags.scenario.fill(0);initial.state.flags.treasure.fill(0);initial.state.flags.global.fill(0);initial.state.flags.area.fill(0);initial.state.flags.local.fill(0);
 for(int bit:completed)initial.state.flags.scenario[bit]=1;
 if(!enter(initial,map,point)){std::printf("FAIL %s entry\n",label);return false;}
 std::deque<Node> pending{initial};std::set<std::wstring> visited{identity(initial)};int edges=0;
 while(!pending.empty()&&visited.size()<8192&&edges<65536){
  auto current=std::move(pending.front());pending.pop_front();
  if(current.map==272&&current.state.flags.scenario[50]==1){std::printf("Library full %s:",label);for(const auto& [room,id]:current.actions)std::printf(" %d/%d",room,id);std::printf("; nodes%zu edges%d\n",visited.size(),edges);return true;}
  const auto ops=operations(current);
  for(const auto& requested:ops){
   auto s=snapshot(current,requested);const auto route=plan_snapshot(s);++edges;
   if(!route.path.success||route.path.walk_phases.empty()||route.path.npc_approach)continue;
   Operation actual=requested;Cell end=route.path.walk_phases.back().cells.back();bool contact=false,moved=false;
   for(const auto& phase:route.path.walk_phases){for(const auto cell:phase.cells){if(cell==current.state.at)continue;moved=true;
    for(const auto& candidate:ops)if(candidate.object&&(candidate.portal||baked_route::step_event(*candidate.object))&&candidate.object->layer==cell.layer&&candidate.object->contains(cell.x,cell.y)){actual=candidate;end=cell;contact=true;break;}
    if(contact)break;
   }if(contact)break;}
   if(!moved&&(actual.portal||actual.waypoint||(actual.object&&baked_route::step_event(*actual.object))))continue;
   Node next=current;if(!apply(next,actual,end))continue;
   if(visited.insert(identity(next)).second)pending.push_back(std::move(next));
  }
 }
 std::printf("FAIL Library full %s: nodes%zu edges%d pending%zu\n",label,visited.size(),edges,pending.size());return false;
}
}
#ifndef FFPR_LIBRARY_POLICY_CHECK
int main(){
 std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");
 if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("../tools/ff5_library_20261101/native_programs.json"),programs))return 2;
 fixture::game_id=game::Id::FF5;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;
 int failures=0,cases=0;const auto run=[&](const char* name,int map,int point,std::initializer_list<int> flags){bool pass=false;std::thread worker([&]{pass=journey::solve(name,map,point,flags);});worker.join();++cases;if(!pass)++failures;};
 run("fresh",275,1,{});run("Ifrit",279,1,{48,396,397});run("burned seal",278,4,{48,163,396});run("maze entry",281,1,{48,163,396});run("Byblos",283,1,{48,163,396});run("Mid",283,1,{48,49,163,396});run("completed return",281,3,{48,49,50,163});
 std::printf("Library complete native episodes: %d cases, failures%d owner%d\n",cases,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
#endif
