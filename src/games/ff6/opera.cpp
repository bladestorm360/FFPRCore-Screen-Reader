// Ports FF6 Patches/OperaPatches.cs (OperaController_Update_Patch): during the opera house's
// button prompts, "Success" when the player presses in time, "Failed" when the prompt passes,
// "Out of time" when the interactive timer runs out.
//
// The C# polled three OperaController flags in a postfix on OperaController.Update (every
// frame). In FF6's GameAssembly.dll that Update only services the limit timer; the flags are
// written elsewhere (offline disassembly, 2026-10-07), and this file hooks those writers:
//   clickedInteractiveIconButton = true   only in OnClickInteractiveIcon() (the touch button's
//                                          callback, and called directly by the opera event
//                                          coroutine on a key press);
//   isFailedInteractiveIconButton = true  only in the PlayOperaEvent coroutine's MoveNext;
//   isInteractiveTimeup = true            only in the StartOperaInteractiveTimer coroutine's
//                                          MoveNext (it runs twice per prompt).
// Each detour compares the flag before and after the original, which is the C#'s
// "changed from false to true" test at the moment it happens. The coroutine steps run only
// while the opera event plays. All three bodies have one RVA each in FF6's dump.cs.
// As in the C#, nothing is spoken unless OperaController.isPlaying.
#include <cstdint>
#include "../../core/core.h"
#include "../../core/game.h"
#include "../../core/hooks.h"
#include "../../core/il2cpp.h"
#include "../../core/log.h"
#include "../../core/modtext.h"
#include "../../core/refs.h"
#include "../../core/say.h"
#include "../../core/ui/preferences.h"
#include "../../core/unity.h"
#include "../../core/displayed_ui.h"
#include "../../core/frame.h"
#include "../../core/ui_util.h"
#include "../../core/text.h"
#include "../../core/input/bindings.h"
#include "../../core/field/scene_events.h"
#include "opera_guide.h"

