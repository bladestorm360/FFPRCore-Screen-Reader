// check-sources: text.cpp json.cpp
// Actual damage, condition-add and special-effect producers; no game is run.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <chrono>
#include <fstream>
#include "../src/core/json.h"
#include "../src/core/battle/battle_message.cpp"
#include "../src/core/battle/special_feedback.cpp"
#include "../src/core/battle/stat_feedback.cpp"
#include "../src/core/condition_labels.h"
namespace fixture {
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

namespace frame{uint64_t count(){return 1;}double now(){return 1;}void cancel_label(const char*){}void after_frames(int,Task,const char*){}void post(Task t){fixture::native_posts.push_back(std::move(t));}int every_frame(Step s,const char*,int){fixture::admission_retries.push_back(std::move(s));return int(fixture::admission_retries.size());}}
namespace events{void on(Kind,Handler){}}
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
int main(){using namespace fixture;namespace b=ffpr::battle;namespace s=b::special_feedback;int failures=0;
 auto expect=[&](bool ok,const char*name){if(!ok){++failures;std::printf("FAIL %s\n",name);}};
 {std::ifstream input("data/ff2/mod_text.json");std::string bytes{std::istreambuf_iterator<char>(input),{}};expect(json::parse(bytes,status_catalog),"real FFII condition translation overrides load");}
 // Replay native direct + bulk reconciliation with status inherited from an
 // earlier battle: no condition_added call or remembered Add history is seeded.
 b::o_RemoveFunction=&remove_native;b::o_RemoveConditionFunction=&remove_bulk;
 for(int n=1;n<=6;++n)for(bool bulk:{false,true}) {
  game_id=static_cast<game::Id>(n);b::removal_reset_state();spoken.clear();
  Node target,status;target.name=L"Ally";status.id=13;status.type=n==2?204:10;
  List live{&status};target.functions=&live;target.conditions={&status};removal_id=13;
  auto cure=[&]{if(bulk)b::d_RemoveConditionFunction(nullptr,&target,nullptr,nullptr,nullptr);else b::d_RemoveFunction(nullptr,&target,13,nullptr);};
  remove_current=remove_functions=true;cure();
  expect(spoken==std::vector<std::wstring>{L"Ally: Poison removed"},"all-six direct/bulk native reconciliation announces carried persistent Poison without Add history");
  spoken.clear();cure();expect(spoken.empty(),"repeated empty reconciliation does not fabricate a removal");
  b::removal_reset_state();target.conditions={&status};live={&status};remove_current=false;cure();
  expect(spoken.empty(),"condition still applied after a function change remains silent");
  b::removal_reset_state();target.conditions={&status};live={&status};remove_current=true;remove_functions=false;cure();
  expect(spoken.empty(),"a remaining stacked native function prevents premature removal speech");
  b::removal_reset_state();target.conditions={&status};live={&status};remove_functions=true;b::g_battle_end_cleanup=true;cure();
  expect(spoken.empty(),"battle-end native cleanup preserves the established silent policy");
  b::removal_reset_state();live.assign(129,&status);target.conditions={&status};cure();
  expect(spoken.empty(),"over-bound condition storage is rejected before observation traversal");
 }
 remove_current=remove_functions=true;removal_id=-1;b::removal_reset_state();spoken.clear();
 for(int n=1;n<=6;++n)for(bool enabled:{false,true}) {
  game_id=static_cast<game::Id>(n);enemy_critical=enabled;b::removal_reset_state();
  Node enemy,critical;enemy.name=L"Giant Rat";enemy.kind=L"BattleEnemyData";critical.id=900;critical.type=4;
  List live{&critical};enemy.conditions={&critical};enemy.functions=&live;
  spoken.clear();b::condition_added(&enemy,900);
  expect(spoken.size()==(enabled?1:0),"all-six hidden enemy Critical follows its independent opt-in setting");
  spoken.clear();enemy.conditions.clear();b::condition_added(&enemy,900);
  expect(spoken.size()==(enabled?1:0),"unconfirmed Add still uses resolved master type rather than leaking Critical through a fallback name");
  enemy.conditions={&critical};b::removal_reset_state();spoken.clear();b::d_RemoveFunction(nullptr,&enemy,900,nullptr);
  expect(spoken.size()==(enabled?1:0),"enemy Critical removal follows the same setting");
  enemy.kind=L"BattlePlayerData";enemy.name=L"Corrad";enemy.conditions={&critical};spoken.clear();b::condition_added(&enemy,900);
  expect(spoken==std::vector<std::wstring>{L"Corrad: Critical"},"party critical health remains audible regardless of enemy disclosure");
 }
 enemy_critical=false;b::removal_reset_state();spoken.clear();
 b::o_CreateDamageView=&basic;b::o_CreateDamageViewFF2=&basic2;b::o_CreateDamageViewFF5=&basic5;
 b::o_utility_damage=&utility;b::o_utility_damage2=&utility2;b::o_render_damage=&render;b::o_render_damage6=&render6;
 s::o_view=s::o_after=s::o_attack=s::o_after_attack=&result_view;
 s::o_after_batch=&after_batch;
 // Nominal healing intent must not override the innermost displayed colour.
 // The numeric argument is deliberately different from the rendered digits.
 Node skeleton;skeleton.name=L"Skeleton";
 for(int n=1;n<=6;++n) {
  game_id=static_cast<game::Id>(n);drawn=94;
  auto create=[&](int hit,bool recovery,bool miss=false){
   spoken.clear();
   if(n==2)b::d_CreateDamageViewFF2(nullptr,&skeleton,9999,miss?2:hit,recovery,true,nullptr);
   else if(n==5)b::d_CreateDamageViewFF5(nullptr,&skeleton,9999,miss?2:hit,recovery,0,nullptr);
   else b::d_CreateDamageView(nullptr,&skeleton,9999,miss?2:hit,recovery,nullptr);
  };
  create(4,false);expect(spoken==std::vector<std::wstring>{L"Skeleton: 94 damage"},"Cure/Recovery intent rendered as damage is announced as damage in every game");
  create(4,true);expect(spoken==std::vector<std::wstring>{L"Skeleton: Recovered 94 HP"},"actual rendered HP recovery keeps its healing wording");
  create(0,true);expect(spoken==std::vector<std::wstring>{L"Skeleton: Recovered 94 HP"},"rendered recovery also wins over plain Hit intent");
  create(4,false,true);expect(spoken==std::vector<std::wstring>{L"Skeleton: Miss"},"miss presentation remains separate from healing intent");
  create(5,false);expect(spoken==std::vector<std::wstring>{L"Skeleton: 94 MP damage"},"explicit MP damage remains MP");
  create(6,true);expect(spoken==std::vector<std::wstring>{L"Skeleton: Recovered 94 MP"},"explicit MP recovery remains MP");
 }
 drawn=1;spoken.clear();
 Node guy;guy.name=L"Guy";Node drain;drain.kind=L"AbsorptionFunction";drain.value=2;List functions{&drain};Dict dict{{&guy,&functions}};Node owner;owner.functions=&dict;
 for(auto id:{game::Id::FF1,game::Id::FF2,game::Id::FF3,game::Id::FF4,game::Id::FF5,game::Id::FF6}){
  game_id=id;expected_type=0;spoken.clear();s::d_view(&owner,&guy,false,nullptr);
  expect(spoken==std::vector<std::wstring>{L"Guy: 1 MP damage"},"actual primary absorption renderer labels MP without speaking internal damage argument");
  expected_type=4;spoken.clear();s::d_attack(&owner,&guy,true,nullptr);expect(spoken==std::vector<std::wstring>{L"Guy: Recovered 1 MP"},"attacker MP recovery stays MP");
  expected_type=2;spoken.clear();s::d_view(&owner,&guy,false,nullptr);expect(spoken==std::vector<std::wstring>{L"Guy: Miss"},"MP scope preserves native misses");
  drain.value=1;expected_type=0;spoken.clear();s::d_view(&owner,&guy,false,nullptr);expect(spoken==std::vector<std::wstring>{L"Guy: 1 damage"},"HP drain remains HP");drain.value=2;
  expect(s::damage_type(0)==0,"scope cannot leak into later attacks");
 }
 Node physical;physical.kind=L"PhysicalFunction";functions.push_back(&physical);expected_type=0;spoken.clear();s::d_view(&owner,&guy,false,nullptr);expect(spoken[0]==L"Guy: 1 damage","a physical primary hit cannot inherit its secondary MP type");
 spoken.clear();s::d_after(&owner,&guy,false,nullptr);expect(spoken[0]==L"Guy: 1 MP damage","secondary MP drain retains its resource through a physical action");functions.pop_back();
 owner.kind=L"BattleFunction13";owner.type=1;owner.functions=nullptr;expected_type=0;spoken.clear();s::d_view(&owner,&guy,false,nullptr);
 expect(spoken==std::vector<std::wstring>{L"Guy: 1 MP damage"},"base percentage absorption reads its outer MP resource with plain Hit and no FunctionBase list");
 owner.type=0;spoken.clear();s::d_view(&owner,&guy,false,nullptr);expect(spoken[0]==L"Guy: 1 damage","percentage HP absorption remains HP");
 for(int n=1;n<=6;++n)for(const auto* variant:{L"BattleFunction13",L"BattleFunction13Serial",L"BattleFunction13FF2",L"BattleFunction13FF3"}) {
  if(n!=2&&std::wstring(variant)==L"BattleFunction13Serial")continue;
  game_id=static_cast<game::Id>(n);owner.kind=variant;
  const bool sibling=owner.kind==L"BattleFunction13FF2"||owner.kind==L"BattleFunction13FF3";
  owner.type=sibling?0:1;owner.sibling_type=sibling?1:0;spoken.clear();expected_type=0;
  s::d_view(&owner,&guy,false,nullptr);expect(spoken==std::vector<std::wstring>{L"Guy: 1 MP damage"},"each actual absorption class reads its own resource field, including FFII sibling");
  expected_type=4;s::d_attack(&owner,&guy,true,nullptr);expect(spoken.back()==L"Guy: Recovered 1 MP","sibling attacker MP recovery is preserved");
  expected_type=2;s::d_view(&owner,&guy,false,nullptr);expect(spoken.back()==L"Guy: Miss","sibling MP scope preserves Miss");
  owner.type=sibling?1:0;owner.sibling_type=sibling?0:1;expected_type=0;s::d_view(&owner,&guy,false,nullptr);expect(spoken.back()==L"Guy: 1 damage","each sibling HP mode ignores the unrelated base-layout value");
  expect(s::damage_type(0)==0,"sibling resource scope restores on return");
 }
 // Replay the two separate visible producers in the user's Parasite sequence.
 owner.kind.clear();owner.functions=&dict;functions.push_back(&physical);spoken.clear();expected_type=0;s::d_view(&owner,&guy,false,nullptr);
 owner.kind=L"BattleFunction13FF2";owner.functions=nullptr;owner.sibling_type=1;s::d_view(&owner,&guy,false,nullptr);
 Node parasite;parasite.name=L"Parasite";expected_type=6;s::d_attack(&owner,&parasite,true,nullptr);
 expect(spoken==std::vector<std::wstring>({L"Guy: 1 damage",L"Guy: 1 MP damage",L"Parasite: Recovered 1 MP"}),"Parasite physical hit, MP drain and MP recovery keep independent labels");
 functions.pop_back();owner.kind.clear();owner.functions=&dict;
 // The previously missing path: native inlined loops bypass all d_view/d_after
 // wrappers. Replay both reported attacks through the actual typed damage,
 // utility, renderer and rendered-digit callback, with no manually seeded MP.
 for(int n=1;n<=6;++n)for(int physical_digits:{12,13}) {
  game_id=static_cast<game::Id>(n);expect(!s::g_secondary_mp,"direct native replay starts with no enclosing MP scope");
  Node main_function,mp_function,caster_unit;main_function.kind=L"BattleFunction4";mp_function.kind=L"BattleFunction13FF2";mp_function.type=0;mp_function.sibling_type=1;caster_unit.name=L"Parasite";
  const bool split=n==1||n==2;Dict secondary{{&guy,nullptr}};if(split)mp_function.after_results=&secondary;
  auto parts=[&]{drawn=physical_digits;expected_type=0;result_view(split?&mp_function:&main_function,&guy,false,nullptr);drawn=1;if(split){victim=&guy;caster=&caster_unit;s::d_after_batch(&mp_function,nullptr);}else{result_view(&mp_function,&guy,false,nullptr);expected_type=6;result_view(&mp_function,&caster_unit,true,nullptr);}};
  spoken.clear();parts();
  expect(spoken==std::vector<std::wstring>({L"Guy: "+std::to_wstring(physical_digits)+L" damage",L"Guy: 1 MP damage",L"Parasite: Recovered 1 MP"}),"direct three-part native replay preserves physical/MP drain/recovery across every game's actual damage ABI");
  expect(!s::g_secondary_mp&&!s::g_after_batch_owner&&b::g_damage_context.unit==nullptr,"each direct part restores its resource/target/phase context");
  if(split){spoken.clear();drawn=physical_digits;expected_type=0;s::d_view(&mp_function,&guy,false,nullptr);expect(spoken==std::vector<std::wstring>{L"Guy: "+std::to_wstring(physical_digits)+L" damage"},"non-inlined primary wrapper also preserves physical damage held by the MP handler");mp_function.after_results=nullptr;}
  expected_type=0;drawn=0;result_view(&mp_function,&guy,false,nullptr);expect(spoken.back()==L"Guy: 0 MP damage","direct rendered zero MP drain remains typed");
  expected_type=2;result_view(&mp_function,&guy,false,nullptr);expect(spoken.back()==L"Guy: Miss","direct MP miss remains Miss");
  expected_type=4;drawn=1;mp_function.sibling_type=0;result_view(&mp_function,&guy,true,nullptr);expect(spoken.back()==L"Guy: Recovered 1 HP","direct HP absorption recovery keeps HP");
  expected_type=0;result_view(&main_function,&guy,false,nullptr);expect(spoken.back()==L"Guy: 1 damage","later ordinary attack cannot inherit a previous MP part");
  mp_function.sibling_type=1;if(split)mp_function.after_results=&secondary;defer_digits=true;spoken.clear();parts();
  expect(spoken.empty()&&digit_callbacks.size()==3,"all three digit callbacks may wait until after the native stack returns");
  mp_function.sibling_type=0;guy.name=L"Later actor";caster_unit.name=L"Later caster";
  auto ready=std::move(digit_callbacks);digit_callbacks.clear();for(auto&f:ready)f();defer_digits=false;guy.name=L"Guy";
  expect(spoken==std::vector<std::wstring>({L"Guy: "+std::to_wstring(physical_digits)+L" damage",L"Guy: 1 MP damage",L"Parasite: Recovered 1 MP"}),"deferred digit callbacks retain each copied name/resource after native owner/context changes");
 }
 game_id=game::Id::FF2;expected_type=0;drawn=1;
 expected_type=0;spoken.clear();s::d_view(&owner,&guy,true,nullptr);expect(spoken[0]==L"Guy: Recovered 1 MP","native rendered recovery polarity survives MP resource correction");
 game_id=game::Id::FF2;Node maria;maria.name=L"Maria";Node amnesia;amnesia.id=14;amnesia.type=205;maria.conditions={&amnesia};spoken.clear();b::condition_added(&maria,14);expect(spoken==std::vector<std::wstring>{L"Maria: Amnesia"},"Fog's actual confirmed Amnesia reaches the condition-add producer");
 Node temporary,persistent;temporary.id=10;temporary.type=10;persistent.id=13;persistent.type=204;maria.conditions={&temporary,&persistent};spoken.clear();b::condition_added(&maria,10);b::condition_added(&maria,13);
 expect(spoken==std::vector<std::wstring>({L"Maria: Venom",L"Maria: Poison"}),"actual FFII infliction producer keeps temporary Venom separate from persistent Poison using the real catalog");
 // Imbibe is an applied unnamed condition, not a Swap/unique operation. Exercise
 // the real existing Add postfix with the condition already confirmed by native
 // processing; names, removal tracking and repeated legitimate applications stay
 // on that producer. No spell-name or parameter-total inference is involved.
 Node gottos;gottos.name=L"Gottos";Node imbibe;imbibe.id=50;imbibe.type=203;gottos.conditions={&imbibe};
 b::o_Add=[](void*,void*,int32_t,const void*){++native_calls;};
 spoken.clear();const int before_imbibe=native_calls;b::d_Add(nullptr,&gottos,50,nullptr);
 expect(native_calls==before_imbibe+1&&spoken==std::vector<std::wstring>{L"Gottos: Attack up, Evasion down"},"Imbibe actual Add postfix names both confirmed stat effects");
 b::d_Add(nullptr,&gottos,50,nullptr);expect(spoken.size()==2,"a repeated native Imbibe application remains audible without text deduplication");
 for(auto game:{game::Id::FF1,game::Id::FF3,game::Id::FF4,game::Id::FF5,game::Id::FF6}){game_id=game;expect(condition_labels::fallback(203).empty(),"FFII's unused condition enum cannot name an effect in another game");}
 game_id=game::Id::FF2;
 expect(condition_labels::fallback(625).empty(),"internal no-select flags stay silent");
 for(int type:{6,7,8,9,10,11,12,32,34,204,205,401,402,403,404,405,406,601,603})expect(!condition_labels::fallback(type).empty(),"named harmful condition types have fallback labels");
 Node brain;brain.name=L"Brain";Node act;act.actor=&brain;Dict swap_targets{{&maria,nullptr}};owner.act=&act;owner.functions=&swap_targets;owner.kind=L"BattleFunction23";Node keynode;keynode.name=L"WM_CHANGE";Node providernode;
 caster=&brain;victim=&maria;provider=&providernode;key=&keynode;s::o_update=&update;s::o_exchange=&exchange;s::o_swap=&view;
 auto flush=[] {auto jobs=std::move(work);work.clear();for(auto&f:jobs)f();auto posts=std::move(native_posts);native_posts.clear();for(auto&f:posts)f();};
 spoken.clear();apply=false;s::d_update(&owner,nullptr);s::d_swap(&owner,&maria,false,nullptr);flush();expect(spoken.empty(),"Swap with no native exchange never claims success");
 apply=true;s::d_update(&owner,nullptr);s::d_swap(&owner,&maria,false,nullptr);flush();expect(spoken==std::vector<std::wstring>{L"Maria: HP and MP exchanged with Brain"},"actual native Swap exchange speaks once at its target view");
 s::d_swap(&owner,&maria,false,nullptr);flush();expect(spoken.size()==1,"one actual exchange is consumed once, without text deduplication");
 keynode.name=L"WM_DISPEL";s::d_update(&owner,nullptr);s::d_swap(&owner,&maria,false,nullptr);flush();expect(spoken.size()==1,"other native unique effects cannot announce Swap");
 keynode.name=L"WM_CHANGE";s::d_update(&owner,nullptr);s::d_swap(&owner,&maria,false,nullptr);++s::g_epoch;flush();expect(spoken.size()==1,"battle-end cancellation rejects a queued obsolete Swap announcement");
 // Actual all-six status refresh + typed direct-write hooks. Only changed native
 // modifier entries/fields cause stat feedback; damage amounts and spell names
 // do not participate. FFII exercises its existing combined removal hook.
 namespace stats=ffpr::battle::stat_feedback;
 stats::original_status=b::o_UpdateTempParamter=&status_modify;
 stats::original_direct=&direct_modify;b::o_Add=&stat_add;b::o_AddFF5=&stat_add5;
 for(auto game:{game::Id::FF1,game::Id::FF2,game::Id::FF3,game::Id::FF4,game::Id::FF5,game::Id::FF6}) {
  game_id=game;stats::reset();spoken.clear();work.clear();Node ally,temp;ally.name=L"Ally";ally.temporary=&temp;
  Dict empty_modifiers,defense_up{{reinterpret_cast<void*>(11),reinterpret_cast<void*>(8)}};
  temp.modifiers=&empty_modifiers;next_modifiers=&empty_modifiers;
  stats::status(nullptr,&ally,nullptr);flush();expect(spoken.empty(),"initial status setup registers names without revealing passive statistics");
  stats::active=true;Node protect;protect.id=22;protect.type=19;ally.conditions={&protect};next_modifiers=&defense_up;
  if(game==game::Id::FF5){b::d_AddFF5(nullptr,&ally,22,true,nullptr);expect(ff5_callback,"FFV's callback Boolean reaches the original unchanged");}
  else b::d_Add(nullptr,&ally,22,nullptr);
  flush();expect(spoken==std::vector<std::wstring>{L"Ally: Protect, Defense up"},"applied condition and actual stat direction share one ordered result in every adapter");
  spoken.clear();stats::status(nullptr,&ally,nullptr);flush();expect(spoken.empty(),"unchanged native stat refresh creates no modification event");
  spoken.clear();next_modifiers=&empty_modifiers;stats::status(nullptr,&ally,nullptr);flush();expect(spoken==std::vector<std::wstring>{L"Ally: Defense down"},"actual modifier removal reports its resulting stat direction");
  spoken.clear();stats::direct(&ally,10,7,nullptr);flush();expect(spoken==std::vector<std::wstring>{L"Ally: Attack up"},"direct typed stat mutation announces the changed native field and bound unit");
  spoken.clear();direct_applies=false;stats::direct(&ally,10,7,nullptr);flush();expect(spoken.empty(),"a native no-op cannot announce a direct statistic change");direct_applies=true;
  stats::direct(&ally,2,-7,nullptr);stats::direct(&ally,3,7,nullptr);flush();expect(spoken.empty(),"current HP and MP remain exclusively owned by rendered damage/recovery speech");
  Dict strength_one{{reinterpret_cast<void*>(4),reinterpret_cast<void*>(1)}},strength_two{{reinterpret_cast<void*>(4),reinterpret_cast<void*>(2)}};
  temp.modifiers=&strength_one;next_modifiers=&strength_two;stats::status(nullptr,&ally,nullptr);flush();expect(spoken==std::vector<std::wstring>{L"Ally: Strength up"},"a later-turn modifier increment speaks without needing another spell-name event");
  spoken.clear();next_modifiers=&defense_up;stats::status(nullptr,&ally,nullptr);stats::cleanup();flush();expect(spoken.empty(),"battle cleanup cancels queued obsolete statistic output");
  stats::active=true;temp.modifiers=&empty_modifiers;next_modifiers=&defense_up;
  auto before=stats::capture(&ally);Node replacement;replacement.modifiers=&defense_up;ally.temporary=&replacement;stats::updated(&ally,before);flush();expect(spoken.empty(),"a replaced native temporary-parameter owner invalidates the old snapshot");ally.temporary=&temp;
  Dict excessive(65,{reinterpret_cast<void*>(10),reinterpret_cast<void*>(1)});temp.modifiers=&excessive;expect(!stats::capture(&ally).valid,"oversized native dictionaries are rejected before entry reads");
  if(game==game::Id::FF2) {
   // Isolate this application from the preceding synthetic Protect scenario;
   // that fixture deliberately replaced its dictionary without a native cure.
   temp.modifiers=&empty_modifiers;Dict drink{{reinterpret_cast<void*>(10),reinterpret_cast<void*>(14)},{reinterpret_cast<void*>(17),reinterpret_cast<void*>(static_cast<intptr_t>(-20))}};
   next_modifiers=&drink;ally.conditions={&imbibe};b::d_Add(nullptr,&ally,50,nullptr);flush();expect(spoken==std::vector<std::wstring>{L"Ally: Attack up, Evasion down"},"Imbibe's descriptive fallback and structured native modifier changes produce one result");
  }
  stats::reset();next_modifiers=nullptr;spoken.clear();work.clear();
  stats::active=true;Node unrefreshed;unrefreshed.name=L"Target";List action_targets{&unrefreshed};
  stats::observe_action(&guy,L"Guy",&action_targets);
  stats::direct(&unrefreshed,10,3,nullptr);flush();
  expect(spoken==std::vector<std::wstring>{L"Target: Attack up"},"a direct-only target binds from the actual action without any previous condition");
  spoken.clear();stats::finish_action(&guy,&action_targets);flush();expect(spoken.empty(),"action completion never repeats a previously observed direct write");
  stats::direct(&unrefreshed,15,10,nullptr);flush();
  expect(spoken.empty(),"critical-health internal Luck writes stay silent in every game");
  unrefreshed.temporary_values[15]=0;stats::finish_action(&guy,&action_targets);flush();
  expect(spoken.empty(),"critical-health Luck restoration cannot leak through action completion");
  {stats::AddScope applied(&unrefreshed);applied.changes[11]=-8;expect(!stats::announce_condition(&unrefreshed,L"KO",5),"KO retains its ordinary condition producer");}
  flush();expect(spoken.empty(),"KO Add never describes removed buff statistics as an active effect");
  unrefreshed.hp=0;unrefreshed.base_values[11]=-8;stats::finish_action(&guy,&action_targets);flush();
  expect(spoken.empty(),"dead-target refresh bookkeeping is silent and advances the comparison baseline");unrefreshed.hp=10;
  unrefreshed.base_values[1]=-20;stats::finish_action(&guy,&action_targets);flush();
  expect(spoken==std::vector<std::wstring>{L"Target: Level down"},"unique/inlined base-stat writes are observed at the exact action-completion boundary");
  spoken.clear();unrefreshed.base_values[2]=100;stats::finish_action(&guy,&action_targets);flush();
  expect(spoken==std::vector<std::wstring>{L"Target: Maximum HP up"},"maximum-resource statistics remain distinct from current HP damage");
  stats::reset();spoken.clear();work.clear();
 }
 // Whole capture -> native continuation -> formatting job -> first fake speech,
 // plus a paused formatter scenario. These timings exclude Unity/native costs,
 // real worker scheduling/Prism/audible delay; they are not game-frame claims.
 game_id=game::Id::FF2;Node profiled,profile_temp;profiled.name=L"Ally";profiled.temporary=&profile_temp;
 Dict profile_empty,profile_dense;
 for(int i=1;i<=28;++i)if(i!=2&&i!=3)profile_dense.push_back({reinterpret_cast<void*>(static_cast<intptr_t>(i)),reinterpret_cast<void*>(1)});
 for(bool dense:{false,true}) {
  std::vector<double> elapsed;stats::reset();stats::active=true;profile_temp.modifiers=&profile_empty;
  for(int i=0;i<1000;++i) {
   next_modifiers=dense?(i%2?&profile_empty:&profile_dense):&profile_empty;spoken.clear();
   const auto begin=std::chrono::steady_clock::now();stats::observe_action(&profiled,L"Ally",nullptr);stats::status(nullptr,&profiled,nullptr);flush();
   elapsed.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count());
  }
  std::sort(elapsed.begin(),elapsed.end());
  std::printf("Stat %s capture-to-first-fake-speech: median %.3f us, p99 %.3f us, max %.3f us (fake native/worker/output only)\n",dense?"26 modifiers":"unchanged",elapsed[500],elapsed[990],elapsed.back());
 }
 spoken.clear();work.clear();profile_temp.modifiers=&profile_empty;const int before_stalled=native_calls;
 for(int i=0;i<64;++i){next_modifiers=i%2?&profile_empty:&profile_dense;stats::status(nullptr,&profiled,nullptr);}
 expect(native_calls==before_stalled+64&&work.size()==64,"a paused stat formatter cannot hold native continuation");
 stats::cleanup();flush();expect(spoken.empty(),"retired queued stat work cannot speak in the next battle");
 stats::reset();stats::active=true;spoken.clear();profile_temp.modifiers=&profile_empty;next_modifiers=&profile_dense;
 background_busy=true;stats::status(nullptr,&profiled,nullptr);
 expect(work.empty()&&stats::pending==1&&!admission_retries.empty(),"a busy worker lane retains the owned stat event without blocking native continuation");
 background_busy=false;for(auto& retry:admission_retries)expect(!retry(),"stat event admission succeeds when the lane recovers");admission_retries.clear();flush();
 expect(spoken.size()==1&&stats::pending==0,"a stat event rejected by initial queue admission remains audible after retry");
 stats::reset();stats::active=true;spoken.clear();profile_temp.modifiers=&profile_empty;
 for(int i=0;i<65;++i){next_modifiers=i%2?&profile_empty:&profile_dense;stats::status(nullptr,&profiled,nullptr);}
 expect(work.size()==64&&stats::pending==64,"stalled stat work has a finite64-event retention bound");flush();
 expect(spoken.size()==64&&spoken.front().find(L"Statistic feedback is busy; some changes could not be read.")==0,"overflow reports an accessible recovery notice while retained results are preserved");
 stats::reset();next_modifiers=nullptr;spoken.clear();work.clear();
 // Isolated bounded observation cost, excluding real native/Unity and speech.
 owner.kind.clear();List busy_functions(16,&drain);Dict busy_targets(64,{nullptr,&busy_functions});busy_targets.back().first=&guy;owner.functions=&busy_targets;
 std::vector<double> samples;bool observed=true;
 for(int i=0;i<2000;++i){const auto start=std::chrono::steady_clock::now();observed&=s::absorption_mp(&owner,&guy,true);samples.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count());}
 std::sort(samples.begin(),samples.end());expect(observed,"maximum bounded MP observation preserves its last-target result");
 std::printf("MP observation 64 targets/16 functions: median %.3f us, p99 %.3f us, max %.3f us (fake native, no audible timing)\n",samples[1000],samples[1980],samples.back());
 game_id=game::Id::FF2;expected_type=0;drawn=1;
 for(bool mp:{false,true}){Node direct;direct.kind=mp?L"BattleFunction13FF2":L"BattleFunction4";direct.sibling_type=1;samples.clear();for(int i=0;i<2000;++i){spoken.clear();const auto start=std::chrono::steady_clock::now();result_view(&direct,&guy,false,nullptr);samples.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count());expect(spoken.size()==1,"direct producer submits one rendered result");}std::sort(samples.begin(),samples.end());std::printf("Direct %s capture-to-fake-speech: median %.3f us, p99 %.3f us, max %.3f us (fake native/output only)\n",mp?"MP":"physical",samples[1000],samples[1980],samples.back());}
 std::printf("%s: actual damage/status/Swap producers, all-six resource scopes; %d failures\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
