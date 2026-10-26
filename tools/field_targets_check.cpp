// Actual native range-entry detours; only native UI/speech boundaries are fake.
#include "../src/core/menus/field_magic_targets.cpp"
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
namespace fixture {
struct Node {bool visible=true;void* list=nullptr;void* cursor=nullptr;Node* parent=nullptr;int index=0;std::wstring line;};
using List=std::vector<void*>;
game::Id game=game::Id::FF3;bool menu=true;int originals=0,clears=0;
std::vector<std::wstring> spoken;
std::map<std::string,std::function<bool()>> queries;
std::map<std::string,actions::Fn> actions;
std::map<events::Kind,std::vector<events::Handler>> events;
void single_cursor(){if(menu&&!queries["FieldMagicTargets.OwnsRange"]())spoken.push_back(L"legacy single");}
void native(void*,const void*){++originals;single_cursor();}
}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class& c,const char*n,const char*a):cls(c),name(n),alt(a){}
Method::Method(Class& c,const char*n,int a):cls(c),name(n),argc(a){}
bool Class::is(void*p){return p!=nullptr;}
int32_t Field::get(){return std::strcmp(name,"contentList")==0?offsetof(fixture::Node,list):offsetof(fixture::Node,cursor);}
}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return true;}void note_fault(const char*){std::abort();}}
namespace game {Id current(){return fixture::game;}}
namespace ffpr::system {bool is_main_menu_state(){return fixture::menu;}}
namespace unity {
bool alive(void*p){return p!=nullptr;}
bool active_in_hierarchy(void*p){return p&&static_cast<fixture::Node*>(p)->visible;}
int list_count(void*p){return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}
void* list_at(void*p,int i){return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}
}
namespace displayed {
void* cursor_row(void*p,void*){return p?static_cast<fixture::Node*>(p)->parent:nullptr;}
std::wstring target(void*p){return unity::active_in_hierarchy(p)?static_cast<fixture::Node*>(p)->line:L"";}
int target_count(void*p){int n=0;for(int i=0;i<unity::list_count(p);++i)if(!target(unity::list_at(p,i)).empty())++n;return n;}
}
namespace ui {
int cursor_index(void*p){return p?static_cast<fixture::Node*>(p)->index:-1;}
std::wstring menu_position(const std::wstring&s,int,int){return s;}
}
namespace ui::character_row {
void clear(){++fixture::clears;}
void announce(void*,void*,std::wstring line,void*){fixture::spoken.push_back(std::move(line));}
}
namespace modtext {std::wstring T(std::wstring_view s){return std::wstring(s);}}
namespace ffpr {void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}}
namespace queries {void define(const char*n,std::function<bool()>f){fixture::queries[n]=std::move(f);}}
namespace actions {void define(const char*n,Fn f){fixture::actions[n]=std::move(f);}bool run(const char*n){auto i=fixture::actions.find(n);if(i==fixture::actions.end())return false;i->second();return true;}}
namespace cursor {void add_suppressor(const char*){}}
namespace events {void on(Kind k,Handler h){fixture::events[k].push_back(std::move(h));}}
int main(){
 using namespace ffpr::menus::field_targets;using namespace fixture;
 int failures=0;const auto expect=[&](bool b,const char*m){if(!b){++failures;std::printf("FAIL %s\n",m);}};
 Void all[]={&all_init<0>,&all_init<1>,&all_init<2>,&all_init<3>,&all_init<4>,&all_init<5>};
 Void single[]={&single_init<0>,&single_init<1>,&single_init<2>,&single_init<3>,&single_init<4>,&single_init<5>};
 Void closed[]={&close<0>,&close<1>,&close<2>,&close<3>,&close<4>,&close<5>};
 Node row,cursor,controller;row.line=L"Corrad, HP 1/153, Mini";List rows{&row};cursor.parent=&row;controller.list=&rows;controller.cursor=&cursor;
 for(int i=0;i<6;++i){
  fixture::game=static_cast<game::Id>(i+1);reset();install();original_all[i]=original_single[i]=original_close[i]=&native;
  spoken.clear();all[i](&controller,nullptr);expect(spoken==std::vector<std::wstring>{L"All allies"}&&owns_range(),"all-six AllInit suppresses nested individual reads and speaks the actual group");
  single_cursor();expect(spoken.size()==1,"all-mode cursor callbacks cannot add individual character rows");
  spoken.clear();
  if(i==4){actions::run("FieldMagicTargets.SingleFF5");native(&controller,nullptr);expect(spoken==std::vector<std::wstring>{L"legacy single"}&&!owns_range(),"FFV retains its existing single-target producer rather than adding a duplicate");}
  else {single[i](&controller,nullptr);expect(spoken==std::vector<std::wstring>{row.line}&&!owns_range(),"single return speaks only the actual highlighted rendered row");}
  spoken.clear();single[i](&controller,nullptr);expect(spoken==std::vector<std::wstring>{L"legacy single"},"initial/repeated single focus preserves the existing cursor producer");
  all[i](&controller,nullptr);closed[i](&controller,nullptr);expect(!owns_range(),"native Close retires range ownership");
  menu=false;spoken.clear();all[i](&controller,nullptr);expect(spoken.empty()&&!owns_range(),"event/battle/preload context cannot announce field range");menu=true;
  controller.visible=false;spoken.clear();all[i](&controller,nullptr);expect(std::find(spoken.begin(),spoken.end(),L"All allies")==spoken.end(),"hidden native controller has no group announcement");controller.visible=true;
 }
 reset();for(const auto event:{events::Kind::MapChanging,events::Kind::MenuClosed,events::Kind::BattleEntering,events::Kind::TitleScreen}){owner=&controller;phase=Phase::All;for(auto& f:fixture::events[event])f();expect(!owns_range(),"native context exit cancels range state");}
 expect(originals>0&&clears>=6,"native originals and detail retirement remain active");
 std::printf("All-six field magic range ownership: %d failures\n",failures);return failures?1:0;
}
