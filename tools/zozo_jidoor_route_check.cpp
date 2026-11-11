// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Existing production routes for all animated early magicite and the static lamp.
#define main retained_jidoor_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace jidoor_fixture {
using namespace floating_fixture;int errors=0,cases=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
Planned direct(map_state::Flags f,Cell at,int entity,int target_map=0){if(!target_map)target_map=f.current_map_id;const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;obstacles(s);for(const auto& o:baked::grids(*baked::submap_by_map_id(target_map)).objects)if(o.entity_id()==entity&&o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn){s.goal.map_id=target_map;s.goal.entity_id=entity;s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(o);s.kind=o.type==baked::ObjType::GotoMap?TargetKind::Transition:TargetKind::Interact;break;}Planned p;std::thread w([&]{p=plan_snapshot(s);});w.join();return p;}
}
int main(){using namespace jidoor_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;auto base=floating_fixture::flags();base.global[0]=1;base.scenario[286]=0;base.scenario[148]=base.scenario[149]=1;base.current_map_id=395;
 const auto* floor=baked::submap_by_map_id(395);const auto arrival=entry(395);
 for(unsigned mask=0;mask<16;++mask){auto f=base;for(int i=0;i<4;++i)f.scenario[size_t(150+i)]=(mask>>i)&1;map_state::Resolver state(f,fixture::records);
  for(const auto& object:baked::grids(*floor).objects){if(object.type==baked::ObjType::AnimEntity&&object.entity_id()>=21&&object.entity_id()<=24){const int id=object.entity_id(),bit=id-21;const bool pending=(mask&(1u<<bit))==0;check(state.active(*floor,object)==pending,"each animated pickup uses its own actual saved receipt");if(pending)check(direct(f,arrival,id).path.success,"every remaining animated pickup has a valid loaded-floor route");}if((mask&1)&&object.type==baked::ObjType::NPC&&baked::str(object.group)==L"ev_e_0307"&&(object.entity_id()==44||(object.entity_id()>=26&&object.entity_id()<=30)))check(!state.active(*floor,object)&&state.active(*floor,object,true),"uncertain party NPCs are not certified destinations and remain possible obstacles");}
 }
 for(int id:{21,22,23,24}){auto f=base;f.current_map_id=383;Cell at=entry(383);bool reached=false;
  for(int leg=0;leg<20;++leg){auto p=direct(f,at,id,395);std::printf("Zozo pickup%d map%d next%d success%d reason%s error%s\n",id,f.current_map_id,p.path.next_map_id,p.path.success,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str());if(!p.path.success)break;if(!p.path.next_map_id){reached=f.current_map_id==395;break;}const auto receipt=p.pending.incoming_transition;if(!receipt)break;f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);}
  check(reached,"all four animated pickups have complete remote-floor routes from Zozo entry");
 }
 {auto f=base;f.scenario[149]=0;map_state::Resolver state(f,fixture::records);for(const auto& o:baked::grids(*floor).objects)if(o.type==baked::ObjType::AnimEntity&&o.entity_id()>=21&&o.entity_id()<=24)check(!state.active(*floor,o),"pickups cannot appear before their native149 enabling scene");}
 {auto f=base;f.current_map_id=611;check(direct(f,entry(611),14).path.success,"the earlier message-only lamp painting remains physically approachable");const auto* room=baked::submap_by_map_id(611);bool edge=false;for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::Warp&&o.entity_id()==14)edge=true;check(!edge,"the earlier lamp painting is not an invented later-mansion entrance");}
 check(fixture::wrong_owner==0,"all route work uses owned data");std::printf("Zozo/Jidoor:%d assertions/%d failures\n",cases,errors);return errors||fixture::wrong_owner?1:0;
}
