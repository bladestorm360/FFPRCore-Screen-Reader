// check-sources: text.cpp json.cpp battle/scan_feedback.cpp
// Actual InitActionState/UI producer/result integration; no game is run.
#define main retained_battle_feedback_main
#include "battle_feedback_check.cpp"
#undef main
namespace fixture {
int view_calls=0,message_calls=0;
bool touch=false,throw_view=false;
void message_original(void*,void*,const void*){++message_calls;}
bool native_action_message(void* action,const void*){
    ++view_calls;const auto& list=*static_cast<List*>(static_cast<Node*>(action)->abilities);
    const int id=static_cast<Node*>(list.front())->id;
    if(id==138)return false; // the real content row has no use-name or battle text
    Node message;message.name=id==140?L"Self-destruct":L"Boost";
    if(touch)ffpr::battle::d_SetSystemMessage(nullptr,&message,nullptr);
    else ffpr::battle::d_SetMessage(nullptr,&message,nullptr);
    if(throw_view)throw 1;
    return true;
}
void native_action_init(void* self,const void* mi){view_message_dispatch(static_cast<Node*>(self)->act,mi);}
void flush_use(){auto use=std::move(action_ui_posts);action_ui_posts.clear();for(auto& task:use)task();auto results=std::move(effect_ui_posts);effect_ui_posts.clear();for(auto& task:results)task();}
}
int main(){
    using namespace fixture;namespace b=ffpr::battle;namespace a=b::action_effects;
    int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    game_id=game::Id::FF3;view_message_dispatch=native_action_original=&native_action_message;
    a::install();b::install();
    b::o_InitActionState=&native_action_init;b::o_EndFinishingState=&empty;
    b::o_SetMessage=b::o_SetSystemMessage=&message_original;
    Node caster;caster.name=L"Rae";Node root_ability;root_ability.id=94;List root_list{&root_ability};
    Node root;root.actor=&caster;root.abilities=&root_list;
    Node child_ability;child_ability.id=137;List child_list{&child_ability};
    Node child;child.actor=&caster;child.abilities=&child_list;child.type=5;
    Node execution,function;function.act=&child;
    auto perform=[&](int id){child_ability.id=id;execution.act=&root;b::d_InitActionState(&execution,nullptr);flush_use();
        b::d_EndFinishingState(&execution,nullptr);execution.act=&child;b::d_InitActionState(&execution,nullptr);
        a::observe_result(&function,&caster,0,0);flush_use();b::d_EndFinishingState(&execution,nullptr);};
    for(bool touch_mode:{false,true}){
        touch=touch_mode;spoken.clear();view_calls=message_calls=0;perform(137);
        expect(spoken==std::vector<std::wstring>{L"Rae: Boost",L"A glowing aura appears around Rae"},"one Boost use plus its actual glow across parent completion and child initialization");
        expect(view_calls==2&&message_calls==2,"both native parent/child UI operations still run exactly once");
        perform(139);expect(spoken.size()==4&&spoken[2]==L"Rae: Boost","separate identical Boost uses remain separate announcements");
        spoken.clear();perform(138);expect(spoken==std::vector<std::wstring>{L"Rae: Boost"},"blank Boost child cannot repeat use or imply an aura");
        spoken.clear();perform(140);expect(spoken==std::vector<std::wstring>{L"Rae: Boost",L"Rae: Self-destruct"},"overboost's different native displayed action remains audible");
    }
    spoken.clear();Node result;result.name=L"Result message";b::d_SetMessage(nullptr,&result,nullptr);flush_use();
    expect(spoken==std::vector<std::wstring>{L"Result message"},"native result messages outside the use-name producer are preserved");
    child.type=0;child_ability.id=137;spoken.clear();execution.act=&child;b::d_InitActionState(&execution,nullptr);flush_use();
    expect(spoken==std::vector<std::wstring>{L"Rae: Boost"},"a normal ability action with the same label is not suppressed by text or ability name");
    child.type=5;Node command;command.id=94;child.command=&command;spoken.clear();
    execution.act=&child;b::d_InitActionState(&execution,nullptr);flush_use();
    expect(spoken==std::vector<std::wstring>{L"Rae: Boost"},"a command-owned action cannot borrow internal-child suppression");child.command=nullptr;
    throw_view=true;bool threw=false;try{view_message_dispatch(&child,nullptr);}catch(int){threw=true;}throw_view=false;
    expect(threw,"native ViewMessage exceptions propagate unchanged");
    spoken.clear();b::d_SetMessage(nullptr,&result,nullptr);flush_use();
    expect(spoken==std::vector<std::wstring>{L"Result message"},"a thrown native use presentation cannot silence a later result");
    child_ability.id=138;expect(!view_message_dispatch(&child,nullptr),"native blank-name return value remains false");child_ability.id=137;
    for(int n:{1,2,4,5,6}){game_id=static_cast<game::Id>(n);spoken.clear();execution.act=&child;b::d_InitActionState(&execution,nullptr);flush_use();
        expect(spoken==std::vector<std::wstring>{L"Rae: Boost"},"other games never receive the FFIII internal-child suppression");}
    std::printf("Boost parent/child use producer: %d failures\n",failures);return failures?1:0;
}
