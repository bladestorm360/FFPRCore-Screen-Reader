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
    bindings::event_guide_context(true);fixture::single_key=int(Key::G);fixture::modifier=1;
    const int before_opera=calls;blocked=checks();
    expect(calls==before_opera&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),
        "all10 native input paths reserve the live Opera guide chord");
    bindings::event_guide_context(false);fixture::modifier=0;blocked=checks();
    expect(calls==before_opera&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),
        "all10 native input paths retain consumed primary through event/modifier end");
    fixture::single_key=-2;checks();expect(calls==before_opera+10,"all10 native paths resume only after physical release");
    bindings::owned_guide_context(false,true);fixture::single_key=int(Key::G);fixture::modifier=1;
    const int before_owned=calls;checks();expect(calls==before_owned+10,"stop-only context leaves the unavailable repeat chord with normal native ownership");
    fixture::modifier=2;blocked=checks();expect(calls==before_owned+10&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),"all10 native APIs reserve Ctrl+G for owned cancellation");
    bindings::owned_guide_context(false,false);fixture::modifier=0;checks();
    expect(calls==before_owned+10,"owned cancellation retains its physical key through context and modifier retirement");
    fixture::single_key=-2;checks();expect(calls==before_owned+20,"native ownership resumes after the actual cancel key release");
    // Battle-only read overlay must survive native item/magic Information
    // ownership without stealing these same keys on the field or in menus.
    const Key party_keys[]{Key::Slash,Key::Period,Key::Comma,Key::M,Key::N};
    for(int i=0;i<5;++i) {
        const auto action="BattleParty.Slot"+std::to_string(i+1);
        const auto label="Battle: party slot "+std::to_string(i+1)+" statistics";
        add_binding(party_keys[i],Mod::None,Context::BattleParty,action.c_str(),label.c_str());
    }
    add_binding(Key::M,Mod::None,Context::Information,"Information.M","Existing information M");
    add_binding(Key::N,Mod::None,Context::Field,"Field.N","Existing field N");
    fixture::single_key=-2;fixture::modifier=0;fixture::blocked=false;fixture::focused=true;fixture::buttons=false;fixture::trigger=0;g_state=RouterState::Normal;
    for(bool submenu:{false,true})for(int i=0;i<5;++i) {
        fixture::battle=true;fixture::information=submenu;fixture::single_key=int(party_keys[i]);fixture::last_action.clear();update();
        expect(fixture::last_action=="BattleParty.Slot"+std::to_string(i+1),"all five party checks dispatch in command and native item/magic information contexts");
        const int before_party=calls;blocked=checks();
        expect(calls==before_party&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),"all10 native APIs reserve the battle-only party check without executing a game action");
    }
    for(Key key:{Key::Alpha2,Key::Alpha3}) {
        fixture::single_key=int(key);fixture::last_action.clear();update();const int before_native=calls;auto native=checks();
        expect(fixture::last_action.empty()&&calls==before_native+10&&std::all_of(native.begin(),native.end(),[](bool v){return v;}),"native keyboard L/R2/3 neither announces a party slot nor loses native input");
    }
    fixture::single_key=int(Key::N);update();const int cached_reads=fixture::key_state_reads;
    for(int callback=0;callback<1000;++callback)checks();
    expect(fixture::key_state_reads==cached_reads,"ten thousand native callbacks read prepared ownership without polling five keys or scanning bindings again");
    fixture::battle=false;fixture::information=false;
    const char* field_actions[]{"WaypointHandler.PathfindToCurrentWaypoint","WaypointHandler.CycleNext","WaypointHandler.CyclePrevious","Mod.AnnounceCurrentMap","Field.N"};
    for(int i=0;i<5;++i) {
        const Key key=party_keys[i];
        fixture::single_key=int(key);fixture::last_action.clear();update();
        expect(fixture::last_action==field_actions[i],"outside battle existing waypoints, map report and field N retain their exact actions");
        expect(!bindings::battle_party_keyboard_in_use(),"party scope owns no physical key outside battle");
    }
    fixture::information=true;fixture::single_key=int(Key::M);fixture::last_action.clear();update();
    expect(fixture::last_action=="Information.M","non-battle information retains its existing M command");
    fixture::battle=true;fixture::key_help=true;fixture::last_action.clear();update();
    expect(fixture::last_action=="Mod.AnnounceCurrentMap"&&!bindings::battle_party_keyboard_in_use(),"key help retains its existing global M and never dispatches the party overlay");
    fixture::key_help=false;fixture::blocked=true;fixture::last_action.clear();update();
    expect(fixture::last_action.empty()&&!bindings::battle_party_keyboard_in_use(),"battle transitions never expose stale party checks");
    fixture::blocked=false;fixture::focused=false;fixture::last_action.clear();update();
    expect(fixture::last_action.empty()&&!bindings::battle_party_keyboard_in_use(),"unfocused battle never owns or dispatches party keys");
    fixture::focused=true;fixture::information=false;fixture::modifier=1;fixture::last_action.clear();update();
    expect(fixture::last_action=="Mod.ToggleMapExitFilter"&&!bindings::battle_party_keyboard_in_use(),"party shortcuts require bare keys and preserve the existing modified M chord");
    fixture::modifier=0;fixture::single_key=-2;fixture::battle=false;update();
    add_binding(Key::F20,Mod::None,Context::Field,"Field.RemapFixture","Remap fixture");
    int remap=-1;const auto& all=bindings::entries();for(size_t i=0;i<all.size();++i)if(all[i].label==L"Remap fixture")remap=int(i);
    std::wstring conflict;expect(remap>=0&&bindings::set(remap,int(Key::F17),2,conflict),"field remap fixture uses a distinct effective modifier chord");
    fixture::single_key=int(Key::F17);fixture::modifier=2;fixture::focused=true;fixture::information=false;fixture::key_help=false;fixture::battle=false;g_state=RouterState::Normal;update();
    expect(fixture::last_action=="Field.RemapFixture","actual dispatcher runs the remapped keyboard chord");const int before_remap=calls,prepared_reads=fixture::key_state_reads;blocked=checks();expect(calls==before_remap&&fixture::key_state_reads==prepared_reads&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),"all ten native APIs use cached remap ownership without reading keys again");
    fixture::modifier=0;update();blocked=checks();expect(calls==before_remap&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),"native APIs retain remapped primary when modifier is released first");
    fixture::battle=true;update();blocked=checks();expect(calls==before_remap&&std::none_of(blocked.begin(),blocked.end(),[](bool v){return v;}),"native APIs retain held primary across context handoff");
    fixture::single_key=int(Key::Count);update();auto released=checks();expect(calls==before_remap+10&&std::all_of(released.begin(),released.end(),[](bool v){return v;}),"all ten native APIs resume after full physical release");
    std::printf("F9/party context interception: actual dispatcher and10 native manager/listener detours: %d failures\n",failures);return failures?1:0;
}
