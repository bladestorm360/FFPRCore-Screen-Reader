// check-sources: text.cpp json.cpp
// Actual damage, condition-add and special-effect producers; no game is run.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <chrono>
#include <fstream>
#include "../src/core/json.h"
#include "../src/core/frame.cpp"
#include "../src/core/battle/battle_message.cpp"
#include "../src/core/battle/scan_feedback.cpp"
#include "../src/core/battle/special_feedback.cpp"
#include "../src/core/battle/stat_feedback.cpp"
#include "../src/core/condition_labels.h"
namespace ffpr::battle::action_effects {
// This isolated scan host intentionally has no Boost action-effect observer;
// the joint boost_use_check exercises the real classifier and use producer.
bool redundant_boost_use(void*) {return false;}
bool invincible_artillery_action() {return false;}
bool invincible_artillery_result(void*) {return false;}
void observe_action(void*,const std::wstring&,const std::wstring&) {}
void finish_action() {}
void observe_result(void*,void*,int,int) {}
}
namespace fixture {
struct Scan { bool first=true; int32_t index=0; };
bool scan_metadata=true;
std::map<events::Kind,std::vector<events::Handler>> event_handlers;
struct Node {void* functions=nullptr;void* act=nullptr;void* targets=nullptr;void* actor=nullptr;void* after_results=nullptr;void* temporary=nullptr;void* modifiers=nullptr;std::array<int32_t,29> temporary_values{},base_values{};int value=0,type=0,id=0,sibling_type=0,hp=10;std::wstring name,kind;std::vector<void*> conditions;};
using List=std::vector<void*>;using Dict=std::vector<std::pair<void*,void*>>;
std::vector<std::wstring> spoken;std::vector<std::function<void()>> work;
std::vector<frame::Task> native_posts;std::vector<frame::Step> admission_retries;bool background_busy=false;
bool defer_digits=false;std::vector<std::function<void()>> digit_callbacks;
bool enemy_critical=false;
Dict* next_modifiers=nullptr;bool direct_applies=true,ff5_callback=false;
json::Value status_catalog;
game::Id game_id=game::Id::FF2;Node *effect=nullptr,*victim=nullptr,*caster=nullptr,*provider=nullptr,*key=nullptr;int drawn=1,native_calls=0,expected_type=-1;bool apply=false;
void empty(void*,const void*){++native_calls;}
void view(void*,void*,bool,const void*){++native_calls;}
void exchange(void*,void*,void*,void*,int32_t,const void*){++native_calls;}
void update(void*,const void*) {++native_calls;if(apply)ffpr::battle::special_feedback::d_exchange(provider,caster,victim,key,0,nullptr);}
void render(void*,void*,int32_t,bool,bool,const void*) {++native_calls;}
void render6(void*,void*,int32_t,unity::Vec2,bool,bool,const void*) {++native_calls;}
void* utility(void*,int32_t value,bool recovery,bool miss,const void*) {
 if(game_id==game::Id::FF6)ffpr::battle::d_render_damage6(nullptr,nullptr,value,{},recovery,miss,nullptr);
 else ffpr::battle::d_render_damage(nullptr,nullptr,value,recovery,miss,nullptr);
 return nullptr;
}
void* utility2(void* unit,int32_t value,bool recovery,bool miss,bool,const void* mi) {return utility(unit,value,recovery,miss,mi);}
void basic(void*,void* unit,int32_t value,int32_t hit,bool recovery,const void*) {ffpr::battle::d_utility_damage(unit,value,recovery,hit==2,nullptr);}
void basic2(void*,void* unit,int32_t value,int32_t hit,bool recovery,bool se,const void*) {ffpr::battle::d_utility_damage2(unit,value,recovery,hit==2,se,nullptr);}
void basic5(void* self,void* unit,int32_t value,int32_t hit,bool recovery,int32_t,const void* mi) {basic(self,unit,value,hit,recovery,mi);}
void result_view(void* self,void* target,bool recovery,const void*) {
 // The game's ViewEntity and AfterViewDamage loops can reach the typed damage
 // producer directly. This dispatch models all six real ABI variants, without
 // manufacturing a surrounding per-unit view hook/resource scope.
 if(game_id==game::Id::FF2)ffpr::battle::d_CreateDamageViewFF2(self,target,987654,expected_type,recovery,true,nullptr);
 else if(game_id==game::Id::FF5)ffpr::battle::d_CreateDamageViewFF5(self,target,987654,expected_type,recovery,0,nullptr);
 else ffpr::battle::d_CreateDamageView(self,target,987654,expected_type,recovery,nullptr);
}
void after_batch(void* self,const void*) {
 const int previous_type=expected_type,previous_digits=drawn;
 expected_type=0;drawn=1;result_view(self,victim,false,nullptr);
 expected_type=6;result_view(self,caster,true,nullptr);
 expected_type=previous_type;drawn=previous_digits;
}
void status_modify(void*,void* unit,const void*) {
 ++native_calls;
 auto* temporary=static_cast<Node*>(static_cast<Node*>(unit)->temporary);
 if(temporary&&next_modifiers)temporary->modifiers=next_modifiers;
}
void direct_modify(void* parameter,int32_t type,int32_t value,const void*) {
 ++native_calls;const int id=ffpr::battle::stat_feedback::slot(type);
 if(type==2||type==3)return; // native current HP/MP, not temporary maximum fields
 if(direct_applies&&id>0&&id<29)static_cast<Node*>(parameter)->temporary_values[id]+=value;
}
void stat_add(void*,void* unit,int32_t,const void*) {
 if(game_id==game::Id::FF2)ffpr::battle::d_UpdateTempParamter(nullptr,unit,nullptr);
 else ffpr::battle::stat_feedback::status(nullptr,unit,nullptr);
}
void stat_add5(void* self,void* unit,int32_t id,bool callback,const void* mi) {
 ff5_callback=callback;stat_add(self,unit,id,mi);
}
}
namespace core {Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
bool Class::is(void*p){if(!p)return false;const auto&kind=static_cast<fixture::Node*>(p)->kind;return kind==std::wstring(name,name+std::strlen(name))||(std::string(name)=="BattleFunction13"&&kind==L"BattleFunction13Serial");}
il2cpp::Class* Class::get(){return nullptr;}
int32_t Field::get(){
 const std::string n=name;
 if(std::string(cls.name)=="ScanBattleFunction"&&!fixture::scan_metadata)return -1;
 if(std::string(cls.name)=="ScanBattleFunction"&&n=="isFirstProcess")return offsetof(fixture::Scan,first);
 if(std::string(cls.name)=="ScanBattleFunction"&&n=="messageDisplayCount")return offsetof(fixture::Scan,index);
 if(n=="<functionBases>k__BackingField")return offsetof(fixture::Node,functions);
 if(n=="<battleActData>k__BackingField")return offsetof(fixture::Node,act);
 if(n=="TargetUnitDatas"||n=="<TargetUnitDatas>k__BackingField")return offsetof(fixture::Node,targets);
 if(n=="<AttackUnitData>k__BackingField")return offsetof(fixture::Node,actor);
 if(n=="<IAfterCalcResultDic>k__BackingField")return offsetof(fixture::Node,after_results);
 if(n=="<Value1>k__BackingField")return offsetof(fixture::Node,value);
 if(n=="absorptionType")return std::string(cls.name)=="BattleFunction13"?offsetof(fixture::Node,type):offsetof(fixture::Node,sibling_type);
 if(n=="<BattleTempParameter>k__BackingField")return offsetof(fixture::Node,temporary);
 if(n=="parameterGroupDic")return offsetof(fixture::Node,modifiers);
 for(size_t i=1;i<ffpr::battle::stat_feedback::fields.size();++i)if(auto* field=ffpr::battle::stat_feedback::fields[i];field&&n==field->name)
  return static_cast<int32_t>(offsetof(fixture::Node,temporary_values)+i*sizeof(int32_t));
 for(size_t i=1;i<ffpr::battle::stat_feedback::base_fields.size();++i)if(auto* field=ffpr::battle::stat_feedback::base_fields[i];field&&n==field->name)
  return static_cast<int32_t>(offsetof(fixture::Node,base_values)+i*sizeof(int32_t));
 return -1;
}
il2cpp::Method* Method::get(){return nullptr;}void* Method::ptr(){return nullptr;}
std::wstring str(void*p){return p?static_cast<fixture::Node*>(p)->name:L"";}
void call_failed_note(const char*){std::abort();}
}
namespace game{Id current(){return fixture::game_id;}}
namespace hooks{bool attach_raw(ref::Method&,void*,void**){return false;}bool attach_addr(void*,const char*,void*,void**){return false;}void note_fault(const char*){std::abort();}}
namespace il2cpp{void* method_pointer(Method*){return nullptr;}Method* virtual_method(void*,Method*){return nullptr;}}
namespace unity{
int list_count(void*p){return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}
void* list_at(void*p,int i){return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}
int dict_count(void*p){return p?static_cast<int>(static_cast<fixture::Dict*>(p)->size()):0;}
bool dict_entry(void*p,int i,uint64_t&k,uint64_t&v,int,int){if(!p||i<0||i>=dict_count(p))return false;const auto&e=(*static_cast<fixture::Dict*>(p))[i];k=reinterpret_cast<uint64_t>(e.first);v=reinterpret_cast<uint64_t>(e.second);return true;}
bool alive(void*p){return p!=nullptr;}
int array_length(void*p){return list_count(p);}
void* array_at(void*p,int i){return list_at(p,i);}
}
namespace ffpr{void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}}
namespace modtext{std::wstring T(std::wstring_view s){if(fixture::game_id==game::Id::FF2)if(const auto* key=fixture::status_catalog.get(std::wstring(s));key)if(const auto* value=key->get(L"en");value&&value->is_string())return value->s;return std::wstring(s);}std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring>a){return text::format(s,a);}const std::wstring& lang(){static const std::wstring value=L"en";return value;}LocaleScope::LocaleScope(std::wstring){}LocaleScope::~LocaleScope(){}}
namespace background{bool post(std::function<void()>f,Lane){if(fixture::background_busy)return false;fixture::work.push_back(std::move(f));return true;}}

