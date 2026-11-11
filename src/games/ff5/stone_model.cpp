// Ghido's one stone moves between five chests and the player's hands. The
// center flag is inverted. Read current native receipts, including the two
// return passages' resets, rather than remembering a presumed step number.
#include "stone_model.h"
#include "../../core/modtext.h"
namespace ff5::stone {
namespace {
struct Identity {int map,id;const wchar_t* raw;};
constexpr Identity chests[]={{371,30,L"sc_e_0252:左上"},{371,29,L"sc_e_0253:右上"},{371,28,L"sc_e_0252:中央"},{371,31,L"sc_e_0252:左下"},{371,32,L"sc_e_0252:右下"}};
bool integer(const json::Value* v,int lo,int hi){return v&&v->type==json::Value::Type::Number&&v->n>=lo&&v->n<=hi&&v->n==int(v->n);}
bool target(const json::Value* row,const Identity& expected,field::baked::ObjType type,field::baked_nav::GuideTarget& out){
 if(!row||!integer(row->get(L"map"),expected.map,expected.map)||!integer(row->get(L"entity"),expected.id,expected.id))return false;
 const auto* raw=row->get(L"raw");const auto* text=row->get(L"text");const auto* x=row->get(L"x");const auto* y=row->get(L"y");const auto* layer=row->get(L"layer");
 if(!raw||raw->s!=expected.raw||!text||!text->is_string()||text->s.empty()||!integer(x,-32768,32767)||!integer(y,-32768,32767)||!integer(layer,0,2))return false;
 const auto* map=field::baked::submap_by_map_id(expected.map);if(!map||map->world)return false;const field::baked::Object* source=nullptr;
 for(const auto& o:field::baked::grids(*map).objects)if(o.type==type&&o.entity_id()==expected.id&&field::baked::object_identifier(o)==expected.raw&&o.tx==int(x->n)&&o.ty==int(y->n)&&o.layer==int(layer->n)){if(source)return false;source=&o;}
 if(!source)return false;out.goal.map_id=expected.map;out.goal.cell={source->layer,source->tx,source->ty};out.goal.avoid_other_events=true;out.goal.allow_scripted_crossings=true;
 out.goal.examine=type==field::baked::ObjType::Entity;out.kind=out.goal.examine?field::TargetKind::Interact:field::TargetKind::Spot;
 if(out.goal.examine){out.goal.entity_id=expected.id;out.goal.raw_name=expected.raw;}out.instruction=T(text->s);return true;
}
}
bool supported(int map){return map>=370&&map<=373;}
Plan plan(const Snapshot& s,const json::Value& puzzles){
 modtext::LocaleScope locale(s.locale);Plan out;out.map=s.map;out.supported=supported(s.map);if(!out.supported)return out;
 if(s.map==373){out.finished=true;out.note=T(L"Puzzle guide finished");return out;}
 const auto unknown=[&]{out.unknown=true;out.targets.clear();out.note=T(L"Navigation data is loading");return out;};
 if(s.cancelled&&s.cancelled())return out;
 const auto* recipe=puzzles.get(L"stone_guide");if(!recipe)return unknown();
 field::baked_nav::GuideTarget lower,wall,finish;
 if(!target(recipe->get(L"lower_entry"),{372,2,L"B2F"},field::baked::ObjType::PointIn,lower)||
    !target(recipe->get(L"wall_switch"),{372,15,L"sc_e_0255:階段が出現する"},field::baked::ObjType::Entity,wall)||
    !target(recipe->get(L"finish_entry"),{373,2,L"B3F"},field::baked::ObjType::PointIn,finish))return unknown();
 if(s.map==372){if(!s.stairs_hidden)return unknown();out.targets.push_back(*s.stairs_hidden?std::move(finish):std::move(wall));return out;}
 for(const auto& v:s.values)if(!v||(*v!=0&&*v!=1))return unknown();
 const auto* rows=recipe->get(L"chests");if(!rows||rows->type!=json::Value::Type::Array||rows->arr.size()!=5)return unknown();
 std::array<field::baked_nav::GuideTarget,5> controls;int occupied=-1,count=*s.values[5];
 for(size_t i=0;i<5;++i){if(!target(&rows->arr[i],chests[i],field::baked::ObjType::Entity,controls[i])||!integer(rows->arr[i].get(L"flag"),flags[i],flags[i])||!integer(rows->arr[i].get(L"occupied_value"),i==2?0:1,i==2?0:1))return unknown();
  if(*s.values[i]==(i==2?0:1)){occupied=int(i);++count;}}
 if(count!=1)return unknown();
 const bool opened=*s.values[6]==1;const int destination=opened?3:0;
 if(opened)out.targets.push_back(lower); // a legal continuation outranks rearrangement
 if(occupied==destination){if(!opened){field::baked_nav::GuideTarget lever;if(!target(recipe->get(L"first_switch"),{370,15,L"sc_e_0254:道がひらく"},field::baked::ObjType::Entity,lever))return unknown();out.targets.push_back(std::move(lever));}return out;}
 if(s.map==370){
  // A return script can move the stone to center/SW. Navigate back first,
  // then read its completed native receipts before choosing a manual chest.
  const auto* room=recipe->get(L"room_return");const auto* text=room?room->get(L"text"):nullptr;
  if(!room||!text||!text->is_string()||text->s.empty()||!integer(room->get(L"map"),371,371)||!integer(room->get(L"layer"),0,0)||!integer(room->get(L"x"),9,9)||!integer(room->get(L"y"),13,13))return unknown();
  field::baked_nav::GuideTarget back;back.goal.map_id=371;back.goal.cell={0,9,13};back.goal.avoid_other_events=back.goal.allow_scripted_crossings=true;back.kind=field::TargetKind::Spot;back.instruction=T(text->s);out.targets.push_back(std::move(back));return out;
 }
 if(*s.values[5]==1){auto into=std::move(controls[size_t(destination)]);const auto* key=recipe->get(opened?L"place_southwest":L"place_northwest");if(!key||!key->is_string()||key->s.empty())return unknown();into.instruction=T(key->s);out.targets.push_back(std::move(into));}
 else if(occupied>=0)out.targets.push_back(std::move(controls[size_t(occupied)]));
 return out.targets.empty()?unknown():out;
}
}
