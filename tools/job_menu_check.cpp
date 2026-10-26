// check-sources: text.cpp
// Actual FFIII job focus/detail consumer; native rendering and metadata are fakes.
#include <map>
#include <cstdio>
#include <cstdlib>
#include "../src/games/ff3/job_menu.cpp"
namespace fixture {
struct Node {void* view=nullptr;void* level=nullptr;int id=1;bool alive=true,visible=true;std::wstring text;std::map<std::string,void*> fields;std::map<std::string,std::wstring> texts;};
using List=std::vector<void*>;
bool active=false,autodetail=false;std::wstring previous;
std::vector<frame::Task> tasks;std::vector<std::wstring> spoken;
int32_t id(void*p,const void*){return p?static_cast<Node*>(p)->id:0;}
void* name(void*p,const void*){return p;}
void flush(){auto pending=std::move(tasks);tasks.clear();for(auto&t:pending)t();}
}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
bool Class::is(void*p){return p!=nullptr;}
int32_t Field::get(){const std::string n=name;if(n=="view")return offsetof(fixture::Node,view);if(n=="infoSkillLevelValueText"||n=="skillLevelValueText")return offsetof(fixture::Node,level);return -1;}
void*Method::ptr(){const std::string n=name;if(n=="get_Id"||n=="get_JobId")return reinterpret_cast<void*>(&fixture::id);if(n=="get_MesIdName")return reinterpret_cast<void*>(&fixture::name);return nullptr;}
il2cpp::Method*Method::get(){return nullptr;}
std::wstring str(void*p){return p?static_cast<fixture::Node*>(p)->text:L"";}void call_failed_note(const char*){std::abort();}
}
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return false;}void note_fault(const char*){std::abort();}}
namespace log_ {void write(const char*,...){} }
namespace unity {
bool alive(void*p){return p&&static_cast<fixture::Node*>(p)->alive;}
bool active_in_hierarchy(void*p){return alive(p)&&static_cast<fixture::Node*>(p)->visible;}
std::wstring ui_text(void*p){return ref::str(p);}
int list_count(void*p){return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}
void*list_at(void*p,int i){return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}
}
namespace displayed {
void*field(void*p,const char*n){if(!p)return nullptr;auto&f=static_cast<fixture::Node*>(p)->fields;auto it=f.find(n);return it==f.end()?nullptr:it->second;}
std::wstring text_field(void*p,const char*n){return unity::active_in_hierarchy(p)?static_cast<fixture::Node*>(p)->texts[n]:L"";}
std::wstring equipment_icon(void*p){return unity::active_in_hierarchy(p)?ref::str(p):L"";}
}
namespace frame {void next_frame(Task t,const char*){fixture::tasks.push_back(std::move(t));}}
namespace ffpr {
void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}
void say_sequence(std::vector<std::wstring> s,bool){for(auto&line:s)fixture::spoken.push_back(std::move(line));}
}
namespace ffpr::status {std::wstring game_message(const std::wstring&s,bool){return s;}}
namespace settings {bool get_bool(const char*,bool){return fixture::autodetail;}}
namespace modtext {std::wstring T(std::wstring_view s){return std::wstring(s);}std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring>a){return text::format(s,a);}}
namespace menu_state {
void set_active_exclusive(const char*){fixture::active=true;}
bool is_active(const char*){return fixture::active;}
void reset(const char*){fixture::active=false;ffpr::ff3::reset_job_menu();}
void register_reset(const char*,std::function<void()>){ }
}
namespace dedup {bool text(const char*,std::wstring_view s,Scope){if(fixture::previous==s)return false;fixture::previous=s;return true;}void reset(const char*){fixture::previous.clear();}}
namespace events {void on(Kind,Handler){}}
namespace queries {void define(const char*,std::function<bool()>){}void define_text(const char*,std::function<std::wstring()>){} }
namespace cursor {void add_suppressor(const char*){} }
int main(){using namespace fixture;namespace j=ffpr::ff3;int failures=0;auto expect=[&](bool ok,const char*why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
Node owner,view,level,character,job,sword,bow,shield;level.text=L"1";view.level=&level;owner.view=&view;character.id=1;job.id=2;job.text=L"Warrior";sword.text=L"Swords";bow.text=L"Bows";shield.text=L"Shields";
List icons{&sword,&bow,&shield};view.fields["infoEquipIconList"]=&icons;view.texts["infoFixedEquipText"]=L"Equippable";
// New icons can still have the preceding layout's culling state at native update.
for(auto*p:icons)static_cast<Node*>(p)->visible=false;
j::on_update_job_info(&owner,&character,&job);expect(spoken.size()==1,"Autodetail off speaks only focused job initially");
for(auto*p:icons)static_cast<Node*>(p)->visible=true;
expect(j::current_job_description()==L"Equippable: Swords, Bows, Shields","I refreshes the now-rendered icon row instead of a stale heading-only cache");
bow.visible=false;expect(j::current_job_description()==L"Equippable: Swords, Shields","hidden icons remain absent");bow.visible=true;
view.texts["infoAllEquipText"]=L"All";expect(j::current_job_description()==L"Equippable: All","native All text replaces hidden pooled icons");view.texts["infoAllEquipText"].clear();
j::reset_job_menu();expect(j::current_job_description().empty(),"closed job context has no stale description");
autodetail=true;spoken.clear();j::on_update_job_info(&owner,&character,&job);expect(spoken.empty()&&tasks.size()==1,"Autodetail owns one native-event deferral");flush();
expect(spoken.size()==2&&spoken[1]==L"Equippable: Swords, Bows, Shields","Autodetail keeps job heading before complete equipment detail");
j::reset_job_menu();spoken.clear();j::on_update_job_info(&owner,&character,&job);j::reset_job_menu();flush();expect(spoken.empty(),"closing cancels pending detail before native reads");
spoken.clear();j::on_update_job_info(&owner,&character,&job);job.text=L"White Mage";j::on_update_job_info(&owner,&character,&job);flush();expect(spoken.size()==2&&spoken[0].find(L"White Mage")==0,"new focus supersedes obsolete job detail");
owner.alive=false;expect(j::current_job_description().empty(),"destroyed native job owner cannot be read");
std::printf("FFIII actual job detail/focus: %d failures\n",failures);return failures?1:0;}
