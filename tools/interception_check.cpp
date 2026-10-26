// check-sources: input/bindings.cpp text.cpp
// Actual keyboard/controller dispatcher and all10 native suppression detours.
#define main retained_transition_main
#define interception_enabled retained_interception_enabled
#define update_native_input_ownership retained_update_native_input_ownership
#include "transition_input_check.cpp"
#undef update_native_input_ownership
#undef interception_enabled
#undef main
#define install interception_install
#define s_install interception_registration
#include "../src/core/input/input_passthrough.cpp"
#undef s_install
#undef install
namespace ref {std::wstring rd_str(void*,Field&){return L"Down";}}
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return false;}void note_fault(const char*){std::abort();}}
namespace settings {void set_bool(const char*,bool){}}
namespace ffpr::fieldnav::autowalk {bool virtual_key(int,bool){return false;}bool virtual_input_active(){return false;}}
namespace input {
bool button_released(int){return false;}
namespace gamepad {bool lstick_up_held(){return false;}bool lstick_down_held(){return false;}bool lstick_left_held(){return false;}bool lstick_right_held(){return false;}}
}
namespace {
int calls=0;
bool key(void*,int32_t,const void*){++calls;return true;}
bool any(void*,const void*){++calls;return true;}
bool down(void*,void*,int32_t,bool,bool,const void*){++calls;return true;}
bool held(void*,void*,int32_t,bool,const void*){++calls;return true;}
bool anyheld(void*,int32_t,const void*){++calls;return true;}
}
int main() {
    using namespace input;
    int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    o_GetKeyDown=o_GetKey=o_GetKeyUp=&key;o_GetAnyKey=&any;
    o_listener_down=&down;o_listener_held=o_listener_up=&held;o_any_held=&anyheld;o_any_down=o_any_up=&any;
    auto checks=[] {return std::vector<bool>{d_GetKeyDown(nullptr,5,nullptr),d_GetKey(nullptr,5,nullptr),d_GetKeyUp(nullptr,5,nullptr),d_GetAnyKey(nullptr,nullptr),
        d_listener_down(nullptr,nullptr,0,false,false,nullptr),d_listener_held(nullptr,nullptr,0,false,nullptr),d_listener_up(nullptr,nullptr,0,false,nullptr),
        d_any_held(nullptr,0,nullptr),d_any_down(nullptr,nullptr),d_any_up(nullptr,nullptr)};};
    g_state=RouterState::ModMode;g_interception=fixture::interception=true;auto blocked=checks();
    expect(calls==0&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),"interception on retains native prefix ownership at every manager/listener entry");
    g_interception=fixture::interception=false;auto passthrough=checks();
    expect(calls==10&&std::all_of(passthrough.begin(),passthrough.end(),[](bool v){return v;}),"interception off forwards all10 exact native signatures once and preserves native scrolling");
    g_state=RouterState::ModMenu;blocked=checks();
    expect(calls==10&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),"the mod's restoration dialog retains exclusive controls while game shortcuts are suspended");
    g_state=RouterState::Normal;fixture::blocked=false;fixture::single_key=int(Key::F9);fixture::modifier=0;
    initialize_bindings();update();expect(fixture::last_action=="Input.ToggleControlInterception","F9 recovery dispatch works while suspended, before game-menu/modal routing");
    fixture::single_key=int(Key::DownArrow);const int before=fixture::actions;
    input::bindings::begin_frame();input::bindings::context(Context::BestiaryDetail);dispatch_registered_bindings(Context::BestiaryDetail);
    expect(fixture::actions==before,"suspended conflicting Bestiary arrows pass through without firing the mod navigation shortcut");
    fixture::interception=g_interception=true;
    input::bindings::begin_frame();input::bindings::context(Context::BestiaryDetail);dispatch_registered_bindings(Context::BestiaryDetail);
    expect(fixture::actions>before,"restoring interception restores the same mod navigation bindings");
    std::printf("F9 interception: actual dispatcher and10 native manager/listener detours: %d failures\n",failures);return failures?1:0;
}
