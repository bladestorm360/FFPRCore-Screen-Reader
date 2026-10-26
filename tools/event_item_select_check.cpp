// check-sources: text.cpp
// Actual FFIII event-item selection and manual detail action, fake native owners.
#include "../src/games/ff3/event_item_select.cpp"
#include <map>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
namespace fixture {
struct Node {void* list=nullptr;std::wstring name,description;bool visible=true;};
using List=std::vector<void*>;Node manager;bool manager_exists=true,autodetail=false;int originals=0;
std::vector<std::wstring> spoken;
std::map<std::string,bool> states;
std::map<std::string,std::wstring> guards;
std::map<std::string,actions::Fn> actions;
std::map<std::string,std::function<bool()>> queries;
std::map<events::Kind,std::vector<events::Handler>> handlers;
void select(void*,int32_t,int32_t,const void*){++originals;}
void close(void*,const void*){++originals;}
}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class& c,const char*n,const char*a):cls(c),name(n),alt(a){}
Method::Method(Class& c,const char*n,int a):cls(c),name(n),argc(a){}
int32_t Field::get(){return offsetof(fixture::Node,list);}
bool Class::is(void*p){return p!=nullptr;}
std::wstring rd_str(void*p,Field&f){if(!p)return {};const auto&n=*static_cast<fixture::Node*>(p);return std::strcmp(f.name,"<NameMessageId>k__BackingField")==0?n.name:n.description;}
void* instance(Class&){return fixture::manager_exists?&fixture::manager:nullptr;}
}
namespace unity {bool alive(void*p){return p!=nullptr;}bool active_in_hierarchy(void*p){return p&&static_cast<fixture::Node*>(p)->visible;}int list_count(void*p){return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}void* list_at(void*p,int i){return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}}
namespace ffpr::status {std::wstring game_message(const std::wstring&s,bool){return s;}}
namespace settings {bool get_bool(const char*,bool){return fixture::autodetail;}}
namespace ffpr {void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}}
namespace modtext {std::wstring T(std::wstring_view s){return std::wstring(s);}}
namespace ui {std::wstring menu_position(const std::wstring&s,int,int){return s;}}
namespace menu_state {bool is_active(const char*n){return fixture::states[n];}void set_active(const char*n,bool b){fixture::states[n]=b;}void reset(const char*n){fixture::states[n]=false;}}
namespace dedup {bool text(const char*n,std::wstring_view s,Scope){if(fixture::guards[n]==s)return false;fixture::guards[n]=s;return true;}void reset(const char*n){fixture::guards.erase(n);}}
namespace actions {void define(const char*n,Fn f){fixture::actions[n]=std::move(f);}}
namespace queries {void define(const char*n,std::function<bool()>f){fixture::queries[n]=std::move(f);}}
namespace events {void on(Kind k,Handler h){fixture::handlers[k].push_back(std::move(h));}}
namespace cursor {void add_suppressor(const char*){}}
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return false;}void note_fault(const char*){std::abort();}}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace log_ {void write(const char*,...){}}
int main(){
 namespace event=ffpr::ff3;using namespace fixture;
 int failures=0;auto expect=[&](bool b,const char*m){if(!b){++failures;std::printf("FAIL %s\n",m);}};
 event::install();event::o_SelectContent=&fixture::select;event::o_ManagerClose=&fixture::close;
 Node canoe,eye,controller;canoe.name=L"Canoe";canoe.description=L"A small vessel used to cross shallow lakes and streams.";eye.name=L"Nepto Eye";eye.description=L"The statue's missing eye.";List rows{&canoe,&eye};controller.list=&rows;
 auto details=[&]{fixture::actions["EventItemSelect.ReadDetails"]();};
 spoken.clear();autodetail=false;event::d_SelectContent(&controller,0,0,nullptr);
 expect(spoken==std::vector<std::wstring>{L"Canoe"},"Autodetail off reads only the focused key item");
 spoken.clear();details();details();expect(spoken==std::vector<std::wstring>{canoe.description,canoe.description},"manual I reads description on every deliberate request");
 spoken.clear();event::d_SelectContent(&controller,1,0,nullptr);expect(spoken==std::vector<std::wstring>{L"Nepto Eye"},"changing items never auto-reads descriptions while off");
 autodetail=true;spoken.clear();event::d_SelectContent(&controller,0,0,nullptr);expect(spoken==std::vector<std::wstring>{canoe.name+L": "+canoe.description},"Autodetail on appends the selected description once");
 eye.description.clear();spoken.clear();event::d_SelectContent(&controller,1,0,nullptr);details();expect(spoken==std::vector<std::wstring>{L"Nepto Eye",L"No description"},"blank description clears the prior item's detail");
 event::d_ManagerClose(&manager,nullptr);spoken.clear();details();expect(spoken.empty()&&!event::should_suppress(),"Close clears active ownership and cached details");
 event::d_SelectContent(&controller,0,0,nullptr);controller.visible=false;spoken.clear();details();expect(spoken.empty()&&!event::should_suppress(),"hidden controllers cannot expose stale details");controller.visible=true;
 event::d_SelectContent(&controller,1,0,nullptr);for(auto& f:handlers[events::Kind::MapChanging])f();spoken.clear();details();expect(spoken.empty(),"map transition retires event-item context before old native objects disappear");
 expect(originals>=7,"native select and close originals remain active");
 std::printf("FFIII event-item Autodetail/I ownership: %d failures\n",failures);return failures?1:0;
}
