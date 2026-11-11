// Surgate permits one carried book. Correct shelving sets a persistent receipt;
// a wrong shelf clears the carried bit without restoring the hidden floor book.
// Entering the room resets every unfinished arrangement. Observe those actual
// states, never retain a linear step number across a native reset.
#include "book_model.h"
#include "../../core/modtext.h"
namespace ff5::book {
namespace {
constexpr library::Control books[]={{17,L"sc_e_0241:「禁断の書」取得"},{18,L"sc_e_0383:「魔物大辞典」取得"},{19,L"sc_e_0385:「ロンカの秘密」取得"}};
constexpr int shelves[3][3]={{21,35,36},{22,50,0},{23,51,52}};
constexpr const wchar_t* shelf_names[]={L"sc_e_0382_1:「禁断の書」格納",L"sc_e_0384:「魔物大辞典」格納",L"sc_e_0386:「ロンカの秘密」格納"};
bool integer(const json::Value* v,int lo,int hi){return v&&v->type==json::Value::Type::Number&&v->n>=lo&&v->n<=hi&&v->n==int(v->n);}
bool bind(const json::Value* row,int id,const wchar_t* name,field::baked::ObjType type,field::baked_nav::GuideTarget& out){
 if(!row||!integer(row->get(L"map"),160,160)||!integer(row->get(L"entity"),id,id))return false;
 const auto* raw=row->get(L"raw");const auto* text=row->get(L"text");const auto* x=row->get(L"x");const auto* y=row->get(L"y");const auto* layer=row->get(L"layer");
 if(!raw||raw->s!=name||!text||!text->is_string()||text->s.empty()||!integer(x,-32768,32767)||!integer(y,-32768,32767)||!integer(layer,0,2))return false;
 const auto* map=field::baked::submap_by_map_id(160);if(!map)return false;const field::baked::Object* found=nullptr;
 for(const auto& o:field::baked::grids(*map).objects)if(o.type==type&&o.entity_id()==id&&field::baked::object_identifier(o)==name&&o.tx==int(x->n)&&o.ty==int(y->n)&&o.layer==int(layer->n)){if(found)return false;found=&o;}
 if(!found)return false;out.goal.map_id=160;out.goal.cell={found->layer,found->tx,found->ty};out.goal.entity_id=id;out.goal.raw_name=name;out.goal.avoid_other_events=true;
 out.goal.examine=type==field::baked::ObjType::Entity||type==field::baked::ObjType::NPC;out.kind=out.goal.examine?field::TargetKind::Interact:field::TargetKind::Transition;out.instruction=T(text->s);return true;
}
}
bool supported(int map){return map==159||map==160;}
std::span<const library::Control> pickups(){return books;}
Plan plan(const Snapshot& s,const json::Value& puzzles){modtext::LocaleScope locale(s.locale);Plan out;out.map=s.map;out.supported=supported(s.map);if(!out.supported)return out;
 const auto unknown=[&]{out.unknown=true;out.targets.clear();out.note=T(L"Navigation data is loading");return out;};
 for(const auto& v:s.values)if(!v||(*v!=0&&*v!=1))return unknown();
 int held=-1,count=0;for(int i=0;i<3;++i)if(s.values[i+3]==1){held=i;++count;if(s.values[i]==1)return unknown();}if(count>1)return unknown();
 const bool complete=s.values[0]==1&&s.values[1]==1&&s.values[2]==1;
 if(complete&&(s.map==159||s.gate_hidden==true)){out.finished=true;out.note=T(L"Puzzle guide finished");return out;}
 if(s.cancelled&&s.cancelled())return out;const auto* root=puzzles.get(L"book_guide");if(!root)return unknown();
 if(s.map==159){
  // Native entry clears partial placed/carried bits. A floor waypoint avoids
  // asking a future actor predicate to borrow the old pre-entry book state.
  const auto* point=root->get(L"return_point");const auto* text=point?point->get(L"text"):nullptr;
  if(!point||!text||!text->is_string()||text->s.empty()||!integer(point->get(L"map"),160,160)||!integer(point->get(L"layer"),1,1)||!integer(point->get(L"x"),12,12)||!integer(point->get(L"y"),16,16))return unknown();
  const auto* map=field::baked::submap_by_map_id(160);if(!map||!field::baked::walkable(field::baked::grids(*map),1,12,16,true))return unknown();
  field::baked_nav::GuideTarget target;target.goal.map_id=160;target.goal.cell={1,12,16};target.goal.avoid_other_events=target.goal.allow_scripted_crossings=true;target.kind=field::TargetKind::Spot;target.instruction=T(text->s);out.targets.push_back(std::move(target));return out;
 }
 if(complete){if(!s.gate_hidden)return unknown();field::baked_nav::GuideTarget npc;
  if(!bind(root->get(L"librarian"),20,L"sc_e_0387:隠し通路が開く",field::baked::ObjType::NPC,npc))return unknown();out.targets.push_back(std::move(npc));return out;}
 bool lost=false;for(int i=0;i<3;++i)if(s.values[i]==0&&s.values[i+3]==0){if(!s.hidden[i])return unknown();lost|=*s.hidden[i];}
 if(lost){field::baked_nav::GuideTarget exit;if(!bind(root->get(L"reset_exit"),1,L"GotoMap",field::baked::ObjType::GotoMap,exit))return unknown();out.targets.push_back(std::move(exit));return out;}
 const auto* rows=root->get(L"books");if(!rows||rows->type!=json::Value::Type::Array||rows->arr.size()!=3)return unknown();
 for(int i=0;i<3;++i)if(!integer(rows->arr[i].get(L"placed_flag"),flags[i],flags[i])||!integer(rows->arr[i].get(L"held_flag"),flags[i+3],flags[i+3]))return unknown();
 if(held>=0){const auto* choices=rows->arr[size_t(held)].get(L"shelves");const int length=held==1?2:3;
  if(!choices||choices->type!=json::Value::Type::Array||int(choices->arr.size())!=length)return unknown();
  for(int i=0;i<length;++i){field::baked_nav::GuideTarget target;if(!bind(&choices->arr[size_t(i)],shelves[held][i],shelf_names[held],field::baked::ObjType::Entity,target))return unknown();out.targets.push_back(std::move(target));}
 }else for(int i=0;i<3;++i)if(s.values[i]==0){field::baked_nav::GuideTarget target;if(!bind(rows->arr[size_t(i)].get(L"pickup"),books[i].id,books[i].raw,field::baked::ObjType::Entity,target))return unknown();out.targets.push_back(std::move(target));break;}
 return out.targets.empty()?unknown():out;
}
}
