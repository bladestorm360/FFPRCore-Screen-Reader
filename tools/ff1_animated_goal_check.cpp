// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Every FF1 action-bearing animated identity, real provider and native arrivals.
#define FFPR_KEY_DOOR_FIXTURE_ONLY
#include "key_door_route_check.cpp"
namespace animated {
using namespace field;using namespace baked_nav;
int run(){
 int failures=0,identities=0,local=0,remote=0,retired=0,wrong=0,unused=0,missing=0;std::set<int> dungeons;
 const auto expect=[&](bool okay,const char* why,int map,int id){if(!okay){++failures;std::printf("FAIL animated %s map%d id%d\n",why,map,id);}};
 for(int i=0;i<baked::submap_count();++i){
  const auto* room=baked::submap_at(i);if(room->world||room->first_map_id<1)continue;
  const auto* record=fixture::records.get(baked::name_of(*room));if(!record)continue;
  const auto* area=record->get(L"area_id");if(!area)continue;
  const baked::Submap* root=room;int entry_source=INT_MAX;
  // Native map IDs need not increase from the entrance (Sunken Shrine starts
  // on3F). Use a real incoming transition from outside this area, preferring
  // its world entrance, rather than inventing an arrival on the lowest floor.
  for(int j=0;j<baked::submap_count();++j){
   const auto* other=baked::submap_at(j);if(other->first_map_id<1)continue;
   const auto* rec=fixture::records.get(baked::name_of(*other));const auto* a=rec?rec->get(L"area_id"):nullptr;
   if(a&&a->n==area->n)continue;
   for(const auto& edge:edges_of(baked::grids(*other),other->first_map_id)){
    const auto* destination=baked::submap_by_map_id(edge.dest_map_id);if(!destination||destination->world)continue;
    const auto* dr=fixture::records.get(baked::name_of(*destination));const auto* da=dr?dr->get(L"area_id"):nullptr;
    const int rank=other->world?0:other->first_map_id;
    if(da&&da->n==area->n&&rank<entry_source){root=destination;entry_source=rank;}
   }
  }
  for(const auto& object:baked::grids(*room).objects){
   if(object.type!=baked::ObjType::AnimEntity||!(object.d&(4|8)))continue;++identities;dungeons.insert(int(area->n));
   std::set<int> bits;
   const auto collect=[&](const json::Value* code){if(code)for(const auto& op:code->arr){const auto& v=op.arr;if(v.size()>=3&&v[0].n==2&&v[1].n>=0&&v[1].n<4&&v[2].n>=0&&v[2].n<256)bits.insert(int(v[1].n)*256+int(v[2].n));}};
   collect(record->get(L"code"));if(const auto* events=record->get(L"events"))for(const auto& event:events->arr)if(const auto* id=event.get(L"id");id&&int(id->n)==object.entity_id())collect(event.get(L"code"));
   if(bits.size()>10)return 2;const std::vector<int> indexes(bits.begin(),bits.end());bool covered=false,retirement=false;
   for(unsigned pattern=0;pattern<(1u<<indexes.size())&&(!covered||!retirement);++pattern){
    map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=root->first_map_id;flags.current_area_id=int(area->n);
    for(size_t bit=0;bit<indexes.size();++bit)flags.scenario[indexes[bit]]=(pattern>>bit)&1;
    map_state::Resolver state(flags,fixture::records);const bool active=state.active(*room,object)&&!state.consumed(*room,object);
    const auto plan=[&](const baked::Submap& origin,const std::wstring& name){
     // Native Onrac sc_e_0031 enters Sunken Shrine3F at101. Point1 is not
     // the submarine's arrival; the exact transfer is pinned in the census.
     const auto* arrival=point_in(origin,origin.first_map_id==98?101:1,state);if(!arrival)return Planned{};
     auto s=keyed::capture(origin,flags,{arrival->layer,arrival->tx,arrival->ty},object);
     s.goal.map_id=room->first_map_id;s.goal.cell={object.layer,object.tx,object.ty};s.goal.raw_name=name;s.goal.entity_id=object.entity_id();s.goal.examine=true;
     const auto result=plan_snapshot(s);
     return result;
    };
    const auto raw=baked::object_identifier(object);
    if(active&&!covered){
     const auto nearby=plan(*room,raw).path;++local;
     if(!nearby.success)std::printf("Local animated map%d id%d reason%ls error%ls\n",room->first_map_id,object.entity_id(),nearby.reason.c_str(),nearby.error.c_str());
     expect(nearby.success||nearby.reason==L"Story gated","active local destination",room->first_map_id,object.entity_id());
     if(root!=room){
      const auto provider=connected_objects_snapshot(root->first_map_id,state);
      const bool listed=std::any_of(provider.begin(),provider.end(),[&](const auto& row){return row.map_id==room->first_map_id&&row.entity_id==object.entity_id()&&row.raw_name==raw;});
      const auto result=plan(*root,raw).path;++remote;
      std::printf("Animated origin%d target%d:%d flags%u provider%d success%d reason%ls\n",root->first_map_id,room->first_map_id,object.entity_id(),pattern,listed,result.success,result.reason.c_str());
      expect(listed,"actual entrance provider lists interaction",room->first_map_id,object.entity_id());
      expect(result.success||result.reason==L"Story gated","active remote destination",room->first_map_id,object.entity_id());
     }
     const auto mismatch=plan(*room,L"another physical object").path;++wrong;
     expect(!mismatch.success&&mismatch.reason==L"Destination is no longer available","raw identity mismatch stays unavailable",room->first_map_id,object.entity_id());covered=true;
     if(room->first_map_id==98&&object.entity_id()==19){
      const auto* arrival=point_in(*room,101,state);if(!arrival)return 2;
      auto absent=keyed::capture(*room,flags,{arrival->layer,arrival->tx,arrival->ty},object);
      absent.obstacles.erase(std::remove_if(absent.obstacles.begin(),absent.obstacles.end(),[&](const auto& actor){return actor.id==object.entity_id()&&actor.raw_name==raw;}),absent.obstacles.end());
      const auto result=plan_snapshot(absent).path;++missing;
      expect(!result.success&&result.reason==L"Destination is no longer available","missing current typed actor stays unavailable",98,19);
     }
    }
    bool alias_active=false;for(const auto& alias:baked::grids(*room).objects)if(alias.type==object.type&&alias.entity_id()==object.entity_id()&&baked::object_identifier(alias)==raw&&state.active(*room,alias)&&!state.consumed(*room,alias))alias_active=true;
    if(!active&&!alias_active&&!retirement){const auto result=plan(*root,raw).path;++retired;expect(!result.success&&result.reason==L"Destination is no longer available","native retired variant stays unavailable",room->first_map_id,object.entity_id());retirement=true;}
   }
   // One native authored duplicate has the never-selected group ev_e_34;
   // the operative Kraken orb uses ev_e_0034. Inventory it without inventing
   // a reachable state for an initializer which never selects that group.
   if(!covered&&room->first_map_id==96&&object.entity_id()==38&&baked::str(object.group)==L"ev_e_34"){
    bool declared=false;if(const auto* code=record->get(L"code"))for(const auto& op:code->arr)if(op.arr.size()>1&&op.arr[0].n==1&&op.arr[1].s==L"ev_e_34")declared=true;
    expect(!declared,"unused Kraken duplicate must remain unselected",96,38);++unused;
   }else expect(covered,"native active profile found",room->first_map_id,object.entity_id());
  }
 }
 expect(identities==16,"complete16 action-bearing animated variants",0,identities);
 expect(remote>=10,"multifloor dungeon coverage",0,remote);
 std::printf("FF1 animated identities%d unused%d areas%zu local%d remote%d retired%d mismatch%d missing%d failures%d owner%d\n",identities,unused,dungeons.size(),local,remote,retired,wrong,missing,failures,fixture::wrong_owner.load());
 return failures||fixture::wrong_owner?1:0;
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff1/maps.bin");if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF1;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{result=animated::run();});worker.join();return result;}
