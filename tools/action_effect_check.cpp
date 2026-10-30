// check-sources:
// Actual per-target native result producer, native Miss-frame ordering and action
// lifetimes. No EffectEntity/sprite callback is installed or used by this module.
#include <windows.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include <cstring>
#include <map>
#include "../src/core/battle/action_effects.cpp"
namespace fixture {
struct Node {std::wstring name;void* abilities=nullptr;void* command=nullptr;void* attacker=nullptr;void* act=nullptr;int id=0,type=0;bool enemy=false;};
using List=std::vector<void*>;
DWORD owner=GetCurrentThreadId();int wrong_owner=0,native_reads=0;bool battle=true,missing=false;
game::Id game_id=game::Id::FF3;
std::vector<std::wstring> spoken;
struct Due {int frames;frame::Task task;};std::vector<Due> due;
std::map<events::Kind,std::vector<events::Handler>> handlers;
void native(){if(GetCurrentThreadId()!=owner)++wrong_owner;else ++native_reads;}
void tick(){auto old=std::move(due);due.clear();for(auto& row:old)if(--row.frames<=0)row.task();else due.push_back(std::move(row));}
void flush(){tick();tick();}
int id(void* p,const void*){native();return static_cast<Node*>(p)->id;}
}
namespace core {Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace game {Id current(){return fixture::game_id;}}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
bool Class::is(void* p){fixture::native();return p&&static_cast<fixture::Node*>(p)->enemy;}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
int32_t Field::get(){fixture::native();if(fixture::missing)return -1;
 if(!std::strcmp(name,"abilityList"))return offsetof(fixture::Node,abilities);
 if(!std::strcmp(name,"<Command>k__BackingField"))return offsetof(fixture::Node,command);
 if(!std::strcmp(name,"<CurrentActType>k__BackingField"))return offsetof(fixture::Node,type);
 if(!std::strcmp(name,"<AttackUnitData>k__BackingField"))return offsetof(fixture::Node,attacker);
 if(!std::strcmp(name,"<battleActData>k__BackingField"))return offsetof(fixture::Node,act);return -1;}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
il2cpp::Method* Method::get(){fixture::native();return reinterpret_cast<il2cpp::Method*>(1);}
void* Method::ptr(){fixture::native();return reinterpret_cast<void*>(&fixture::id);}
void call_failed_note(const char*){std::abort();}
}
namespace ffpr::battle {
ref::Class kBattleEnemyData{"Last.Battle","BattleEnemyData",game::kFF3};
bool in_battle(){return fixture::battle;}
std::wstring unit_name(void* p){fixture::native();return p?static_cast<fixture::Node*>(p)->name:L"";}
}
namespace unity {
int list_count(void* p){fixture::native();return p?int(static_cast<fixture::List*>(p)->size()):0;}
void* list_at(void* p,int i){fixture::native();return static_cast<fixture::List*>(p)->at(i);}
}
namespace frame {bool on_main_thread(){return GetCurrentThreadId()==fixture::owner;}void after_frames(int n,Task task,const char*){fixture::due.push_back({n,std::move(task)});}}
namespace modtext {
std::wstring T(std::wstring_view key){return std::wstring(key);}
std::wstring TF(std::wstring_view key,std::initializer_list<std::wstring> args){std::wstring out(key);size_t i=0;for(const auto& arg:args){auto token=L"{"+std::to_wstring(i++)+L"}";auto at=out.find(token);if(at!=out.npos)out.replace(at,token.size(),arg);}return out;}
}
namespace ffpr {void say(std::wstring_view text,bool interrupt){fixture::native();if(interrupt)std::abort();fixture::spoken.emplace_back(text);}}
namespace events {void on(Kind kind,Handler handler){fixture::handlers[kind].push_back(std::move(handler));}}
int main(){
 namespace a=ffpr::battle::action_effects;using namespace fixture;int failures=0;
 auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 expect(a::install()==0&&a::enabled,"required result metadata initializes with zero aura/sprite hooks");
 Node caster;caster.name=L"Rae";Node root_ability;root_ability.id=94;List root_list{&root_ability};
 Node root;root.abilities=&root_list;root.attacker=&caster;
 Node child_ability;child_ability.id=137;List child_list{&child_ability};Node child;child.abilities=&child_list;child.attacker=&caster;
 Node function;function.act=&child;
 a::observe_action(&root,L"Rae",L"Boost");expect(spoken.empty()&&due.empty(),"action name alone proves no result");
 a::observe_result(&function,&caster,0,0);expect(spoken.empty()&&due.size()==1,"result waits for the native UI/Miss frame");
 tick();expect(spoken==std::vector<std::wstring>{L"A glowing aura appears around Rae"},"Boost uses the requested sentence with one actor name and no heading");
 a::observe_result(&function,&caster,0,0);flush();expect(spoken.size()==1,"two native dictionaries cannot repeat one function/target result");
 for(int i=0;i<20;++i)tick();expect(spoken.size()==1,"later animation frames have no result producer or speech");
 a::finish_action();a::observe_result(&function,&caster,0,0);flush();expect(spoken.size()==1,"late result after action completion cannot borrow context");
 a::observe_action(&root,L"Rae",L"Boost");a::observe_result(&function,&caster,0,0);flush();expect(spoken.size()==2,"separate identical Boost uses remain separate events");
 child_ability.id=139;a::observe_action(&root,L"Rae",L"Boost");a::observe_result(&function,&caster,0,0);flush();expect(spoken.size()==3,"second actual Focus child has the same visual cue");
 const auto old=spoken.size();
 for(int id:{94,138,140}){child_ability.id=id;a::observe_action(&root,L"Rae",L"Boost");a::observe_result(&function,&caster,0,0);}
 flush();expect(spoken.size()==old,"parent, blank and overboost variants cannot imply a Focus aura");
 child_ability.id=137;Node other;other.name=L"Corrad";child.attacker=&other;
 a::observe_action(&root,L"Rae",L"Boost");a::observe_result(&function,&caster,0,0);flush();expect(spoken.size()==old,"result owner must be the native action actor");child.attacker=&caster;
 root_ability.id=87;function.act=&root;spoken.clear();
 a::observe_action(&root,L"Rae",L"Guard");a::observe_result(&function,&caster,0,0);flush();
 expect(spoken==std::vector<std::wstring>{L"A glowing barrier appears around Rae"},"Guard describes its actual visible barrier at the successful self-result");
 for(int i=0;i<20;++i)a::observe_result(&function,&caster,0,0);flush();
 expect(spoken.size()==1,"Guard result identity does not repeat with animation/update callbacks");
 spoken.clear();a::observe_action(&root,L"Rae",L"Guard");
 a::observe_result(&function,&caster,0,2);a::observe_result(&function,&other,0,0);a::observe_result(&function,&caster,1,0);flush();
 expect(spoken.empty(),"Guard misses, another target and nonzero results cannot claim a self barrier");
 a::observe_action(&root,L"Rae",L"Guard");ffpr::battle::message_line::action_started(L"Rae",L"Guard");
 a::observe_result(&function,&caster,0,0);ffpr::battle::message_line::game_message(L"Guard",false);flush();
 expect(spoken==std::vector<std::wstring>{L"Rae: Guard",L"A glowing barrier appears around Rae"},"native Guard use precedes its meaningful visual description");
 root_ability.id=96;function.act=&root;Node one,two,three;one.name=L"Enemy1";two.name=L"Enemy2";three.name=L"Enemy3";one.enemy=two.enemy=three.enemy=true;
 spoken.clear();a::observe_action(&root,L"Rae",L"Draw Attacks");
 a::observe_result(&function,&one,0,0);
 a::observe_result(&function,&two,0,2); // existing renderer alone owns Miss
 frame::after_frames(1,[]{ffpr::say(L"Enemy2: Miss",false);},"ExistingRenderedMiss");
 a::observe_result(&function,&three,0,0);tick();
 expect(spoken==std::vector<std::wstring>{L"Enemy1: Affected by Draw Attacks",L"Enemy2: Miss",L"Enemy3: Affected by Draw Attacks"},"mixed enemy successes/miss retain target identity and native UI order");
 a::observe_result(&function,&one,0,0);flush();expect(spoken.size()==3,"same result phase has no repeated enemy cue");
 a::observe_action(&root,L"Rae",L"Draw Attacks");a::observe_result(&function,&one,0,0);flush();expect(spoken.size()==4,"new action can affect the same enemy again");
 one.name=three.name=L"Goblin";spoken.clear();a::observe_action(&root,L"Rae",L"Draw Attacks");
 a::observe_result(&function,&one,0,0);a::observe_result(&function,&three,0,0);flush();expect(spoken.size()==2&&spoken[0]==spoken[1],"distinct enemies with equal names are not text-deduplicated");
 spoken.clear();a::observe_action(&root,L"Rae",L"Draw Attacks");
 for(int hit:{-1,1,2,3,4,5,6,7})a::observe_result(&function,&one,0,hit);
 a::observe_result(&function,&one,1,0);a::observe_result(&function,&caster,0,0);flush();expect(spoken.empty(),"misses, nonzero damage, recovery and friendly targets never become enemy success");
 Node old_action=root,old_function;old_function.act=&old_action;
 a::observe_action(&root,L"Rae",L"Draw Attacks");a::observe_result(&old_function,&one,0,0);flush();
 expect(spoken.empty(),"another action owned by the same actor cannot borrow this Draw lifetime");
 Node command;command.id=96;root.command=&command;root.abilities=nullptr;
 a::observe_action(&root,L"Rae",L"Draw Attacks");a::observe_result(&function,&one,0,0);flush();expect(spoken.size()==1,"fixed command without an ability list has native ownership");root.abilities=&root_list;
 spoken.clear();a::observe_action(&root,L"Rae",L"Draw Attacks");a::observe_result(&function,&one,0,0);a::finish_action();
 a::observe_action(&root,L"Rae",L"Draw Attacks");flush();expect(spoken.size()==1,"next action never cancels an already displayed valid result");
 spoken.clear();a::observe_result(&function,&one,0,0);for(auto& fn:handlers[events::Kind::BattleEnd])fn();flush();expect(spoken.empty(),"battle lifecycle cancels copied pending result speech");
 a::observe_action(&root,L"Rae",L"Draw Attacks");ffpr::battle::message_line::action_started(L"Rae",L"Draw Attacks");a::observe_result(&function,&one,0,0);
 ffpr::battle::message_line::game_message(L"Draw Attacks",false);flush();expect(spoken==std::vector<std::wstring>{L"Rae: Draw Attacks",L"Goblin: Affected by Draw Attacks"},"native action/message pairing precedes per-enemy result");
 spoken.clear();a::observe_action(&root,L"Rae",L"Draw Attacks");std::array<Node,33> enemies;
 for(auto& enemy:enemies){enemy.name=L"Goblin";enemy.enemy=true;a::observe_result(&function,&enemy,0,0);}expect(a::count==32,"per-action result identity storage is bounded");flush();expect(spoken.size()==32,"bounded distinct target results all remain audible");
 spoken.clear();a::observe_action(&root,L"Rae",L"Draw Attacks");std::thread wrong([&]{a::observe_result(&function,&one,0,0);});wrong.join();expect(wrong_owner==0&&due.empty(),"wrong-thread callback performs no native read or speech");
 root_ability.id=1;a::observe_action(&root,L"Rae",L"Attack");int reads=native_reads;
 const auto start=std::chrono::steady_clock::now();for(int i=0;i<10000;++i)a::observe_result(&function,&one,0,0);
 const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
 expect(native_reads==reads,"unrelated results take no metadata/name path");
 // Complete worst-capacity episode: root capture, every target's result,
 // copied UI deferrals and fake speech. This excludes real native/audio cost.
 root_ability.id=96;double episode_total=0,episode_peak=0;int episode_reads=0;
 for(int iteration=0;iteration<100;++iteration){
  spoken.clear();const int before_reads=native_reads;const auto episode_start=std::chrono::steady_clock::now();
  a::observe_action(&root,L"Rae",L"Draw Attacks");
  for(size_t i=0;i<32;++i)a::observe_result(&function,&enemies[i],0,0);
  flush();const auto cost=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-episode_start).count();
  episode_total+=cost;episode_peak=std::max(episode_peak,cost);episode_reads=std::max(episode_reads,native_reads-before_reads);
  expect(spoken.size()==32,"complete maximum-target episode preserves every independent result");
 }
 std::printf("100 complete32-target episodes %.3fms aggregate, %.3fms peak; max%d fake SDK observations/episode; actual native/audio unmeasured\n",episode_total,episode_peak,episode_reads);
 missing=true;a::install();expect(!a::enabled,"missing required metadata fails closed");missing=false;game_id=game::Id::FF2;expect(a::install()==0,"other games add no hooks");
 std::printf("Per-target effects: %d failures; 10000 unrelated result callbacks %.3fms aggregate; wrong-owner=%d; no game run\n",failures,elapsed,wrong_owner);
 return failures?1:0;
}
