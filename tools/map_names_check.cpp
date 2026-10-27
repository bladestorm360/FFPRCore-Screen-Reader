// check-sources: json.cpp text.cpp
// Production current-map/exit resolver with fake native masters/messages; no game.
#include <cstdio>
#include <fstream>
#include <iterator>
#include "../src/core/field/map_names.cpp"
namespace fixture {
using Dict=std::map<int,void*>;
struct Map {int area=31;std::wstring title=L"None",asset=L"Map_30161";int floor=13;};
Dict maps,areas,lists;void* map_list=&maps;void* area_list=&areas;void* master=&lists;
int current=314;std::wstring area=L"area",language=L"en";
Map map;json::Value titles;std::function<std::wstring()> map_key;
int get_area(void* p,const void*){return static_cast<Map*>(p)->area;}
int get_floor(void* p,const void*){return static_cast<Map*>(p)->floor;}
void* get_title(void* p,const void*){return &static_cast<Map*>(p)->title;}
void* get_asset(void* p,const void*){return &static_cast<Map*>(p)->asset;}
void* get_area_name(void*,const void*){return &area;}
void* provision(const void*){return &current;}
void* user(const void*){return nullptr;}
std::wstring localized(const std::wstring& key){return language+L":"+(key==L"same_text_alias"?L"area":key);}
}
namespace core {Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace queries {void define_text(const char*,std::function<std::wstring()> f){fixture::map_key=std::move(f);}}
namespace log_ {void write(const char*,...) {}}
namespace hooks {void note_fault(const char*){std::abort();}}
namespace data {const json::Value& game_json(const char*){return fixture::titles;}}
namespace modtext {
std::wstring T(std::wstring_view key){return fixture::localized(std::wstring(key));}
std::wstring TF(std::wstring_view key,std::initializer_list<std::wstring> args){return text::format(key,args);}
}
namespace field {
std::wstring get_message(const std::wstring& key,bool){return fixture::localized(key);}
std::wstring translate_entity_name(const std::wstring& key){return key;}
namespace r {
ref::Class kMasterManager{"Fixture","Master"},kMessageManager{"Fixture","Message"};
ref::Field fMasterList{kMasterManager,"lists"},fProvisionCurrentMapId{kMasterManager,"current"},fUserDataCurrentMapId{kMasterManager,"current"};
ref::Method mMapGetAreaId{kMasterManager,"get_AreaId",0},mMapGetMapTitle{kMasterManager,"get_MapTitle",0},
 mMapGetFloor{kMasterManager,"get_Floor",0},mAreaGetAreaName{kMasterManager,"get_AreaName",0},
 mMapGetAssetName{kMasterManager,"get_AssetName",0},mProvisionGetInstance{kMasterManager,"Provision",0},
 mUserDataInstance{kMasterManager,"User",0};
}
}
namespace ref {
Class::Class(const char* ns_,const char* name_,uint8_t g,const char* a):ns(ns_),name(name_),games(g),assembly(a){}
Field::Field(Class& c,const char* n,const char* a):cls(c),name(n),alt(a){}
Method::Method(Class& c,const char* n,int a):cls(c),name(n),argc(a){}
int32_t Field::get(){return 0;}
il2cpp::Method* Method::get(){return nullptr;}
void* Method::ptr(){
 const std::string n=name;
 if(n=="get_AreaId")return reinterpret_cast<void*>(&fixture::get_area);
 if(n=="get_MapTitle")return reinterpret_cast<void*>(&fixture::get_title);
 if(n=="get_Floor")return reinterpret_cast<void*>(&fixture::get_floor);
 if(n=="get_AreaName")return reinterpret_cast<void*>(&fixture::get_area_name);
 if(n=="get_AssetName")return reinterpret_cast<void*>(&fixture::get_asset);
 if(n=="Provision")return reinterpret_cast<void*>(&fixture::provision);
 return reinterpret_cast<void*>(&fixture::user);
}
void* instance(Class& c){return &c==&field::r::kMasterManager?static_cast<void*>(&fixture::master):static_cast<void*>(&fixture::area);}
std::wstring str(void* p){return p?*static_cast<std::wstring*>(p):L"";}
void call_failed_note(const char*){std::abort();}
}
namespace il2cpp {Class* object_class(Object* p){return reinterpret_cast<Class*>(p);}int32_t field_offset(Class*,const char*){return 0;}}
namespace unity {void dict_for_each(void* p,void(*f)(uint64_t,uint64_t,void*),void* s,int,int){for(auto [k,v]:*static_cast<fixture::Dict*>(p))f(k,reinterpret_cast<uint64_t>(v),s);}}
int main(){
 using namespace fixture;int failures=0,count=0;
 auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 lists={{0,&map_list},{1,&area_list}};areas={{31,&area}};field::install();
 for(int game=1;game<=6;++game){
  const std::string path="data/ff"+std::to_string(game)+"/map_titles.json";
  std::ifstream input(path,std::ios::binary);std::string raw{std::istreambuf_iterator<char>(input),{}};
  titles=json::Value{}; // Match production's fresh resource value per game.
  expect(json::parse(raw,titles),"embedded title resource parses");
  const auto* floors=titles.get(L"@entryFloors");expect(floors&&floors->is_object(),"all-six generated title namespace");
  if(!floors)continue;
  for(const wchar_t* locale:{L"en",L"ja",L"fr",L"it",L"de",L"es",L"ko",L"zht",L"zhc",L"ru",L"th",L"pt"}){
   language=locale;field::clear_map_name_cache();
   // Compare resolved labels, including different keys with the same text. Each
   // consumer must continue returning the label on repeated requests.
   current=999999;maps={{current,&map}};map.floor=0;area=L"area";
   for(const wchar_t* key:{L"area",L"same_text_alias"}){
    map.title=key;field::clear_map_name_cache();
    const auto expected=localized(area);
    for(int repeat=0;repeat<2;++repeat){
     expect(field::current_map_name()==expected,"equal resolved parts current map");
     expect(map_key()==expected,"equal resolved parts map-key query");
     expect(field::map_exit_name(current)==expected,"equal resolved parts exit and cache");
    }
   }
   map.title=L"area room";field::clear_map_name_cache();
   expect(field::current_map_name()==localized(area)+L" "+localized(L"area room"),"distinct prefixed title remains intact");
   area=L"";map.title=L"room";
   expect(field::current_map_name()==localized(L"room"),"title only");
   map.title=L"None";expect(field::resolve_map_name_by_id(current).empty(),"empty area and title");
   area=L"area";expect(field::current_map_name()==localized(area),"area only");
   // Ur's well and shed are genuine GotoMap destinations. Check the actual
   // authored resources through all current/map-key/remote consumers so these
   // entrances cannot collapse back to indistinguishable area-only names.
   if(game==3){
    for(int id:{336,337,338}){
     current=id;maps={{id,&map}};map.title=L"None";map.floor=0;
     auto suffix=id==338?localized(L"Well"):localized(L"Shed");
     if(id==337)suffix+=L" "+localized(L"Back Room");
     const auto expected=localized(area)+L" "+suffix;
     expect(field::current_map_name()==expected,"Ur physical entrance current-map title");
     expect(map_key()==expected,"Ur physical entrance map-key title");
     expect(field::map_exit_name(id)==expected,"Ur physical entrance remote/exit title");
    }
   }
   for(const auto& [id,key]:floors->obj){
    if(titles.get(id))continue; // Authored names have separate precedence checks.
    current=std::stoi(id);maps={{current,&map}};map.title=L"None";map.floor=13;
    const auto expected=localized(area)+L" "+localized(key.s);
    expect(field::current_map_name()==expected,"current map uses native key instead of internal floor");
    expect(map_key()==expected,"map key query shares current-map resolver");
    expect(field::map_exit_name(current)==expected,"remote/exit name and cache share native key");++count;
   }
  }
 }
 expect(count==4944,"all412 non-authored native floor keys exercised in all12 locales");
 titles=json::Value{};
 expect(json::parse(R"({"314":{"text":"Statue Room"},"@entryFloors":{"314":"native"}})",titles),"precedence fixture");
 current=314;maps={{314,&map}};map.title=L"master";
 expect(field::current_map_name()==localized(area)+L" "+localized(L"Statue Room"),"authored room wins over both native/master floors");
 titles.obj.erase(L"314");expect(field::current_map_name()==localized(area)+L" "+localized(L"master"),"explicit master title retains precedence");
 titles.obj.clear();map.title=L"None";map.floor=-2;
 expect(field::current_map_name()==localized(area)+L" B2","unverified maps retain existing basement fallback");
 map.floor=0;expect(field::current_map_name()==localized(area),"no inferred floor when numeric and native titles absent");
 std::printf("%s native floor labels: %d all-six/current/map-key/exit/cache/locale checks, %d failures\n",failures?"FAIL":"PASS",count,failures);
 return failures?1:0;
}
