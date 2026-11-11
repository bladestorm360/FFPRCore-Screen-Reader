// Jachol's real switch is Entity8. Native code moves it below a random skull
// graphic and shows it for three seconds. Its off-map bake home is identity
// metadata only;the existing loaded-object route snapshot owns its actual pose.
#include "jachol_model.h"
#include "../../core/modtext.h"
#include <cmath>
namespace ff5::jachol {
bool supported(int map){return map==284||map==285;}
library::Plan plan(const Snapshot& s,const json::Value& root){
 modtext::LocaleScope locale(s.locale);library::Plan out;out.map=s.map;out.supported=supported(s.map);
 if(!out.supported||(s.cancelled&&s.cancelled()))return out;
 const auto* guide=root.get(L"jachol_guide");const auto* targets=guide?guide->get(L"targets"):nullptr;
 const auto unknown=[&](){out.unknown=true;out.targets.clear();out.floor_phase=-1;out.finish_map=0;out.note=T(L"Story guidance is waiting for current game state.");};
 if(!targets||targets->obj.size()!=10){unknown();return out;}
 const auto add=[&](const wchar_t* key){
  if(out.unknown)return;
  const auto* row=targets->get(key);if(!row||out.targets.size()>=2){unknown();return;}
  const auto number=[&](const wchar_t* k,int lo,int hi,bool optional=false){const auto* v=row->get(k);return !v?optional:v->type==json::Value::Type::Number&&std::isfinite(v->n)&&v->n>=lo&&v->n<=hi&&v->n==int(v->n);};
  if(!number(L"map",284,285)||!number(L"entity",0,32767,true)||!number(L"layer",0,2)||!number(L"x",-32768,32767)||!number(L"y",-32768,32767)){unknown();return;}
  const auto n=[&](const wchar_t* k,int fallback=0){const auto* v=row->get(k);return v?int(v->n):fallback;};
  const auto text=[&](const wchar_t* k){const auto* v=row->get(k);return v?v->s:std::wstring();};
  field::baked_nav::GuideTarget t;t.goal.map_id=n(L"map");t.goal.entity_id=n(L"entity",-1);t.goal.raw_name=text(L"raw");t.goal.cell={n(L"layer"),n(L"x"),n(L"y")};
  const auto* sm=field::baked::submap_by_map_id(t.goal.map_id);if(!sm||text(L"text").empty()||text(L"text").size()>256){unknown();return;}
  if(t.goal.entity_id>=0){int count=0;for(const auto& o:field::baked::grids(*sm).objects)if((o.type==field::baked::ObjType::Entity||o.type==field::baked::ObjType::TreasureBox)&&o.entity_id()==t.goal.entity_id&&field::baked::object_identifier(o)==t.goal.raw_name&&field::baked::str(o.group)==text(L"group")&&o.layer==t.goal.cell.layer&&o.tx==t.goal.cell.x&&o.ty==t.goal.cell.y)++count;if(count!=1){unknown();return;}}
  else if(!field::baked::walkable(field::baked::grids(*sm),t.goal.cell.layer,t.goal.cell.x,t.goal.cell.y,true)){unknown();return;}
  t.kind=t.goal.entity_id<0?field::TargetKind::Spot:field::TargetKind::Interact;t.goal.avoid_other_events=true;t.goal.allow_scripted_crossings=true;t.instruction=T(text(L"text"));out.targets.push_back(std::move(t));
 };
 const auto bit=[&](int id){return s.flags.scenario[size_t(id)];};
 if(s.map==285){
  if(!bit(435)||!s.flags.treasure[105]||!s.flags.treasure[106]){unknown();return out;}
  if(bit(435)==0)add(L"treasure_a");else if(s.flags.treasure[105]==0)add(L"treasure_b");else if(s.flags.treasure[106]==0)add(L"treasure_c");
  else{if(!bit(125)){unknown();return out;}add(L"climb");out.finish_map=bit(125)==1?173:1;}
  return out;
 }
 if(!bit(407)||!bit(408)){unknown();return out;}
 if(bit(407)==0){
  if(bit(408)==0){add(L"start_left");add(L"start_right");}
  else if(!s.chest_door_hidden)unknown();else add(*s.chest_door_hidden?L"second_floor":L"chest_door");
  return out;
 }
 // Stage on a real invariant walking cell before waiting. The native phase
 // callback later supplies a short fresh route to the actual moved Entity8.
 const auto* hold=targets->get(L"staging");const auto* columns=guide->get(L"skull_columns");
 const auto coordinate=[](const json::Value* v){return v&&v->type==json::Value::Type::Number&&std::isfinite(v->n)&&v->n>=0&&v->n<=32767&&v->n==int(v->n);};
 if(!hold||!columns||columns->arr.size()!=8||!coordinate(hold->get(L"x"))||!coordinate(hold->get(L"y"))){unknown();return out;}
 const field::baked_route::Cell center{1,int(hold->get(L"x")->n),int(hold->get(L"y")->n)};
 if(s.player.layer!=center.layer||std::abs(s.player.y-center.y)>1||std::abs(s.player.x-center.x)>8){add(L"staging");return out;}
 if(!s.skull_hidden){unknown();return out;}
 if(*s.skull_hidden||(!s.fresh&&!s.timers_frozen)){
  out.note=T(s.timers_frozen?L"Resume timers until the skull switch appears.":L"Wait for the skull switch to appear.");return out;
 }
 const auto selection=s.flags.area[1];
 if(!selection||*selection<1||*selection>8){unknown();return out;}
 const auto& column=columns->arr[size_t(*selection-1)];if(column.type!=json::Value::Type::Number||!std::isfinite(column.n)||column.n!=int(column.n)||column.n<0||column.n>32767){unknown();return out;}
 if(!s.timers_frozen&&std::abs(s.player.x-int(column.n))>6){
  out.note=T(s.timers_frozen?L"Resume timers until the skull switch appears.":L"Wait for the skull switch to appear.");return out;
 }
 add(L"skull");if(!out.unknown){out.floor_phase=1;out.phase_map=284;}return out;
}
}
