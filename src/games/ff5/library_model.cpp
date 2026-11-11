#include "library_model.h"
#include "../../core/field/entity_translator.h"
#include "../../core/modtext.h"

namespace ff5::library {
namespace {
using namespace field;
constexpr Control upper[]={{9,L"sc_e_0220:本棚が動く"},{10,L"sc_e_0220:本棚が動く"},
 {14,L"sc_e_0220:本棚が動く"},{15,L"sc_e_0220:本棚が動く"},{16,L"sc_e_0220:本棚が動く"},
 {17,L"sc_e_0220:本棚が動く"},{21,L"sc_e_0220:本棚が動く"},{27,L"sc_e_0220:本棚が動く"}};
constexpr Control lower[]={{13,L"sc_e_0223:本棚が動く"},{14,L"sc_e_0431:バトル後、通路が出現"},
 {15,L"sc_e_0223:本棚が動く"},{16,L"sc_e_0223:本棚が動く"},{21,L"開いた本"}};
bool integer(const json::Value& v,int lo,int hi){return v.type==json::Value::Type::Number&&v.n>=lo&&v.n<=hi&&v.n==int(v.n);}
std::wstring instruction(int map,int id,bool examine,bool waypoint,bool portal) {
 if(portal)return T(L"Continue through the passage.");
 if(waypoint)return T(L"Move off this spot, then repeat the guide.");
 if((map==279&&id==4)||(map==281&&id==14))return T(L"Examine the book and win the battle.");
 if(map==283&&id==7)return T(L"Follow the route and win the battle.");
 if(map==283&&id==1)return TF(L"Talk to {0}.",{translate_entity_name_snapshot(L"ミド")});
 if(map==278&&id==17)return T(L"Approach the blocked bookcase to clear the passage.");
 return T(examine?L"Examine the bookcase to move the shelves.":L"Follow the route to move the shelves, then wait.");
}
}
std::span<const Control> controls(int map){return map==275?std::span<const Control>(upper):map==281?std::span<const Control>(lower):std::span<const Control>{};}
bool supported(int map){return map>=272&&map<=283;}
Plan plan(const Snapshot& s,const json::Value& puzzles) {
 modtext::LocaleScope locale(s.locale);Plan out;out.map=s.map;out.supported=supported(s.map);
 if(!out.supported)return out;
 const auto loading=[&]{out.unknown=true;out.targets.clear();out.note=T(L"Navigation data is loading");return out;};
 // The actual final receipt is Mid50 AND the native return to exterior272.
 // Victory49 alone never finishes this guide, including resumed visits.
 if(s.map==272&&s.values[2]==1){out.finished=true;out.note=T(L"Puzzle guide finished");return out;}
 for(const auto& value:s.values)if(!value||(*value!=0&&*value!=1))return loading();
 if(s.cancelled&&s.cancelled())return out;
 if(s.map<=274) {
  const int destination=s.values[2]==1?272:275;
  const auto* map=baked::submap_by_map_id(destination);if(!map)return loading();
  const baked::Object* entry=nullptr;
  for(const auto& object:baked::grids(*map).objects)if(object.type==baked::ObjType::PointIn&&object.a==1){if(entry)return loading();entry=&object;}
  if(!entry)return loading();
  baked_nav::GuideTarget target;target.goal.map_id=destination;target.goal.cell={entry->layer,entry->tx,entry->ty};
  target.goal.avoid_other_events=true;target.kind=TargetKind::Spot;target.instruction=T(L"Continue through the passage.");
  out.targets.push_back(std::move(target));return out;
 }
 int bits=0,hidden=0;
 for(size_t i=0;i<flags.size();++i)if(s.values[i]==1)bits|=1<<i;
 const auto observed=controls(s.map);
 for(size_t i=0;i<observed.size();++i){if(!s.hidden[i])return loading();if(*s.hidden[i])hidden|=1<<i;}
 const auto* resource=puzzles.get(L"library_policy");if(!resource)return loading();
 const auto* version=resource->get(L"version");const auto* rows=resource->get(L"rows");
 if(!version||!integer(*version,1,1)||!rows||rows->type!=json::Value::Type::Array||rows->arr.empty()||rows->arr.size()>512)return loading();
 const auto key=std::to_wstring(s.map)+L":"+std::to_wstring(bits)+L":"+std::to_wstring(hidden);
 const json::Value* selected=nullptr;
 for(const auto& row:rows->arr) {
  if(s.cancelled&&s.cancelled())return out;
  const auto* candidate=row.get(L"key");if(!candidate||candidate->s!=key)continue;
  if(selected)return loading();selected=&row;
 }
 const auto* targets=selected?selected->get(L"targets"):nullptr;
 if(!targets||targets->type!=json::Value::Type::Array||targets->arr.empty()||targets->arr.size()>32)return loading();
 const auto* map=baked::submap_by_map_id(s.map);if(!map||map->world)return loading();const auto& grid=baked::grids(*map);
 bool on_contact=false;
 for(const auto& item:grid.objects)if(item.layer==s.player.layer&&item.contains(s.player.x,s.player.y)&&
    (item.type==baked::ObjType::GotoMap||baked_route::step_event(item)))on_contact=true;
 for(const auto& row:targets->arr) {
  if(row.type!=json::Value::Type::Array||row.arr.size()!=5||!integer(row.arr[0],0,4096)||!integer(row.arr[1],0,2)||
     !integer(row.arr[2],0,grid.width-1)||!integer(row.arr[3],0,grid.height-1)||!integer(row.arr[4],0,1))return loading();
  const int id=int(row.arr[0].n);const baked_route::Cell cell{int(row.arr[1].n),int(row.arr[2].n),int(row.arr[3].n)};
  const bool examine=row.arr[4].n==1;
  const baked::Object* object=nullptr;
  for(const auto& item:grid.objects)if(item.entity_id()==id&&item.layer==cell.layer&&item.tx==cell.x&&item.ty==cell.y&&
      (item.type==baked::ObjType::Event||item.type==baked::ObjType::Entity||item.type==baked::ObjType::NPC||item.type==baked::ObjType::GotoMap)){object=&item;break;}
  if(!object&&id!=0)return loading();
  bool portal=object&&object->type==baked::ObjType::GotoMap;
  if(object&&!portal)for(const auto& item:grid.objects)if(item.type==baked::ObjType::Warp&&item.entity_id()==id&&
      item.layer==cell.layer&&item.contains(cell.x,cell.y)&&supported(item.a)){portal=true;break;}
  const bool automatic=object&&(portal||baked_route::step_event(*object));
  // Native contact controls/doors do not fire merely because the player is
  // already standing there. The policy includes a safe step-off alternative.
  // Step-off coordinates are navigation scaffolding only while occupying a
  // contact point. Offering them again on ordinary floor could bounce between
  // two neighbours without ever operating the next real control.
  if((!object&&(!on_contact||s.player==cell))||(automatic&&s.player.layer==cell.layer&&object->contains(s.player.x,s.player.y)))continue;
  baked_nav::GuideTarget target;target.goal.map_id=s.map;target.goal.cell=cell;target.goal.examine=examine;
  target.goal.avoid_other_events=true;target.kind=examine?TargetKind::Interact:TargetKind::Spot;
  if(object){target.goal.entity_id=id;target.goal.raw_name=baked::object_identifier(*object);}
  target.instruction=instruction(s.map,id,examine,!object,portal);out.targets.push_back(std::move(target));
 }
 return out.targets.empty()?loading():out;
}
}
