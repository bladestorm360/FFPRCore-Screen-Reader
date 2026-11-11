// Read-only bounded observation of the exact native Select occurrence. The
// native interpreter/window owns all waiting, choices and timer/quest writes.
#include "choices.h"
#include "choice_model.h"
#include "../../../core/core.h"
#include "../../../core/frame.h"
#include "../../../core/hooks.h"
#include "../../../core/modtext.h"
#include "../../../core/refs.h"
#include "../../../core/unity.h"
#include "../../../core/field/field_api.h"
#include "../../../core/field/map_state.h"
#include "../../../core/ui/preferences.h"

namespace ffpr::ff6::palace {
namespace {
// Dump output leaves nested namespaces blank; native lookup needs the outer
// declaring type's namespace before resolving its dot-separated nested name.
ref::Class kExternalMessage{"Last.Interpreter.Instructions","External.Message",game::kFF6};
ref::Method mSelect{kExternalMessage,"Select",1};
ref::Class kCore{"Last.Interpreter","Core",game::kFF6};
ref::Class kCounter{"Last.Interpreter","Core.ProgramCounter",game::kFF6};
ref::Class kMnemonic{"Last.Interpreter.Instructions","Format.Mnemonic",game::kFF6};
ref::Class kOperands{"Last.Interpreter.Instructions","Format.Operands",game::kFF6};
ref::Field fCounter{kCore,"<programCounter>k__BackingField"};
ref::Field fClock{kCounter,"<clockCount>k__BackingField"};
ref::Field fAddress{kCounter,"<currentAddress>k__BackingField"};
ref::Field fInstructions{kCore,"mnemonics"};
ref::Field fCurrent{kCore,"currentInstruction"};
ref::Field fOpcode{kMnemonic,"mnemonic"};
ref::Field fOperands{kMnemonic,"operands"};
ref::Field fStrings{kOperands,"sValues"};
Question g_question;
bool g_watch=false,g_battle=false,g_available=false;
using Select=int(*)(void*,const void*);
Select o_select=nullptr;
void clear(){g_question={};}
bool read_key(void* value,std::array<wchar_t,32>& into) {
    into={};
    return !value||il2cpp::read_string(static_cast<il2cpp::String*>(value),into.data(),into.size());
}
void observe(void* core) {
    // IsFirstClock is exactly counter.clockCount==1 in the matching native
    // body. Repeated waiting ticks copy no arrays/strings and create no work.
    void* counter=ref::rd<void*>(core,fCounter);
    if(!counter||ref::rd<int32_t>(counter,fClock)!=1)return;
    clear();
    if(!kCore.is(core))return;
    void* current=ref::rd<void*>(core,fCurrent);
    void* instructions=ref::rd<void*>(core,fInstructions);
    const int count=unity::array_length(instructions);
    const int pc=ref::rd<int32_t>(counter,fAddress);
    if(!current||count<1||count>4096||pc<0||pc>=count||unity::array_at(instructions,pc)!=current)return;
    void* operands=ref::rd<void*>(current,fOperands);
    void* strings=ref::rd<void*>(operands,fStrings);
    const int length=unity::array_length(strings);
    if(length<1||length>8)return;
    Question copied;
    for(int i=0;i<6&&i<length;++i)
        if(!read_key(unity::array_at(strings,i),copied.options[static_cast<size_t>(i)]))return;
    if(!option(copied,0).empty()&&length<6)return; // same six-ID native producer
    // Only actual immediately preceding Msg, with at most two intervening
    // native Wait/Nop rows. Never infer a question from a stale dialogue page.
    for(int back=1;back<=3&&pc-back>=0;++back) {
        void* previous=unity::array_at(instructions,pc-back);
        std::array<wchar_t,32> opcode{};
        if(!read_key(ref::rd<void*>(previous,fOpcode),opcode))return;
        const std::wstring_view name=opcode.data();
        if(name==L"Wait"||name==L"Nop")continue;
        if(name==L"Msg") {
            void* args=ref::rd<void*>(previous,fOperands);
            void* keys=ref::rd<void*>(args,fStrings);
            const int size=unity::array_length(keys);
            if(size<1||size>8||!read_key(unity::array_at(keys,0),copied.prompt))return;
        }
        break;
    }
    copied.valid=true;g_question=copied;
}
int d_select(void* core,const void* mi) {
    if(g_watch&&!g_battle&&frame::on_main_thread())
        hooks::guarded("FF6.Palace.Select",[&]{observe(core);});
    return o_select(core,mi);
}
void map_ready(){clear();const int map=field::current_map_id();g_watch=map==176||map==177;}
}
bool choice_available() {
    return g_available&&g_watch&&!g_battle&&g_question.valid&&frame::on_main_thread()&&
        identify(g_question)!=Kind::None&&queries::is("MessageChoice.IsOpen");
}
bool choice_enabled(){return identify(g_question)==Kind::FinishTour?ui::prefs::navigation_puzzle_guidance():ui::prefs::other_puzzle_guidance();}
std::wstring choice_instruction() {
    if(!choice_available())return {};
    if(!choice_enabled())return T(identify(g_question)==Kind::FinishTour?L"Navigation puzzle guidance is off":L"Non-navigation puzzle guidance is off");
    State state;const auto kind=identify(g_question);
    // These small observations happen only on an explicit puzzle request.
    // No full flags/party/actor capture, native pointer retention or polling.
    if(kind==Kind::Ask||kind==Kind::More)
        for(int i=1;i<=3;++i)state.local[static_cast<size_t>(i)]=field::map_state::local_value(177,i);
    else if(kind==Kind::Recall)state.local[4]=field::map_state::local_value(177,4);
    else if(kind==Kind::Resume) {
        if(const auto bit=field::map_state::scenario_flag(256))state.break_done=*bit==1;
    } else if(kind==Kind::FinishTour) {
        bool missing=false,unfinished=false;
        for(int flag=230;flag<=253;++flag) {
            const auto bit=field::map_state::scenario_flag(flag);
            if(!bit)missing=true;
            else if(*bit==0){unfinished=true;break;}
            else if(*bit!=1)missing=true;
        }
        if(unfinished)state.tour_done=false;
        else if(!missing)state.tour_done=true;
    }
    const int selected=answer(g_question,state).option;
    if(selected==1)return T(L"Choose the first option.");
    if(selected==2)return T(L"Choose the second option.");
    if(selected==3)return T(L"Choose the third option.");
    return T(L"Story guidance is waiting for current game state.");
}
int install_choices() {
    // All native fields resolve once through the existing boot/name checker.
    // An incomplete binding disables this optional observation rather than
    // inventing an answer or intercepting the game's original interpreter.
    if(!fCounter||!fClock||!fAddress||!fInstructions||!fCurrent||!fOpcode||!fOperands||!fStrings)return 0;
    g_available=hooks::attach(mSelect,&d_select,&o_select);
    if(!g_available)return 0;
    events::on(events::Kind::MapChanged,[]{map_ready();});
    events::on(events::Kind::MapChanging,[]{g_watch=false;clear();});
    events::on(events::Kind::BattleEntering,[]{g_battle=true;clear();});
    events::on(events::Kind::BattleEnd,[]{g_battle=false;map_ready();});
    events::on(events::Kind::TitleScreen,[]{g_watch=false;g_battle=false;clear();});
    return 1;
}
}
