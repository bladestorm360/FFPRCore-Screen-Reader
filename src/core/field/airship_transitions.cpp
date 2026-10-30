#include "airship_transitions.h"
namespace field::air_transitions {
namespace {
constexpr Rule rules[]={
 {1,341,0,L"地上全体へ：エンタープライズ",1u<<6},
 {1,345,4,L"地上全体へ：ノーチラス",(1u<<8)|(1u<<9)},
 {1,349,4,L"地上全体へ：インビンシブル",1u<<11},
 {2,356,1,L"地上一部→浮遊大陸",1u<<15},
 {4,343,1,L"浮遊大陸へ移動：エンタープライズ",1u<<6},
 {4,347,1,L"浮遊大陸へ移動：ノーチラス",(1u<<8)|(1u<<9)},
 {4,351,1,L"浮遊大陸へ移動：インビンシブル",1u<<10}
};
}
const Rule* match(int game_id,int map,int script,const std::wstring& raw) {
 if(game_id!=3)return nullptr;
 for(const auto& rule:rules)if(rule.map==map&&rule.script==script&&raw==rule.raw)return &rule;
 return nullptr;
}
const Rule* match(int game_id,int map,const baked::Object& object) {
 if(object.type!=baked::ObjType::Event)return nullptr;
 return match(game_id,map,object.a,baked::object_identifier(object));
}
bool possible_raw(int game_id,int map,const std::wstring& raw) {
 if(game_id!=3)return false;for(const auto& rule:rules)if(rule.map==map&&raw==rule.raw)return true;return false;
}
bool possible_aircraft(int game_id,int map,int id) {
 if(game_id!=3||id<=0||id>=32)return false;
 for(const auto& rule:rules)if(rule.map==map&&(rule.mask&(1u<<id)))return true;
 return false;
}
bool accepts(const Rule& rule,int id){return id>0&&id<32&&(rule.mask&(1u<<id))!=0;}
const Rule* saronia_entry(int game_id,int map,int script,const std::wstring& raw) {
 // Both native Enterprise flight variants can meet this unrestricted event.
 // Their ship forms13/16 take off through the existing paired-mode planner.
 static const Rule entry{4,120,105,L"飛空艇大破",(1u<<6)|(1u<<15)};
 return game_id==3&&map==entry.map&&script==entry.script&&raw==entry.raw?&entry:nullptr;
}
const Rule* saronia_entry(int game_id,int map,const baked::Object& object) {
 return object.type==baked::ObjType::Event?saronia_entry(game_id,map,object.a,baked::object_identifier(object)):nullptr;
}
}
