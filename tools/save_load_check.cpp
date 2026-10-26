// check-sources: text.cpp
// Real save detours/readers with owned fake native views. No save I/O or game.
#include <map>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include "../src/core/system/save_load.cpp"

namespace fixture {
struct Node {
    void* view = nullptr;
    void* popup = nullptr;
    void* cursor = nullptr;
    void* rows = nullptr;
    void* message = nullptr;
    void* title = nullptr;
    void* party = nullptr;
    bool alive = true, visible = true;
    int index = 0;
    std::wstring text;
    std::map<std::string, std::wstring> texts;
};
using List = std::vector<void*>;
Node manager;
bool menu_open = false;
int singleton_calls = 0, native_calls = 0;
void* popup_owner = nullptr;
std::map<std::string, bool> states;
std::map<std::string, void*> owners;
std::vector<std::wstring> spoken;
std::vector<std::function<void()>> delayed;
std::function<std::wstring()> popup_reader;
std::function<bool()> popup_current;
std::function<void()> native_entry;
void phase(void*, const void*) { ++native_calls; }
void open(void*, bool, void*, const void*) { ++native_calls; if (native_entry) native_entry(); }
void list_active(void*, bool, bool, bool, const void*) { ++native_calls; }
void popup_active(void*, bool, const void*) { ++native_calls; }
bool is_open(void*, const void*) { return menu_open; }
void* slot(void* p, const void*) { return p; }
void* party(void* p, const void*) { return static_cast<Node*>(p)->party; }
void* name(void* p, const void*) { return p; }
void flush() { auto tasks = std::move(delayed); delayed.clear(); for (auto& task : tasks) task(); }
void publish_popup() {
    if (!popup_reader || !popup_current || !popup_current()) return;
    auto value = popup_reader();
    if (!value.empty()) spoken.push_back(value);
}
}
namespace core { Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
bool Class::is(void* p) { return p != nullptr; }
int32_t Field::get() {
    const std::string n = name;
    if (n == "view") return offsetof(fixture::Node,view);
    if (n == "savePopup") return offsetof(fixture::Node,popup);
    if (n == "selectCursor") return offsetof(fixture::Node,cursor);
    if (n == "contentList") return offsetof(fixture::Node,rows);
    if (n == "messageText") return offsetof(fixture::Node,message);
    if (n == "titleText") return offsetof(fixture::Node,title);
    return -1;
}
void* instance(Class&) { ++fixture::singleton_calls; return &fixture::manager; }
void* Method::ptr() {
    const std::string n = name;
    if (n == "get_IsOpen") return reinterpret_cast<void*>(&fixture::is_open);
    if (n == "get_SlotData") return reinterpret_cast<void*>(&fixture::slot);
    if (n == "get_PartyList") return reinterpret_cast<void*>(&fixture::party);
    if (n == "get_Name") return reinterpret_cast<void*>(&fixture::name);
    return nullptr;
}
il2cpp::Method* Method::get() { return nullptr; }
std::wstring str(void* p) { return p ? static_cast<fixture::Node*>(p)->text : L""; }
void call_failed_note(const char*) { std::abort(); }
}
namespace hooks { bool attach_raw(ref::Method&,void*,void**) { return false; } void note_fault(const char*) { std::abort(); } }
namespace unity {
bool alive(void* p) { return p && static_cast<fixture::Node*>(p)->alive; }
bool active_in_hierarchy(void* p) { return alive(p) && static_cast<fixture::Node*>(p)->visible; }
void* transform(void* p) { return p; }
void* parent(void*) { return nullptr; }
void* get_component(void*,ref::Class&) { return nullptr; }
void* get_component_in_children(void*,ref::Class&,bool) { return nullptr; }
void* find_descendant(void*,const wchar_t*,int) { return nullptr; }
int child_count(void*) { return 0; }
void* child(void*,int) { return nullptr; }
std::wstring name(void* p) { return ref::str(p); }
std::wstring ui_text(void* p) { return ref::str(p); }
int list_count(void* p) { return p ? static_cast<int>(static_cast<fixture::List*>(p)->size()) : 0; }
void* list_at(void* p,int i) { return p && i >= 0 && i < list_count(p) ? (*static_cast<fixture::List*>(p))[i] : nullptr; }
}
namespace displayed {
bool visible(void* p) { return unity::active_in_hierarchy(p); }
bool container_visible(void* p) { return visible(p); }
void* field(void* p,const char* n) { return p && std::string(n) == "view" ? static_cast<fixture::Node*>(p)->view : nullptr; }
std::wstring text(void* p) { return visible(p) ? ref::str(p) : L""; }
std::wstring text_field(void* p,const char* n) { return visible(p) ? static_cast<fixture::Node*>(p)->texts[n] : L""; }
std::wstring pair(void* p,const char* a,const char* b) { auto x=text_field(p,a),y=text_field(p,b); return x.empty()||y.empty()?L"":x+L" "+y; }
}
namespace ffpr { void say(std::wstring_view s,bool) { fixture::spoken.emplace_back(s); } }
namespace frame { void next_frame(Task t,const char*) { fixture::delayed.push_back(std::move(t)); } }
namespace ui {
int cursor_index(void* p) { return p ? static_cast<fixture::Node*>(p)->index : -1; }
std::wstring menu_position(const std::wstring& s,int i,int n) { return s+L" ("+std::to_wstring(i+1)+L" of "+std::to_wstring(n)+L")"; }
}
namespace modtext { std::wstring T(std::wstring_view s) { return std::wstring(s); } }
namespace menu_state {
void set_active(const char* n,bool v) { fixture::states[n]=v; }
bool is_active(const char* n) { return fixture::states[n]; }
void reset(const char* n) { fixture::states[n]=false; }
}
namespace field::cache { void* get(ref::Class& c) { return fixture::owners[c.name]; } void register_object(ref::Class& c,void* p) { fixture::owners[c.name]=p; } }
namespace queries { void define(const char*,std::function<bool()>) {} }
namespace log_ { void write(const char*,...) {} }
namespace ffpr::dialogue {
std::wstring read_focused_button(void* p,ref::Field&,ref::Field&) { return displayed::text_field(p,"button"); }
void read_popup_when_ready(void*,const char*,std::function<std::wstring()> reader,std::function<bool()> current) {
    fixture::popup_reader=std::move(reader); fixture::popup_current=std::move(current);
}
namespace popup_state {
void* active_popup() { return fixture::popup_owner; }
void set_active(const char*,void* p,ref::Field*,bool) { fixture::popup_owner=p; fixture::popup_reader={}; }
void clear() { fixture::popup_owner=nullptr; fixture::popup_reader={}; }
}
}

