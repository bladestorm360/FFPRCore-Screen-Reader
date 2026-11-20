// Actual pause-entry hook with a fake visible controller and cursor.
#include "../src/core/menus/field_pause.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace fixture {
struct Node {
    void* view = nullptr;
    void* cursor = nullptr;
    void* root = nullptr;
    void* commands = nullptr;
    std::wstring label;
    int index = 0;
    bool shown = true;
};
using List = std::vector<void*>;
bool blocked = false;
int originals = 0;
int opened_type = -1;
std::vector<frame::Task> pending;
std::vector<std::wstring> spoken;
void native_open(void*, void*, const void*) { ++originals; }
void native_open6(void*, void*, int32_t type, const void*) { ++originals; opened_type = type; }
bool open(void* node, const void*) { return static_cast<Node*>(node)->shown; }
void* text(void* node, const void*) { return node; }
void flush() {
    auto ready = std::move(pending);
    pending.clear();
    for (auto& task : ready) task();
}
}

namespace ref {
Class::Class(const char* n,const char* c,uint8_t g,const char* a):ns(n),name(c),games(g),assembly(a) {}
Field::Field(Class& c,const char* n,const char* a):cls(c),name(n),alt(a) {}
Method::Method(Class& c,const char* n,int a):cls(c),name(n),argc(a) {}
bool Class::is(void* p) { return p != nullptr; }
int32_t Field::get() {
    if (!std::strcmp(name,"view")) return offsetof(fixture::Node,view);
    if (!std::strcmp(name,"selectCommandCursor")) return offsetof(fixture::Node,cursor);
    if (!std::strcmp(name,"pauseRoot")) return offsetof(fixture::Node,root);
    if (!std::strcmp(name,"commandControllerList")) return offsetof(fixture::Node,commands);
    std::abort();
}
il2cpp::Method* Method::get() { return nullptr; }
void* Method::ptr() {
    if (!std::strcmp(name,"get_IsOpen")) return reinterpret_cast<void*>(&fixture::open);
    if (!std::strcmp(name,"get_CommandText")) return reinterpret_cast<void*>(&fixture::text);
    return nullptr;
}
void call_failed_note(const char*) { std::abort(); }
}
namespace core { Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g) {} }
namespace game { Id current() { return Id::FF6; } }
namespace hooks { bool attach_raw(ref::Method&,void*,void**) { return true; } void note_fault(const char*) { std::abort(); } }
namespace events { void on(Kind,Handler) {} }
namespace frame { void next_frame(Task t,const char*) { fixture::pending.push_back(std::move(t)); } }
namespace ffpr::system { bool input_transition_blocked() { return fixture::blocked; } }
namespace ffpr { void say_sequence(std::vector<std::wstring> lines,bool) { fixture::spoken = std::move(lines); } }
namespace modtext { std::wstring T(std::wstring_view s) { return std::wstring(s); } }
namespace ui { int cursor_index(void* p) { return static_cast<fixture::Node*>(p)->index; } }
namespace unity {
bool alive(void* p) { return p != nullptr; }
bool active_in_hierarchy(void* p) { return p && static_cast<fixture::Node*>(p)->shown; }
int list_count(void* p) { return p ? static_cast<int>(static_cast<fixture::List*>(p)->size()) : 0; }
void* list_at(void* p,int i) { return static_cast<fixture::List*>(p)->at(i); }
std::wstring ui_text(void* p) { return static_cast<fixture::Node*>(p)->label; }
}

int main() {
    using namespace ffpr::menus::field_pause;
    using namespace fixture;
    Node controller, view, root, cursor, back, controls;
    back.label = L"Return to field";
    controls.label = L"Controls";
    List rows{&back, &controls};
    view.root = &root;
    view.commands = &rows;
    controller.view = &view;
    controller.cursor = &cursor;
    original_open = &native_open;
    original_open6 = &native_open6;

    cursor.index = 1;
    on_open(&controller,nullptr,nullptr);
    if (originals != 1 || pending.size() != 1 || !spoken.empty()) return 1;
    flush();
    if (spoken != std::vector<std::wstring>{L"Pause", L"Controls"}) return 2;
    cursor.index = 0;
    on_open(&controller,nullptr,nullptr);
    flush();
    if (spoken != std::vector<std::wstring>{L"Pause", L"Return to field"}) return 3;
    spoken.clear();
    on_open(&controller,nullptr,nullptr);
    root.shown = false;
    flush();
    if (!spoken.empty()) return 4;
    root.shown = true;
    blocked = true;
    on_open(&controller,nullptr,nullptr);
    flush();
    if (!spoken.empty()) return 5;
    blocked = false;
    on_open6(&controller,nullptr,2,nullptr);
    flush();
    if (opened_type != 2 || spoken != std::vector<std::wstring>{L"Pause", L"Return to field"}) return 6;
    std::puts("Field pause: title, actual focus, reentry and native readiness verified");
}
