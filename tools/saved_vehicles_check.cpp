// check-sources: field/saved_vehicles.cpp
// Actual ready native-owner reader, with value/lifetime/collection controls.
#include "field/field_internal.h"
#include "field/baked_map.h"
#include "frame.h"
#include <cstdio>
#include <cstring>
#include <limits>
namespace fixture {
game::Id game=game::Id::FF3;bool owner=true,ready=true,user_ready=true,storage_ready=true,method=true;
int count=-2,reads=0,enabled_calls=0,transport_type=6;
struct Save {unity::Vec3 cell;int id=9,map=4;} saved;
struct Owned {Save* data=&saved;bool enabled=true;} owned;
std::vector<void*> list{&owned};struct User {void* list=&fixture::list;} user;
void native(){++reads;}
bool enabled(void* self,const void*){++enabled_calls;native();return static_cast<Owned*>(self)->enabled;}
}
namespace game {Id current(){return fixture::game;}}
namespace frame {bool on_main_thread(){return fixture::owner;}}
namespace ffpr::system {bool is_field_navigation_ready(){return fixture::ready;}}
namespace field::baked {const TransportRow* transport(int id){static TransportRow row{9,6,10,3,0};row.type=fixture::transport_type;return id==9?&row:nullptr;}}
namespace ref {
Class::Class(const char* n,const char* v,uint8_t mask,const char* a):ns(n),name(v),games(mask),assembly(a){}
il2cpp::Class* Class::get(){fixture::native();return reinterpret_cast<il2cpp::Class*>(this);}
bool Class::is(void* p){fixture::native();return p&&(p==&fixture::owned||p==&fixture::saved);}
Field::Field(Class& c,const char* n,const char* a):cls(c),name(n),alt(a){}
int32_t Field::get(){fixture::native();
 if(!std::strcmp(name,"<OwnedTransportationList>k__BackingField"))return offsetof(fixture::User,list);
 if(!std::strcmp(name,"saveData"))return offsetof(fixture::Owned,data);
 if(!std::strcmp(name,"position"))return offsetof(fixture::Save,cell);
 if(!std::strcmp(name,"id"))return offsetof(fixture::Save,id);
 if(!std::strcmp(name,"mapId"))return offsetof(fixture::Save,map);return -1;
}
Method::Method(Class& c,const char* n,int a):cls(c),name(n),argc(a){}
il2cpp::Method* Method::get(){fixture::native();return fixture::method?reinterpret_cast<il2cpp::Method*>(1):nullptr;}
void* Method::ptr(){fixture::native();return fixture::method?reinterpret_cast<void*>(&fixture::enabled):nullptr;}
void call_failed_note(const char*){}
}
namespace il2cpp {
void* read_static_ptr(Class* k,const char*){fixture::native();auto* c=reinterpret_cast<ref::Class*>(k);
 return !std::strcmp(c->name,"UserDataManager")?(fixture::user_ready?static_cast<void*>(&fixture::user):nullptr):(fixture::storage_ready?static_cast<void*>(&fixture::saved):nullptr);
}
}
namespace unity {
int list_count(void* p){fixture::native();return fixture::count!=-2?fixture::count:p?static_cast<int>(static_cast<std::vector<void*>*>(p)->size()):-1;}
void* list_at(void* p,int index){fixture::native();return static_cast<std::vector<void*>*>(p)->at(index);}
}
int main(){using field::saved_world_vehicles;int failures=0;
 const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 fixture::saved.cell={151,74,0};auto actual=saved_world_vehicles();
 expect(actual.size()==1&&actual[0].id==9&&actual[0].map==4&&actual[0].cell.x==151,"enabled craft copies its real saved map and native cell");
 fixture::saved.cell={46,57,0};actual=saved_world_vehicles();expect(actual.size()==1&&actual[0].cell.x==46,"later capture uses the moved departure position");
 for(int type:{0,1,4,5}){fixture::transport_type=type;expect(saved_world_vehicles().empty(),"internal Symbol/Content/player records cannot invent a parked vehicle");}fixture::transport_type=6;
 fixture::owned.enabled=false;expect(saved_world_vehicles().empty(),"native release flag excludes locked/disabled craft");fixture::owned.enabled=true;
 fixture::storage_ready=false;const int calls=fixture::enabled_calls;expect(saved_world_vehicles().empty()&&fixture::enabled_calls==calls,"missing interpreter never invokes a singleton getter");fixture::storage_ready=true;
 fixture::user_ready=false;expect(saved_world_vehicles().empty(),"missing user data is not created");fixture::user_ready=true;
 fixture::count=33;expect(saved_world_vehicles().empty(),"oversized collection is rejected before iteration");fixture::count=-2;
 fixture::saved.cell.x=std::numeric_limits<float>::infinity();expect(saved_world_vehicles().empty(),"nonfinite position is rejected");fixture::saved.cell.x=46;
 fixture::saved.map=99;expect(saved_world_vehicles().empty(),"interior/unrelated saved map cannot become a world craft");fixture::saved.map=4;
 fixture::owner=false;const int before=fixture::reads;expect(saved_world_vehicles().empty()&&fixture::reads==before,"workers read no native pointers");fixture::owner=true;
 for(int number:{1,2,4,5,6}){fixture::game=static_cast<game::Id>(number);const int count=fixture::reads;expect(saved_world_vehicles().empty()&&fixture::reads==count,"other games abstain before native access");}
 std::printf("Saved vehicle reader: %d failures; no native allocation, clone, scene scan or retained game pointer\n",failures);return failures?1:0;
}
