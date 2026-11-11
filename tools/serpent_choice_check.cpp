// check-sources: text.cpp
// Actual rendered arrow producers and story-controller dispatch; no game run.
#define FFPR_SERPENT_REAL
#define main retained_serpent_story_main
#include "ff6_story_runtime_check.cpp"
#undef main
#include "../src/games/ff6/serpent_choices.cpp"
namespace serpent_fixture {
int init_calls=0,arrow_calls=0,finish_calls=0,rendered=-1,cases=0,errors=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
void original_arrow(void*,int32_t direction,const void*){++arrow_calls;rendered=direction;}
void original_key_init(void* self,const void* mi){++init_calls;ffpr::ff6::serpent::d_key_arrow(self,0,mi);}
void original_touch_init(void* self,const void* mi){++init_calls;ffpr::ff6::serpent::d_touch_arrow(self,0,mi);}
void original_finish(void*,const void*){++finish_calls;}
}
int main(){
 if(retained_serpent_story_main())return 1;
 using namespace serpent_fixture;using namespace ffpr::ff6::serpent;
 o_key_init=&original_key_init;o_touch_init=&original_touch_init;o_key_arrow=o_touch_arrow=&original_arrow;o_key_finish=o_touch_finish=&original_finish;
 fixture::map=573;fixture::ready=false;fixture::focused=true;fixture::speech.clear();fixture::posts.clear();
 d_key_init(nullptr,nullptr);check(choice_available()&&rendered==0&&fixture::speech.empty()&&fixture::posts.size()==1,"real initial arrow is copied without inline speech");
 fixture::flush();check(fixture::speech==std::vector<std::wstring>{L"Junction: Right."},"rendered Right is announced once after native initialization");
 const int reads=fixture::state_reads,requests=fixture::requests;ffpr::ff6::story::next(true);
 check(fixture::speech.back()==L"Choose Right at the junction."&&fixture::state_reads==reads&&fixture::requests==requests,"Shift+G supplies the actual Right route during blocked field input without capture/search");
 d_key_arrow(nullptr,1,nullptr);fixture::flush();check(rendered==1&&fixture::speech.back()==L"Left","changed rendered arrow remains ordinary UI information");
 const auto before=fixture::posts.size(),lines=fixture::speech.size();const auto began=std::chrono::steady_clock::now();
 for(int i=0;i<10000;++i)d_key_arrow(nullptr,1,nullptr);
 const double repeated=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
 check(fixture::posts.size()==before&&fixture::speech.size()==lines,"unchanged native renderer writes schedule no work or speech");
 ffpr::ff6::story::next(true);ffpr::ff6::story::next(true);check(fixture::speech.size()==lines+2,"identical explicit guide repeats are never deduplicated");
 fixture::nav_guidance=false;actions::run("GameModule.PuzzleGuidePolicyChanged");ffpr::ff6::story::next(true);
 check(fixture::speech.back()==L"Navigation puzzle guidance is off","timed solution obeys navigation policy");
 d_key_arrow(nullptr,0,nullptr);fixture::flush();check(fixture::speech.back()==L"Right","solution-off preserves the actual selected arrow");fixture::nav_guidance=true;
 d_key_finish(nullptr,nullptr);check(!choice_available()&&finish_calls==1,"native finish retires availability and preserves original call");
 fixture::speech.clear();d_key_init(nullptr,nullptr);auto stale=fixture::posts.back();d_key_finish(nullptr,nullptr);fixture::flush();stale();
 check(fixture::speech.empty(),"closed timed window cannot revive a late initial announcement");
 for(int i=0;i<2;++i){d_key_init(nullptr,nullptr);fixture::flush();d_key_finish(nullptr,nullptr);}
 check(fixture::speech==std::vector<std::wstring>{L"Junction: Right.",L"Junction: Right."},"new native occurrences repeat identical visible information");
 d_touch_init(nullptr,nullptr);fixture::flush();check(choice_available()&&g_origin==2,"Touch has the same scoped native lifecycle");
 d_touch_arrow(nullptr,1,nullptr);fixture::flush();check(fixture::speech.back()==L"Left","Touch arrow changes use the actual enum");
 events::fire(events::Kind::BattleEntering);check(!choice_available(),"battle entry retires timed choice state");events::fire(events::Kind::BattleEnd);
 d_touch_init(nullptr,nullptr);events::fire(events::Kind::MapChanging);fixture::flush();check(!choice_available(),"map change cancels queued native UI output");
 fixture::map=96;const auto outside=fixture::speech.size();d_key_init(nullptr,nullptr);fixture::flush();check(!choice_available()&&fixture::speech.size()==outside,"unrelated maps do not acquire trench choice ownership");
 fixture::map=573;g_keys=false;const int delegated=init_calls;d_key_init(nullptr,nullptr);check(!choice_available()&&init_calls==delegated+1,"partially bound interface delegates without owning a timed window");g_keys=true;
 d_key_init(nullptr,nullptr);events::fire(events::Kind::TitleScreen);fixture::flush();check(!choice_available(),"title retires the old occurrence");
 check(fixture::wrong_owner==0&&arrow_calls>=10000,"all observation/speech stays on owner and every original renderer write is retained");
 std::printf("Serpent arrows:%d assertions/%d failures;10000 unchanged native writes %.3fms aggregate in throttled fake-native fixture,not game/audible evidence\n",cases,errors,repeated);return errors?1:0;
}
