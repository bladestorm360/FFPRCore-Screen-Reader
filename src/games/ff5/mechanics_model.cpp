// Rules select real controls from current receipts. They never execute native
// scripts,write flags,advance from proximity or retain a step counter on reload.
#include "mechanics_model.h"
#include "../../core/modtext.h"
#include <cmath>
namespace ff5::mechanics {
namespace {
bool integer(const json::Value* v,int lo,int hi){return v&&v->type==json::Value::Type::Number&&std::isfinite(v->n)&&v->n>=lo&&v->n<=hi&&v->n==int(v->n);}
std::wstring str(const json::Value& row,const wchar_t* key){const auto* v=row.get(key);return v&&v->type==json::Value::Type::String?v->s:std::wstring();}
std::optional<bool> here(const json::Value& row,int map,const wchar_t* key=L"maps"){const auto* maps=row.get(key);if(!maps||maps->type!=json::Value::Type::Array||maps->arr.size()>128)return {};for(const auto& v:maps->arr){if(!integer(&v,1,32767))return {};if(v.n==map)return true;}return false;}
std::optional<bool> matches(const json::Value* rows,const Snapshot& s){
 if(!rows)return true;if(rows->type!=json::Value::Type::Array||rows->arr.size()>32)return {};
 for(const auto& row:rows->arr){if(row.arr.size()!=4||!integer(&row.arr[1],0,1023)||!integer(&row.arr[3],-32768,32767))return {};
  const auto& bank=row.arr[0].s;const size_t i=size_t(row.arr[1].n);std::optional<int> value;
  if(bank==L"scenario")value=s.flags.scenario[i];else if(bank==L"treasure")value=s.flags.treasure[i];
  else if(bank==L"global"&&i<s.flags.global.size())value=s.flags.global[i];
  else if(bank==L"area"&&i<s.flags.area.size())value=s.flags.area[i];
  else if(bank==L"local"&&i<s.flags.local.size()&&s.flags.current_map_id==s.map)value=s.flags.local[i];
  if(!value)return {};const int want=int(row.arr[3].n);const auto& op=row.arr[2].s;
  if(op!=L"eq"&&op!=L"ne"&&op!=L"ge"&&op!=L"lt")return {};
  if(!(op==L"eq"?*value==want:op==L"ne"?*value!=want:op==L"ge"?*value>=want:*value<want))return false;
 }return true;
}
std::optional<field::baked_nav::GuideTarget> target(const json::Value& row){
 if(!integer(row.get(L"map"),1,32767)||!integer(row.get(L"layer"),0,2)||!integer(row.get(L"x"),-32768,32767)||!integer(row.get(L"y"),-32768,32767)||!integer(row.get(L"entity"),0,32767))return {};
 const auto n=[&](const wchar_t* key){return int(row.get(key)->n);};field::baked_nav::GuideTarget t;
 t.goal.map_id=n(L"map");t.goal.entity_id=n(L"entity");t.goal.raw_name=str(row,L"raw");t.goal.cell={n(L"layer"),n(L"x"),n(L"y")};
 const auto* map=field::baked::submap_by_map_id(t.goal.map_id);if(!map||map->world)return {};
 int found=0;for(const auto& o:field::baked::grids(*map).objects){
  if(o.type!=field::baked::ObjType::Entity&&o.type!=field::baked::ObjType::NPC&&o.type!=field::baked::ObjType::ShopNPC&&o.type!=field::baked::ObjType::Event&&o.type!=field::baked::ObjType::TreasureBox&&o.type!=field::baked::ObjType::GotoMap)continue;
  if(o.entity_id()==t.goal.entity_id&&field::baked::object_identifier(o)==t.goal.raw_name&&field::baked::str(o.group)==str(row,L"group")&&o.layer==t.goal.cell.layer&&o.tx==t.goal.cell.x&&o.ty==t.goal.cell.y)++found;
 }
 const auto text=str(row,L"text");if(found!=1||text.empty()||text.size()>512)return {};
 t.goal.avoid_other_events=true;t.goal.allow_scripted_crossings=true;const auto mode=str(row,L"kind");
 if(mode!=L"spot"&&mode!=L"interact"&&mode!=L"transition")return {};
 t.kind=mode==L"spot"?field::TargetKind::Spot:mode==L"transition"?field::TargetKind::Transition:field::TargetKind::Interact;t.instruction=T(text);return t;
}
library::Plan exdeath(const Snapshot& snapshot, const json::Value& root) {
 library::Plan out;
 out.map = snapshot.map;
 out.supported = true;
 out.guide_id = L"exdeath_castle_crossing";
 out.observe_waypoint_arrival = false;
 const auto unknown = [&] {
  out.unknown = true;
  out.note = T(L"Story guidance is waiting for current game state.");
  return out;
 };
 if (!snapshot.policy.navigation) {
  out.note = T(L"Navigation puzzle guidance is off");
  return out;
 }
 if (snapshot.flags.current_map_id != snapshot.map || snapshot.flags.current_area_id != 40) return unknown();

 const auto& player = snapshot.player;
 const wchar_t* step = nullptr;
 const wchar_t* alternative = nullptr;
 if (snapshot.map == 397) {
  const auto layout = snapshot.flags.area[0];
  if (!layout || *layout < 0 || *layout > 9 || player.layer != 2) return unknown();
  const bool north_corridor = player.x == 6 && player.y >= 2 && player.y <= 6;
  if (north_corridor || (*layout >= 4 && *layout <= 6)) {
   step = L"north";
  } else if (player.y == 6 && player.x >= 5 && player.x <= 7) {
   // The initializer exposes the north return contact in these layouts.
   // It restores layout five before an onward strict route can cross it.
   step = L"north_reset";
  } else if (player == field::baked_route::Cell{2,6,10}) {
   if (*layout == 0 && !snapshot.exdeath_stop_visible) return unknown();
   if (*layout == 0 && *snapshot.exdeath_stop_visible) {
    step = L"stop";
   } else {
    out.puzzle_types = 1;
    out.note = T(L"Step South once, then North once to restart the floor.");
    return out;
   }
  } else {
   // The native stopped layout is authoritative. Every unsuccessful layout
   // has a real return to the center, including after an encounter.
   step = *layout == 0 ? L"start" : L"retry";
  }
 } else {
  const auto bridge = snapshot.flags.area[1];
  if (!bridge || (*bridge != 0 && *bridge != 1)) return unknown();
  const bool in_exit_corridor = snapshot.map == 401 && player.x == 18 && player.y >= 11;
  const bool on_east_side = snapshot.map == 401 && (player.x > 18 || (player.x == 18 && player.y == 10));
  // Three isolated landing components share this map ID. Their native
  // stairs return to the lower floor, where the ordinary planner resumes.
  if (snapshot.map == 401 && player.x <= 8) {
   step = L"west_return";
   alternative = L"west_return_alt";
  } else if (snapshot.map == 401 && player.x >= 10 && player.x <= 12) {
   step = L"middle_return";
  } else if (snapshot.map == 401 && player.x == 26 && player.y <= 7) {
   step = L"east_return";
  } else if (in_exit_corridor || (*bridge == 0 && on_east_side)) {
   step = L"exit";
   out.finish_map = 402;
  } else {
   step = *bridge == 1 ? L"restore" : L"collapse";
  }
 }

 const auto* guides = root.get(L"story_guides");
 if (!guides || guides->arr.size() > 16) return unknown();
 for (const auto& guide : guides->arr) {
  if (str(guide,L"id") != out.guide_id) continue;
  const auto* targets = guide.get(L"targets");
  const auto append = [&](const wchar_t* key) {
   const auto* row = targets ? targets->get(key) : nullptr;
   auto selected = row ? target(*row) : std::nullopt;
   if (!selected) return false;
   out.targets.push_back(std::move(*selected));
   return true;
  };
  if (!append(step) || (alternative && !append(alternative))) {
   out.targets.clear();
   return unknown();
  }
  out.puzzle_types = 1;
  return out;
 }
 return unknown();
}

library::Plan fork(const Snapshot& s,const json::Value& root){
 library::Plan out;out.map=s.map;out.supported=true;out.guide_id=L"fork_tower";out.observe_waypoint_arrival=false;
 const auto unknown=[&](){out.unknown=true;out.targets.clear();out.note=T(L"Story guidance is waiting for current game state.");return out;};
 const auto bit=[&](int i)->std::optional<int>{auto v=s.flags.scenario[size_t(i)];return v&&(*v==0||*v==1)?v:std::nullopt;};
 const auto open=bit(142),split=bit(143),swap=bit(144),done=bit(145);if(!done)return unknown();if(*done){out.finished=true;return out;}
 if(!s.policy.any()){out.note=T(L"Puzzle guidance is off");return out;}
 if(!open||!split||!swap)return unknown();if(!*open){out.note=T(L"Story gated");return out;}
 const auto* guides=root.get(L"story_guides");if(!guides||guides->arr.size()>16)return unknown();const json::Value* guide=nullptr;
 for(const auto& row:guides->arr)if(str(row,L"id")==out.guide_id){if(guide)return unknown();guide=&row;}
 const auto* data=guide?guide->get(L"targets"):nullptr;if(!data)return unknown();const wchar_t* key=nullptr;const wchar_t* advice=nullptr;
 if(s.map==443){
  if(!*split){key=L"start";advice=L"Prepare magic users for the left tower and physical attackers for the right tower.";}
  else{
   int assigned=0,left=0,right=0;bool gap=false;
   for(int i=0;i<4;++i){const auto a=bit(228+3*i),l=bit(229+3*i),r=bit(230+3*i);if(!a||!l||!r||*l+*r!=*a||(*a&&gap))return unknown();if(*a){++assigned;left+=*l;right+=*r;}else gap=true;}
   if(assigned==4||*swap)return unknown();
   // Suggest two on each side, but preserve any legal player-selected prefix.
   // The final choice must leave both sides nonempty, as the native script does.
   const bool use_left=assigned==3?(left==0||(right!=0&&left<2)):left<2;
   key=use_left?L"left":L"right";advice=use_left?L"Before crossing, prepare this character to use magic.":L"Before crossing, prepare this character for physical attacks.";
  }
 }else if(s.map==452){if(!*split||!*swap)return unknown();key=L"black";advice=L"Take the spell immediately, then win the battle with magic.";out.finish_map=19;}
 else if(s.map==444||s.map==648||s.map==649||(s.map>=445&&s.map<=451)){
  if(!*split||*swap)return unknown();key=L"swap";advice=L"Use magic in the left tower.";
 }else{if(!*split||!*swap)return unknown();key=L"white";advice=L"Win this battle with physical attacks. Do not use magic.";}
 if(s.policy.other){out.puzzle_types=2;out.note=T(advice);}
 if(!s.policy.navigation)return out;
 const auto add=[&](const wchar_t* name){const auto* row=data->get(name);auto t=row?target(*row):std::nullopt;if(!t)return false;
  if(s.policy.other)t->instruction=T(advice)+L" "+t->instruction;out.targets.push_back(std::move(*t));return true;};
 if(!add(key))return unknown();if(std::wstring_view(key)==L"swap"&&(!add(L"swap648")||!add(L"swap649")))return unknown();
 out.puzzle_types|=1;out.note.clear();return out;
}
library::Plan desert(const Snapshot& s,const json::Value& root){
 library::Plan out;out.map=s.map;out.supported=true;out.observe_waypoint_arrival=false;
 const auto unknown=[&](){out.unknown=true;out.targets.clear();out.note=T(L"Story guidance is waiting for current game state.");return out;};
 const auto done=s.flags.scenario[59];if(!done||(*done!=0&&*done!=1))return unknown();
 const auto* data=root.get(L"desert_guide");if(!data)return unknown();
 out.guide_id=*done?L"desert_crossing":s.policy.other?L"desert_sandworm":L"desert_crossing";
 if(*done&&!s.policy.navigation){
  if(s.guide_id==L"desert_sandworm"){out.finished=true;out.puzzle_types=2;}
  else out.note=T(L"Navigation puzzle guidance is off");return out;
 }
 if(!*done&&!s.policy.any()){out.note=T(L"Puzzle guidance is off");return out;}
 const auto add=[&](field::baked_nav::GuideTarget t){t.goal.avoid_other_events=true;t.goal.allow_scripted_crossings=false;out.targets.push_back(std::move(t));return out;};
 const auto fixed=[&](const wchar_t* key)->std::optional<field::baked_nav::GuideTarget>{const auto* row=data->get(key);return row?target(*row):std::nullopt;};
 if(!*done){
  const auto prerequisite=s.flags.scenario[58],intro=s.flags.scenario[287],later=s.flags.scenario[63];if(!prerequisite||!intro||!later)return unknown();
  if(*prerequisite==0){
   out.guide_id=L"desert_crossing";if(!s.policy.navigation){out.note=T(L"Story gated");return out;}
   auto exit=fixed(L"north");if(!exit)return unknown();exit->instruction=T(L"Story gated")+L". "+exit->instruction;out.puzzle_types=1;return add(std::move(*exit));
  }
  if(*prerequisite!=1||(*intro!=0&&*intro!=1)||*later!=0)return unknown();
  out.puzzle_types=s.policy.other?2:1;
  if(!*intro){
   field::baked_nav::GuideTarget t;t.goal.map_id=286;t.goal.entity_id=-1;t.goal.cell={2,53,13};t.kind=field::TargetKind::Spot;
   t.instruction=T(L"Step South, then request guidance again.");
   if(s.policy.other)t.instruction+=L" "+T(L"Choose the first option, then win the battle.");return add(std::move(t));
  }
  auto retry=fixed(L"retry");if(!retry)return unknown();
  // The exact initializer moves this native NPC from its off-map home after
  // intro287;the complete callback59 retires it. No new native read is needed.
  retry->goal.cell={2,55,13};retry->instruction=T(s.policy.other?L"Choose the first option, then win the battle.":L"Interact, then request guidance again.");return add(std::move(*retry));
 }
 const auto later=s.flags.scenario[63];if(!later||(*later!=0&&*later!=1)||s.player.layer!=2||s.player.x<0||s.player.x>=64||s.player.y<0||s.player.y>=64)return unknown();
 const auto* rows=data->get(L"rows");if(!rows||rows->arr.size()!=2||rows->arr[*later].s.size()!=8192)return unknown();
 constexpr std::wstring_view alphabet=L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 const auto& row=rows->arr[*later].s;const int at=(s.player.y*64+s.player.x)*2;const auto hi=alphabet.find(row[at]),lo=alphabet.find(row[at+1]);if(hi==alphabet.npos||lo==alphabet.npos)return unknown();const int goal=int(hi*64+lo);
 out.puzzle_types=1;
 if(goal==58*64+49){auto exit=fixed(L"south");if(!exit)return unknown();return add(std::move(*exit));}
 field::baked_nav::GuideTarget t;t.goal.map_id=286;t.goal.entity_id=-1;t.goal.cell={2,goal%64,goal/64};t.kind=field::TargetKind::Spot;t.instruction=T(L"Let the current carry you, then request guidance again.");
 const auto* currents=data->get(L"currents");if(!currents||currents->arr.size()!=17)return unknown();
 for(const auto& r:currents->arr)if(r.get(L"x")&&r.get(L"y")&&r.get(L"x")->n==t.goal.cell.x&&r.get(L"y")->n==t.goal.cell.y){auto real=target(r);if(!real)return unknown();t.goal=real->goal;t.kind=field::TargetKind::Spot;break;}
 const auto* map=field::baked::submap_by_map_id(286);if(!map||!field::baked::current_bits(field::baked::grids(*map),2,t.goal.cell.x,t.goal.cell.y))return unknown();return add(std::move(t));
}
library::Plan ship(const Snapshot& s,const json::Value& root){
 library::Plan out;out.map=s.map;out.supported=true;out.guide_id=L"fire_ship_passages";out.observe_waypoint_arrival=false;
 const auto unknown=[&](){out.unknown=true;out.targets.clear();out.note=T(L"Story guidance is waiting for current game state.");return out;};
 if(!s.policy.navigation){out.note=T(L"Navigation puzzle guidance is off");return out;}
 const auto* guide=root.get(L"fire_ship_guide");if(!guide)return unknown();
 int code=0;
 if(s.map==269){if(s.guide_id!=out.guide_id)return unknown();code=35;}
 else{
  if(s.player.layer!=1||s.player.x<0||s.player.x>=25||s.player.y<0||s.player.y>=27)return unknown();int mask=0;
  for(int i=0;i<7;++i){const auto v=s.flags.scenario[301+i];if(!v||(*v!=0&&*v!=1))return unknown();mask|=*v<<i;}
  const auto* rows=guide->get(L"rows");if(!rows||rows->arr.size()!=128)return unknown();const auto& row=rows->arr[mask];if(row.type!=json::Value::Type::String||row.s.size()!=675)return unknown();
  const std::wstring_view alphabet=L".ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgh";const auto found=alphabet.find(row.s[s.player.y*25+s.player.x]);if(found==alphabet.npos||found==0)return unknown();code=int(found);
 }
 const json::Value* row=nullptr;int direction=-1;
 if(code<=32){const auto* controls=guide->get(L"controls");if(!controls||controls->arr.size()!=8)return unknown();row=&controls->arr[(code-1)/4];direction=(code-1)%4;}
 else row=guide->get(code==33?L"exit":code==34?L"reset_exit":L"return_exit");
 if(!row||!integer(row->get(L"entity"),1,32767)||!integer(row->get(L"layer"),0,2)||!integer(row->get(L"x"),0,255)||!integer(row->get(L"y"),0,255))return unknown();
 field::baked_nav::GuideTarget t;t.goal.map_id=s.map;t.goal.entity_id=int(row->get(L"entity")->n);t.goal.cell={int(row->get(L"layer")->n),int(row->get(L"x")->n),int(row->get(L"y")->n)};t.goal.raw_name=str(*row,L"raw");
 const auto* map=field::baked::submap_by_map_id(s.map);if(!map||map->world)return unknown();int matches=0;
 for(const auto& o:field::baked::grids(*map).objects)if(o.entity_id()==t.goal.entity_id&&field::baked::object_identifier(o)==t.goal.raw_name&&o.layer==t.goal.cell.layer&&o.tx==t.goal.cell.x&&o.ty==t.goal.cell.y&&o.type==(direction<0?field::baked::ObjType::GotoMap:field::baked::ObjType::Entity))++matches;
 if(matches!=1)return unknown();t.goal.avoid_other_events=true;t.goal.allow_scripted_crossings=true;
 if(direction>=0){
  constexpr int dx[]{0,1,0,-1},dy[]{-1,0,1,0};constexpr const wchar_t* names[]{L"North",L"East",L"South",L"West"};
  t.goal.cell.x-=dx[direction];t.goal.cell.y-=dy[direction];t.goal.entity_id=-1;t.goal.raw_name.clear();t.kind=field::TargetKind::Spot;t.instruction=TF(L"Face {0}. Operate the switch.",{T(names[direction])});
 }else{t.kind=field::TargetKind::Transition;t.instruction=T(L"Take this exit, then request guidance again.");if(code==33)out.finish_map=271;}
 out.puzzle_types=1;out.targets.push_back(std::move(t));return out;
}
}
library::Plan plan(const Snapshot& s,const json::Value& root){
 modtext::LocaleScope locale(s.locale);library::Plan out;out.map=s.map;
 if((!supported(s.map)&&!context_types(s.map,s.guide_id))||(s.cancelled&&s.cancelled()))return out;
 if (s.guide_id == L"karnak_escape" && (s.map == 1 || s.map == 581)) {
  // The first world arrival is an intermediate scene. Only the final world
  // callback records 47 after the explosion has recorded 992.
  out.supported = true;
  out.guide_id = s.guide_id;
  if (!s.policy.navigation) {
   out.note = T(L"Navigation puzzle guidance is off");
  } else if (s.map == 1 && s.flags.scenario[47] == 1 && s.flags.scenario[992] == 1) {
   out.finished = true;
  } else {
   out.note = T(L"Story guidance is waiting for current game state.");
  }
  return out;
 }
 if(s.map==286)return desert(s,root);
 if(s.map>=397&&s.map<=401)return exdeath(s,root);
 if(s.map==443||(s.map>=444&&s.map<=461)||(s.map>=648&&s.map<=651))return fork(s,root);
 if(s.map==270||(s.map==269&&s.guide_id==L"fire_ship_passages"))return ship(s,root);
 const auto unknown=[&](){out.supported=true;out.unknown=true;out.targets.clear();out.puzzle_types=0;out.note=T(L"Story guidance is waiting for current game state.");};
 const auto* guides=root.get(L"story_guides");if(!guides||guides->type!=json::Value::Type::Array||guides->arr.size()>16){unknown();return out;}
 for(const auto& guide:guides->arr){
  const auto present=here(guide,s.map);if(!present){unknown();return out;}
  const auto id=str(guide,L"id");if(!*present&&(id!=s.guide_id||!context_types(s.map,id)))continue;
  const auto selected=matches(guide.get(L"select_when"),s);if(!selected){unknown();return out;}if(!*selected)continue;
  out.supported=true;out.guide_id=id;const auto kind=field::puzzle_policy::kind(root,id);
  if(id.empty()||kind==field::puzzle_policy::Kind::Unknown){unknown();return out;}
  if(!field::puzzle_policy::any_enabled(root,id,s.policy)){out.note=T(field::puzzle_policy::disabled_key(kind));return out;}
  if(const auto* end=guide.get(L"complete_scenario")){
   if(!integer(end,0,1023)){unknown();return out;}const auto done=s.flags.scenario[size_t(end->n)];if(!done){unknown();return out;}if(*done==1){
    // A native reward can precede a required safe return (Gogo's timed tower).
    // Entry notices still use the reward receipt;an already-owned guide may
    // finish its exact exit journey. No target arrival fabricates completion.
    if(const auto* row=guide.get(L"return_target")){
     auto exit=target(*row);const auto* finish=guide.get(L"return_map");
     if(!exit||!integer(finish,1,32767)||!field::baked::submap_by_map_id(int(finish->n))){unknown();return out;}
     const auto purpose=field::puzzle_policy::kind(root,id,str(*row,L"text"));
     if(!s.policy.allows(purpose)){out.note=T(field::puzzle_policy::disabled_key(purpose));return out;}
     out.puzzle_types=field::puzzle_policy::mask(purpose);out.targets.push_back(std::move(*exit));out.finish_map=int(finish->n);return out;
    }
    out.finished=true;return out;
   }
  }
  const auto eligible=matches(guide.get(L"when"),s);if(!eligible){unknown();return out;}if(!*eligible){out.supported=false;continue;}
  // Istory's sheep landing is a separate native terrain component. The
  // authored upper exit is a recovery operation,not a wall-opening shortcut.
  // One cancellable indoor flood on Navigation distinguishes the components;
  // the ordinary planner still validates the actual exit/live obstacles.
  if(*present)if(const auto* recovery=guide.get(L"recovery")){
   const auto* a=recovery->get(L"anchor");const auto* t=recovery->get(L"target");
   const auto anchor=a?target(*a):std::optional<field::baked_nav::GuideTarget>{};const auto exit=t?target(*t):std::optional<field::baked_nav::GuideTarget>{};
   const auto* map=field::baked::submap_by_map_id(s.map);
   if(!anchor||!exit||anchor->goal.map_id!=s.map||exit->goal.map_id!=s.map||!map||map->world||size_t(map->width)*map->height>65536){unknown();return out;}
   field::baked_route::Request req;req.submap=map;req.start=s.player;req.passages=true;req.ignore_hazards=true;req.transportation_id=1;req.cancelled=s.cancelled;
   const auto reachable=field::baked_route::flood(req);
   if(s.cancelled&&s.cancelled())return {};
   if(!reachable.reachable(anchor->goal.cell)){
    const auto purpose=field::puzzle_policy::kind(root,id,str(*t,L"text"));
    if(!s.policy.allows(purpose)){out.note=T(field::puzzle_policy::disabled_key(purpose));return out;}
    out.puzzle_types=field::puzzle_policy::mask(purpose);out.targets.push_back(*exit);return out;
   }
  }
  const auto* rules=guide.get(L"rules");if(!rules||rules->type!=json::Value::Type::Array||rules->arr.size()>128){unknown();return out;}
  for(const auto& rule:rules->arr){
   if(s.cancelled&&s.cancelled())return {};const auto match=matches(rule.get(L"when"),s);if(!match){unknown();return out;}if(!*match)continue;
   const auto* targets=rule.get(L"targets");if(!targets||targets->type!=json::Value::Type::Array||targets->arr.empty()||targets->arr.size()>16){unknown();return out;}
   for(const auto& row:targets->arr){
    auto t=target(row);if(!t){unknown();return out;}const auto text=str(row,L"text");
    // The strict indoor puzzle policy deliberately forbids world search.
    // Only this guide's exact authored/owned transit context may use the
    // ordinary world-entry planner. On town arrival strict selection resumes.
    if(!*present){t->goal.avoid_other_events=false;t->goal.allow_world_transit=true;}
    const auto step_kind=field::puzzle_policy::kind(root,id,text);if(!s.policy.allows(step_kind)){out.targets.clear();out.puzzle_types=0;out.note=T(field::puzzle_policy::disabled_key(step_kind));return out;}
    out.puzzle_types|=field::puzzle_policy::mask(step_kind);out.targets.push_back(std::move(*t));
   }return out;
  }
  unknown();return out; // unsatisfied data never silently becomes completion
 }
 out.note=T(L"No puzzle guide for this map");return out;
}
}
