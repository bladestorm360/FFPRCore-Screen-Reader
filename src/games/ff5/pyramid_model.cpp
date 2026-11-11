// Current-floor native receipts choose the next real control. Departing a floor
// can clear its puzzle bits,so an old missing bit never sends the player back
// through an already completed floor. All work uses owned data on Navigation.
#include "pyramid_model.h"
#include "../../core/modtext.h"
#include <cmath>
namespace ff5::pyramid {
bool supported(int map){return map>=410&&map<=426;}
library::Plan plan(const Snapshot& s,const json::Value& puzzles){
 modtext::LocaleScope locale(s.locale);library::Plan out;out.map=s.map;out.supported=supported(s.map);
 if(!out.supported||(s.cancelled&&s.cancelled()))return out;
 const auto* guide=puzzles.get(L"pyramid_guide");const auto* targets=guide?guide->get(L"targets"):nullptr;
 if(!targets||targets->obj.size()>48){out.unknown=true;return out;}
 const auto uncertain=[&](){out.unknown=true;out.targets.clear();out.floor_phase=-1;out.note=T(L"Story guidance is waiting for current game state.");};
 const auto bit=[&](int i)->std::optional<int>{return s.flags.scenario[size_t(i)];};
 const auto add=[&](const wchar_t* key){
  if(out.unknown)return;
  const auto* row=targets->get(key);if(!row||out.targets.size()>=4){uncertain();return;}
  const auto integer=[&](const wchar_t* k,int lo,int hi,bool optional=false){const auto* v=row->get(k);return !v?optional:v->type==json::Value::Type::Number&&std::isfinite(v->n)&&v->n>=lo&&v->n<=hi&&v->n==int(v->n);};
  if(!integer(L"map",410,426)||!integer(L"entity",-1,32767,true)||!integer(L"layer",0,2)||!integer(L"x",-32768,32767)||!integer(L"y",-32768,32767)){uncertain();return;}
  const auto num=[&](const wchar_t* k,int fallback=0){const auto* v=row->get(k);return v?int(v->n):fallback;};
  const auto str=[&](const wchar_t* k){const auto* v=row->get(k);return v?v->s:std::wstring();};
  field::baked_nav::GuideTarget t;t.goal.map_id=num(L"map");t.goal.entity_id=num(L"entity",-1);t.goal.raw_name=str(L"raw");t.goal.cell={num(L"layer",1),num(L"x"),num(L"y")};
  const auto* sm=field::baked::submap_by_map_id(t.goal.map_id);if(!sm||sm->world||str(L"text").empty()||str(L"text").size()>256){uncertain();return;}
  if(t.goal.entity_id>=0){int found=0;for(const auto& o:field::baked::grids(*sm).objects){if(o.type!=field::baked::ObjType::Entity&&o.type!=field::baked::ObjType::NPC&&o.type!=field::baked::ObjType::Event)continue;
    if(o.entity_id()==t.goal.entity_id&&field::baked::str(o.group)==str(L"group")&&field::baked::object_identifier(o)==t.goal.raw_name&&o.layer==t.goal.cell.layer&&o.tx==t.goal.cell.x&&o.ty==t.goal.cell.y)++found;}if(found!=1){uncertain();return;}
  }else if(!field::baked::walkable(field::baked::grids(*sm),t.goal.cell.layer,t.goal.cell.x,t.goal.cell.y,true)){uncertain();return;}
  t.goal.avoid_other_events=true;t.goal.allow_scripted_crossings=true;t.kind=t.goal.entity_id<0?field::TargetKind::Spot:str(L"kind")==L"interact"?field::TargetKind::Interact:field::TargetKind::Spot;t.instruction=T(str(L"text"));out.targets.push_back(std::move(t));
 };
 if(!bit(137)){uncertain();return out;}if(bit(137)==1){out.finished=true;return out;}
 if(s.map==410){if(!bit(136)){uncertain();return out;}add(bit(136)==0?L"entrance":L"first_floor");return out;}
 if(s.map>=411&&s.map<=414){
  if(s.map==411){for(int i:{378,379,380,387})if(!bit(i)){uncertain();return out;}
   // The coffin alcove returns through the real middle corridor,not through
   // its coffin or a guessed direct connection to the needle gallery.
   if(s.player.x>=13&&s.player.x<=19&&s.player.y>=14&&s.player.y<=16){add(L"coffin_exit");return out;}
   if(s.player.x>=13&&s.player.y<=20&&bit(387)==0){add(L"quicksand_stop");return out;}
   if(bit(378)==0)add(L"needle_left");else if(bit(379)==1)add(L"needle_middle_off");else if(bit(380)==0)add(L"needle_right");else add(L"snake_room");
  }else if(s.map==412)add(L"return_basement");else if(s.map==413){
   if(!s.corridor_hidden[0]){uncertain();return out;}
   if(!*s.corridor_hidden[0])add(L"corridor_start");else{add(L"return_middle_a");
    for(size_t i=1;i<3;++i){if(!s.corridor_hidden[i]){uncertain();return out;}if(!*s.corridor_hidden[i])add(i==1?L"corridor_enemy_a":L"corridor_enemy_b");}}
  }else add(L"return_coffin");return out;
 }
 if(s.map==415){
  for(int i:{384,381,382,383,385,386})if(!bit(i)){uncertain();return out;}
  if(bit(384)==0){add(L"snake_start");return out;}
  if(bit(382)==0)add(L"snake_middle");else if(bit(381)==0)add(L"snake_left");else if(bit(383)==0)add(L"snake_right");else add(L"third_floor");
  if(bit(385)==0)add(L"snake_enemy_a");if(bit(386)==0)add(L"snake_enemy_b");return out;
 }
 if(s.map==416){add(L"fourth_floor");return out;}
 if(s.map==417){if(!bit(436)||!bit(325)){uncertain();return out;}add(bit(436)==0?L"coffin":bit(325)==0?L"coffin_enemy":L"fifth_floor");return out;}
 if(s.map==419&&((s.player.x>16&&s.player.y<11)||(s.player.x==16&&s.player.y<7))){if(!bit(400)){uncertain();return out;}add(bit(400)==0?L"fifth_door":L"fifth_door_close");return out;}
 if(s.map==418||s.map==419||s.map==420){add(L"sixth_floor");return out;}
 if(s.map==421){
  for(int i:{390,391,392})if(!bit(i)){uncertain();return out;}
  // Returning from the side room resets all three bits on native entry.
  // The first switch is then across a real fall;the second is reachable on
  // this platform and opens the safe onward route without a mandatory fall.
  if(bit(390)==0&&bit(391)==0&&s.player.x>=12&&s.player.y<=12){add(L"sand_second");return out;}
  if(bit(390)==0&&bit(391)==1){
   if(bit(392)==0){if(!s.last_sand_switch_hidden){uncertain();return out;}add(*s.last_sand_switch_hidden?L"seventh_floor":L"sand_third");}
   else add(L"seventh_floor");return out;
  }
  if(bit(390)==0)add(L"sand_first");else if(bit(391)==0)add(L"sand_second");else if(bit(392)==0){
   // Native live operation hides the third fall only after392;the reloaded
   // initializer instead reuses391. The actual hidden control distinguishes
   // those two valid native states without changing the native behavior.
   if(!s.last_sand_switch_hidden){uncertain();return out;}
   add(*s.last_sand_switch_hidden?L"seventh_floor":L"sand_third");
  }else add(L"seventh_floor");return out;
 }
 if(s.map==422){if(!bit(388)){uncertain();return out;}add(bit(388)==0?L"wall_passage":L"return_sixth_east");return out;}
 if(s.map==423){add(L"eighth_floor");return out;}
 if(s.map==424){
  const auto phase=s.flags.area[0];if(!phase||(*phase!=0&&*phase!=1)){uncertain();return out;}
  const bool left=s.player.x<=12;
  // Only the western return room has a no-slide approach to the tablet.
  // A player arriving from its eastern entrance comes back here,then uses
  // the same invariant holds in reverse before crossing to the west.
  if(s.player.y<=10&&s.player.x<=12){add(L"upper_exit_left");return out;}
  if(s.player.y<=10&&s.player.x>12&&(s.player.x!=14||s.player.y!=10)){add(L"safe_north_right");return out;}
  const bool hold=s.player.y<=13&&(s.player.x==11||s.player.x==13);
  const bool right_north=s.player.x==14&&s.player.y==10;
  const int needed=(right_north||(hold&&left))?0:1;
  if(*phase!=needed||(!s.phase_fresh&&!s.timers_frozen)){out.note=T(s.timers_frozen&&*phase!=needed?L"Resume timers until the floor changes.":L"Wait for the floor to change.");return out;}
  add(right_north?L"safe_hold_right":hold?(left?L"safe_north_left":L"safe_hold_left"):(left?L"safe_hold_left":L"safe_hold_right"));out.floor_phase=needed;return out;
 }
 if(s.map==425&&s.player.x>=13){add(L"return_eighth_right");return out;}
 if(s.map==425||s.map==426)add(L"tablet");return out;
}
}
