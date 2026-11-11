// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Complete real letter, purchase/unlock, reunion and key/chest journeys.
#define main retained_cyan_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace cyan_fixture {
using namespace floating_fixture;int errors=0,cases=0;std::optional<Cell> dragon_pose;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
ffpr::ff6::story::Plan model(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){context(f);ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
RouteSnapshot snapshot(map_state::Flags f,Cell at){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;obstacles(s);if(s.map_id==472&&dragon_pose)for(auto& o:s.obstacles)if(o.npc&&o.id==2&&o.raw_name==L"ストームドラゴン")o.cell=*dragon_pose;return s;}
Planned direct(map_state::Flags f,Cell at,int entity,int target_map=0){auto s=snapshot(f,at);if(!target_map)target_map=f.current_map_id;for(const auto& o:baked::grids(*baked::submap_by_map_id(target_map)).objects)if(o.entity_id()==entity&&o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn){s.goal.map_id=target_map;s.goal.entity_id=entity;s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(o);s.kind=o.type==baked::ObjType::GotoMap?TargetKind::Transition:TargetKind::Interact;break;}Planned p;std::thread w([&]{p=plan_snapshot(s);});w.join();return p;}
bool manual_unlock(map_state::Flags f,Cell at){for(int leg=0;leg<16;++leg){auto p=direct(f,at,85,383);std::printf("Rust Rid map%d leg%d success%d next%d manual%d\n",f.current_map_id,leg,p.path.success,p.path.next_map_id,p.active&&p.active->key_approach);if(!p.path.success)return false;if(p.active&&p.active->key_approach)return !p.path.walk_phases.empty()&&p.path.walk_phases.back().manual_after;const auto receipt=p.pending.incoming_transition;if(!receipt)return false;f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);}return false;}
bool journey(map_state::Flags& f,Cell& at,int expected){
 for(int leg=0;leg<24;++leg){auto advice=model(f,at);if(advice.unknown||advice.finished||advice.targets.empty())return false;auto s=snapshot(f,at);s.candidates=std::move(advice.targets);Planned p;std::thread w([&]{p=plan_snapshot(s);});w.join();
  std::printf("Cyan map%d from%d,%d,%d goal%d/%d success%d next%d trap%d reason%s error%s %s\n",f.current_map_id,at.layer,at.x,at.y,p.goal.map_id,p.goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str(),text::to_utf8(p.path.full_description).c_str());
  if(!p.path.success||p.path.required_trap||p.goal.entity_id!=expected)return false;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
  if(receipt->map_id==p.goal.map_id&&receipt->entity_id==p.goal.entity_id)return true;
 }return false;
}
}
int main(){using namespace cyan_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");json::Value proof;if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes)||!json::parse(read("docs/re/cyan_zozo_native_20261104.json"),proof))return 2;
 const auto pose=[&](const wchar_t* id){const auto& row=proof.get(L"final_player_cells")->get(id)->arr;return Cell{int(row[0].n),int(row[1].n),int(row[2].n)};};
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;auto f=floating_fixture::flags();for(int bit:{148,155,297,353})f.scenario[bit]=1;f.party_cyan=false;f.current_map_id=274;Cell at=entry(274);
 {const auto* floor=baked::submap_by_map_id(474);const baked::Object* edge=nullptr;for(const auto& o:baked::grids(*floor).objects)if(o.type==baked::ObjType::Warp&&o.entity_id()==16)edge=&o;if(!edge)return 2;for(int mode=0;mode<3;++mode){auto g=f;g.scenario[365]=mode==0?std::optional<int>{}:std::optional<int>(mode==2);map_state::Resolver state(g,fixture::records);check(!state.consumed(*floor,*edge)==(mode==2),"unknown/early365 cannot turn the initial reunion scene into a free summit crossing");}}
 check(journey(f,at,14),"Maranda town entry reaches the real Lola introduction");f.scenario[357]=1;
 check(model(f,at).puzzle_types==2,"reading and accepting the reply is Other guidance");check(model(f,at,{true,false}).targets.empty(),"Other off withholds the letter choices");check(!model(f,at,{false,true}).targets.empty(),"letter choices work independently of Navigation");
 check(journey(f,at,15),"the opened physical letter is reachable");f.scenario[529]=1;
 check(model(f,at).targets[0].goal.entity_id==14&&!model(f,at).finished,"declining529 returns to Lola rather than claiming delivery");check(journey(f,at,14),"retry uses actual Lola1060");f.scenario[358]=1;
 check(model(f,at).puzzle_types==1&&model(f,at).targets[0].goal.entity_id==69,"only actual accepted358 advances to the native pigeon");check(journey(f,at,69),"accepted reply has a complete house-to-pigeon route");f.current_map_id=274;at=pose(L"274");f.scenario[359]=1;check(model(f,at).finished,"actual full native359 return finishes the letter delivery");
 f.current_map_id=383;at=entry(383);auto base=f;
 for(int mode=0;mode<3;++mode){auto g=base;g.scenario[361]=mode==0?std::optional<int>{}:std::optional<int>(mode==2);auto door=direct(g,at,104);check(door.path.success,"door approach remains valid with missing,absent or owned Rust Rid");auto beyond=direct(g,at,85);check(mode==2?beyond.path.success:!beyond.path.success&&beyond.path.reason==L"Story gated","only actual Rust Rid361 validates the still-closed onward route");if(mode==2)check(manual_unlock(g,at),"owned Rust Rid stops the full multi-floor journey for real manual unlocking");}
 check(journey(f,at,143),"the optional delivered-letter breadcrumb is reachable");f.scenario[360]=1;
 check(journey(f,at,100),"actual merchant100 is reached before buying");check(model(f,at).puzzle_types==2&&model(f,at).targets[0].goal.entity_id==100,"failed purchase remains the paid-choice objective");f.scenario[361]=1;
 check(journey(f,at,104),"purchased Rust Rid reaches the exact closed door");f.scenario[362]=1;
 check(journey(f,at,16),"complete Zozo/pub/mountain journey reaches the real reunion event");f.scenario[365]=1;f.current_map_id=474;at=pose(L"474");
 check(model(f,at).targets[0].goal.entity_id==1&&!model(f,at).finished,"Cyan joining365 continues to the chest key");check(direct(f,at,7).path.success,"the actual locked chest front stays approachable before owning its key");check(journey(f,at,1),"actual post-reunion pose reaches the key on the summit");f.scenario[366]=1;
 check(journey(f,at,7),"actual key pickup pose returns to the locked chest");check(model(f,at).finish_on_scenario==367&&!model(f,at).finished,"key possession alone cannot finish before chest367");f.scenario[367]=1;check(model(f,at).finished,"actual opened chest367 finishes the Cyan/key guide");
 f.current_map_id=472;at=entry(472,2);check(model(f,at).targets.empty(),"the finished Cyan guide does not pick up the optional dragon switch");
 const auto release=direct(f,at,1);check(release.path.success&&!release.path.walk_phases.empty(),"ordinary navigation reaches the optional release switch");
 if(release.path.success&&!release.path.walk_phases.empty())at=release.path.walk_phases.back().cells.back();
 f.scenario[363]=1;
 json::Value patrols;if(!json::parse(read("tools/npc_route_positions.json"),patrols))return 2;const json::Value* route=nullptr;for(const auto& row:patrols.arr)if(row.get(L"submap")->s==L"Map_30671_1"&&int(row.get(L"id")->n)==2)route=&row;if(!route)return 2;const auto& first=route->get(L"cells")->arr.front().arr;dragon_pose=Cell{int(first[0].n),int(first[1].n),int(first[2].n)};
 check(direct(f,at,2).path.success,"ordinary navigation reaches the released dragon's live patrol pose");
 f=base;f.scenario[359]=0;check(model(f,entry(383)).note==L"Deliver Lola’s reply to the pigeon in Maranda, then return to Zozo.","merchant's real prior delivery requirement is explained");f.scenario[361]=1;check(!model(f,entry(383)).targets.empty(),"an already-bought Rust Rid does not require repeating the letter quest");f.scenario[353]=0;check(!model(f,entry(383)).supported,"ruined-world guide is not selected before the native airship chapter");
 check(fixture::wrong_owner==0,"all route work uses owned state without native pointer access");std::printf("Cyan/Zozo:%d assertions/%d failures\n",cases,errors);return errors||fixture::wrong_owner?1:0;
}
