// check-sources: text.cpp
// Actual cached reader/admission and binding store; native collections are fake.
#define main binding_fixture_entry
#include "bindings_check.cpp"
#undef main
#include "../src/core/input/native_keyboard.cpp"
#include <thread>
#include <cstddef>
namespace native_fixture {
DWORD owner=GetCurrentThreadId();int wrong_owner=0,reads=0;
void observe(){++reads;if(GetCurrentThreadId()!=owner)++wrong_owner;}
struct Row {int type=1,device=0,code=0;bool valid=true;};
struct List {std::vector<void*> rows;int override_count=-1;};
struct Dict {std::vector<std::pair<uint64_t,uint64_t>> rows;int override_count=-1;};
struct Config {void* mappings=nullptr;};struct Listener {void* config=nullptr;};
Listener* instance=nullptr;
std::map<std::string,actions::Fn> actions;
}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
il2cpp::Class* Class::get(){native_fixture::observe();return reinterpret_cast<il2cpp::Class*>(this);}
bool Class::is(void*p){native_fixture::observe();return p&&static_cast<native_fixture::Row*>(p)->valid;}
int32_t Field::get(){native_fixture::observe();const std::string key=name;if(key=="<KeyConfig>k__BackingField")return offsetof(native_fixture::Listener,config);if(key=="config")return offsetof(native_fixture::Config,mappings);if(key=="inputType")return offsetof(native_fixture::Row,type);if(key=="deviceType")return offsetof(native_fixture::Row,device);if(key=="keyCode")return offsetof(native_fixture::Row,code);return -1;}
}
namespace frame {bool on_main_thread(){return GetCurrentThreadId()==native_fixture::owner;} }
namespace actions {void define(const char*n,Fn fn){native_fixture::actions[n]=std::move(fn);} }
namespace il2cpp {
void* read_static_ptr(Class*,const char*){native_fixture::observe();return native_fixture::instance;}
bool read_string(String*p,wchar_t*out,size_t size){native_fixture::observe();if(!p)return false;const auto&s=*reinterpret_cast<std::wstring*>(p);if(s.size()>=size)return false;std::copy(s.begin(),s.end(),out);out[s.size()]=0;return true;}
}
namespace unity {
int dict_count(void*p){native_fixture::observe();if(!p)return 0;const auto&d=*static_cast<native_fixture::Dict*>(p);return d.override_count>=0?d.override_count:int(d.rows.size());}
bool dict_entry(void*p,int i,uint64_t&k,uint64_t&v,int,int){native_fixture::observe();if(!p||i<0||i>=int(static_cast<native_fixture::Dict*>(p)->rows.size()))return false;const auto pair=static_cast<native_fixture::Dict*>(p)->rows[size_t(i)];k=pair.first;v=pair.second;return k!=UINT64_MAX;}
int list_count(void*p){native_fixture::observe();if(!p)return 0;const auto&v=*static_cast<native_fixture::List*>(p);return v.override_count>=0?v.override_count:int(v.rows.size());}
void* list_at(void*p,int i){native_fixture::observe();if(!p||i<0||i>=int(static_cast<native_fixture::List*>(p)->rows.size()))return nullptr;return static_cast<native_fixture::List*>(p)->rows[size_t(i)];}
}
int main(){using namespace native_fixture;namespace b=input::bindings;using K=input::Key;int failures=0,cases=0;auto expect=[&](bool ok,const char*why){++cases;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 input::native_keyboard::install();const int route=b::add("key.field.route","Route",false,1,int(K::P));const int modal=b::add("key.menu.fixture","Modal",false,-1,int(K::Return));std::wstring conflict;
 expect(!b::set(route,int(K::F12),0,conflict)&&b::entries()[route].primary==int(K::P),"unknown native config refuses new gameplay binding and preserves old value");expect(b::conflict_message(conflict).find(L"unavailable")!=std::wstring::npos,"unknown config has its own accessible feedback");
 std::wstring action=L"Action",cancel=L"Cancel";
 Row enter{1,0,input::native_keyboard::kUnityKeys[int(K::A)]};Row escape{1,0,input::native_keyboard::kUnityKeys[int(K::Escape)]};
 List enter_list{{&enter}},cancel_list{{&escape}};
 Dict keyboard{{{reinterpret_cast<uint64_t>(&action),reinterpret_cast<uint64_t>(&enter_list)},
                {reinterpret_cast<uint64_t>(&cancel),reinterpret_cast<uint64_t>(&cancel_list)}}};
 Dict root{{{0,reinterpret_cast<uint64_t>(&keyboard)}}};Config config{&root};Listener listener{&config};instance=&listener;
 const auto refresh=[&]{native_fixture::actions.at("Input.RefreshKeyboardConflicts")();};refresh();expect(input::native_keyboard::known&&input::native_keyboard::count==2,"bounded actual nested configuration copies two current game keys");const int captured=reads;
 for(int mods=0;mods<8;++mods)expect(!b::set(route,int(K::A),mods,conflict)&&conflict==L"Action","native key remains a conflict even with modifiers");expect(reads==captured,"every picker/capture admission uses owned data without another native read");expect(b::set(route,int(K::F12),2,conflict),"free key and modifier chord accepted");expect(b::set(modal,int(K::A),0,conflict),"exclusive mod-menu context can reuse a native gameplay key");
 enter.code=input::native_keyboard::kUnityKeys[int(K::F12)];refresh();expect(!b::set(route,int(K::F12),2,conflict),"fresh settings entry sees game-side key changes");enter.code=input::native_keyboard::kUnityKeys[int(K::LeftControl)];refresh();expect(!b::set(route,int(K::F14),2,conflict),"native modifier assignment also conflicts with a chord");expect(b::set(route,int(K::F14),0,conflict),"unrelated bare key stays available");
 for(int failure=0;failure<7;++failure){root.override_count=failure==0?9:-1;keyboard.override_count=failure==1?65:-1;enter_list.override_count=failure==2?17:-1;enter.type=failure==3?2:1;enter.device=failure==4?1:0;enter.valid=failure!=5;action=failure==6?std::wstring(65,L'x'):L"Action";refresh();expect(!input::native_keyboard::known&&!b::set(route,int(K::F13),0,conflict),"oversized/unsupported/malformed input never grants a new binding");}
 root.override_count=keyboard.override_count=enter_list.override_count=-1;enter={1,0,input::native_keyboard::kUnityKeys[int(K::A)]};action=L"Action";refresh();const int owner_reads=reads;std::thread worker([&]{input::native_keyboard::refresh();});worker.join();expect(reads==owner_reads&&input::native_keyboard::known&&input::native_keyboard::count==2,"off-owner refresh performs no read or cache mutation");refresh();
 fixture::keys[int(K::F14)]=fixture::edges[int(K::F14)]=true;b::context(input::Context::Field);expect(b::remapped_keyboard_in_use(),"current remapped primary owns native input");b::context(input::Context::Global);expect(b::remapped_keyboard_in_use(),"context handoff retains physical ownership through release");fixture::neutral();expect(!b::remapped_keyboard_in_use(),"released primary retires ownership");
 expect(b::set(route,int(K::F14),2,conflict),"free control chord can be assigned");fixture::keys[int(K::F14)]=fixture::keys[int(K::LeftControl)]=true;b::context(input::Context::Field);expect(b::remapped_keyboard_in_use(),"modifier chord owns its primary");fixture::keys[int(K::LeftControl)]=false;b::begin_frame();b::context(input::Context::Field);expect(b::remapped_keyboard_in_use(),"releasing modifier first does not leak held primary to native input");fixture::neutral();expect(!b::remapped_keyboard_in_use(),"complete release returns input");
 const int recovery=b::add("key.0.Toggle control interception.-1","Toggle control interception",false,0,int(K::F9));expect(!b::set(recovery,int(K::A),0,conflict),"recovery cannot overlap a mod-menu key");b::initialize_keyboard_controls();const int capture=b::capture_cancel_index;expect(b::set(capture,int(K::F15),0,conflict),"capture cancellation itself is remappable");expect(!b::set(capture,-1,0,conflict),"capture always retains a cancel key");b::capture(route);b::update_capture();fixture::edges[int(K::F15)]=true;b::update_capture();expect(!b::capturing(),"effective capture cancel chord retires without changing the route binding");
 fixture::neutral();const int standalone=b::add("key.modifier.fixture","Modifier fixture",false,int(input::Context::BestiaryDetail),int(K::F20));const int other=b::add("key.modifier.other","Other modifier fixture",false,int(input::Context::BestiaryDetail),int(K::F22));
 expect(b::set(standalone,int(K::LeftShift),0,conflict),"standalone modifier can be assigned in a context with no competing chord");expect(!b::set(standalone,int(K::LeftShift),1,conflict),"modifier cannot require itself as a second modifier");fixture::keys[int(K::LeftShift)]=fixture::edges[int(K::LeftShift)]=true;b::context(input::Context::BestiaryDetail);expect(b::pressed(standalone)&&b::remapped_keyboard_in_use(),"standalone modifier dispatch and native ownership use the same normalized chord");fixture::neutral();
 expect(!b::set(other,int(K::G),1,conflict),"new chord cannot reuse a standalone modifier action in the same context");b::reset(standalone);expect(b::set(other,int(K::G),1,conflict),"free modifier chord can be assigned after removing standalone action");expect(!b::set(standalone,int(K::LeftShift),0,conflict),"standalone action cannot take a modifier already used by a chord");b::reset(other);
 expect(wrong_owner==0,"no native observation moved to a worker");std::printf("Keyboard native/conflict/capture ownership:%d assertions/%d failures/owner%d\n",cases,failures,wrong_owner);return failures||wrong_owner?1:0;
}
