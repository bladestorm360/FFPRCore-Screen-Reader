// check-sources: field/field_refs.cpp
// Actual native collection producer; fake ready collection/property boundaries.
#include "../src/core/field/field_internal.h"
#include "../src/core/field/baked_map.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <map>
namespace fixture {
struct Node {void* controller=nullptr;void* transportation=nullptr;void* info=nullptr;void* entities=nullptr;void* colliders=nullptr;void* models=nullptr;void* foot=nullptr;void* steps=nullptr;void* object=nullptr;int type=8,id=8;bool enabled=true;};
using List=std::vector<void*>;using Dict=std::vector<std::pair<uint64_t,uint64_t>>;
Node root,controller,transportation,info,foot,model,party,vehicle;
List entities,colliders;Dict models,steps;
int reads=0,roots=0,map=1;uint64_t tick=1;game::Id game=game::Id::FF3;
void native(){++reads;}
void* object(void* p,const void*){native();return static_cast<Node*>(p)->object;}
bool enabled(void* p,const void*){native();return static_cast<Node*>(p)->enabled;}
int type(void* p,const void*){native();return static_cast<Node*>(p)->type;}
int id(void* p,const void*){native();return static_cast<Node*>(p)->id;}
void* message(void*,const void*){native();return nullptr;}
bool interactive(void*,int32_t,bool,const void*){native();return true;}
}
namespace game {Id current(){return fixture::game;}}
namespace frame {uint64_t count(){return fixture::tick;}}
namespace ref {
Class::Class(const char* n,const char* c,uint8_t g,const char* a):ns(n),name(c),games(g),assembly(a){}
bool Class::is(void* p){fixture::native();return p!=nullptr;}
il2cpp::Class* Class::get(){return nullptr;}
void* instance(Class&){return nullptr;}void* new_str(const std::wstring&){return nullptr;}
Field::Field(Class& c,const char* n,const char* a):cls(c),name(n),alt(a){}
int32_t Field::get(){fixture::native();using N=fixture::Node;
 for(auto [key,offset]:{std::pair{"fieldController",offsetof(N,controller)}, {"transportation",offsetof(N,transportation)}, {"infoData",offsetof(N,info)}, {"entityList",offsetof(N,entities)}, {"colliderEntityList",offsetof(N,colliders)}, {"modelList",offsetof(N,models)}, {"footEvent",offsetof(N,foot)}, {"stepOnTriggerList",offsetof(N,steps)}})if(!std::strcmp(name,key))return static_cast<int32_t>(offset);return -1;}
Method::Method(Class& c,const char* n,int a):cls(c),name(n),argc(a){}
il2cpp::Method* Method::get(){return nullptr;}
void* Method::ptr(){fixture::native();const std::string n=name;
 if(n=="get_MapObject")return reinterpret_cast<void*>(&fixture::object);
 if(n=="get_Enable")return reinterpret_cast<void*>(&fixture::enabled);
 if(n=="get_Type")return reinterpret_cast<void*>(&fixture::type);
 if(n=="get_Id")return reinterpret_cast<void*>(&fixture::id);
 if(n=="get_MessageId")return reinterpret_cast<void*>(&fixture::message);
 if(n=="CheckNeedInteractive")return reinterpret_cast<void*>(&fixture::interactive);return nullptr;}
std::wstring str(void*){return L"";}void call_failed_note(const char*){}
}
namespace unity {
bool alive(void* p){return p!=nullptr;}void* game_object(void* p){return p;}
int list_count(void* p){fixture::native();return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}
void* list_at(void* p,int index){fixture::native();return static_cast<fixture::List*>(p)->at(index);}
int dict_count(void* p){fixture::native();return p?static_cast<int>(static_cast<fixture::Dict*>(p)->size()):0;}
bool dict_entry(void* p,int index,uint64_t& key,uint64_t& value,int,int){fixture::native();auto row=static_cast<fixture::Dict*>(p)->at(index);key=row.first;value=row.second;return true;}
}
namespace il2cpp {void* find_method_overload_param(Class*,const char*,int,Class*,int){return nullptr;}void* method_pointer(Method*){return nullptr;}Method* virtual_method(void*,Method*){return nullptr;}Class* object_class(void*){return nullptr;}}
namespace log_ {void write(const char*,...){}}
namespace field {
namespace {struct VehicleInfo {int type,id;std::wstring message_id;int map_id;};std::unordered_map<void*,VehicleInfo> g_vehicle_types;std::vector<int> g_auto_groups;int g_auto_map=0;
bool skip_transport_type(int type){return type<=1||(!game::is(game::Id::FF1)&&(type==4||type==5));}}
void* field_map(){fixture::native();++fixture::roots;return &fixture::root;}
int current_map_id(){fixture::native();return fixture::map;}
namespace baked {const TransportRow* transport(int){return nullptr;}}
namespace party_entities {bool next_native(void*,int& index,void*& out){fixture::native();if(index++)return false;out=&fixture::party;return true;}}
#include "../src/core/field/entity_cursor.inl"
}
int main(){using namespace fixture;int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 std::vector<Node> actors(1535);for(auto& actor:actors){entities.push_back(&actor);steps.push_back({steps.size(),reinterpret_cast<uint64_t>(&actor)});}
 root.controller=&controller;controller.entities=&entities;controller.colliders=&colliders;controller.transportation=&transportation;controller.foot=&foot;
 transportation.info=&info;info.models=&models;foot.steps=&steps;model.object=&vehicle;models={{8,reinterpret_cast<uint64_t>(&model)}};colliders={&actors[0]};
 for(int n=1;n<=6;++n){fixture::game=static_cast<game::Id>(n);reads=roots=0;field::EntityCursor cursor;cursor.cache_frame_roots=n==3;std::vector<void*> output;const auto begin=std::chrono::steady_clock::now();
  int slices=1,slots=0;void* value=nullptr;while(cursor.next(value)){if(value)output.push_back(value);if(++slots%256==0){++tick;++slices;}}
  expect(output.size()==1537&&output[0]==&actors[0]&&output[1535]==&vehicle&&output[1536]==&party,"all entity/transport/party sources and cross-list identity guard are retained");
  expect(n==3?roots==slices:roots==slots+1,"only the qualified consumer shares roots within the same owner frame");
  std::printf("Cursor game%d slots%d unique%zu slices%d roots%d reads%d fake-native-wall%.3fus\n",n,slots,output.size(),slices,roots,reads,std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count());
 }
 fixture::game=game::Id::FF3;field::EntityCursor resumed;resumed.cache_frame_roots=true;void* value=nullptr;
 expect(resumed.next(value)&&value==&actors[0],"fresh qualified cursor starts with its current native collection");
 Node replacement=controller,moved;auto replacement_list=entities;replacement_list[1]=&moved;replacement.entities=&replacement_list;
 root.controller=&replacement;++tick;
 expect(resumed.next(value)&&value==&moved,"a resumed frame replaces roots and observes the current collection slot");
 root.controller=nullptr;++tick;
 expect(!resumed.next(value),"destroyed/unavailable roots are not dereferenced from an earlier frame");root.controller=&controller;
 std::printf("Entity cursor: %d failures\n",failures);return failures?1:0;
}