namespace speech {void request_load(){}}
namespace sound {void initialize(){}}
namespace log_ {void write(const char*,...) {}}

namespace events{void on(Kind kind,Handler handler){fixture::event_handlers[kind].push_back(std::move(handler));}}
namespace dedup{bool text(const char*,std::wstring_view,Scope){return true;}void reset(const char*){}}

namespace ffpr::battle::rendered {void damage(void*,bool,std::function<void(int)>f){if(fixture::defer_digits){const int digits=fixture::drawn;fixture::digit_callbacks.push_back([f=std::move(f),digits]{f(digits);});}else f(fixture::drawn);}void hit_count(void*){}}
namespace ffpr::battle {
ref::Class kBattleEnemyData{"Last.Battle","BattleEnemyData"};
std::wstring unit_name(void*p){return p?static_cast<fixture::Node*>(p)->name:L"Unknown";}
std::wstring action_name(void*){return {};}
void action_actor_started(void*){}void action_actor_finished(){}
void* unit_parameter(void*p){return p;}void* unit_info(void*p){return p;}void* info_parameter(void*p){return p;}void* info_functions(void*p){return p?static_cast<fixture::Node*>(p)->functions:nullptr;}
void* function_condition(void*p){return p;}void* function_unit(void*p){return p;}
int param_current_hp(void*p){return p?static_cast<fixture::Node*>(p)->hp:0;}bool is_out_battle(void*){return false;}bool is_in_battle(){return true;}
int condition_id(void*p){return p?static_cast<fixture::Node*>(p)->id:0;}int condition_type(void*p){return p?static_cast<fixture::Node*>(p)->type:0;}
std::vector<void*> confirmed_conditions(void*p){return p?static_cast<fixture::Node*>(p)->conditions:std::vector<void*>{};}
void* current_condition_list(void*p){return p?&static_cast<fixture::Node*>(p)->conditions:nullptr;}
std::vector<void*> current_conditions(void*p){return confirmed_conditions(p);}std::vector<void*> list_items(void*p){return p?*static_cast<fixture::List*>(p):fixture::List{};}
std::wstring resolve_condition_name(void*p,int id,int*type){const int value=p?condition_type(p):(id==900?4:0);if(type)*type=value;return condition_labels::fallback(value);}
std::wstring strip_icon(std::wstring_view s){return std::wstring(s);}bool blank(const std::wstring&s){return s.empty();}
il2cpp::Method* find_method_param_type(il2cpp::Class*,const char*,int,int,const char*){return nullptr;}
}
namespace fixture {
int removal_id=-1;bool remove_current=true,remove_functions=true;
void remove_native(void*,void* unit,int32_t id,const void*) {
 auto* node=static_cast<Node*>(unit);auto erase=[&](List& list){list.erase(std::remove_if(list.begin(),list.end(),[&](void* p){return removal_id<0||static_cast<Node*>(p)->id==id;}),list.end());};
 if(remove_current)erase(node->conditions);
 if(remove_functions&&node->functions)erase(*static_cast<List*>(node->functions));
}
void remove_bulk(void* self,void* unit,void*,void*,const void* mi){remove_native(self,unit,removal_id,mi);}
}
namespace settings {bool get_bool(const char* key,bool fallback){return std::strcmp(key,"EnemyCriticalStatus")==0?fixture::enemy_critical:fallback;}}

