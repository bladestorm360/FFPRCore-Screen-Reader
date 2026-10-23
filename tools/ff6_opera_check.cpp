// check-sources: text.cpp
// Real opera detours share the native-boundary harness, without game/UI input.
#define main story_runtime_fixture_main
#include "ff6_story_runtime_check.cpp"
#undef main
#include "../src/games/ff6/opera.cpp"
namespace fixture {
struct Opera {bool playing=true,clicked=false,failed=false,timeup=false;void* canvas=nullptr;};
struct State {void* opera;};
bool canvas=false;int phase=0;
bool event_step(void* state,const void*){
    auto* opera=static_cast<Opera*>(static_cast<State*>(state)->opera);
    if(phase==0)canvas=true;
    else if(phase==1)canvas=false;
    else if(phase==2)opera->failed=true;
    return true;
}
bool timer_step(void* state,const void*){static_cast<Opera*>(static_cast<State*>(state)->opera)->timeup=true;return true;}
}
namespace ref {il2cpp::Class* Class::get(){return nullptr;} }
namespace il2cpp {
Class* find_nested_prefix(Class*,const char*){return nullptr;}
int32_t field_offset(Class*,const char*){return -1;}
Method* find_method(Class*,const char*,int){return nullptr;}
void* method_pointer(Method*){return nullptr;}
}
namespace hooks {bool attach_addr(void*,const char*,void*,void**){return false;} }
namespace unity {bool active_self(void* object){fixture::native();return object&&*static_cast<bool*>(object);} }
int main(){
    int failures=story_runtime_fixture_main();
    auto expect=[&](bool v,const char* name){if(!v){++failures;std::printf("FAIL %s\n",name);}};
    fixture::offsets={{"isPlaying",offsetof(fixture::Opera,playing)},
        {"clickedInteractiveIconButton",offsetof(fixture::Opera,clicked)},
        {"isFailedInteractiveIconButton",offsetof(fixture::Opera,failed)},
        {"isInteractiveTimeup",offsetof(fixture::Opera,timeup)},
        {"interactiveIconCanvasObject",offsetof(fixture::Opera,canvas)}};
    fixture::Opera opera;opera.canvas=&fixture::canvas;fixture::State state{&opera};
    ffpr::ff6::g_event_this=ffpr::ff6::g_timer_this=0;
    ffpr::ff6::o_EventMoveNext=&fixture::event_step;ffpr::ff6::o_TimerMoveNext=&fixture::timer_step;
    fixture::speech.clear();ffpr::ff6::d_EventMoveNext(&state,nullptr);
    expect(fixture::speech.size()==1&&fixture::speech.back()==L"Opera action: confirm now.","native canvas rising edge announces action window");
    ffpr::ff6::d_EventMoveNext(&state,nullptr);
    expect(fixture::speech.size()==1,"same native prompt occurrence is not repeated on each coroutine frame");
    fixture::phase=1;ffpr::ff6::d_EventMoveNext(&state,nullptr);
    fixture::phase=0;ffpr::ff6::d_EventMoveNext(&state,nullptr);
    expect(fixture::speech.size()==2,"a new prompt speaks identical content again");
    fixture::phase=2;ffpr::ff6::d_EventMoveNext(&state,nullptr);
    expect(fixture::speech.back()==L"Failed","existing failed flag feedback remains intact");
    fixture::canvas=false;const auto before=fixture::speech.size();ffpr::ff6::d_TimerMoveNext(&state,nullptr);
    expect(fixture::speech.size()==before+1&&fixture::speech.back()==L"Out of time","failure timer produces timeout feedback without claiming an action prompt");
    opera.playing=false;opera.failed=false;fixture::phase=0;const auto stopped=fixture::speech.size();ffpr::ff6::d_EventMoveNext(&state,nullptr);
    expect(fixture::speech.size()==stopped,"inactive opera cannot produce a prompt");
    std::printf("FF6 opera producer check: %d failures\n",failures);return failures?1:0;
}
