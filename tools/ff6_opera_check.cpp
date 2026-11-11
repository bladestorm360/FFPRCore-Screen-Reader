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
frame::Task initial;
void read_initial(){initial=std::move(posts.back());posts.pop_back();initial();}
bool event_context=false,visible=true;
struct Multiple {void* list;void* cursor;};
int cursor=0;
std::wstring row=L"Displayed lyric";
void simple(void*,const void*){}
void play(void* p,int32_t,const void*){static_cast<Opera*>(p)->playing=true;}
void* select(void*,void*,void*,float,bool,const void*){return nullptr;}
void focus(void*,int32_t i,const void*){cursor=i;}
bool event_step(void* state,const void*){
    auto* opera=static_cast<Opera*>(static_cast<State*>(state)->opera);
    if(phase==0)canvas=true;
    else if(phase==1)canvas=false;
    else if(phase==2)opera->failed=true;
    return true;
}
bool timer_step(void* state,const void*){static_cast<Opera*>(static_cast<State*>(state)->opera)->timeup=true;return true;}
}
namespace il2cpp {
Class* find_nested_prefix(Class*,const char*){return nullptr;}
int32_t field_offset(Class*,const char*){return -1;}
Method* find_method(Class*,const char*,int){return nullptr;}
void* method_pointer(Method*){return nullptr;}
}
namespace hooks {bool attach_addr(void*,const char*,void*,void**){return false;} }
namespace unity {bool active_self(void* object){fixture::native();return object&&*static_cast<bool*>(object);} int list_count(void* p){return p?int(static_cast<std::vector<void*>*>(p)->size()):0;}void* list_at(void* p,int i){return (*static_cast<std::vector<void*>*>(p))[i];} }
namespace displayed {bool container_visible(void*){return fixture::visible;}void* field(void* p,const char*){return p;}std::wstring text_field(void*,const char*){return fixture::row;} }
namespace ui {int cursor_index(void*){return fixture::cursor;}std::wstring menu_position(const std::wstring& s,int i,int n){return s+L" "+std::to_wstring(i+1)+L" of "+std::to_wstring(n);} }
namespace input::bindings {void event_guide_context(bool b){fixture::event_context=b;}bool event_guide_context(){return fixture::event_context;} }
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
    using namespace ffpr::ff6;
    install();
    o_Play=&fixture::play;o_Destroy=&fixture::simple;o_Show=&fixture::simple;o_Close=&fixture::simple;
    o_Select=&fixture::select;o_Focus=&fixture::focus;
    fixture::offsets["contentList"]=offsetof(fixture::Multiple,list);
    fixture::offsets["selectCursor"]=offsetof(fixture::Multiple,cursor);
    opera.playing=false;retire();d_Play(&opera,200,nullptr);
    expect(fixture::event_context&&g_opera==&opera,"accepted native performance arms only read-only event context");
    constexpr const wchar_t* ids[3][2]={{L"E0318_00_118_c_01",L"E0318_00_118_c_02"},
        {L"E0318_01_118_c_01",L"E0318_01_118_c_02"},{L"E0318_02_118_c_01",L"E0318_02_118_c_02"}};
    fixture::RoofName keys[3][2];
    for(int q=0;q<3;++q)for(int i=0;i<2;++i){keys[q][i].length=int(wcslen(ids[q][i]));wcscpy_s(keys[q][i].chars,ids[q][i]);}
    std::vector<void*> rows{&opera,&opera};fixture::Multiple multiple{&rows,&fixture::cursor};
    for(int q=0;q<3;++q)for(int reverse=0;reverse<2;++reverse) {
        std::vector<void*> array{&keys[q][reverse],&keys[q][1-reverse]};
        d_Select(&opera,nullptr,&array,3.f,false,nullptr);
        expect(g_choice.question==q+1&&g_choice.correct==(reverse? (q==1?1:2):(q==1?2:1)),"actual native question order determines the correct option");
        const auto answer=g_choice.correct;d_Play(&opera,200,nullptr);
        expect(g_choice.correct==answer,"ignored native Play cannot reset the live question");
        const auto before_hint=fixture::speech.size();guide();expect(fixture::speech.back()==L"Wait for the next opera cue.","unshown choice cannot reveal a stale cursor phase");
        d_Show(&multiple,nullptr);fixture::read_initial();
        expect(fixture::speech.size()==before_hint+2&&fixture::speech.back()==L"Displayed lyric 1 of 2","native Show reads the initial displayed option once");
        d_Focus(&multiple,1,nullptr);expect(fixture::speech.back()==L"Displayed lyric 2 of 2","native arrow focus reads changed displayed selection");
        guide();expect(fixture::speech.back().find(L"Opera lyric ")==0,"Shift+G repeats current qualified answer");
        d_Close(&multiple,nullptr);guide();expect(fixture::speech.back()==L"Wait for the next opera cue.","closed choice cannot repeat stale answer");
        fixture::cursor=0;
    }
    std::vector<void*> bad{&keys[0][0],&keys[1][1]};
    d_Select(&opera,nullptr,&bad,0,false,nullptr);expect(!g_choice.question,"mixed question array fails closed");
    bad[1]=bad[0];d_Select(&opera,nullptr,&bad,0,false,nullptr);expect(!g_choice.question,"duplicate array fails closed");
    bad={&keys[1][0],&keys[1][1]};
    d_Select(&opera,nullptr,&bad,0,false,nullptr);d_Show(&multiple,nullptr);
    fixture::ready=false;const auto jobs=fixture::jobs.size();
    ffpr::ff6::story::next(true);
    expect(fixture::speech.back()==L"Opera lyric 2 of 3: choose option 2 of 2."&&fixture::jobs.size()==jobs,
        "actual story guide dispatch works while field navigation is unavailable without worker/native movement");
    fixture::read_initial();expect(fixture::speech.back().find(L"Opera lyric 2 of 3: choose option 2 of 2.")==0,
        "active guide includes current answer with displayed initial selection");
    fixture::other_guidance=false;actions::run("GameModule.PuzzleGuidePolicyChanged");
    read_choice(&multiple,true);
    expect(fixture::speech.back().find(L"Displayed lyric")==0&&fixture::speech.back().find(L"choose option")==std::wstring::npos,
        "disabled lyric guidance preserves the current rendered choice without revealing an answer");
    guide();expect(fixture::speech.back()==L"Non-navigation puzzle guidance is off","explicit Opera solution obeys its independent type toggle");
    fixture::other_guidance=true;
    auto late=fixture::initial;d_Close(&multiple,nullptr);const auto after_close=fixture::speech.size();late();
    expect(fixture::speech.size()==after_close,"late initial read cannot revive a closed native choice");
    ffpr::ff6::story::stop(false);expect(fixture::event_context,"stopping guide keeps native read-only context available for restart");
    const auto generation=g_choice_generation;d_Destroy(&opera,nullptr);
    expect(!fixture::event_context&&!g_opera&&g_choice_generation>generation,"native destroy retires input ownership and pending choice occurrence");
    fixture::ready=true;
    const auto began=std::chrono::steady_clock::now();
    opera.playing=true;opera.failed=false;fixture::phase=1;
    for(int i=0;i<7200;++i)d_EventMoveNext(&state,nullptr);
    const double aggregate=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
    std::printf("Opera fixture: 7200 actual coroutine-detour callbacks aggregate %.3fms; fake native/speech boundaries, natural frames/audio unmeasured\n",aggregate);
    std::printf("Opera choices: 6 ordered/reversed phases, rendered initial/focus/close and malformed lifecycle verified\n");
    std::printf("FF6 opera producer check: %d failures\n",failures);return failures?1:0;
}
