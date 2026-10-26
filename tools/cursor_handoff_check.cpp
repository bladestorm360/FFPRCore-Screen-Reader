// check-sources: text.cpp
// Actual deferred generic cursor consumer: a specific row can claim ownership
// after scheduling. Preserve ordinary fallback and equal-text repeated events.
#include <cstdio>
#include <cstdlib>
#include "../src/core/system/menu_text_discovery.cpp"
namespace fixture {
struct Node {std::wstring text=L"Corrad";bool alive=true;};
bool owned=false,map_owned=false;std::vector<frame::Task> pending;std::vector<std::wstring> spoken;
}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
}
namespace unity {
bool alive(void*p){return p&&static_cast<fixture::Node*>(p)->alive;}
void* game_object(void*p){return p;}void* transform(void*p){return p;}void* parent(void*){return nullptr;}
std::wstring name(void*){return L"nameText";}
ref::Class& text_class(){static ref::Class c{"UnityEngine.UI","Text"};return c;}
void* get_component(void*p,ref::Class&){return p;}
void* get_component_in_children(void*p,ref::Class&,bool){return p;}
std::wstring ui_text(void*p){return p?static_cast<fixture::Node*>(p)->text:L"";}
int child_count(void*){return 0;}void* child(void*,int){return nullptr;}
std::vector<void*> components_in_children(void*,ref::Class&,bool){return {};}
}
namespace queries {
bool is(const char*n){return (std::string(n)=="ItemMenuState.ShouldSuppress"&&fixture::owned)||(std::string(n)=="MapPanels.IsActive"&&fixture::map_owned);}
std::wstring text(const char*){return {};}
}
namespace hooks {void note_fault(const char*){std::abort();}}
namespace frame {void next_frame(Task a,const char*){fixture::pending.push_back(std::move(a));}}
namespace cursor {const CursorEvent& current(){static CursorEvent e;return e;}}
namespace ffpr::system {
bool should_read_save_slot(){return false;}
std::wstring try_read_save_slot(void*,int,int&){return {};}
int field_command_count(void*){return -1;}
void with_cursor_event(void*,int,void*,void(*fn)(void*),void*state){fn(state);}
}
namespace ffpr::dialogue {int title_active_command_count(void*){return -1;}}
namespace ui {int cursor_index(void*){return 0;}std::wstring menu_position(const std::wstring&s,int,int){return s;}}
namespace ffpr {void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}}
int main(){using namespace fixture;int failures=0;
auto expect=[&](bool ok,const char*why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
auto drain=[] {auto work=std::move(pending);pending.clear();for(auto&f:work)f();};
Node row;
 owned=false;spoken.clear();ffpr::system::wait_and_read_cursor(&row);
 owned=true;drain();expect(spoken.empty(),"specific target takes deferred ownership; no later name-only read");
 owned=false;ffpr::system::wait_and_read_cursor(&row);drain();
 expect(spoken==std::vector<std::wstring>{L"Corrad"},"native owner releases to ordinary generic reader");
 ffpr::system::wait_and_read_cursor(&row);drain();
 expect(spoken==std::vector<std::wstring>{L"Corrad",L"Corrad"},"legitimate equal-text focus events are retained");
 ffpr::system::wait_and_read_cursor(&row);map_owned=true;drain();expect(spoken.size()==2,"native map claims pending generic location read without text deduplication");map_owned=false;
 row.alive=false;ffpr::system::wait_and_read_cursor(&row);drain();expect(spoken.size()==2,"destroyed cursor never reads stale UI");row.alive=true;
std::printf("Deferred generic/specific cursor handoff: %d failures\n",failures);return failures?1:0;
}
