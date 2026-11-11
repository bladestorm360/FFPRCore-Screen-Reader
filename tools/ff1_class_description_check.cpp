// check-sources: text.cpp
// Actual FF1 adapter with a separate serialized description panel, as native.
#ifndef FFPR_CLASS_READER_SOURCE
#define FFPR_CLASS_READER_SOURCE "../src/games/ff1/job_selection.cpp"
#endif
#include FFPR_CLASS_READER_SOURCE
#include <map>
#include <cstring>
#include <cstdio>
namespace fixture {
struct Node {void* contents=nullptr;void* view=nullptr;void* panel=nullptr;bool alive=true,active=true;int index=0;std::wstring value;std::map<std::string,std::wstring> text;};
using List=std::vector<void*>;
std::vector<frame::Task> tasks;std::vector<std::wstring> spoken;
std::map<std::string,std::function<bool()>> query_handlers;
std::map<std::string,std::function<std::wstring()>> texts;
std::map<std::string,actions::Fn> actions;
std::map<events::Kind,std::vector<events::Handler>> handlers;
bool autodetail=false;int tree_scans=0,description_reads=0;
void flush(){auto pending=std::move(tasks);tasks.clear();for(auto& t:pending)t();}
}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
bool Class::is(void*p){return p!=nullptr;}
int32_t Field::get(){if(std::strcmp(name,"contentList")==0)return offsetof(fixture::Node,contents);if(std::strcmp(name,"view")==0)return offsetof(fixture::Node,view);if(std::strcmp(name,"discriptionParent")==0)return offsetof(fixture::Node,panel);return -1;}
}
namespace unity {
bool alive(void*p){return p&&static_cast<fixture::Node*>(p)->alive;}
bool active_in_hierarchy(void*p){return alive(p)&&static_cast<fixture::Node*>(p)->active;}
int list_count(void*p){return p?int(static_cast<fixture::List*>(p)->size()):0;}
void* list_at(void*p,int i){return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}
std::wstring ui_text(void*p){return p?static_cast<fixture::Node*>(p)->value:L"";}
std::vector<void*> texts(void*p,bool){++fixture::tree_scans;std::vector<void*> out;if(p)for(auto row:*static_cast<fixture::List*>(static_cast<fixture::Node*>(p)->contents))out.push_back(static_cast<fixture::Node*>(row)->view);return out;}
}
namespace displayed {
bool container_visible(void*p){return unity::active_in_hierarchy(p);}
std::wstring text_field(void*p,const char* name){if(std::strcmp(name,"discriptionText")==0)++fixture::description_reads;return unity::active_in_hierarchy(p)?static_cast<fixture::Node*>(p)->text[name]:L"";}
}
namespace ui {int cursor_index(void*p){return p?static_cast<fixture::Node*>(p)->index:-1;}std::wstring menu_position(const std::wstring&s,int,int){return s;}}
namespace settings {bool get_bool(const char*,bool){return fixture::autodetail;}}
namespace frame {void next_frame(Task t,const char*){fixture::tasks.push_back(std::move(t));}}
namespace ffpr {void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}}
namespace queries {void define(const char*n,std::function<bool()>f){fixture::query_handlers[n]=std::move(f);}void define_text(const char*n,std::function<std::wstring()>f){fixture::texts[n]=std::move(f);}}
namespace actions {void define(const char*n,Fn f){fixture::actions[n]=std::move(f);}}
namespace events {void on(Kind k,Handler h){fixture::handlers[k].push_back(std::move(h));}}
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return false;}void note_fault(const char*){std::abort();}}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace log_ {void write(const char*,...){} }
int main(){using namespace fixture;namespace reader=ffpr::ff1;
 int cases=0,failures=0;const auto expect=[&](bool ok,const char* why){++cases;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 reader::install();Node controller,panel,cursor,warrior,mage,warrior_view,mage_view;warrior.view=&warrior_view;mage.view=&mage_view;warrior_view.value=L"Warrior";mage_view.value=L"White Mage";warrior_view.text["jobNameText"]=warrior_view.value;mage_view.text["jobNameText"]=mage_view.value;List rows{&warrior,&mage};controller.contents=&rows;controller.panel=&panel;
 controller.text["discriptionText"]=L"Strong.";reader::on_select_content(&controller,&cursor);flush();
 expect(spoken==std::vector<std::wstring>{L"Warrior"},"Autodetail off reads the selected rendered name only");
 const auto detail=[]{return fixture::texts.at("JobSelectionPatches.GetCurrentDescription")();};
 expect(query_handlers.at("JobSelectionPatches.IsActive")()&&detail()==L"Strong.","I reaches the dedicated sibling panel, including descriptions shorter than ten characters");
 expect(detail()==L"Strong."&&detail()==L"Strong.","every deliberate I request remains available");
 autodetail=true;spoken.clear();reader::on_select_content(&controller,&cursor);flush();
 expect(spoken==std::vector<std::wstring>{L"Warrior. Strong."},"Autodetail includes the same current native description");
 spoken.clear();reader::on_select_content(&controller,&cursor);cursor.index=1;controller.text["discriptionText"]=L"Healing magic.";reader::on_select_content(&controller,&cursor);flush();
 expect(spoken==std::vector<std::wstring>{L"White Mage. Healing magic."},"rapid focus cancels the older class announcement");
 panel.active=false;expect(detail().empty(),"hidden description panel never exposes stored text");panel.active=true;
 controller.active=false;expect(!query_handlers.at("JobSelectionPatches.IsActive")()&&detail().empty(),"closed native list retires manual detail access");controller.active=true;
 controller.text["discriptionText"].clear();expect(detail().empty(),"blank current description never retains the previous job");
 for(const auto& localized:std::vector<std::wstring>{L"戦士",L"Guerrier",L"Guerriero",L"Krieger",L"Guerrero",L"전사",L"戰士",L"战士",L"Воин",L"Guerreiro",L"นักรบ"}){
  cursor.index=0;warrior_view.text["jobNameText"]=localized;controller.text["discriptionText"]=L"localized native description";spoken.clear();reader::on_select_content(&controller,&cursor);flush();expect(spoken==std::vector<std::wstring>{localized+L". localized native description"},"class reader accepts native localized labels without an English whitelist");
 }
 spoken.clear();reader::on_select_content(&controller,&cursor);reader::reset_state();flush();expect(spoken.empty()&&detail().empty(),"reset cancels deferred speech and manual state");
 spoken.clear();reader::on_select_content(&controller,&cursor);controller.alive=false;const auto reads=description_reads;flush();expect(spoken.empty()&&detail().empty()&&description_reads==reads,"destroyed controller cancels before description reads");controller.alive=true;
 for(int index:{-1,2,99}){cursor.index=index;reader::on_select_content(&controller,&cursor);expect(!query_handlers.at("JobSelectionPatches.IsActive")(),"invalid native cursor never retains an old class");}
 List excess(13,&warrior);controller.contents=&excess;cursor.index=0;reader::on_select_content(&controller,&cursor);expect(!query_handlers.at("JobSelectionPatches.IsActive")(),"over-budget native row pool is rejected");
 controller.contents=&rows;
 for(auto event:{events::Kind::MapChanging,events::Kind::TitleScreen,events::Kind::SceneLoaded}){
  reader::on_select_content(&controller,&cursor);spoken.clear();for(const auto& callback:handlers[event])callback();flush();
  expect(spoken.empty()&&detail().empty(),"native scene/field/title lifetime retires deferred and manual class state");
 }
 expect(tree_scans==0,"ready selection and I perform no hierarchy scan");
 std::printf("FF1 actual class descriptions: %d cases/%d failures; hierarchy scans%d\n",cases,failures,tree_scans);return failures?1:0;
}