namespace ffpr::ff6 {
namespace {

constexpr uint8_t k6 = game::kFF6;

ref::Class  kOperaController{"Last.FF6.Opera", "OperaController", k6};
ref::Field  fIsPlaying{kOperaController, "isPlaying"};
ref::Field  fClicked{kOperaController, "clickedInteractiveIconButton"};
ref::Field  fFailed{kOperaController, "isFailedInteractiveIconButton"};
ref::Field  fTimeup{kOperaController, "isInteractiveTimeup"};
ref::Field  fInteractiveCanvas{kOperaController, "interactiveIconCanvasObject"};
ref::Method mOnClickInteractiveIcon{kOperaController, "OnClickInteractiveIcon", 0};
ref::Method mPlay{kOperaController,"Play",1},mDestroy{kOperaController,"Destory",0},
    mSelect{kOperaController,"SelectMessage",4},mSelectFailed{kOperaController,"PlayOperaEventSelectFailed",1};
// PlayOpera assigns the KeyInput multiple controller, not MessageSelectController
// or the similarly named Touch type. Its Show writes native initial focus and
// visibility synchronously; subsequent SetForcusIndex writes changed focus.
ref::Class kMultiple{"Last.UI.KeyInput","MessageMultipleController",k6};
ref::Field fContents{kMultiple,"contentList"},fCursor{kMultiple,"selectCursor"};
ref::Method mShow{kMultiple,"Show",0},mClose{kMultiple,"Close",0},
    mEnable{kMultiple,"SetEnable",1},mFocus{kMultiple,"SetForcusIndex",1};
void* g_opera=nullptr;
void* g_choice_view=nullptr;
opera_guide::Choice g_choice;
bool g_prompt=false,g_choice_open=false,g_lyric_failed=false;
enum class Outcome {None,Success,Failed,Timeout};
Outcome g_outcome=Outcome::None;
unsigned g_choice_generation=0;

void close_choice() {
    g_choice_view=nullptr;g_choice_open=false;++g_choice_generation;
    frame::cancel_label("Opera.ChoiceInitial");
}
void retire() {
    close_choice();g_opera=nullptr;g_choice={};g_prompt=false;g_lyric_failed=false;g_outcome=Outcome::None;
    input::bindings::event_guide_context(false);
}
std::wstring choice_hint() {
    return TF(L"Opera lyric {0} of 3: choose option {1} of 2.",
        {text::num(g_choice.question),text::num(g_choice.correct)});
}
void guide() {
    if(!ui::prefs::other_puzzle_guidance()){ffpr::say(T(L"Non-navigation puzzle guidance is off"),true);return;}
    if(!g_opera)return;
    if(g_lyric_failed||g_outcome==Outcome::Failed)ffpr::say(T(L"Failed"));
    else if(g_outcome==Outcome::Timeout)ffpr::say(T(L"Out of time"));
    else if(g_choice_open&&g_choice.question)
        ffpr::say(choice_hint());
    else if(g_prompt)ffpr::say(T(L"Opera action: confirm now."));
    else ffpr::say(T(L"Wait for the next opera cue."));
}

void read_choice(void* self,bool initial) {
    if(!g_opera||self!=g_choice_view||!g_choice_open||!displayed::container_visible(self))return;
    void* list=ref::rd<void*>(self,fContents);
    const int count=unity::list_count(list),index=ui::cursor_index(ref::rd<void*>(self,fCursor));
    // Opera has exactly two visible lyric options. Refuse a reused controller
    // with an unexpected list rather than reading unrelated UI or unbounded text.
    if(count!=2||index<0||index>=2)return;
    const auto value=displayed::text_field(displayed::field(unity::list_at(list,index),"view"),"text");
    if(!value.empty()) {
        auto line=ui::menu_position(value,index,count);
        if(initial&&ui::prefs::other_puzzle_guidance()&&queries::is("GameModule.PuzzleGuideActive"))line=choice_hint()+L" "+line;
        ffpr::say(line,!initial);
    }
}
using Play=void(*)(void*,int32_t,const void*);
Play o_Play=nullptr;
void d_Play(void* self,int32_t id,const void* mi) {
    hooks::guarded("FF6.Opera.Play.prefix",[&] {
        // The native method ignores Play while already playing. Do not erase a
        // live choice/prompt on that ignored request. All pointers stay here on
        // the native owner; worker snapshots never contain them.
        if(self&&!ref::rd<bool>(self,fIsPlaying)&&(id==100||id==101||id==200||id==301)) {
            retire();g_opera=self;input::bindings::event_guide_context(true);
        }
    });
    o_Play(self,id,mi);
}
using Simple=void(*)(void*,const void*);
Simple o_Destroy=nullptr,o_Show=nullptr,o_Close=nullptr;
void d_Destroy(void* self,const void* mi) {
    if(self==g_opera)retire();
    o_Destroy(self,mi);
}
using Select=void*(*)(void*,void*,void*,float,bool,const void*);
Select o_Select=nullptr;
void* d_Select(void* self,void* message,void* choices,float limit,bool auto_close,const void* mi) {
    hooks::guarded("FF6.Opera.SelectMessage.prefix",[&] {
        if(self!=g_opera)return;
        close_choice();g_choice={};g_prompt=false;g_lyric_failed=false;g_outcome=Outcome::None;
        if(unity::array_length(choices)!=2)return;
        wchar_t a[32]{},b[32]{};
        if(il2cpp::read_string(static_cast<il2cpp::String*>(unity::array_at(choices,0)),a,32)&&
           il2cpp::read_string(static_cast<il2cpp::String*>(unity::array_at(choices,1)),b,32))
            g_choice=opera_guide::choice(a,b);
    });
    return o_Select(self,message,choices,limit,auto_close,mi);
}
using Failed=void*(*)(void*,void*,const void*);
Failed o_SelectFailed=nullptr;
void* d_SelectFailed(void* self,void* message,const void* mi) {
    if(self==g_opera) {close_choice();g_lyric_failed=true;g_prompt=false;}
    return o_SelectFailed(self,message,mi);
}
void d_Show(void* self,const void* mi) {
    o_Show(self,mi);
    hooks::guarded("FF6.Opera.ChoiceShow",[&] {
        if(!g_opera||!g_choice.question||!kMultiple.is(self))return;
        close_choice();g_choice_view=self;g_choice_open=true;
        const auto generation=g_choice_generation;
        // Match the ordinary choice reader's game-owned one-frame population
        // boundary. A native focus change/close cancels this occurrence.
        frame::next_frame([self,generation] {
            if(generation==g_choice_generation)read_choice(self,true);
        },"Opera.ChoiceInitial");
    });
}
void d_Close(void* self,const void* mi) {
    if(self==g_choice_view)close_choice();
    o_Close(self,mi);
}
using Enable=void(*)(void*,bool,const void*);
Enable o_Enable=nullptr;
void d_Enable(void* self,bool enable,const void* mi) {
    if(!enable&&self==g_choice_view)close_choice();
    o_Enable(self,enable,mi);
}
using Focus=void(*)(void*,int32_t,const void*);
Focus o_Focus=nullptr;
void d_Focus(void* self,int32_t index,const void* mi) {
    o_Focus(self,index,mi);
    hooks::guarded("FF6.Opera.ChoiceFocus",[&] {
        if(self!=g_choice_view||!g_choice_open)return;
        ++g_choice_generation;frame::cancel_label("Opera.ChoiceInitial");
        read_choice(self,false);
    });
}

// <>4__this of each coroutine state machine (resolved at install).
int32_t g_event_this = -1;
int32_t g_timer_this = -1;

bool flag(void* opera, ref::Field& f) { return opera && ref::rd<bool>(opera, f); }
bool playing(void* opera) { return flag(opera, fIsPlaying); }
bool action_prompt(void* opera) {
    return playing(opera) && !flag(opera,fClicked) && !flag(opera,fFailed) &&
        unity::active_self(ref::rd<void*>(opera,fInteractiveCanvas));
}

using OnClick_t = void (*)(void* self, const void* mi);
OnClick_t o_OnClickInteractiveIcon = nullptr;

void d_OnClickInteractiveIcon(void* self, const void* mi)
{
    bool before = hooks::guarded_r("FF6.Opera.OnClickInteractiveIcon.prefix", true, [&] { return flag(self, fClicked); });
    o_OnClickInteractiveIcon(self, mi);
    hooks::guarded("FF6.Opera.OnClickInteractiveIcon", [&] {
        if(self==g_opera&&flag(self,fClicked)) {g_prompt=false;g_outcome=Outcome::Success;}
        if (!before && flag(self, fClicked) && playing(self)) ffpr::say(T(L"Success"));
    });
}

using MoveNext_t = bool (*)(void* self, const void* mi);
MoveNext_t o_EventMoveNext = nullptr;
MoveNext_t o_TimerMoveNext = nullptr;

void* opera_of(void* state_machine, int32_t this_offset)
{
    return il2cpp::read_field<void*>(state_machine, this_offset);
}

bool d_EventMoveNext(void* self, const void* mi)
{
    void* opera = opera_of(self, g_event_this);
    bool before = hooks::guarded_r("FF6.Opera.PlayOperaEvent.prefix", true, [&] { return flag(opera, fFailed); });
    const bool prompt_before=hooks::guarded_r("FF6.Opera.Prompt.prefix",true,[&]{return action_prompt(opera);});
    bool result = o_EventMoveNext(self, mi);
    hooks::guarded("FF6.Opera.PlayOperaEvent", [&] {
        const bool prompt_after=action_prompt(opera);
        if(opera==g_opera) {
            g_prompt=prompt_after;
            if(prompt_after)g_outcome=Outcome::None;
            if(!before&&flag(opera,fFailed))g_outcome=Outcome::Failed;
        }
        if (!before && flag(opera, fFailed) && playing(opera)) ffpr::say(T(L"Failed"));
        // This exact coroutine activates the interactive canvas at the native
        // action window and checks that same canvas before accepting confirm.
        // Its delayed failure timer is not a prompt producer. Observe each
        // native inactive->active occurrence, never speech-content equality.
        if(!prompt_before&&prompt_after)ffpr::say(T(L"Opera action: confirm now."));
        if(opera==g_opera&&!result&&!playing(opera))retire();
    });
    return result;
}

bool d_TimerMoveNext(void* self, const void* mi)
{
    void* opera = opera_of(self, g_timer_this);
    bool before = hooks::guarded_r("FF6.Opera.StartOperaInteractiveTimer.prefix", true, [&] { return flag(opera, fTimeup); });
    bool result = o_TimerMoveNext(self, mi);
    hooks::guarded("FF6.Opera.StartOperaInteractiveTimer", [&] {
        if(opera==g_opera&&!before&&flag(opera,fTimeup)) {g_prompt=false;g_outcome=Outcome::Timeout;}
        if (!before && flag(opera, fTimeup) && playing(opera)) ffpr::say(T(L"Out of time"));
    });
    return result;
}

/// A coroutine's MoveNext and its <>4__this offset, by the coroutine's name.
void* coroutine_move_next(const char* prefix, int32_t& this_offset)
{
    il2cpp::Class* outer = kOperaController.get();
    il2cpp::Class* sm = outer ? il2cpp::find_nested_prefix(outer, prefix) : nullptr;
    if (!sm) {
        LOGF("[Opera] coroutine %s not found", prefix);
        return nullptr;
    }
    this_offset = il2cpp::field_offset(sm, "<>4__this");
    il2cpp::Method* m = il2cpp::find_method(sm, "MoveNext", 0);
    if (!m || this_offset < 0) {
        LOGF("[Opera] coroutine %s: MoveNext or <>4__this missing", prefix);
        return nullptr;
    }
    return il2cpp::method_pointer(m);
}

int install()
{
    int n = 0;
    actions::define("Opera.Guide",[]{guide();});
    queries::define("GameModule.PuzzleGuideDuringEvent",[]{return input::bindings::event_guide_context();});
    events::on(events::Kind::MapChanging,[]{retire();});
    events::on(events::Kind::BattleEntering,[]{retire();});
    events::on(events::Kind::SceneLoaded,[]{retire();});
    n+=hooks::attach(mPlay,&d_Play,&o_Play)?1:0;
    n+=hooks::attach(mDestroy,&d_Destroy,&o_Destroy)?1:0;
    n+=hooks::attach(mSelect,&d_Select,&o_Select)?1:0;
    n+=hooks::attach(mSelectFailed,&d_SelectFailed,&o_SelectFailed)?1:0;
    n+=hooks::attach(mShow,&d_Show,&o_Show)?1:0;
    n+=hooks::attach(mClose,&d_Close,&o_Close)?1:0;
    n+=hooks::attach(mEnable,&d_Enable,&o_Enable)?1:0;
    n+=hooks::attach(mFocus,&d_Focus,&o_Focus)?1:0;
    if (hooks::attach(mOnClickInteractiveIcon, &d_OnClickInteractiveIcon, &o_OnClickInteractiveIcon)) ++n;
    if (void* p = coroutine_move_next("<PlayOperaEvent>d__", g_event_this)) {
        if (hooks::attach_addr(p, "OperaController.PlayOperaEvent.MoveNext", reinterpret_cast<void*>(&d_EventMoveNext),
                               reinterpret_cast<void**>(&o_EventMoveNext))) ++n;
    }
    if (void* p = coroutine_move_next("<StartOperaInteractiveTimer>d__", g_timer_this)) {
        if (hooks::attach_addr(p, "OperaController.StartOperaInteractiveTimer.MoveNext",
                               reinterpret_cast<void*>(&d_TimerMoveNext), reinterpret_cast<void**>(&o_TimerMoveNext))) ++n;
    }
    if (n < 11) LOGF("[Opera] %d of 11 opera hooks installed", n);
    return n;
}

core::Install s_install{"FF6.Opera", &install, 510, game::kFF6};

} // namespace
} // namespace ffpr::ff6
