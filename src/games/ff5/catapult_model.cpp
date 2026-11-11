// Five receipt-backed clue actions. Current native visibility also handles a
// re-entry/shortcut that has already retired an earlier clue or exposed a later
// control. No native pointer, VM execution or layout search enters this model.
#include "catapult_model.h"
#include <algorithm>
#include "../../core/modtext.h"
namespace ff5::catapult {
namespace {
constexpr library::Control actors[]={{30,L"sc_e_0224:中央の部屋の右奥の花壇"},
 {31,L"sc_e_0224:右の部屋の便箋"},{22,L"sc_e_0224:左の部屋の左のツボ"},
 {23,L"sc_e_0224:本棚から落ちた本"},{18,L"sc_e_0224:宝箱をとれるようになる"}};
bool integer(const json::Value* v,int lo,int hi){return v&&v->type==json::Value::Type::Number&&v->n>=lo&&v->n<=hi&&v->n==int(v->n);}
}
bool supported(int map){return map==293;}
std::span<const library::Control> controls(int map){return supported(map)?std::span<const library::Control>(actors):std::span<const library::Control>{};}
Plan plan(const Snapshot& s,const json::Value& puzzles){
 modtext::LocaleScope locale(s.locale);Plan out;out.map=s.map;out.supported=supported(s.map);if(!out.supported)return out;
 if(s.values[4]==1){out.finished=true;out.note=T(L"Puzzle guide finished");return out;}
 const auto loading=[&]{out.unknown=true;out.targets.clear();out.note=T(L"Navigation data is loading");return out;};
 for(const auto& bit:s.values)if(!bit||(*bit!=0&&*bit!=1))return loading();
 for(const auto& hidden:s.hidden)if(!hidden)return loading();
 if(s.cancelled&&s.cancelled())return out;
 const auto* recipe=puzzles.get(L"catapult_guide");const auto* steps=recipe?recipe->get(L"steps"):nullptr;
 if(!steps||steps->type!=json::Value::Type::Array||steps->arr.size()!=5)return loading();
 const auto* map=field::baked::submap_by_map_id(s.map);if(!map||map->world)return loading();
 std::array<field::baked_nav::GuideTarget,5> targets;
 for(size_t i=0;i<5;++i){const auto& row=steps->arr[i];
  const auto* entity=row.get(L"entity");const auto* mid=row.get(L"map");const auto* x=row.get(L"x");const auto* y=row.get(L"y");const auto* layer=row.get(L"layer");
  const auto* raw=row.get(L"raw");const auto* text=row.get(L"text");const auto* receipt=row.get(L"done_scenario");
  if(!integer(entity,actors[i].id,actors[i].id)||!integer(mid,s.map,s.map)||!integer(receipt,flags[i],flags[i])||
     !integer(x,-32768,32767)||!integer(y,-32768,32767)||!integer(layer,0,2)||!raw||raw->s!=actors[i].raw||!text||!text->is_string()||text->s.empty())return loading();
  const field::baked::Object* source=nullptr;
  for(const auto& object:field::baked::grids(*map).objects)if(object.type==field::baked::ObjType::Entity&&object.entity_id()==actors[i].id&&
      field::baked::object_identifier(object)==actors[i].raw&&object.tx==int(x->n)&&object.ty==int(y->n)&&object.layer==int(layer->n)){
   if(source)return loading();source=&object;
  }
  // Book23's home is intentionally off map. The existing exact current native
  // object capture owns its actual pose; do not invent a remote placement.
  if(!source)return loading();auto& target=targets[i];target.goal.map_id=s.map;target.goal.entity_id=actors[i].id;target.goal.raw_name=actors[i].raw;
  target.goal.cell={source->layer,source->tx,source->ty};target.goal.examine=true;target.goal.avoid_other_events=true;
  target.kind=field::TargetKind::Interact;target.instruction=T(text->s);
 }
 std::vector<size_t> order;const auto offer=[&](size_t i){if(!*s.hidden[i]&&std::find(order.begin(),order.end(),i)==order.end())order.push_back(i);};
 // A native later-stage receipt/visible final switch outranks retired clues.
 if(s.values[3]==1)offer(4);
 for(size_t i=0;i<3;++i)if(s.values[i]==0)offer(i);
 if(s.values[3]==0)offer(3);
 offer(4);offer(3);
 // Completed clues stay retired. Returning to the previous planter instead
 // of crossing the rest contact would trap the guide in a receipt-stable loop.
 for(size_t i:order)out.targets.push_back(std::move(targets[i]));
 // The central-room entry has a real automatic rest contact. Prefer the next
 // clue whenever it is already reachable, otherwise stop on this exact event
 // and let its native No response return control before replanning. Arrival
 // is never a puzzle receipt and no flag is synthesized for this transit.
 const auto* entry=recipe->get(L"entry_step");const auto* id=entry?entry->get(L"entity"):nullptr;
 const auto* mid=entry?entry->get(L"map"):nullptr;const auto* raw=entry?entry->get(L"raw"):nullptr;
 const auto* text=entry?entry->get(L"text"):nullptr;
 if(!integer(id,17,17)||!integer(mid,s.map,s.map)||!raw||raw->s!=L"sc_e_0432:休息できる"||!text||!text->is_string()||text->s.empty())return loading();
 const field::baked::Object* contact=nullptr;
 for(const auto& o:field::baked::grids(*map).objects)if(o.type==field::baked::ObjType::Event&&o.entity_id()==17&&field::baked::object_identifier(o)==raw->s){if(contact)return loading();contact=&o;}
 if(!contact||!integer(entry->get(L"x"),contact->tx,contact->tx)||!integer(entry->get(L"y"),contact->ty,contact->ty)||!integer(entry->get(L"layer"),contact->layer,contact->layer))return loading();
 field::baked_nav::GuideTarget transit;transit.goal.map_id=s.map;transit.goal.entity_id=17;transit.goal.raw_name=raw->s;
 transit.goal.cell={contact->layer,contact->tx,contact->ty};transit.goal.avoid_other_events=true;transit.kind=field::TargetKind::Spot;transit.instruction=T(text->s);
 if(s.player!=transit.goal.cell)out.targets.push_back(std::move(transit));
 return out.targets.empty()?loading():out;
}
}