namespace fixture {
Node message;bool done=false,throw_native=false,nested_once=false;int originals=0;
void native_set_message(void*,void*,const void*) { ++native_calls; }
bool native_scan(void* self,const void* mi) {
 ++originals;
 if(throw_native)throw 7;
 if(nested_once){nested_once=false;ffpr::battle::scan_feedback::detour(self,mi);}
 ffpr::battle::d_SetMessage(nullptr,&message,mi);
 if(self)static_cast<Scan*>(self)->first=false;
 return done;
}
void fire_fixture(events::Kind kind){for(auto& handler:event_handlers[kind])handler();}
void tick(){frame::run_tick();}
}
int main() {
 using namespace fixture;namespace scan=ffpr::battle::scan_feedback;namespace b=ffpr::battle;
 int failures=0;auto expect=[&](bool ok,const char* msg){if(!ok){++failures;std::printf("FAIL %s\n",msg);}};
 game_id=game::Id::FF3;scan::install();scan::original=&native_scan;b::o_SetMessage=&native_set_message;
 message.name=L"HP 123";
 for(int i=0;i<22;++i){b::battle_system_message(&message);tick();}
 expect(spoken.empty(),"unscoped repeated producer postpones original two-frame line");
 tick();tick();expect(spoken.size()==1,"unscoped baseline flushes only after refreshes stop");spoken.clear();
 Scan state;
 for(int i=0;i<22;++i){expect(!scan::detour(&state,nullptr),"false native return preserved");tick();}
 expect(spoken.size()==1&&spoken[0]==L"HP 123","repeated HP producer emits once within two frames");
 state.index=1;message.name=L"Weakness";for(int i=0;i<22;++i){scan::detour(&state,nullptr);tick();}
 expect(spoken.size()==2&&spoken[1]==L"Weakness","index change emits later displayed weakness once");
 state.index=2;for(int i=0;i<3;++i){scan::detour(&state,nullptr);tick();}
 expect(spoken.size()==3&&spoken[2]==L"Weakness","identical text on a new index remains a new event");
 state.index=0;state.first=true;message.name=L"HP 123";for(int i=0;i<3;++i){scan::detour(&state,nullptr);tick();}
 expect(spoken.size()==4&&spoken[3]==L"HP 123","same address new cast resets identity");
 state.index=1;message.name=L"Weakness";done=true;expect(scan::detour(&state,nullptr),"true native return preserved");tick();tick();
 done=false;scan::detour(&state,nullptr);tick();tick();expect(spoken.size()==6,"completion retires native identity");
 fire_fixture(events::Kind::BattleEntering);state.index=0;state.first=true;message.name=L"Weakness";
 scan::detour(&state,nullptr);tick();tick();expect(spoken.size()==7&&spoken.back()==L"Weakness","weak-only native line does not invent HP");
 b::battle_system_message(&message);tick();tick();expect(spoken.size()==8,"ordinary message outside source remains unchanged");
 state.first=true;state.index=-1;scan::detour(&state,nullptr);tick();tick();expect(spoken.size()==9,"invalid index falls back to ordinary reader");
 game_id=game::Id::FF2;state.index=0;state.first=true;scan::detour(&state,nullptr);tick();tick();scan::detour(&state,nullptr);tick();tick();
 expect(spoken.size()==11,"other game never applies Scan guard");game_id=game::Id::FF3;
 throw_native=true;try{scan::detour(&state,nullptr);}catch(int){}throw_native=false;
 b::battle_system_message(&message);tick();tick();expect(spoken.size()==12,"exception restores source scope");
 spoken.clear();fire_fixture(events::Kind::BattleEntering);state.first=true;state.index=0;message.name=L"   ";
 scan::detour(&state,nullptr);tick();message.name=L"HP";scan::detour(&state,nullptr);tick();tick();
 expect(spoken.size()==1&&spoken[0]==L"HP","blank native message does not consume index event");
 fire_fixture(events::Kind::BattleEnd);scan::detour(&state,nullptr);tick();tick();
 expect(spoken.size()==2,"battle end retires a still-active index");
 fire_fixture(events::Kind::SceneLoaded);scan::detour(&state,nullptr);tick();tick();
 expect(spoken.size()==3,"scene load retires a still-active index");
 scan_metadata=false;scan::detour(&state,nullptr);tick();tick();scan::detour(&state,nullptr);tick();tick();
 expect(spoken.size()==5,"missing metadata leaves ordinary message handling intact");scan_metadata=true;
 scan::detour(nullptr,nullptr);tick();tick();expect(spoken.size()==6,"null native owner falls back safely");
 fire_fixture(events::Kind::BattleEntering);spoken.clear();state.first=true;nested_once=true;
 scan::detour(&state,nullptr);tick();tick();expect(spoken.size()==1,"nested native scope restores parent and retains one index event");
 fire_fixture(events::Kind::BattleEnd);fire_fixture(events::Kind::SceneLoaded);
 state.first=true;state.index=0;message.name=L"HP";spoken.clear();const int before=originals;
 const auto start=std::chrono::steady_clock::now();
 for(int i=0;i<10000;++i)scan::detour(&state,nullptr);
 const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();const int queued=frame::pending();tick();tick();
 expect(originals-before==10000&&spoken.size()==1,"10000 refreshes preserve one native call and one event");
 std::printf("Scan 10000 actual detour+message callbacks %.3fms aggregate; pending=%d; native fixture only, no game-frame claim\n",ms,queued);
 std::printf("Scan producer and actual scheduler: %d failures; native calls=%d\n",failures,originals);return failures?1:0;
}
