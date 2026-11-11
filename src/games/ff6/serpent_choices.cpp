// Serpent Trench uses a timed arrow UI rather than the normal text-choice menu.
// Observe actual rendered-arrow writers, never an Update/clock poll or input
// override. Enum0 is Right,1 is Left; both Right branches visit the real caves.
#include "serpent_choices.h"
#include "../../core/core.h"
#include "../../core/frame.h"
#include "../../core/hooks.h"
#include "../../core/modtext.h"
#include "../../core/refs.h"
#include "../../core/say.h"
#include "../../core/input/input.h"
#include "../../core/ui/preferences.h"
#include "../../core/field/field_api.h"

namespace ffpr::ff6::serpent {
namespace {
ref::Class kKeys{"Serial.FF6.UI.KeyInput","ChoiceDirectionController",game::kFF6};
ref::Class kTouch{"Serial.FF6.UI.Touch","ChoiceDirectionController",game::kFF6};
ref::Method mKeyInit{kKeys,"SelectingInit",0},mKeyArrow{kKeys,"UpdateDirectionArrow",1},mKeyFinish{kKeys,"Finish",0};
ref::Method mTouchInit{kTouch,"SelectingInit",0},mTouchArrow{kTouch,"UpdateDirectionArrow",1},mTouchFinish{kTouch,"Finish",0};
using Simple=void(*)(void*,const void*);
using Arrow=void(*)(void*,int32_t,const void*);
Simple o_key_init=nullptr,o_key_finish=nullptr,o_touch_init=nullptr,o_touch_finish=nullptr;
Arrow o_key_arrow=nullptr,o_touch_arrow=nullptr;
bool g_keys=false,g_touch=false,g_active=false,g_battle=false;
int g_origin=0,g_initializing=0,g_direction=-1;
uint64_t g_context=0;
void retire(){g_active=false;g_origin=g_initializing=0;g_direction=-1;++g_context;frame::cancel_label("FF6.Serpent.Choice");}
void publish(int direction,bool initial) {
    const auto context=g_context;
    frame::next_frame([context,direction,initial] {
        if(context!=g_context||!g_active||g_battle||field::current_map_id()!=573||!input::game_window_focused())return;
        const auto selected=T(direction==0?L"Right":L"Left");
        // Normal UI information stays accessible with solution guidance off.
        // A new occurrence may repeat the same direction; there is no content
        // deduplication. Current native value guards repeated renderer writes.
        auto line=initial?TF(L"Junction: {0}.",{selected}):selected;
        ffpr::say(line,!initial);
    },"FF6.Serpent.Choice");
}
void init(void* self,const void* mi,int origin,bool enabled,Simple original) {
    if(!frame::on_main_thread()||!enabled||field::current_map_id()!=573){original(self,mi);return;}
    retire();g_initializing=origin;
    original(self,mi); // its real arrow writer supplies the displayed enum
    hooks::guarded("FF6.Serpent.Initial",[&] {
        if(g_initializing!=origin)return;
        g_initializing=0;
        if(g_direction<0||g_direction>1)return;
        g_origin=origin;g_active=true;publish(g_direction,true);
    });
}
void arrow(void* self,int32_t direction,const void* mi,int origin,bool enabled,Arrow original) {
    original(self,direction,mi);
    if(!frame::on_main_thread()||!enabled||direction<0||direction>1)return;
    hooks::guarded("FF6.Serpent.Arrow",[&] {
        if(g_initializing==origin){g_direction=direction;return;}
        if(!g_active||g_origin!=origin||g_direction==direction)return;
        g_direction=direction;publish(direction,false);
    });
}
void finish(void* self,const void* mi,int origin,bool enabled,Simple original) {
    if(frame::on_main_thread()&&enabled&&g_origin==origin)retire();
    original(self,mi);
}
void d_key_init(void* p,const void* m){init(p,m,1,g_keys,o_key_init);}
void d_key_arrow(void* p,int32_t d,const void* m){arrow(p,d,m,1,g_keys,o_key_arrow);}
void d_key_finish(void* p,const void* m){finish(p,m,1,g_keys,o_key_finish);}
void d_touch_init(void* p,const void* m){init(p,m,2,g_touch,o_touch_init);}
void d_touch_arrow(void* p,int32_t d,const void* m){arrow(p,d,m,2,g_touch,o_touch_arrow);}
void d_touch_finish(void* p,const void* m){finish(p,m,2,g_touch,o_touch_finish);}
}
bool choice_available(){return frame::on_main_thread()&&g_active&&!g_battle&&field::current_map_id()==573;}
std::wstring choice_instruction(){
    if(!choice_available())return {};
    return T(ui::prefs::navigation_puzzle_guidance()?L"Choose Right at the junction.":L"Navigation puzzle guidance is off");
}
int install_choices(){
    // Partial attachment is inert for that interface: every attached detour
    // still calls its original, but cannot own a window without all three
    // source-qualified start/writer/finish producers.
    const bool a=hooks::attach(mKeyInit,&d_key_init,&o_key_init),b=hooks::attach(mKeyArrow,&d_key_arrow,&o_key_arrow),c=hooks::attach(mKeyFinish,&d_key_finish,&o_key_finish);
    const bool d=hooks::attach(mTouchInit,&d_touch_init,&o_touch_init),e=hooks::attach(mTouchArrow,&d_touch_arrow,&o_touch_arrow),f=hooks::attach(mTouchFinish,&d_touch_finish,&o_touch_finish);
    g_keys=a&&b&&c;g_touch=d&&e&&f;
    events::on(events::Kind::MapChanging,[]{retire();});
    events::on(events::Kind::BattleEntering,[]{g_battle=true;retire();});
    events::on(events::Kind::BattleEnd,[]{g_battle=false;});
    events::on(events::Kind::TitleScreen,[]{g_battle=false;retire();});
    return int(a)+int(b)+int(c)+int(d)+int(e)+int(f);
}
}