int main() {
    using namespace fixture;
    namespace s = ffpr::system;
    int failures=0;
    auto expect=[&](bool ok,const char* why) { if(!ok) { ++failures; std::printf("FAIL %s\n",why); } };
    s::o_script_open=&open; s::o_script_close=s::o_script_select=s::o_script_overwrite=s::o_script_complete=&phase;
    s::o_SaveListSetActive=&list_active;
    s::o_TitleLoadSetPopupActive=s::o_MenuLoadSetPopupActive=s::o_MenuSaveSetPopupActive=s::o_InterruptionSetEnablePopup=&popup_active;
    s::o_InterruptionInitComplite=&phase;
    for (int game=1;game<=6;++game) {
        Node owner,view,list,cursor,row,row_view,empty,empty_view,popup,message,title,other;
        List rows{&row,&empty}; owner.view=&view;view.popup=&popup;list.cursor=&cursor;list.rows=&rows;
        row.view=&row_view;empty.view=&empty_view;popup.message=&message;popup.title=&title;
        row_view.texts={{"slotNameText",L"File"},{"slotNumText",L"1"},{"timeStampDate",L"10/24/2026"},
            {"timeStampTime",L"13:19"},{"charaNameText",L"Firion"},{"areaNameText",L"World Map"},
            {"fixedTimeText",L"Time"},{"hourText",L"33"},{"timeSeparateText",L":"},{"minuteText",L"14"}};
        empty_view.texts={{"slotNameText",L"File"},{"slotNumText",L"2"},{"emptyText",L"Empty"},{"areaNameText",L"stale map"}};
        menu_open=false;owners.clear();spoken.clear();singleton_calls=0;
        native_entry=[&] { s::d_script_select(&owner,nullptr);s::d_SaveListSetActive(&list,true,true,false,nullptr); };
        s::d_script_open(&owner,true,nullptr,nullptr); flush();
        expect(spoken.size()==1 && spoken[0]==L"File 1, 10/24/2026, 13:19, Firion, World Map, Time 33:14 (1 of 2)","scripted/completed save reads the ready row outside MenuManager");
        expect(singleton_calls==0,"completed save context performs no singleton discovery");
        int count=-1;cursor.index=1;
        expect(s::try_read_save_slot(&cursor,1,count)==L"File 2: Empty" && count==2,"empty slot has its own number and excludes stale data");
        expect(s::try_read_save_slot(&other,0,count).empty(),"serialized row reader owns only its exact cursor");
        message.text=L"Overwrite save data?";title.text=L"Completed Game Save";popup.texts["button"]=L"No";
        spoken.clear();s::d_script_overwrite(&owner,nullptr);publish_popup();
        expect(spoken.size()==1 && spoken[0]==L"Overwrite save data? No" && s::save_load_should_suppress(),"overwrite body precedes initial No button");
        s::d_script_select(&owner,nullptr);expect(!popup_reader && !s::save_load_should_suppress(),"return to slot list retires old popup and suppression");
        for (const auto* result : {L"Save complete.",L"Could not save data."}) {
            spoken.clear();message.text.clear();message.visible=false;title.text=result;popup.texts["button"]=L"Back";
            s::d_script_complete(&owner,nullptr);publish_popup();
            expect(spoken.size()==1 && spoken[0]==std::wstring(result)+L" Back","completion reads actual success or failure title with hidden blank body");
        }
        spoken.clear();message.text=L"not rendered";message.visible=false;s::d_script_overwrite(&owner,nullptr);publish_popup();
        expect(spoken.empty(),"hidden nonempty body cannot fall back to an unrelated title");
        s::d_script_close(&owner,nullptr);publish_popup();expect(!popup_reader && !s::should_read_save_slot(),"closing retires pending popup and endgame owner");
        // Preserve existing title-load, menu-load/save and quick-save producers.
        message.visible=true;message.text=L"Proceed?";popup.texts["button"]=L"No";owner.popup=&popup;
        for (auto detour : {&s::d_TitleLoadSetPopupActive,&s::d_MenuLoadSetPopupActive,&s::d_MenuSaveSetPopupActive,&s::d_InterruptionSetEnablePopup}) {
            spoken.clear();detour(&owner,true,nullptr);publish_popup();
            expect(spoken.size()==1&&spoken[0]==L"Proceed? No","existing alternate popup path still reads body and choice");
            detour(&owner,false,nullptr);expect(!popup_reader,"existing close cancels body reader");
        }
        spoken.clear();message.text.clear();message.visible=false;title.text=L"Save complete.";
        s::d_InterruptionInitComplite(&owner,nullptr);publish_popup();
        expect(spoken.size()==1&&spoken[0]==L"Save complete. No","quicksave in-place completion remains audible");s::clear_popup_state();
        s::g_script_save=&owner;owner.alive=false;expect(!s::should_read_save_slot(),"destroyed scripted owner cannot permit background autosave speech");
        s::g_script_save=nullptr;s::g_save_list=nullptr;native_entry={};
        std::printf("FF%d: script/endgame and title/menu/quick-save producer cases checked\n",game);
    }
    std::printf("Save paths: %d failures\n",failures);return failures?1:0;
}
