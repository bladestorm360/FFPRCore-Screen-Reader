#include "bindings.h"
#include "input_internal.h"
#include "../settings.h"
#include "../text.h"
#include "../modtext.h"
#include "../say.h"
#include "../ui/preferences.h"
#include <algorithm>
#include <cmath>

namespace input::bindings {
namespace {
std::vector<Entry>& catalog() { static auto* v = new std::vector<Entry>; return *v; }
bool current[kPadInputs]{}, previous[kPadInputs]{}, owned[kPadInputs]{};
int capture_index = -1;
int capture_cancel_index=-1;
int capture_pad_first=-1;
bool capture_ready = false;
Context game_context=Context::Global;
bool pad_ready=false,had_pad=false;
bool allow_keyboard=true;
bool keyboard_owned[int(Key::Count)]{},remapped_keyboard_owned=false;
NativeKeyboardCheck game_keyboard_check=nullptr;
NativeKeyboardResult last_native_failure=NativeKeyboardResult::Clear;
bool party_context=false,party_keyboard_owned=false;
int party_bindings[5]{-1,-1,-1,-1,-1};
bool event_guide=false;
bool owned_guide_repeat=false,owned_guide_stop=false;
int guide_bindings[2]{-1,-1},guide_owned[2]{-1,-1};
std::string setting(const Entry& e, const char* suffix) { return "Binding." + e.id + suffix; }
int mods() { return (shift_held()?1:0) | (ctrl_held()?2:0) | (alt_held()?4:0); }
int modifier_bit(int primary) {
    return primary==int(Key::LeftShift)||primary==int(Key::RightShift)?1:
        primary==int(Key::LeftControl)||primary==int(Key::RightControl)?2:
        primary==int(Key::LeftAlt)||primary==int(Key::RightAlt)?4:0;
}
int chord_mods(int primary,int actual){return actual&~modifier_bit(primary);}
bool modifier_key(int k) { return k >= int(Key::LeftShift) && k <= int(Key::RightWindows); }
bool enabled(const Entry& e) { return e.layout < 0 || e.layout == ui::prefs::navigation_keymap(); }
bool scope_matches(const Entry& e,Context context) {
    if(e.scope==int(Context::BattleParty))return party_context;
    return e.scope==0||e.scope==int(context);
}
bool recovery(const Entry& e){return e.id=="key.0.Toggle control interception.-1";}
void refresh_remapped_ownership(bool admit=true) {
    remapped_keyboard_owned=false;
    const bool focus=game_window_focused();
    for(int key=0;key<int(Key::Count);++key)if(keyboard_owned[key]) {
        keyboard_owned[key]=focus&&key_in_use(Key(key));
        remapped_keyboard_owned|=keyboard_owned[key];
    }
    if(!admit||!focus||!allow_keyboard)return;
    const int actual=mods();
    for(const auto& e:catalog())if(!e.controller&&scope_matches(e,game_context)&&enabled(e)&&e.primary>=0&&
        (e.primary!=e.default_primary||e.modifier!=e.default_modifier)&&chord_mods(e.primary,actual)==e.modifier&&key_in_use(Key(e.primary))) {
        keyboard_owned[e.primary]=true;remapped_keyboard_owned=true;
    }
}
void refresh_party_ownership() {
    party_keyboard_owned=false;
    if(!party_context||!allow_keyboard)return;
    const int actual=mods();
    for(const int index:party_bindings)if(index>=0) {
        const auto& e=catalog()[static_cast<size_t>(index)];
        if(enabled(e)&&e.primary>=0&&chord_mods(e.primary,actual)==e.modifier&&key_in_use(Key(e.primary))&&game_window_focused()) {
            party_keyboard_owned=true;return;
        }
    }
}
bool down(const Entry& e, bool edge) {
    if (!game_window_focused() || !enabled(e) || e.primary < 0) return false;
    if (!e.controller) {
        const int actual=chord_mods(e.primary,mods());
        return actual == e.modifier && (edge ? key_pressed(Key(e.primary)) : key_held(Key(e.primary)));
    }
    return current[e.primary] && (e.modifier < 0 || current[e.modifier]) && (!edge || !previous[e.primary] || (e.modifier >= 0 && !previous[e.modifier]));
}
int find(const char* id) { const auto& v=catalog(); for(size_t i=0;i<v.size();++i) if(v[i].id==id) return int(i); return -1; }
void own(const Entry& e) {
    if(e.primary>=0 && (current[e.primary] || previous[e.primary])) owned[e.primary]=true;
    if(e.modifier>=0 && current[e.modifier]) owned[e.modifier]=true;
}
}
int add(const char* id, const char* label, bool controller, int scope, int primary, int modifier, int layout) {
    if(int i=find(id); i>=0) return i;
    Entry e{id,text::widen(label),controller,scope,layout,primary,modifier,primary,modifier};
    const int p=settings::get_int(setting(e,".Input").c_str(),primary);
    const int m=settings::get_int(setting(e,".Modifier").c_str(),modifier);
    if(p>=-1 && p<(controller?kPadInputs:int(Key::Count)) && m>=(controller?-1:0) && m<(controller?kPadInputs:8) && (!controller || p<0 || p!=m) && (controller||chord_mods(p,m)==m)) { e.primary=p; e.modifier=m; }
    catalog().push_back(std::move(e));const int index=int(catalog().size()-1);
    if(scope==int(Context::BattleParty))for(int& saved:party_bindings)if(saved<0){saved=index;break;}
    return index;
}
void initialize_keyboard_controls() {
    capture_cancel_index=add("key.capture.cancel","Cancel binding capture",false,-4,int(Key::Escape));
    if(catalog()[capture_cancel_index].primary<0)reset(capture_cancel_index);
}
const std::vector<Entry>& entries() { return catalog(); }
std::wstring scope_name(int scope) {
    if(scope==int(Context::BattleParty))return T(L"Battle party");
    const wchar_t* names[]={L"Global",L"Field",L"Battle",L"Shop",L"Status",L"Bestiary",L"Results",L"Controls",L"Information"};
    if(scope>=0 && scope<9)return T(names[scope]);
    switch(scope) {
    case -1:case -18:return T(L"Mod menu");
    case -2:return T(L"Confirmation");case -3:return T(L"Text input");case -4:return T(L"Binding capture");
    case -10:return T(L"Global");case -11:return T(L"Field");case -12:return T(L"Information");
    case -13:return T(L"Puzzle");case -14:return T(L"Mod mode: dialogue");case -15:return T(L"Mod mode: results");
    case -16:return T(L"Mod mode: battle");case -17:return T(L"Mod mode: field");default:return T(L"Unknown");
    }
}
std::wstring input_name(bool pad, int p) {
    if(p<0) return T(L"Unbound");
    if(!pad) return text::widen(key_name(Key(p)));
    if(p<kButtonCount) return button_label(p);
    const wchar_t* names[]={L"Left trigger",L"Right trigger",L"Left stick up",L"Left stick down",L"Left stick left",L"Left stick right",L"Right stick up",L"Right stick down",L"Right stick left",L"Right stick right"};
    return T(names[p-kButtonCount]);
}
std::wstring value(int index) {
    const auto& e=catalog().at(index); std::wstring s;
    if(e.primary<0) return T(L"Unbound");
    if(e.controller) { if(e.modifier>=0) s=input_name(true,e.modifier)+L" + "; }
    else { if(e.modifier&2)s+=L"Control + "; if(e.modifier&1)s+=L"Shift + "; if(e.modifier&4)s+=L"Alt + "; }
    return s+input_name(e.controller,e.primary);
}
bool set(int index, int p, int m, std::wstring& conflict) {
    auto& e=catalog().at(index);
    last_native_failure=NativeKeyboardResult::Clear;
    if(p < -1 || p >= (e.controller?kPadInputs:int(Key::Count)) || m < (e.controller?-1:0) || m >= (e.controller?kPadInputs:8) || (e.controller && p>=0 && p==m)) return false;
    if(!e.controller&&chord_mods(p,m)!=m)return false;
    if(e.id=="key.capture.cancel"&&p<0){conflict=e.label;return false;}
    for(size_t i=0;p>=0 && i<catalog().size();++i) {
        const auto& other=catalog()[i];
        if(int(i)==index || other.controller!=e.controller || other.primary<0) continue;
        bool same=other.primary==p && other.modifier==m;
        if(!e.controller) {
            const int primary_bit=modifier_bit(p),other_bit=modifier_bit(other.primary);
            same|=primary_bit&&(other.modifier&primary_bit)&&chord_mods(p,other.modifier)==m;
            same|=other_bit&&(m&other_bit)&&chord_mods(other.primary,m)==other.modifier;
        }
        if(e.controller) {
            // A global mode/menu button must never fire underneath a chord.
            same|=m<0 && (p==other.primary || p==other.modifier);
            same|=other.modifier<0 && (other.primary==p || other.primary==m);
            same|=m>=0 && other.modifier>=0 && other.primary==m && other.modifier==p;
        }
        if(!same)continue;
        if(e.layout>=0 && other.layout>=0 && e.layout!=other.layout) continue;
        if(p==e.default_primary && m==e.default_modifier && other.primary==other.default_primary && other.modifier==other.default_modifier && e.scope!=other.scope && !e.controller)continue;
        const bool original_click_chord=e.controller && e.scope==-11 && other.scope==-11 &&
            (e.id=="pad.field.normalize" || other.id=="pad.field.normalize") &&
            p==e.default_primary && m==e.default_modifier && other.primary==other.default_primary && other.modifier==other.default_modifier;
        if(original_click_chord)continue;
        // Global overlaps game contexts, but menu/modal contexts own their own inputs.
        const bool controller_global=e.controller && (e.scope==-10 || other.scope==-10) && e.scope!=-2 && e.scope!=-3 && other.scope!=-2 && other.scope!=-3;
        const auto party_overlap=[](int a,int b){return a==int(Context::BattleParty)&&
            (b==int(Context::Battle)||b==int(Context::Information)||b==int(Context::Status));};
        const bool recovery_overlap=!e.controller&&(recovery(e)||recovery(other))&&e.scope!=-4&&other.scope!=-4;
        if(e.scope!=other.scope && !(e.scope>=0 && other.scope>=0 && (!e.scope || !other.scope)) &&
           !party_overlap(e.scope,other.scope)&&!party_overlap(other.scope,e.scope)&&!controller_global&&!recovery_overlap) continue;
        conflict=other.label; return false;
    }
    if(!e.controller&&p>=0&&game_keyboard_check) {
        last_native_failure=game_keyboard_check(e,p,m,conflict);
        if(last_native_failure!=NativeKeyboardResult::Clear)return false;
    }
    e.primary=p; e.modifier=m;
    settings::set_int(setting(e,".Input").c_str(),p); settings::set_int(setting(e,".Modifier").c_str(),m);
    return true;
}
void native_keyboard_check(NativeKeyboardCheck check){game_keyboard_check=check;}
bool native_controls_unavailable(){return last_native_failure==NativeKeyboardResult::Unavailable;}
std::wstring conflict_message(const std::wstring& conflict) {
    if(last_native_failure==NativeKeyboardResult::Unavailable)
        return T(L"Game controls unavailable. Close this menu and try again after loading.");
    if(last_native_failure==NativeKeyboardResult::Conflict)
        return T(L"This key is assigned to the game. Choose another key.");
    return TF(L"Already assigned to {0}. Unbind it first.",{T(conflict)});
}
void reset(int i) { auto& e=catalog().at(i); e.primary=e.default_primary; e.modifier=e.default_modifier; settings::set_int(setting(e,".Input").c_str(),e.primary); settings::set_int(setting(e,".Modifier").c_str(),e.modifier); }
void reset_all() { for(size_t i=0;i<catalog().size();++i)reset(int(i)); }
bool pressed(int i) { return i>=0 && (catalog().at(i).scope!=int(Context::BattleParty)||party_context) && down(catalog().at(i),true); }
bool held(int i) { return i>=0 && (catalog().at(i).scope!=int(Context::BattleParty)||party_context) && down(catalog().at(i),false); }
bool keyboard_in_use(Context context) {
    if(!allow_keyboard)return false;
    const int actual=mods();
    for(const auto& e:catalog()) if(!e.controller && scope_matches(e,context) && enabled(e) && e.primary>=0 && chord_mods(e.primary,actual)==e.modifier && key_in_use(Key(e.primary))) {
        if((e.label==L"Report game speed" || e.label==L"Report encounters") && e.primary==e.default_primary && e.modifier==e.default_modifier)continue;
        return true;
    }
    return false;
}
void context(Context value) {game_context=value;refresh_remapped_ownership();}
void battle_party_context(bool active) {party_context=active;refresh_party_ownership();}
bool battle_party_context() {return party_context;}
bool battle_party_keyboard_in_use() {return party_keyboard_owned;}
bool remapped_keyboard_in_use() {
    return remapped_keyboard_owned;
}
void mark_event_guide(int index) {
    for(int& saved:guide_bindings)if(saved==index)return;
    for(int& saved:guide_bindings)if(saved<0){saved=index;return;}
}
void event_guide_context(bool active) {event_guide=active;}
bool event_guide_context() {return event_guide;}
void owned_guide_context(bool repeat,bool stop) {owned_guide_repeat=repeat;owned_guide_stop=stop;}
bool guide_next_allowed() {return event_guide||owned_guide_repeat;}
bool guide_stop_allowed() {return event_guide||owned_guide_stop;}
bool guide_command_context() {return guide_next_allowed()||guide_stop_allowed();}
bool guide_binding_allowed(int index) {
    return (index==guide_bindings[0]&&guide_next_allowed())||
        (index==guide_bindings[1]&&guide_stop_allowed());
}
bool event_guide_keyboard_in_use() {
    if(!guide_command_context()&&guide_owned[0]<0&&guide_owned[1]<0)return false;
    if(!game_window_focused()) {guide_owned[0]=guide_owned[1]=-1;return false;}
    bool used=false;
    for(int i=0;i<2;++i) {
        // Keep the consumed physical key through release, even if its modifier
        // or the native performance ends first. Never replay it as confirm.
        if(guide_owned[i]>=0) {
            if(key_in_use(Key(guide_owned[i])))used=true;
            else guide_owned[i]=-1;
        }
        if(guide_binding_allowed(guide_bindings[i])&&allow_keyboard&&guide_bindings[i]>=0) {
            const auto& e=catalog()[guide_bindings[i]];
            if(e.primary>=0&&enabled(e)&&chord_mods(e.primary,mods())==e.modifier&&key_in_use(Key(e.primary))) {
                guide_owned[i]=e.primary;used=true;
            }
        }
    }
    return used;
}
bool keyboard_action_pressed(const char* action) {
    for(size_t i=0;i<catalog().size();++i) if(!catalog()[i].controller && catalog()[i].id.find(action)!=std::string::npos && pressed(int(i)))return true;
    return false;
}
void begin_frame() {
    refresh_remapped_ownership(false);
    if((guide_owned[0]>=0||guide_owned[1]>=0)&&!game_window_focused())guide_owned[0]=guide_owned[1]=-1;
    for(int i=0;i<kPadInputs;++i) { previous[i]=current[i]; owned[i]=owned[i] && current[i]; }
    for(int i=0;i<kButtonCount;++i)current[i]=button_held(i)||button_pressed(i);
    current[kLeftTrigger]=left_trigger()>.5f; current[kRightTrigger]=right_trigger()>.5f;
    const float x[]={left_stick_x(),right_stick_x()},y[]={left_stick_y(),right_stick_y()};
    for(int i=0;i<2;++i) { const int b=kLeftUp+4*i;
        const bool vertical=std::fabs(y[i])>.4883f && std::fabs(y[i])>std::fabs(x[i])+.2441f;
        const bool horizontal=std::fabs(x[i])>.4883f && std::fabs(x[i])>std::fabs(y[i])+.2441f;
        // Physical snapshots publish positive Y for up on both sticks.
        current[b]=vertical&&y[i]>0;current[b+1]=vertical&&y[i]<0;
        current[b+2]=horizontal&&x[i]<0;current[b+3]=horizontal&&x[i]>0;
    }
    current[kLeftUp]|=gamepad::lstick_up_pressed();current[kLeftDown]|=gamepad::lstick_down_pressed();
    current[kLeftLeft]|=gamepad::lstick_left_pressed();current[kLeftRight]|=gamepad::lstick_right_pressed();
    current[kRightUp]|=gamepad::rstick_up_pressed();current[kRightDown]|=gamepad::rstick_down_pressed();
    current[kRightLeft]|=gamepad::rstick_left_pressed();current[kRightRight]|=gamepad::rstick_right_pressed();
    const bool available=game_window_focused()&&gamepad_available();
    if(!available || !had_pad)pad_ready=false;
    had_pad=available;
    if(!pad_ready) {
        bool neutral=true;for(bool b:current)neutral&=!b;
        pad_ready=available&&neutral;
        for(int i=0;i<kPadInputs;++i)current[i]=previous[i]=owned[i]=false;
    }
}
bool pad(const char* id, bool release) {
    const int i=find(id); if(i<0 || capture_index>=0)return false;
    const auto& e=catalog()[i]; if(e.primary<0)return false;
    const bool chord=e.modifier<0 || current[e.modifier] || (release && previous[e.modifier]);
    if(chord)own(e);
    return release ? previous[e.primary] && (e.modifier<0 || previous[e.modifier]) && !down(e,false) : chord && down(e,true);
}
bool pad_held(const char* id) { const int i=find(id); if(i<0)return false; const auto& e=catalog()[i]; if(down(e,false)) {own(e); return true;} return false; }
bool consumed(int i) { return i>=0 && i<kPadInputs && owned[i]; }
bool navigation_request_pressed() { return keyboard_action_pressed("Navigate to current entity / stop autowalk") || keyboard_action_pressed("Pathfind to waypoint") || pressed(find("pad.field.route")); }
bool navigation_controller_input(int physical) {
    const int index=find("pad.field.route");if(index<0)return false;
    const auto& e=catalog()[index];
    return e.primary>=0 && ((physical==e.primary && down(e,false)) || (physical==e.modifier && current[physical]));
}
void keyboard_allowed(bool allowed) {allow_keyboard=allowed;refresh_party_ownership();refresh_remapped_ownership();}
void capture(int i) {
    initialize_keyboard_controls();capture_index=i;capture_ready=false;capture_pad_first=-1;
    ffpr::say_menu(TF(catalog()[i].controller?L"Press a controller button, trigger, or stick direction. {0} cancels.":L"Press a key. {0} cancels.",{value(capture_cancel_index)}),true);
}
bool capturing() { return capture_index>=0; }
void cancel_capture() { capture_index=-1; capture_ready=false;capture_pad_first=-1; }
bool update_capture() {
    if(capture_index<0)return false;
    if(!game_window_focused()) {capture_ready=false;capture_pad_first=-1;return true;}
    if(pressed(capture_cancel_index)) {cancel_capture();ffpr::say_menu(T(L"Cancelled"),true);return true;}
    const auto& e=catalog()[capture_index];
    if(!capture_ready) {
        bool held_input=false;
        if(e.controller) for(bool b:current)held_input|=b;
        else for(int k=0;k<int(Key::Count);++k) if(!modifier_key(k))held_input|=key_held(Key(k));
        capture_ready=!held_input; return true;
    }
    int p=-1,m=e.controller?-1:mods();
    if(e.controller) {
        int edge=-1;for(int i=0;i<kPadInputs;++i)if(current[i]&&!previous[i]){edge=i;break;}
        if(capture_pad_first<0)capture_pad_first=edge;
        if(capture_pad_first>=0) {
            if(edge>=0 && edge!=capture_pad_first && current[capture_pad_first]) {p=edge;m=capture_pad_first;}
            else if(!current[capture_pad_first])p=capture_pad_first;
        }
    }
    else for(int i=0;i<int(Key::Count);++i)if(!modifier_key(i)&&key_pressed(Key(i))){p=i;break;}
    if(p>=0) { std::wstring conflict; const int index=capture_index; if(set(index,p,m,conflict)) {cancel_capture();ffpr::say_menu(TF(L"Binding set to {0}",{value(index)}),true);} else {capture_ready=false;capture_pad_first=-1;ffpr::say_menu(conflict_message(conflict),true);} }
    return true;
}
}
