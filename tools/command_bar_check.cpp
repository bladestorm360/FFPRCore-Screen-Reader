// check-sources: text.cpp
// Actual command-bar consumers: startup/event preload and legitimate menu entry.
#include <map>
#include <cstdio>
#include <cstdlib>
#include "../src/core/system/command_bar.cpp"
namespace fixture {
struct Node { void* cursor=nullptr;void* rows=nullptr;void* data=nullptr;int id=1,state=1,index=0;bool alive=true,active=true; };
using List=std::vector<void*>;
bool menu=false;
game::Id current_game=game::Id::FF1;
std::map<std::string,void*> owners;
std::map<std::string,long long> indices;
std::vector<std::wstring> spoken;
std::vector<frame::Step> pending;
}
namespace core { Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
bool Class::is(void*p){return p!=nullptr;}
int32_t Field::get(){std::string n=name;
 if(n=="selectCursor")return offsetof(fixture::Node,cursor);
 if(n=="contentList"||n=="contents")return offsetof(fixture::Node,rows);
 if(n=="<Data>k__BackingField")return offsetof(fixture::Node,data);
 if(n=="<Id>k__BackingField"||n=="<EquipmentCommandIdCash>k__BackingField")return offsetof(fixture::Node,id);
 return -1;}
}
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return false;}void note_fault(const char*){std::abort();}}
namespace log_ {void write(const char*,...){} }
namespace unity {
bool alive(void*p){return p&&static_cast<fixture::Node*>(p)->alive;}
bool active_in_hierarchy(void*p){return alive(p)&&static_cast<fixture::Node*>(p)->active;}
int list_count(void*p){return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}
void*list_at(void*p,int i){return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}
}
namespace field::cache {
void*get(ref::Class&c){return fixture::owners[c.name];}
void register_object(ref::Class&c,void*p){fixture::owners[c.name]=p;}
void clear(ref::Class&c){fixture::owners.erase(c.name);}
}
namespace game {Id current(){return fixture::current_game;}}
namespace ffpr::system {bool is_main_menu_state(){return fixture::menu;}}
namespace frame {int every_frame(Step s,const char*,int){fixture::pending.push_back(std::move(s));return 1;}}
namespace dedup {
bool index(const char*k,long long i,Scope){auto it=fixture::indices.find(k);if(it!=fixture::indices.end()&&it->second==i)return false;fixture::indices[k]=i;return true;}
void reset(const char*k){fixture::indices.erase(k);}
}
namespace ui {
int cursor_index(void*p){return p?static_cast<fixture::Node*>(p)->index:-1;}
int state_tag(void*p,ref::Field&){return p?static_cast<fixture::Node*>(p)->state:-1;}
std::wstring menu_position(const std::wstring&s,int,int){return s;}
}
namespace ffpr {void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}}
namespace modtext {std::wstring T(std::wstring_view s){return std::wstring(s);}}
namespace queries {void define(const char*,std::function<bool()>){} }
int main(){using namespace fixture;namespace s=ffpr::system;int failures=0;
auto expect=[&](bool ok,const char*why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
for(int n=1;n<=6;++n){current_game=static_cast<game::Id>(n);Node item,equip,magic,bar,cur,row,data;List rows{&row};bar.cursor=&cur;bar.rows=&rows;row.data=&data;
owners["ItemWindowController"]=&item;owners["EquipmentWindowController"]=&equip;owners["AbilityWindowController"]=&magic;magic.state=4;
menu=false;spoken.clear();s::schedule_item_read(&bar);s::schedule_equip_read(&bar);if(n==1)s::schedule_magic_read(&bar);
for(auto&step:pending)expect(!step(),"background preload ends its bounded reader immediately");pending.clear();
expect(spoken.empty()&&!s::is_command_bar_active(),"non-menu intro/field preload speaks no command and claims no cursor");
menu=true;s::try_read_item(&bar);expect(spoken.size()==1&&spoken.back()==L"Use","real menu still reads Use");
s::try_read_item(&bar);expect(spoken.size()==1,"existing same-index event guard retained");
cur.index=1;data.id=2;rows.push_back(&row);s::try_read_item(&bar);expect(spoken.size()==2&&spoken.back()==L"Sort","real menu move reads next command");
cur.index=0;spoken.clear();s::try_read_equip(&bar);expect(spoken.size()==1&&spoken.back()==L"Equip","real equipment command remains audible");
if(n==1){data.id=1;spoken.clear();s::try_read_magic(&bar);expect(spoken.size()==1&&spoken.back()==L"Use","FF1 magic command remains audible");}
menu=false;s::try_read_item(&bar);menu=true;cur.index=0;data.id=1;spoken.clear();s::try_read_item(&bar);expect(spoken.size()==1,"menu re-entry re-arms existing index guard");
indices.clear();std::printf("FF%d preload and real command focus checked\n",n);}
std::printf("Command contexts: %d failures\n",failures);return failures?1:0;}
