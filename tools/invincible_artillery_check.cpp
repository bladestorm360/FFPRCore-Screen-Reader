// check-sources: text.cpp json.cpp battle/scan_feedback.cpp
// Actual action/UI producers with the real FFIII ability identity, no game run.
#define main retained_battle_feedback_main
#include "battle_feedback_check.cpp"
#undef main
namespace fixture {
constexpr auto artillery=L"The artillery is providing cover!";
int hit_calls=0,ui_calls=0;bool touch=false,throw_hit=false,hit_return=true;
void artillery_ui(void*,void*,const void*){++ui_calls;}
bool artillery_hit(void*,void*,const void*){
 ++hit_calls;Node message;message.name=artillery;
 if(touch)ffpr::battle::d_SetSystemMessage(nullptr,&message,nullptr);
 else ffpr::battle::d_SetMessage(nullptr,&message,nullptr);
 if(throw_hit)throw 1;
 return hit_return;
}
void flush_artillery(){auto posted=std::move(action_ui_posts);action_ui_posts.clear();for(auto& task:posted)task();}
}
int main(){
 using namespace fixture;namespace b=ffpr::battle;namespace a=b::action_effects;
 int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 game_id=game::Id::FF3;hit_message_dispatch=native_hit_original=&artillery_hit;
 a::install();b::install();b::o_InitActionState=b::o_EndFinishingState=&empty;
 b::o_SetMessage=b::o_SetSystemMessage=&artillery_ui;
 Node ability;ability.id=141;List abilities{&ability};Node act;act.abilities=&abilities;act.name=artillery;
 Node execution;execution.act=&act;
 const auto start=[&]{b::d_InitActionState(&execution,nullptr);};
 const auto hit=[&]{return hit_message_dispatch(&ability,nullptr,nullptr);};
 for(bool touch_mode:{false,true})for(bool early:{false,true}){
  touch=touch_mode;b::removal_reset_state();spoken.clear();hit_calls=ui_calls=0;
  start();if(!early)flush_artillery();
  expect(hit(),"native hit presentation keeps its true return value");flush_artillery();
  expect(spoken==std::vector<std::wstring>{artillery},"one anonymous artillery cue across early or late native conclusion");
  expect(hit_calls==1&&ui_calls==1,"native hit and displayed UI still run exactly once");
  hit_return=false;expect(!hit(),"native false result is unchanged");hit_return=true;flush_artillery();
  expect(spoken.size()==1,"additional callbacks for this artillery action do not repeat the cue");
  // Result rendering and unrelated messages are independent native producers.
  spoken.clear();b::damage_view(L"Enemy1",42,0,false);b::damage_view(L"Enemy2",0,2,false);
  Node result;result.name=L"A different result";b::d_SetMessage(nullptr,&result,nullptr);flush_artillery();
  expect(spoken==std::vector<std::wstring>{L"Enemy1: 42 damage",L"Enemy2: Miss",L"A different result"},"damage, misses and other result messages remain audible");
  spoken.clear();start();flush_artillery();hit();flush_artillery();
  expect(spoken==std::vector<std::wstring>{artillery},"the next artillery occurrence is read again");
 }
 spoken.clear();b::removal_reset_state();hit();flush_artillery();
 expect(spoken==std::vector<std::wstring>{artillery},"without a prepared opening cue, native conclusion remains a fallback");
 act.name.clear();spoken.clear();start();flush_artillery();spoken.clear();hit();flush_artillery();
 expect(spoken==std::vector<std::wstring>{artillery},"unavailable opening description cannot silence the native result");act.name=artillery;
 start();flush_artillery();spoken.clear();ability.id=87;hit();flush_artillery();
 expect(spoken==std::vector<std::wstring>{artillery},"different ability identity is never suppressed, even with identical text");ability.id=141;
 throw_hit=true;bool threw=false;try{hit();}catch(int){threw=true;}throw_hit=false;
 expect(threw,"native exceptions propagate");spoken.clear();Node ordinary;ordinary.name=artillery;b::d_SetMessage(nullptr,&ordinary,nullptr);flush_artillery();
 expect(spoken==std::vector<std::wstring>{artillery},"exception unwinds the source scope before another identical message");
 for(auto boundary:{events::Kind::BattleEntering,events::Kind::BattleEnd,events::Kind::SceneLoaded}){
  start();flush_artillery();events::fire(boundary);spoken.clear();hit();flush_artillery();
  expect(spoken==std::vector<std::wstring>{artillery},"battle/scene retirement cannot borrow the previous action's prepared cue");
 }
 // Include the entire action/late-conclusion episode, not just one helper.
 std::vector<double> samples;for(int i=0;i<1000;++i){
  spoken.clear();const auto began=std::chrono::steady_clock::now();start();flush_artillery();hit();flush_artillery();
  samples.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-began).count());
  expect(spoken==std::vector<std::wstring>{artillery}&&action_ui_posts.empty(),"repeated independent encounters retain one cue without a task backlog");
 }std::sort(samples.begin(),samples.end());
 std::printf("Artillery whole episode, fake native/UI/speech: median %.3fus p99 %.3fus max %.3fus; 1000 episodes\n",samples[500],samples[990],samples.back());
 for(int n:{1,2,4,5,6}){
  game_id=static_cast<game::Id>(n);spoken.clear();hit();flush_artillery();
  expect(spoken==std::vector<std::wstring>{artillery},"other games retain their native conclusion messages");
 }
 std::printf("Invincible artillery producers: %d failures\n",failures);return failures?1:0;
}
