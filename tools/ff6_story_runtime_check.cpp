// check-sources: text.cpp
// Real guide native-owner controller and epochs, held worker/route completions.
#include "../src/games/ff6/story_guidance.cpp"
#include "../src/games/ff6/zone_eater_roof_observation.cpp"
#include "../src/games/ff6/owzer_doors.cpp"
#include "../src/games/ff6/sealed_gate_bridges.cpp"
#include "../src/games/ff6/defense_watch.cpp"
#include <cstdio>
#include <thread>
#include <chrono>
#include <limits>
namespace fixture {
DWORD owner=GetCurrentThreadId();int wrong_owner=0,party_events=0,retired=0,requests=0;
bool ready=true;int map=498;std::wstring scene=L"Field";
int verbosity=-1,floating_control=155;
bool nav_guidance=true,other_guidance=true;
bool held_plate=false;int predicted=0;
bool released_refresh=false;int first_target=-1;int finish_exit=0;
int finish_scenario=-1;
std::vector<int> exit_through;
struct RoofClock {float required=5,elapsed=0;bool finished=false;};
struct RoofGlobal {RoofClock* timer=nullptr;void* scripts=nullptr;};
struct RoofManager {RoofGlobal* global=nullptr;};
struct RoofName {char header[16]{};int32_t length=9;wchar_t chars[64]=L"sc_e_0850";};
RoofClock roof_clock;RoofGlobal roof_global;RoofManager roof_manager;
RoofManager* roof_instance=nullptr;RoofName roof_name;
std::vector<void*> roof_names{&roof_name};
bool timers_frozen=false;int timer_originals=0,string_reads=0;
bool focused=true,mod_menu=false;
bool choice=false,dialogue=false,battle=false;int state_reads=0,scalar_reads=0;
struct DoorLocals {char header[32]{};int values[8]{};};
struct DoorStorage {int map=445;void* values=nullptr;};
DoorLocals door_locals;DoorStorage door_storage;DoorStorage* door_instance=nullptr;
int door_phase=0,door_originals=0;bool door_wait=false;double door_test_time=-1;
void native();
void original_owzer(void*,const void*){++door_originals;door_locals.values[2]=1;door_locals.values[3]=door_phase;}
int bridge_mode=1,bridge_phase=0,bridge_originals=0;
bool bridge_immediate=false;
void original_bridge_a(void*,const void*){++bridge_originals;door_locals.values[1]=1;door_locals.values[2]=bridge_phase;}
void original_bridge_b(void*,const void*){++bridge_originals;door_locals.values[1]=2;door_locals.values[3]=bridge_phase;}
field::baked_route::Cell owner_cell{1,2,3};
struct Actor {int id=0;unity::Vec3 position;};
struct SavedEntity {unity::Vec3 position;};
struct SavedParty {int map=0;SavedEntity* player=nullptr;std::wstring serialized;};
struct SavedData {void* parties=nullptr;};
struct SaveManager {SavedData* data=nullptr;};
SaveManager* save_manager=nullptr;
void native();
int save_reads=0,pose_strings=0;size_t maximum_string=0;
void* save_instance(const void*){native();++save_reads;return save_manager;}
int updates=0;
void update(void*,const void*){++updates;}
field::map_state::Flags flags;
std::vector<std::function<void()>> jobs,posts;
frame::Step step;
std::string step_label;
std::map<events::Kind,std::vector<events::Handler>> handlers;
std::map<std::string,actions::Fn> actions;
std::map<std::string,std::function<bool()>> queries;
std::vector<std::wstring> speech;
std::function<void(field::PathInfo)> route;
std::map<std::string,int> offsets;
void native(){if(GetCurrentThreadId()!=owner)++wrong_owner;}
void flush(){auto q=std::move(jobs);jobs.clear();std::thread worker([&]{for(auto& fn:q)fn();});worker.join();auto p=std::move(posts);posts.clear();for(auto& fn:p)fn();}
void original_party(void*,int32_t slot,const void*){flags.global[4]=slot;}
void original_timer(void*,float seconds,void*,void* names,bool,const void*){
 native();++timer_originals;roof_clock={seconds,0,false};roof_global={&roof_clock,names};roof_manager.global=&roof_global;
}
}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
void* Method::ptr(){fixture::native();return std::string(cls.name)=="SaveMapManager"&&std::string(name)=="get_Instance"?reinterpret_cast<void*>(&fixture::save_instance):nullptr;}
il2cpp::Method* Method::get(){fixture::native();return nullptr;}
int32_t Field::get(){fixture::native();auto at=fixture::offsets.find(name);return at==fixture::offsets.end()?-1:at->second;}
bool Class::is(void* p){fixture::native();return p!=nullptr;}
il2cpp::Class* Class::get(){fixture::native();return reinterpret_cast<il2cpp::Class*>(this);}
void call_failed_note(const char*){}
std::wstring rd_str(void*,Field&){fixture::native();return {};}
std::wstring rd_str_bounded(void* obj,Field& field,size_t limit){fixture::native();++fixture::pose_strings;fixture::maximum_string=std::max(fixture::maximum_string,limit);if(!obj||std::string(field.name)!="playerEntity")return {};const auto& value=static_cast<fixture::SavedParty*>(obj)->serialized;return value.size()<=limit?value:std::wstring();}
}
namespace hooks {
void note_fault(const char*){std::abort();}
bool attach_raw(ref::Method& m,void*,void** original){
 if(std::string(m.name)=="<SetGimmickTimerEvent0443>b__734_0"){*original=reinterpret_cast<void*>(&fixture::original_bridge_a);return true;}
 if(std::string(m.name)=="<SetGimmickTimerEvent0444>b__737_0"){*original=reinterpret_cast<void*>(&fixture::original_bridge_b);return true;}
 *original=std::string(m.name)=="<SetGimmickTimerOwzer>b__742_0"?reinterpret_cast<void*>(&fixture::original_owzer):std::string(m.name)=="SetGrobalTimer"?reinterpret_cast<void*>(&fixture::original_timer):reinterpret_cast<void*>(&fixture::original_party);return true;}
}
namespace il2cpp {
void* read_static_ptr(Class* k,const char* name){fixture::native();if(std::string(reinterpret_cast<ref::Class*>(k)->name)=="DataStorage"&&std::string(name)=="instance")return fixture::door_instance;return std::string(reinterpret_cast<ref::Class*>(k)->name)=="TimerInstanceManager"&&std::string(name)=="instance"?fixture::roof_instance:nullptr;}
bool read_string(String* p,wchar_t* out,size_t count){fixture::native();++fixture::string_reads;if(!p||!out||count<1)return false;const auto* s=static_cast<fixture::RoofName*>(p);if(s->length<0||size_t(s->length)>=count)return false;std::copy_n(s->chars,s->length,out);out[s->length]=0;return true;}
}
namespace actions {
void define(const char*n,Fn f){fixture::actions[n]=std::move(f);}
bool run(const char*n){fixture::native();if(auto at=fixture::actions.find(n);at!=fixture::actions.end()){at->second();return true;}return false;}
}
namespace queries {
void define(const char* n,std::function<bool()> f){fixture::queries[n]=std::move(f);}
bool is(const char*n){fixture::native();if(std::string(n)=="GameStatePatches.IsFieldNavigationReady")return fixture::ready;if(std::string(n)=="TimerHelper.TimersFrozen")return fixture::timers_frozen;if(std::string(n)=="ModMenu.IsOpen")return fixture::mod_menu;if(std::string(n)=="MessageChoice.IsOpen")return fixture::choice;if(std::string(n)=="MessageWindowPatches.IsInDialogue")return fixture::dialogue;if(auto at=fixture::queries.find(n);at!=fixture::queries.end())return at->second();return false;}
}
namespace menu_state {bool any_active(){fixture::native();return false;} }
namespace events {
void on(Kind kind,Handler h){fixture::handlers[kind].push_back(std::move(h));}
void fire(Kind kind){fixture::native();if(kind==Kind::PartyChanging)++fixture::party_events;for(auto& h:fixture::handlers[kind])h();}
}
namespace frame {
bool on_main_thread(){return GetCurrentThreadId()==fixture::owner;}
double now(){if(fixture::door_test_time>=0)return fixture::door_test_time;return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}
int every_frame(Step s,const char* label,int){fixture::native();fixture::step_label=label;fixture::step=std::move(s);return 1;}
void cancel_label(const char* label){fixture::native();if(fixture::step_label==label)fixture::step={};}
void post(Task t){fixture::posts.push_back(std::move(t));}
void next_frame(Task t,const char*){fixture::native();fixture::posts.push_back(std::move(t));}
}
namespace background {bool post(std::function<void()> f,Lane){fixture::native();fixture::jobs.push_back(std::move(f));return true;} }
namespace data {bool game_data_ready(){return true;} }
namespace modtext {
const std::wstring& lang(){fixture::native();static const std::wstring en=L"en";return en;}
std::wstring T(std::wstring_view s){return std::wstring(s);}
std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring> args){std::wstring out(s);int i=0;for(const auto& a:args){const auto token=L"{"+std::to_wstring(i++)+L"}";auto at=out.find(token);if(at!=out.npos)out.replace(at,token.size(),a);}return out;}
}
namespace ffpr {
void say(std::wstring_view line,bool){fixture::native();fixture::speech.emplace_back(line);}
void say_sequence(std::vector<std::wstring> lines,bool){for(auto& line:lines)say(line,false);}
void say(std::wstring_view line,bool interrupt,speech::Timing){say(line,interrupt);}
void say_sequence(std::vector<std::wstring> lines,bool interrupt,speech::Timing){say_sequence(std::move(lines),interrupt);}
}
namespace settings {bool get_bool(const char* key,bool v){return std::string(key)==ui::prefs::kNavigationPuzzleGuidance?fixture::nav_guidance:std::string(key)==ui::prefs::kOtherPuzzleGuidance?fixture::other_guidance:v;}int get_int(const char* key,int v){return std::string(key)==ui::prefs::kPuzzleGuideVerbosity&&fixture::verbosity>=0?fixture::verbosity:v;} }
namespace input {bool game_window_focused(){fixture::native();return fixture::focused;} }
namespace log_ {void write(const char*,...){} }
namespace unity {int array_length(void* a){fixture::native();if(a==&fixture::door_locals)return 8;return a?int(static_cast<std::vector<void*>*>(a)->size()):0;} void* array_at(void* a,int i){fixture::native();return a&&i>=0&&i<array_length(a)?(*static_cast<std::vector<void*>*>(a))[i]:nullptr;} void* transform(void* p){fixture::native();return p;} Vec3 local_position(void* p){fixture::native();return static_cast<fixture::Actor*>(p)->position;} }
namespace field {
namespace r {ref::Class kFieldEntity{"Last.Entity.Field","FieldEntity"},kFieldNonPlayer{"Last.Entity.Field","FieldNonPlayer"},kPropertyEntity{"Last.Map","PropertyEntity"};ref::Field fPropEntityId{kPropertyEntity,"<EntityId>k__BackingField"};}
void* entity_property(void* p){fixture::native();return p;}
bool entity_is_active(void* p){fixture::native();return p!=nullptr;}
bool EntityCursor::next(void*&){fixture::native();return false;}
int current_map_id(){fixture::native();return fixture::map;}
void* map_handle(){fixture::native();return nullptr;}void* player(){fixture::native();return nullptr;}
const std::wstring& scene_name(){fixture::native();return fixture::scene;}
std::wstring map_exit_name(int id){fixture::native();return std::to_wstring(id);}
}
namespace field::map_state {Flags snapshot(){fixture::native();++fixture::state_reads;return fixture::flags;} std::optional<int> scenario_flag(int n){fixture::native();++fixture::scalar_reads;return n>=0&&n<1024?fixture::flags.scenario[size_t(n)]:std::optional<int>{};} }
namespace field::scanner {void invalidate_map(){fixture::native();} }
namespace field::baked_nav {
std::wstring map_name_of(int){fixture::native();return L"Fixture map";}
struct ExitSnapshot {std::array<std::optional<baked_route::Cell>,64> cells;};
std::shared_ptr<ExitSnapshot> begin_exit_capture(bool,bool,bool){fixture::native();auto out=std::make_shared<ExitSnapshot>();out->cells=ffpr::ff6::defense_watch::cells;return out;}
void capture_exit_obstacle(ExitSnapshot&,void*){fixture::native();}
void finish_exit_capture(ExitSnapshot&){fixture::native();}
DefenseAdvice defense_advice(ExitSnapshot& snapshot,const DefenseContext& context,std::function<bool()>){DefenseAdvice out;for(const auto& goal:context.enemies)if(snapshot.cells[static_cast<size_t>(goal.entity_id)]){out.intercept=goal;out.intercept->cell=*snapshot.cells[static_cast<size_t>(goal.entity_id)];out.warning=L"Terra in danger. Party 2 closest.";out.closest_party=1;break;}return out;}
bool cell_at(void*,unity::Vec3 at,baked_route::Cell& out){fixture::native();out={1,static_cast<int>(at.x),static_cast<int>(at.y)};return true;}
bool prepared(){return true;}void invalidate(){fixture::native();}void clear_pending(bool){fixture::native();}
void clear_preview_destination(const MapGoal&){fixture::native();++fixture::retired;}
bool player_cell(void*,void*,baked_route::Cell& out){fixture::native();out=fixture::owner_cell;return true;}
void request_guidance(std::vector<GuideTarget> candidates,std::function<void(PathInfo)> fn,const map_state::Flags* release,const DefenseContext*){fixture::native();fixture::first_target=candidates.front().goal.entity_id;if(release&&release->area[1]==0)++fixture::predicted;++fixture::requests;fixture::route=std::move(fn);}
}
namespace ffpr::fieldnav::audio_loops {void stop_all(){fixture::native();}void start_if_enabled(){fixture::native();} }
#ifndef FFPR_SERPENT_REAL
namespace ffpr::ff6::serpent {
bool choice_available(){return false;}
std::wstring choice_instruction(){return {};}
int install_choices(){return 0;}
}
#endif
#ifndef FFPR_PALACE_REAL
namespace ffpr::ff6::palace {
bool choice_available(){return false;}
bool choice_enabled(){return true;}
std::wstring choice_instruction(){return {};}
int install_choices(){return 0;}
}
#endif
namespace ffpr::ff6::story {
bool supported_map(int map){if(map>=334&&map<=343)return true;if(map>=290&&map<=295)return true;if(map==311||(map>=488&&map<=492))return true;return map==274||map==278||map==383||map==472||map==474||map==475||map==201||map==433||map==434||map==496||map==300||map==304||map==597||(map>=190&&map<=193)||map==143||map==162||map==88||map==2||map==218||map==249||map==41||map==48||map==50||map==52||map==60||map==626||map==584||map==540||map==407||map==408||map==498||map==499||map==318||map==320||map==117||map==504||map==507||map==372||map==577||map==466||map==486||map==445||map==386||map==387||map==431||map==631;}
Plan plan(const Snapshot& input){
 Plan out;out.map=input.map;out.supported=true;out.finish_on_exit=fixture::finish_exit;out.finish_on_scenario=fixture::finish_scenario;out.finish_through=fixture::exit_through;out.introduction=L"Do not repeat this introduction";
 out.puzzle_types=input.map==162||input.map==88||input.map==466||input.map==386||input.map==540?2u:1u;
 if(input.map==88){field::baked_nav::GuideTarget target;target.goal.map_id=88;target.goal.entity_id=10;target.instruction=L"Choose the first option.";out.targets.push_back(target);return out;}
 if(input.map==143||input.map==162||input.map==41||input.map==48||input.map==50||input.map==52||input.map==60||input.map==626||input.map==218){field::baked_nav::GuideTarget target;target.goal.map_id=input.map;target.goal.entity_id=fixture::floating_control;target.instruction=input.map==60||input.map==41||input.map==218?L"Choose the second option.":L"Choose the first option.";out.targets.push_back(target);return out;}
 if(input.map==407){Plan::BridgeCrossing bridge;bridge.mode=fixture::bridge_mode;bridge.phase=0;bridge.immediate=fixture::bridge_immediate;bridge.cells={input.player,{1,input.player.x+1,input.player.y}};out.bridge=std::move(bridge);out.note=L"Bridge preview";return out;}
 if(input.map==408){field::baked_nav::GuideTarget target;target.goal.map_id=408;target.goal.entity_id=34;target.instruction=L"Choose the first option.";out.targets.push_back(target);return out;}
 if(input.map==540){field::baked_nav::GuideTarget target;target.goal.map_id=540;target.goal.entity_id=2;target.instruction=L"Choose the first option to feed at least 22 coral at once.";out.targets.push_back(target);return out;}
 if(input.map==431||input.map==631){field::baked_nav::GuideTarget target;target.goal.map_id=input.map;target.goal.entity_id=fixture::floating_control;target.instruction=L"Choose the second option.";out.targets.push_back(target);return out;}
 if(input.map==445&&fixture::door_wait){out.owzer_door_wait=true;out.note=L"Left door preview";return out;}
 if(input.map==504){field::baked_nav::GuideTarget roof;roof.goal.map_id=504;roof.goal.cell={1,37,12};roof.kind=field::TargetKind::Spot;if(input.player==field::baked_route::Cell{1,9,19}){roof.goal.cell={1,9,20};out.finish_on_exit=505;out.roof_timed=false;}else out.roof_timed=true;out.targets.push_back(roof);out.roof_start=input.player;return out;}
 field::baked_nav::GuideTarget target;target.goal.map_id=input.map==386?386:input.map==466?466:498;target.goal.cell={1,4,5};target.goal.entity_id=input.map==386?11:input.map==466?0:42;target.instruction=input.map==386?L"Set the clock to 6:10:50. Choose options 3, 1, 5.":input.map==466?L"Examine the gravestone. Choose Yes, then options 4, 1, 2, 1.":L"Guide";out.targets.push_back(target);
 if(input.map==318){out.defense.emplace();for(int id:{18,19}){field::baked_nav::MapGoal goal;goal.map_id=318;goal.entity_id=id;out.defense->enemies.push_back(goal);}}
 if(fixture::released_refresh){target.goal.entity_id=108;out.targets.insert(out.targets.begin(),target);}if(fixture::held_plate){out.anticipated_release=input.flags;out.anticipated_release->area[1]=0;out.hold_note=L"Stay on this switch";}return out;
}
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    using namespace ffpr::ff6::story;int failures=0;
    auto expect=[&](bool v,const char* name){if(!v){++failures;std::printf("FAIL %s\n",name);}};
    {
        fixture::offsets["<SaveData>k__BackingField"]=offsetof(fixture::SaveManager,data);
        fixture::offsets["otherPartyDataList"]=offsetof(fixture::SavedData,parties);
        fixture::offsets["mapId"]=offsetof(fixture::SavedParty,map);
        fixture::offsets["<PlayerEntity>k__BackingField"]=offsetof(fixture::SavedParty,player);
        fixture::offsets["position"]=offsetof(fixture::SavedEntity,position);
        fixture::SavedEntity entity{{16,32,0}};
        fixture::SavedParty current{318,&entity,{}},parked{318,nullptr,L"{\"position\":{\"x\":48,\"y\":64}}"},foreign{499,&entity,{}};
        std::vector<void*> rows{&current,&parked,&foreign};
        fixture::SavedData data{&rows};fixture::SaveManager manager{&data};fixture::save_manager=&manager;
        Snapshot captured;captured.map=318;capture_parties(captured);
        expect(captured.parties[0].position&&captured.parties[0].position->x==16&&
            captured.parties[1].serialized==parked.serialized&&!captured.parties[2].position,
            "defense captures only ready same-map native poses and the bounded serialized fallback");
        entity.position.x=99;parked.serialized=L"changed";
        expect(captured.parties[0].position->x==16&&captured.parties[1].serialized.find(L"48")!=std::wstring::npos,
            "published party observations own their Vec3 and string independently of native memory");
        const int reads=fixture::save_reads;Snapshot outside;outside.map=498;capture_parties(outside);
        expect(fixture::save_reads==reads,"Phoenix uses native switch receipts without an extra saved-party read");
        rows.push_back(&foreign);Snapshot oversized;oversized.map=318;capture_parties(oversized);
        expect(!oversized.parties[0].position&&oversized.parties[0].map==0,"unexpected native party-array length is rejected before row reads");
        rows.pop_back();parked.serialized.assign(16385,L'x');Snapshot too_long;too_long.map=318;capture_parties(too_long);
        expect(too_long.parties[1].serialized.empty()&&fixture::maximum_string==16384,
            "oversized serialized party data is not copied");
        fixture::save_manager=nullptr;
    }
    fixture::flags.global[4]=0;fixture::flags.global[5]=2;install();
    next(true);expect(g_active&&fixture::jobs.size()==1,"explicit guide captures and queues one worker");
    const auto context=g_context;fixture::ready=false;d_party(nullptr,1,nullptr);
    expect(g_context>context&&fixture::party_events==1&&g_party_cleanup,"party setter invalidates old context without doing ready cleanup");
    fixture::flush();expect(fixture::requests==0&&fixture::speech.empty(),"old party worker cannot publish or speak");
    expect(fixture::step&&fixture::step(),"party follow-up waits for native field readiness");
    fixture::ready=true;expect(!fixture::step(),"ready party continuation is finite");
    fixture::flush();expect(fixture::requests==0&&g_slot==1,"party changes update ownership without unsolicited directions");
    expect(!fixture::speech.empty()&&fixture::speech[0]==L"Party 2","party index is one-based in speech");
    next(true);fixture::flush();expect(fixture::requests==1,"Shift+G obtains a route for the actual current party");
    field::PathInfo path;path.success=true;path.guidance_index=0;path.guidance=L"Guide";path.description=L"North";
    auto previous=fixture::route;fixture::route(path);expect(fixture::speech.back()==L"North Guide","requested directions omit party summaries and introductions");const auto spoken=fixture::speech.size();
    fixture::actions["GameModule.FieldStateChanged"]();previous(path);
    expect(fixture::speech.size()==spoken,"script state change rejects an old route completion before refresh");
    fixture::released_refresh=true;fixture::step();fixture::flush();next(true);fixture::flush();
    expect(fixture::first_target==42,"native plate-release refresh keeps the pending objective instead of sending the party back to the old plate");
    fixture::released_refresh=false;fixture::route(path);
    field::baked_nav::MapGoal other;other.map_id=498;other.entity_id=7;other.cell={1,6,7};g_party_goals[0]=other;
    fixture::ready=false;d_party(nullptr,0,nullptr);fixture::ready=true;fixture::step();
    expect(g_last_goal&&g_last_goal->entity_id==7&&g_party_goals[1]&&g_party_goals[1]->entity_id==42,
        "each native party restores its own pending objective rather than borrowing the other party's goal");
    stop(false);fixture::flush();
    expect(!g_active&&fixture::retired>0,"stopping retires only the guide preview");
    previous(path);expect(!g_active,"late route cannot revive stopped guidance");
    fixture::held_plate=true;fixture::flags.area[1]=1;next(true);fixture::flush();
    field::PathInfo blocked;fixture::route(blocked);
    expect(fixture::predicted==1&&fixture::flags.area[1]==1&&fixture::speech.back().find(L"Stay on this switch")!=std::wstring::npos,
        "no safe departure asks the party to stay, without changing the held native value");
    path.guidance=L"Continue";fixture::route(path);
    expect(fixture::speech.back().find(L"Continue")!=std::wstring::npos,"a safe departure after another party opens a route gives the real next objective");
    stop(false);fixture::held_plate=false;
    {
        namespace watch=ffpr::ff6::defense_watch;
        fixture::map=318;fixture::flags.global[5]=3;fixture::offsets["<EntityId>k__BackingField"]=offsetof(fixture::Actor,id);
        fixture::flags.scenario[13]=1;fixture::flags.scenario[14]=0;
        watch::install();watch::o_update=&fixture::update;watch::reset();fixture::speech.clear();
        fixture::Actor terra{32,{10,10,0}},enemy{18,{10,14,0}};
        watch::d_update(&terra,nullptr);watch::d_update(&enemy,nullptr);
        expect(!watch::pending&&fixture::speech.empty(),"four-tile enemy produces no danger warning");
        enemy.position.y=13;watch::d_update(&enemy,nullptr);
        expect(watch::pending&&fixture::speech.empty(),"native entry into three-tile range starts one owned capture without waiting");
        fixture::step();fixture::flush();
        expect(fixture::speech==std::vector<std::wstring>({L"Terra in danger. Party 2 closest."}),"one brief warning names the closest interception party");
        for(int i=0;i<20;++i)watch::d_update(&enemy,nullptr);
        expect(fixture::speech.size()==1&&!watch::pending,"unchanged native updates do not repeat the danger warning");
        next(true);fixture::flush();path.defense_warning=L"Terra in danger. Party 2 closest.";fixture::route(path);
        next(true);fixture::flush();fixture::route(path);
        expect(fixture::speech.size()==3&&fixture::speech.back().find(L"Terra in danger.")==0,"each explicit Shift+G repeats current danger advice");
        stop(false);enemy.position.y=14;watch::d_update(&enemy,nullptr);enemy.position.y=13;watch::d_update(&enemy,nullptr);
        fixture::step();fixture::flush();expect(fixture::speech.size()==4,"leaving and re-entering danger creates a new native occurrence");
        enemy.position.y=14;watch::d_update(&enemy,nullptr);enemy.position.y=13;watch::d_update(&enemy,nullptr);fixture::step();
        watch::retire();fixture::flush();expect(fixture::speech.size()==4,"map retirement cancels a parked warning");
        enemy.position.y=14;watch::d_update(&enemy,nullptr);enemy.position.y=13;watch::d_update(&enemy,nullptr);fixture::step();
        fixture::flags.scenario[14]=1;fixture::flush();expect(fixture::speech.size()==4,"completed native phase rejects an otherwise current warning on the same map");
    }
    stop(false);fixture::map=320;fixture::finish_exit=317;fixture::ready=true;
    next(true);fixture::flush();
    expect(g_active&&g_finish_on_exit==317,"completed native light stage arms a specific exit without claiming arrival");
    events::fire(events::Kind::MapChanging);fixture::map=317;events::fire(events::Kind::MapChanged);
    const auto before_exit=fixture::speech.size();fixture::ready=false;
    expect(fixture::step&&fixture::step()&&fixture::speech.size()==before_exit,"exit completion waits for native map readiness");
    fixture::ready=true;expect(!fixture::step()&&!g_active&&fixture::speech.back()==L"Puzzle guide finished",
        "only the actual expected northern arrival completes the light guide");
    fixture::map=320;next(true);fixture::flush();fixture::map=321;events::fire(events::Kind::MapChanged);fixture::step();
    expect(g_active&&!g_finish_on_exit,"leaving by a different exit clears the arm without a false completion");
    fixture::map=320;next(true);fixture::flush();events::fire(events::Kind::BattleEntering);
    expect(!g_finish_on_exit,"battle/failure invalidates a previously armed exit");
    stop(false);fixture::finish_exit=0;
    fixture::map=499;fixture::flags.global[5]=2;fixture::finish_exit=262;next(true);fixture::flush();
    expect(g_active&&g_finish_on_exit==262,"Phoenix's final objective arms its exact native reunion arrival");
    fixture::ready=false;const auto before_reunion=fixture::speech.size();
    for(int map:{584,267,266}) {
        events::fire(events::Kind::MapChanging);fixture::map=map;events::fire(events::Kind::MapChanged);
        expect(fixture::step&&fixture::step()&&g_active&&g_finish_on_exit==262&&fixture::speech.size()==before_reunion,
            "intermediate reunion movie maps neither finish nor discard the pending guide exit");
    }
    events::fire(events::Kind::MapChanging);fixture::map=262;events::fire(events::Kind::MapChanged);fixture::ready=true;
    expect(fixture::step&&!fixture::step()&&!g_active&&fixture::speech.back()==L"Puzzle guide finished",
        "the complete reunion chain finishes only after the last native map is ready");
    fixture::finish_exit=0;
    fixture::map=117;fixture::finish_scenario=177;fixture::flags.scenario[177]=0;
    next(true);fixture::flush();
    expect(g_active&&g_finish_on_scenario==177,"Opera arms its actual persistent completion receipt");
    events::fire(events::Kind::BattleEntering);
    expect(g_active&&g_finish_on_scenario==177&&!g_finish_on_exit,"Opera's completion survives its native battle while transient exit arms still retire");
    fixture::ready=false;events::fire(events::Kind::BattleEnd);
    expect(fixture::step&&fixture::step()&&g_active,"battle completion waits for field readiness");
    fixture::ready=true;expect(!fixture::step()&&g_active,"an unfinished Opera battle cannot claim story completion");
    fixture::ready=false;const auto before_opera_end=fixture::speech.size();
    for(int map:{118,114,123,120}) {
        events::fire(events::Kind::MapChanging);fixture::map=map;events::fire(events::Kind::MapChanged);
        expect(fixture::step&&fixture::step()&&g_active&&fixture::speech.size()==before_opera_end,
            "Opera's victory scene chain cannot finish before native readiness and its real flag");
    }
    fixture::flags.scenario[177].reset();fixture::ready=true;
    expect(!fixture::step()&&g_active,"an unknown completion flag never certifies success");
    fixture::flags.scenario[177]=1;actions::run("GameModule.FieldStateChanged");
    expect(fixture::step&&!fixture::step()&&!g_active&&g_finish_on_scenario==-1&&fixture::speech.back()==L"Puzzle guide finished",
        "native flag177 finishes the whole Opera sequence once on the ready final map");
    fixture::map=117;fixture::flags.scenario[177]=0;next(true);fixture::flush();stop(false);
    const auto after_stop=fixture::speech.size();fixture::flags.scenario[177]=1;events::fire(events::Kind::MapChanged);
    expect(!g_active&&g_finish_on_scenario==-1&&fixture::speech.size()==after_stop,"cancelled Opera guidance cannot announce a later native victory");
    fixture::finish_scenario=-1;
    fixture::map=507;fixture::finish_scenario=410;fixture::flags.scenario[410]=0;next(true);fixture::flush();
    expect(g_active&&g_finish_on_scenario==410,"Gogo guidance arms the native joining receipt without proximity completion");
    fixture::ready=false;const auto before_join=fixture::speech.size();fixture::flags.scenario[410]=1;actions::run("GameModule.FieldStateChanged");
    expect(fixture::step&&fixture::step()&&g_active&&fixture::speech.size()==before_join,"joining flag during naming/dialogue waits for normal field readiness");
    fixture::ready=true;expect(!fixture::step()&&!g_active&&fixture::speech.back()==L"Puzzle guide finished","completed native joining finishes only after the dialogue releases field control");
    fixture::finish_scenario=-1;
    fixture::map=372;fixture::finish_exit=1;next(true);fixture::flush();
    expect(g_active&&g_finish_on_exit==1,"ordinary train departure arms only the actual world-map exit");
    fixture::ready=false;const auto before_train_exit=fixture::speech.size();events::fire(events::Kind::MapChanging);fixture::map=1;events::fire(events::Kind::MapChanged);
    expect(fixture::step&&fixture::step()&&g_active&&fixture::speech.size()==before_train_exit,"train departure scene waits for normal world-map readiness");
    fixture::ready=true;expect(!fixture::step()&&!g_active&&fixture::speech.back()==L"Puzzle guide finished","ready world1 arrival completes the entire ordinary train journey");fixture::finish_exit=0;
    fixture::map=577;fixture::finish_exit=2;next(true);fixture::flush();const auto tomb_route=fixture::route;
    fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=1;events::fire(events::Kind::MapChanged);
    expect(fixture::step()&&g_active,"Darill movie-only world1 cannot finish the native departure");
    events::fire(events::Kind::MapChanging);fixture::map=268;events::fire(events::Kind::MapChanged);expect(fixture::step()&&g_active,"airship movie retains the pending tomb journey");
    events::fire(events::Kind::MapChanging);fixture::map=2;events::fire(events::Kind::MapChanged);expect(fixture::step()&&g_active,"world2 scene still waits for actual field control");
    fixture::ready=true;expect(!fixture::step()&&!g_active&&fixture::speech.back()==L"Puzzle guide finished","actual ready world2 completes Darill departure");const auto tomb_speech=fixture::speech.size();tomb_route(path);expect(fixture::speech.size()==tomb_speech,"obsolete tomb route cannot speak after completion");fixture::finish_exit=0;
    fixture::map=486;fixture::finish_exit=476;fixture::exit_through={486,484,482,480,477};next(true);fixture::flush();const auto tower_route=fixture::route;
    for(int map:{484,482,480,477}){events::fire(events::Kind::MapChanging);fixture::map=map;events::fire(events::Kind::MapChanged);expect(!fixture::step()&&g_active&&g_finish_on_exit==476,"reviewed ordinary tower floors retain the destination completion arm");}
    fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=476;events::fire(events::Kind::MapChanged);expect(fixture::step()&&g_active,"actual tower entrance still waits for normal field readiness");
    fixture::ready=true;expect(!fixture::step()&&!g_active&&g_finish_through.empty()&&fixture::speech.back()==L"Puzzle guide finished","only ready476 completes the full tower return");const auto tower_speech=fixture::speech.size();tower_route(path);expect(fixture::speech.size()==tower_speech,"completed tower rejects a stale route publication");
    fixture::map=486;next(true);fixture::flush();fixture::map=479;events::fire(events::Kind::MapChanged);fixture::step();expect(!g_finish_on_exit&&g_finish_through.empty(),"an unreviewed detour still disarms completion like the existing immediate-exit guides");
    fixture::map=486;next(true);fixture::flush();events::fire(events::Kind::BattleEntering);expect(!g_finish_on_exit&&g_finish_through.empty(),"battle clears a tower continuation arm");stop(false);fixture::finish_exit=0;fixture::exit_through.clear();
    fixture::map=466;next(true);fixture::flush();fixture::route(path);const auto epitaph_route=fixture::route;
    fixture::ready=false;fixture::choice=true;const int epitaph_reads=fixture::state_reads,epitaph_requests=fixture::requests;const auto epitaph_speech=fixture::speech.size();
    expect(queries::is("GameModule.PuzzleGuideRepeatDuringEvent"),"actual published epitaph instruction enables native-choice repeat");next(true);next(true);
    expect(fixture::speech.size()==epitaph_speech+2&&fixture::speech.back()==L"Examine the gravestone. Choose Yes, then options 4, 1, 2, 1.","each explicit native-choice request repeats the exact owned answer without deduplication");
    expect(fixture::state_reads==epitaph_reads&&fixture::requests==epitaph_requests&&fixture::jobs.empty(),"epitaph repeat observes no flags, worker or route");
    fixture::choice=false;fixture::dialogue=true;next(true);expect(fixture::speech.size()==epitaph_speech+3,"initial Yes dialogue can repeat the same instruction");
    fixture::map=577;expect(!queries::is("GameModule.PuzzleGuideRepeatDuringEvent"),"another map cannot reuse an epitaph instruction");fixture::map=466;stop(false);expect(!queries::is("GameModule.PuzzleGuideRepeatDuringEvent")&&g_repeat_instruction.empty(),"stop clears the owned epitaph repeat");const auto stopped_epitaph=fixture::speech.size();epitaph_route(path);expect(fixture::speech.size()==stopped_epitaph&&g_repeat_instruction.empty(),"late epitaph result cannot restore cancelled output");fixture::dialogue=false;fixture::ready=true;
    {
        // Actual new native reader + reset producer + existing controller. The
        // rule model's physical choices are independently checked on real bakes.
        using namespace fixture;stop(false);map=504;fixture::ready=true;owner_cell={1,37,18};roof_instance=&roof_manager;
        offsets["<GrobalTimer>k__BackingField"]=offsetof(RoofManager,global);
        offsets["<ScriptList>k__BackingField"]=offsetof(RoofGlobal,scripts);
        offsets["<Timer>k__BackingField"]=offsetof(RoofGlobal,timer);
        offsets["requiredSec"]=offsetof(RoofClock,required);offsets["elapsedSec"]=offsetof(RoofClock,elapsed);offsets["isFinished"]=offsetof(RoofClock,finished);
        original_timer(&roof_manager,5,nullptr,&roof_names,false,nullptr);roof_clock.elapsed=.3f;
        Snapshot observed;observed.map=504;capture_roof_timer(observed);
        expect(observed.roof_seconds&&std::abs(*observed.roof_seconds-4.7f)<.001f&&!observed.roof_frozen,"native roof timer copies exact owned values without mutation");
        expect(roof_clock.elapsed==.3f&&roof_clock.required==5&&!roof_clock.finished,"timer observation is read only");
        const auto roof_path=[] {
            field::PathInfo p;p.success=true;p.guidance_index=0;p.description=L"6 North";
            field::route_progress::Phase phase;for(int y=18;y>=12;--y)phase.cells.push_back({1,37,y});p.walk_phases.push_back(std::move(phase));return p;
        };
        const auto deliver=[&]{auto callback=std::move(route);expect(bool(callback),"one bounded pending route callback exists");if(callback)callback(roof_path());};
        const auto preview=[&]{owner_cell={1,37,18};next(true);flush();deliver();expect(g_roof_pending&&!g_roof_pending->armed,"first request previews; it cannot arm a timed leg");};
        const auto go_count=[&]{return std::count(fixture::speech.begin(),fixture::speech.end(),L"Go now.");};
        preview();expect(fixture::speech.back().find(L"6 North")!=std::wstring::npos,"preview speaks the real directions before the movement window");
        const auto before_arm=requests;next(true);
        expect(g_roof_pending&&g_roof_pending->armed&&requests==before_arm&&fixture::speech.back()==L"Wait here for Go now.","second request arms and waits when current cycle is already late");
        fixture::ready=false;d_roof_set(&roof_manager,5,nullptr,&roof_names,false,nullptr);
        expect(step&&step()&&requests==before_arm,"native reset schedules one continuation and cannot route while input is unavailable");
        fixture::ready=true;expect(step&&!step()&&requests==before_arm+1,"native ready boundary requests exactly one release route");
        roof_clock.elapsed=.4f;const auto before_late=go_count();deliver();
        expect(g_roof_pending&&g_roof_pending->armed&&go_count()==before_late,"worker/route delay past the fresh window never announces Go");
        d_roof_set(&roof_manager,5,nullptr,&roof_names,false,nullptr);expect(step&&!step(),"next exact native cycle retries the already requested leg");
        roof_clock.elapsed=.1f;deliver();expect(!g_roof_pending&&go_count()==before_late+1,"only a fresh revalidated identical path announces the short Go cue");
        expect(g_last_goal&&g_last_goal->cell==field::baked_route::Cell{1,37,12},"released route owns the memorized safe-area destination");

        preview();roof_clock.elapsed=2;next(true);const auto before_move=go_count();owner_cell={1,36,18};
        d_roof_set(&roof_manager,5,nullptr,&roof_names,false,nullptr);if(step)step();
        expect(!g_roof_pending&&go_count()==before_move,"changed position cancels a waiting release instead of saying wait on unsafe ground");
        preview();roof_clock.elapsed=2;next(true);const auto before_cancel=go_count();stop(false);
        const int before_strings=string_reads;d_roof_set(&roof_manager,5,nullptr,&roof_names,false,nullptr);
        expect(!g_roof_pending&&go_count()==before_cancel&&string_reads==before_strings,"cancelled guide adds no timer-name reads or later Go cue");
        preview();roof_clock.elapsed=2;next(true);events::fire(events::Kind::MapChanging);d_roof_set(&roof_manager,5,nullptr,&roof_names,false,nullptr);
        expect(!g_roof_pending&&go_count()==before_cancel,"map/load transition retires the pending roof episode");

        stop(false);timers_frozen=true;roof_clock.elapsed=3;preview();const auto before_frozen=timer_originals;next(true);deliver();
        expect(!g_roof_pending&&go_count()==before_cancel+1&&timer_originals==before_frozen&&roof_clock.elapsed==3,"existing explicit timer freeze permits a leg without writing/resetting the timer");
        timers_frozen=false;stop(false);preview();roof_clock.elapsed=2;next(true);
        RoofName foreign;std::copy_n(L"sc_e_0851",10,foreign.chars);std::vector<void*> foreign_names{&foreign};
        const auto before_foreign=requests;d_roof_set(&roof_manager,5,nullptr,&foreign_names,false,nullptr);
        expect(!g_roof_resume&&requests==before_foreign,"another five-second script cannot release the roof guide");
        std::vector<void*> too_many{&roof_name,&roof_name};d_roof_set(&roof_manager,5,nullptr,&too_many,false,nullptr);
        expect(!g_roof_resume,"multi-script timer is unknown rather than a guessed roof timer");
        Snapshot bad;bad.map=504;capture_roof_timer(bad);expect(!bad.roof_seconds,"unsupported timer list is not copied");
        roof_instance=nullptr;bad={};bad.map=504;capture_roof_timer(bad);expect(!bad.roof_seconds,"absent singleton is not created or dereferenced");
        roof_instance=&roof_manager;original_timer(&roof_manager,5,nullptr,&roof_names,false,nullptr);roof_clock.elapsed=std::numeric_limits<float>::quiet_NaN();
        bad={};bad.map=504;capture_roof_timer(bad);expect(!bad.roof_seconds,"nonfinite native timer remains unknown");
        original_timer(&roof_manager,5,nullptr,&roof_names,false,nullptr);roof_name.length=10;bad={};bad.map=504;capture_roof_timer(bad);expect(!bad.roof_seconds,"exact native script length rejects prefix/NUL aliases");roof_name.length=9;
        stop(false);preview();fixture::ready=false;const int before_unready_reads=string_reads;next(true);
        expect(g_roof_pending&&g_roof_pending->armed&&string_reads==before_unready_reads,"readiness confirmation during animation copies no native timer or actor");
        fixture::ready=true;stop(false);
        preview();roof_clock.elapsed=2;next(true);const auto before_focus=go_count();focused=false;
        d_roof_set(&roof_manager,5,nullptr,&roof_names,false,nullptr);if(step)step();
        expect(g_roof_pending&&!g_roof_pending->armed&&go_count()==before_focus,"unfocused game never receives Go; renewed player readiness is required");focused=true;stop(false);
        preview();roof_clock.elapsed=2;next(true);mod_menu=true;
        d_roof_set(&roof_manager,5,nullptr,&roof_names,false,nullptr);if(step)step();
        expect(g_roof_pending&&!g_roof_pending->armed&&go_count()==before_focus,"mod settings cannot be interrupted by a timed release");mod_menu=false;stop(false);
        preview();next(true);focused=false;deliver();
        expect(g_roof_pending&&!g_roof_pending->armed&&go_count()==before_focus,"focus loss while route work is queued suppresses the stale Go cue");focused=true;stop(false);
        Snapshot wrong_thread;wrong_thread.map=504;
        std::thread other([&]{capture_roof_timer(wrong_thread);});other.join();expect(!wrong_thread.roof_seconds,"reader cannot inspect native timer pointers on a worker");
        Snapshot measured;measured.map=504;double sum_us=0,peak_us=0;
        for(int i=0;i<10000;++i){const auto begin=std::chrono::steady_clock::now();capture_roof_timer(measured);const double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count();sum_us+=us;peak_us=std::max(peak_us,us);}
        std::printf("Roof native timer capture10000: mean%.3fus peak%.3fus; capped fixture, not game-frame/audible evidence\n",sum_us/10000,peak_us);
        owner_cell={1,9,19};next(true);flush();auto final=std::move(route);field::PathInfo final_path;final_path.success=true;final_path.guidance_index=0;final_path.description=L"1 South";if(final)final(final_path);
        expect(g_finish_on_exit==505&&!g_roof_pending,"only the last actual safe area arms the505 completion boundary");
        events::fire(events::Kind::MapChanging);map=505;fixture::ready=false;events::fire(events::Kind::MapChanged);expect(step&&step()&&g_active,"roof completion waits for native destination readiness");
        fixture::ready=true;expect(step&&!step()&&!g_active&&fixture::speech.back()==L"Puzzle guide finished","actual505 arrival completes the roof guide once");
        stop(false);owner_cell={1,2,3};roof_instance=nullptr;map=498;
    }
    {
        using namespace fixture;
        stop(false);map=445;owner_cell={1,23,23};door_wait=true;door_storage.values=&door_locals;door_instance=&door_storage;
        offsets["<currentMapId>k__BackingField"]=offsetof(DoorStorage,map);offsets["map"]=offsetof(DoorStorage,values);
        finish_scenario=-1;finish_exit=0;exit_through.clear();door_test_time=100;
        const auto count_go=[&]{return std::count(fixture::speech.begin(),fixture::speech.end(),L"Go now.");};
        const auto preview=[&]{stop(false);next(true);flush();expect(g_owzer_pending&&!g_owzer_armed,"door staging first previews one cardinal step");};
        const auto arm=[&]{preview();next(true);expect(g_owzer_armed,"second explicit request arms only this exact native door");};
        preview();const auto initial=count_go();door_phase=0;d_cycle(nullptr,nullptr);flush();expect(count_go()==initial,"unarmed preview never receives an unsolicited timed cue");
        next(true);door_phase=1;d_cycle(nullptr,nullptr);flush();expect(count_go()==initial&&g_owzer_armed,"middle opening cannot release left-door advice");
        door_phase=2;d_cycle(nullptr,nullptr);flush();expect(count_go()==initial,"right opening cannot release left-door advice");
        door_phase=0;d_cycle(nullptr,nullptr);expect(count_go()==initial&&g_owzer_posted,"native callback only queues a bounded owned notification");flush();expect(count_go()==initial+1&&!g_owzer_pending,"fresh actual left opening releases exactly one Go cue");
        arm();const auto before=count_go();d_cycle(nullptr,nullptr);door_test_time+=.3;flush();expect(count_go()==before&&g_owzer_armed,"stale quarter-window notification waits for the next true opening");d_cycle(nullptr,nullptr);flush();expect(count_go()==before+1,"next fresh opening recovers without polling or changing the clock");
        for(int reason=0;reason<5;++reason){arm();const auto quiet=count_go();d_cycle(nullptr,nullptr);if(reason==0)focused=false;if(reason==1)mod_menu=true;if(reason==2)owner_cell={1,24,23};if(reason==3)events::fire(events::Kind::MapChanging);if(reason==4)events::fire(events::Kind::BattleEntering);flush();expect(count_go()==quiet&&!g_owzer_pending,"focus/menu/movement/map/battle invalidates queued door cue");focused=true;mod_menu=false;owner_cell={1,23,23};}
        arm();const auto cancelled=count_go();d_cycle(nullptr,nullptr);stop(false);flush();expect(count_go()==cancelled,"explicit stop rejects the queued native cue");
        arm();door_instance=nullptr;d_cycle(nullptr,nullptr);flush();expect(count_go()==cancelled&&g_owzer_armed,"missing storage is unknown and is never created");door_instance=&door_storage;
        door_storage.map=444;d_cycle(nullptr,nullptr);flush();expect(count_go()==cancelled,"a foreign DataStorage map cannot release the door");door_storage.map=445;
        std::thread foreign([&]{d_cycle(nullptr,nullptr);});foreign.join();flush();expect(count_go()==cancelled,"off-owner callback performs no observation or publication");
        door_phase=1;double sum=0,peak=0;for(int i=0;i<10000;++i){const auto a=std::chrono::steady_clock::now();d_cycle(nullptr,nullptr);const double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-a).count();sum+=us;peak=std::max(peak,us);}
        std::printf("Owzer observer10000: mean%.3fus peak%.3fus; bounded fixture, not game responsiveness\n",sum/10000,peak);
        stop(false);door_wait=false;door_instance=nullptr;door_test_time=-1;map=498;owner_cell={1,2,3};
    }
    {
        stop(false);fixture::map=386;fixture::ready=true;fixture::choice=fixture::dialogue=false;next(true);fixture::flush();
        auto result=std::move(fixture::route);field::PathInfo path;path.success=true;path.guidance_index=0;path.guidance=L"Set the clock to 6:10:50. Choose options 3, 1, 5.";if(result)result(path);
        expect(g_last_goal&&g_last_goal->map_id==386&&g_last_goal->entity_id==11&&!g_repeat_instruction.empty(),"only the actually published clock control owns its cached solution");
        fixture::choice=true;fixture::ready=false;const auto reads=fixture::state_reads,requests=fixture::requests;const auto lines=fixture::speech.size();next(true);next(true);
        expect(fixture::state_reads==reads&&fixture::requests==requests&&fixture::speech.size()==lines+2&&fixture::speech.back()==path.guidance,"every explicit clock-choice repeat speaks the solution without flags, routes, workers or text deduplication");
        g_last_goal->entity_id=14;expect(!epitaph_repeat_available(),"the reward chest cannot inherit the clock-choice repeat");g_last_goal->entity_id=11;fixture::map=387;expect(!epitaph_repeat_available(),"hint floor cannot reuse another floor's open clock menu");fixture::map=386;
        events::fire(events::Kind::MapChanging);expect(!epitaph_repeat_available()&&g_repeat_instruction.empty(),"map/load changes retire the clock solution");
        fixture::choice=false;fixture::ready=true;stop(false);fixture::map=498;
    }
    {
        using namespace fixture;stop(false);map=407;fixture::ready=true;owner_cell={1,8,4};
        door_storage.map=407;door_storage.values=&door_locals;door_instance=&door_storage;
        offsets["<currentMapId>k__BackingField"]=offsetof(DoorStorage,map);offsets["map"]=offsetof(DoorStorage,values);
        door_test_time=200;finish_scenario=-1;finish_exit=0;exit_through.clear();
        const auto go=[&]{return std::count(fixture::speech.begin(),fixture::speech.end(),L"Go now.");};
        for(int mode:{1,2}){
            bridge_mode=mode;const auto cycle=[&]{if(mode==1)d_bridge_a(nullptr,nullptr);else d_bridge_b(nullptr,nullptr);};
            const auto preview=[&]{stop(false);next(true);flush();expect(g_bridge_pending&&!g_bridge_pending->armed,"bridge preview waits for a second deliberate readiness request");};
            preview();auto before=go();bridge_phase=0;cycle();flush();expect(go()==before,"unarmed native bridge ticks produce no cue");
            next(true);bridge_phase=1;cycle();flush();expect(go()==before&&g_bridge_pending&&g_bridge_pending->armed,"wrong native bridge phase cannot release a crossing");
            bridge_phase=0;cycle();expect(go()==before&&g_bridge_posted,"native bridge callback posts without speaking or waiting");flush();expect(go()==before+1&&!g_bridge_pending,"fresh matching native bridge phase produces exactly one cue");cycle();flush();expect(go()==before+1,"one request cannot produce repeated bridge cues");
            preview();next(true);before=go();cycle();door_test_time+=.3;flush();expect(go()==before&&g_bridge_pending,"late bridge notification waits for another complete native window");cycle();flush();expect(go()==before+1,"the following fresh native cycle can release the waiting crossing");
            preview();next(true);before=go();cycle();focused=false;flush();expect(go()==before&&!g_bridge_pending,"focus loss retires a queued bridge cue");focused=true;
            preview();next(true);cycle();++owner_cell.x;flush();expect(go()==before&&!g_bridge_pending,"moving away from the preview start suppresses the bridge cue");--owner_cell.x;
            preview();next(true);cycle();stop(false);flush();expect(go()==before&&!g_bridge_pending,"stopping guidance cancels queued bridge advice");
            preview();next(true);cycle();events::fire(events::Kind::BattleEntering);flush();expect(go()==before&&!g_bridge_pending,"battle entry retires bridge advice");
            preview();next(true);door_instance=nullptr;cycle();flush();expect(go()==before&&g_bridge_pending,"missing bridge storage is unknown and is never created");door_instance=&door_storage;
            bridge_phase=2;cycle();flush();expect(go()==before,"invalid native bridge phase cannot become an opening");
            bridge_phase=0;std::thread foreign([&]{cycle();});foreign.join();flush();expect(go()==before,"foreign-thread bridge callback performs no native observation or speech");
            stop(false);
        }
        stop(false);bridge_mode=2;bridge_phase=0;d_bridge_b(nullptr,nullptr);bridge_immediate=true;
        auto lines=fixture::speech.size();next(true);flush();expect(!g_bridge_pending&&fixture::speech.size()==lines+1&&fixture::speech.back()==L"Bridge preview","in-progress same-phase suffix is spoken immediately without waiting on a temporary tile");
        stop(false);bridge_phase=1;d_bridge_b(nullptr,nullptr);lines=fixture::speech.size();next(true);flush();expect(!g_bridge_pending&&fixture::speech.size()==lines,"changed native phase rejects an obsolete immediate crossing");bridge_immediate=false;
        stop(false);map=408;next(true);flush();auto delivered=std::move(fixture::route);field::PathInfo selected;selected.success=true;selected.guidance_index=0;selected.guidance=L"Choose the first option.";if(delivered)delivered(selected);
        fixture::choice=true;fixture::ready=false;const auto reads=state_reads,calls=requests;lines=fixture::speech.size();next(true);next(true);
        expect(state_reads==reads&&requests==calls&&fixture::speech.size()==lines+2&&fixture::speech.back()==selected.guidance,"native bridge choice repeats only its cached instruction without snapshots, routing or text deduplication");
        events::fire(events::Kind::MapChanging);expect(!epitaph_repeat_available(),"bridge-choice cache retires on the native map boundary");fixture::choice=false;fixture::ready=true;stop(false);
        door_instance=nullptr;door_test_time=-1;map=498;owner_cell={1,2,3};
    }
    stop(false);fixture::map=498;fixture::verbosity=1;fixture::speech.clear();next(true);fixture::flush();
    {auto delivered=std::move(fixture::route);field::PathInfo selected;selected.success=true;selected.guidance_index=0;selected.description=L"3 West";selected.guidance=L"Operate the switch.";if(delivered)delivered(selected);}
    expect(!fixture::speech.empty()&&fixture::speech.back().find(L"Puzzle guide:")!=std::wstring::npos&&fixture::speech.back().find(L"3 West")!=std::wstring::npos,"FFVI Verbose activation retains actual directions and action");
    stop(false);fixture::verbosity=-1;fixture::speech.clear();next(true);fixture::flush();
    {auto delivered=std::move(fixture::route);field::PathInfo selected;selected.success=true;selected.guidance_index=0;selected.description=L"3 West";selected.guidance=L"Operate the switch.";if(delivered)delivered(selected);}
    expect(!fixture::speech.empty()&&fixture::speech.back().find(L"Puzzle guide:")==std::wstring::npos&&fixture::speech.back().find(L"Operate the switch.")!=std::wstring::npos,"FFVI default Minimal retains directions/action and omits activation metadata");stop(false);
    for(int mid:{431,631})for(int control:{155,11}){
        stop(false);fixture::map=mid;fixture::floating_control=control;next(true);fixture::flush();
        auto delivered=std::move(fixture::route);field::PathInfo selected;selected.success=true;selected.guidance_index=0;selected.guidance=L"Choose the second option.";if(delivered)delivered(selected);
        fixture::choice=true;fixture::ready=false;const int reads=fixture::state_reads,calls=fixture::requests;const auto lines=fixture::speech.size();next(true);next(true);
        expect(fixture::state_reads==reads&&fixture::requests==calls&&fixture::speech.size()==lines+2&&fixture::speech.back()==selected.guidance,"both Floating Continent airship choices repeat owned instructions on both aliases without native snapshots or routes");
        g_last_goal->entity_id=71;expect(!epitaph_repeat_available(),"the nearby boss cannot inherit an airship-choice instruction");g_last_goal->entity_id=control;
        events::fire(events::Kind::MapChanging);expect(!epitaph_repeat_available(),"native map boundary retires Floating Continent choice cache");fixture::choice=false;fixture::ready=true;stop(false);
    }
    fixture::map=631;fixture::floating_control=11;fixture::finish_exit=191;fixture::exit_through={119,431,605,608,334,582,1,584,2,192,190};next(true);fixture::flush();
    for(int mid:fixture::exit_through){
        fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=mid;events::fire(events::Kind::MapChanged);
        expect(fixture::step&&fixture::step()&&g_active,"native catastrophe transition waits for input readiness");
        fixture::ready=true;expect(!fixture::step()&&g_active&&g_finish_on_exit==191,"exact native intermediate map cannot finish the Floating Continent guide");
    }
    events::fire(events::Kind::MapChanging);fixture::map=191;events::fire(events::Kind::MapChanged);fixture::ready=false;
    expect(fixture::step&&fixture::step()&&g_active,"final191 still waits for native field readiness");fixture::ready=true;
    expect(!fixture::step()&&!g_active&&fixture::speech.back()==L"Puzzle guide finished","only final ready191 finishes the complete native escape chain");fixture::finish_exit=0;fixture::exit_through.clear();
    stop(false);fixture::map=540;fixture::finish_scenario=461;fixture::flags.scenario[461]=0;next(true);fixture::flush();
    {auto delivered=std::move(fixture::route);field::PathInfo selected;selected.success=true;selected.guidance_index=0;selected.guidance=L"Choose the first option to feed at least 22 coral at once.";if(delivered)delivered(selected);fixture::choice=true;fixture::ready=false;const auto reads=fixture::state_reads,calls=fixture::requests;const auto lines=fixture::speech.size();next(true);next(true);expect(fixture::state_reads==reads&&fixture::requests==calls&&fixture::speech.size()==lines+2&&fixture::speech.back()==selected.guidance,"coral choice repeats owned instructions without reads or routing");fixture::choice=false;fixture::ready=true;}
    events::fire(events::Kind::BattleEntering);events::fire(events::Kind::BattleEnd);
    for(int mid:{311,305,310,305,310}){events::fire(events::Kind::MapChanging);fixture::map=mid;events::fire(events::Kind::MapChanged);expect(fixture::step&&!fixture::step()&&g_active,"early Hidon scene maps cannot complete without461");}
    fixture::flags.scenario[461]=1;fixture::ready=false;events::fire(events::Kind::MapChanged);expect(fixture::step&&fixture::step()&&g_active,"final Hidon receipt still waits for field readiness");fixture::ready=true;expect(!fixture::step()&&!g_active,"final ready461 completes the whole native scene chain");fixture::finish_scenario=-1;
    for(auto [mid,control]:{std::pair{60,1},std::pair{50,11},std::pair{50,6},std::pair{52,1},std::pair{41,12},std::pair{48,19},std::pair{218,5},std::pair{88,10},std::pair{143,61}}){
        stop(false);fixture::map=mid;fixture::floating_control=control;fixture::ready=true;next(true);fixture::flush();
        auto delivered=std::move(fixture::route);field::PathInfo selected;selected.success=true;selected.guidance_index=0;selected.guidance=mid==60||mid==41||mid==218?L"Choose the second option.":L"Choose the first option.";if(delivered)delivered(selected);
        fixture::choice=true;fixture::ready=false;const auto reads=fixture::state_reads,calls=fixture::requests,lines=static_cast<int>(fixture::speech.size());next(true);next(true);
        expect(fixture::state_reads==reads&&fixture::requests==calls&&fixture::speech.size()==size_t(lines+2)&&fixture::speech.back()==selected.guidance,"infiltration choice repeats its actually published instruction without native reads, routing or deduplication");
        events::fire(events::Kind::MapChanging);expect(!epitaph_repeat_available(),"infiltration choice cache cannot cross a native floor change");fixture::choice=false;fixture::ready=true;
    }
    for(int other_scenarios:{0,1}){
        stop(false);fixture::map=626;fixture::floating_control=3;fixture::finish_scenario=64;fixture::flags.scenario[64]=0;next(true);fixture::flush();
        events::fire(events::Kind::BattleEntering);events::fire(events::Kind::BattleEnd);fixture::ready=false;fixture::flags.scenario[64]=1;events::fire(events::Kind::MapChanging);fixture::map=584;events::fire(events::Kind::MapChanged);
        expect(fixture::step&&fixture::step()&&g_active,"Tunnel Armor completion cannot speak during its native transfer/dialogue");
        if(other_scenarios)for(int mid:{14,588,14,3,18,19}){events::fire(events::Kind::MapChanging);fixture::map=mid;events::fire(events::Kind::MapChanged);expect(fixture::step&&fixture::step()&&g_active,"all-scenarios continuation still waits for actual field readiness");}
        fixture::ready=true;expect(!fixture::step()&&!g_active,"native64 completes on ready scenario selection or the full later continuation");fixture::finish_scenario=-1;
    }
    stop(false);fixture::map=249;fixture::finish_scenario=336;fixture::flags.scenario[336]=0;next(true);fixture::flush();
    fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=2;events::fire(events::Kind::MapChanged);
    expect(fixture::step&&fixture::step()&&g_active,"Gerad surfacing stays active through the hidden-player world animation");
    events::fire(events::Kind::MapChanging);fixture::map=249;events::fire(events::Kind::MapChanged);fixture::flags.scenario[336]=1;
    expect(fixture::step&&fixture::step()&&g_active,"final336 cannot speak until the castle is actually ready");
    fixture::ready=true;expect(!fixture::step()&&!g_active,"native ready336 finishes the complete Gerad journey after249-world2-249");fixture::finish_scenario=-1;
    stop(false);fixture::map=143;fixture::floating_control=28;fixture::finish_exit=396;next(true);fixture::flush();fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=396;events::fire(events::Kind::MapChanged);
    expect(fixture::step&&fixture::step()&&g_active,"Vector final factory arrival waits for native field readiness");fixture::ready=true;expect(!fixture::step()&&!g_active,"ready actual396 completes Vector entry");fixture::finish_exit=0;
    stop(false);fixture::map=162;fixture::floating_control=18;fixture::finish_scenario=217;fixture::flags.scenario[217]=0;next(true);fixture::flush();fixture::ready=false;fixture::flags.scenario[217]=1;events::fire(events::Kind::MapChanged);
    expect(fixture::step&&fixture::step()&&g_active,"camp217 cannot announce completion during its native scene");fixture::ready=true;expect(!fixture::step()&&!g_active,"camp completes only at ready native217");fixture::finish_scenario=-1;
    for(bool death:{false,true}){
        stop(false);fixture::map=death?191:597;fixture::finish_scenario=306;fixture::flags.scenario[306]=0;next(true);fixture::flush();
        const std::vector<int> stages=death?std::vector<int>{2,193,192,190,191,597,192,2}:std::vector<int>{192,2};
        for(int mid:stages){fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=mid;events::fire(events::Kind::MapChanged);expect(fixture::step&&fixture::step()&&g_active,"neither island outcome completes at a native intermediate scene");}
        fixture::flags.scenario[306]=1;events::fire(events::Kind::MapChanging);fixture::map=2;events::fire(events::Kind::MapChanged);expect(fixture::step&&fixture::step()&&g_active,"full raft306 still waits for field readiness after its final world scene");fixture::ready=true;expect(!fixture::step()&&!g_active,"both island outcomes complete only at the full ready raft arrival");fixture::finish_scenario=-1;
    }
    stop(false);fixture::map=466;next(true);fixture::flush();
    auto held_type_route=std::move(fixture::route);fixture::other_guidance=false;
    actions::run("GameModule.PuzzleGuidePolicyChanged");const auto quiet=fixture::speech.size();
    if(held_type_route){field::PathInfo path;path.success=true;path.guidance_index=0;path.guidance=L"Forbidden old answer";held_type_route(path);}
    expect(!g_active&&fixture::speech.size()==quiet&&g_repeat_instruction.empty(),"type change quietly retires held route and cached native-choice answer");
    fixture::nav_guidance=false;const int old_reads=fixture::state_reads,old_requests=fixture::requests;next(true);
    expect(fixture::state_reads==old_reads&&fixture::requests==old_requests&&fixture::speech.back()==L"Puzzle guidance is off","both types off avoids all snapshot and routing work");
    fixture::nav_guidance=fixture::other_guidance=true;
    stop(false);fixture::map=300;fixture::ready=true;fixture::finish_scenario=456;fixture::flags.scenario[455]=1;fixture::flags.scenario[456]=0;next(true);fixture::flush();
    expect(g_active,"Cyan waking455 does not complete the final pickup step");
    fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=304;events::fire(events::Kind::MapChanged);fixture::flags.scenario[456]=1;
    expect(fixture::step&&fixture::step()&&g_active,"actual final456 cannot announce completion while its acquisition scene is active");
    fixture::ready=true;expect(!fixture::step()&&!g_active,"only full ready native456 completes the post-waking reward step");fixture::finish_scenario=-1;
    stop(false);fixture::map=496;fixture::ready=true;fixture::finish_scenario=398;fixture::flags.scenario[397]=1;fixture::flags.scenario[398]=0;next(true);fixture::flush();
    expect(g_active,"Yeti battle397 does not complete actual recruitment398");fixture::ready=false;fixture::flags.scenario[398]=1;events::fire(events::Kind::MapChanged);
    expect(fixture::step&&fixture::step()&&g_active,"Yeti naming/joining398 must wait for native field readiness");fixture::ready=true;expect(!fixture::step()&&!g_active,"full ready398 completes the entire Yeti guide");fixture::finish_scenario=-1;
    stop(false);fixture::map=434;fixture::ready=true;fixture::finish_scenario=312;fixture::flags.scenario[311]=1;fixture::flags.scenario[312]=0;next(true);fixture::flush();
    expect(g_active,"Tzen child311 cannot finish before leaving the house");fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=201;events::fire(events::Kind::MapChanged);
    expect(fixture::step&&fixture::step()&&g_active,"return to Tzen alone is not final312 completion");fixture::flags.scenario[312]=1;
    expect(fixture::step&&fixture::step()&&g_active,"early312 cannot finish during Sabin joining and the rescue scene");fixture::ready=true;expect(!fixture::step()&&!g_active,"only full ready312 finishes Tzen rescue");fixture::finish_scenario=-1;
    stop(false);fixture::map=274;fixture::ready=true;fixture::finish_scenario=359;fixture::flags.scenario[358]=1;fixture::flags.scenario[359]=0;next(true);fixture::flush();
    fixture::ready=false;events::fire(events::Kind::MapChanging);fixture::map=2;events::fire(events::Kind::MapChanged);
    expect(fixture::step&&fixture::step()&&g_active,"Lola pigeon flight is an intermediate native scene,not delivery completion");events::fire(events::Kind::MapChanging);fixture::map=274;events::fire(events::Kind::MapChanged);fixture::flags.scenario[359]=1;
    expect(fixture::step&&fixture::step()&&g_active,"letter return359 still waits for full native readiness");fixture::ready=true;expect(!fixture::step()&&!g_active,"full ready native359 completes the letter delivery");fixture::finish_scenario=-1;
    stop(false);fixture::map=474;fixture::ready=true;fixture::finish_scenario=367;fixture::flags.scenario[365]=0;fixture::flags.scenario[366]=fixture::flags.scenario[367]=0;next(true);fixture::flush();
    fixture::ready=false;fixture::flags.scenario[365]=1;events::fire(events::Kind::MapChanged);expect(fixture::step&&fixture::step()&&g_active,"Cyan joining365 cannot complete the key/chest journey");fixture::flags.scenario[366]=1;expect(fixture::step&&fixture::step()&&g_active,"key366 cannot complete an unopened chest");fixture::flags.scenario[367]=1;
    expect(fixture::step&&fixture::step()&&g_active,"chest367 cannot finish during its native acquisition scene");fixture::ready=true;expect(!fixture::step()&&!g_active,"full ready native367 completes the real chest objective");fixture::finish_scenario=-1;
    for(int rescue:{372,373}){
        stop(false);fixture::map=492;fixture::ready=true;fixture::finish_scenario=rescue;fixture::flags.scenario[size_t(rescue)]=0;fixture::flags.scenario[926]=1;next(true);fixture::flush();
        fixture::ready=false;events::fire(events::Kind::BattleEntering);events::fire(events::Kind::BattleEnd);
        for(int mid:{491,2,311,rescue==372?180:310,311}){events::fire(events::Kind::MapChanging);fixture::map=mid;events::fire(events::Kind::MapChanged);expect(fixture::step&&fixture::step()&&g_active,"cave battle/voyage/dream stages and visit926 cannot finish the rescue");}
        fixture::flags.scenario[size_t(rescue)]=1;expect(fixture::step&&fixture::step()&&g_active,"cave rescue receipt still waits for the complete native return");fixture::ready=true;expect(!fixture::step()&&!g_active,"full ready branch-specific inn return completes the entire cave journey");fixture::finish_scenario=-1;
    }
    stop(false);fixture::map=343;fixture::ready=true;fixture::finish_scenario=41;fixture::flags.scenario[41]=0;next(true);fixture::flush();fixture::ready=false;events::fire(events::Kind::BattleEntering);events::fire(events::Kind::BattleEnd);
    expect(fixture::step&&fixture::step()&&g_active,"Vargas battle ending is not full recruitment completion");fixture::flags.scenario[41]=1;expect(fixture::step&&fixture::step()&&g_active,"41 still waits for the final native Sabin membership call");fixture::ready=true;expect(!fixture::step()&&!g_active,"full ready41 finishes the mountain and mandatory Blitz episode");fixture::finish_scenario=-1;
    stop(false);fixture::map=293;fixture::ready=true;fixture::finish_scenario=434;fixture::flags.scenario[434]=0;next(true);fixture::flush();fixture::ready=false;events::fire(events::Kind::BattleEntering);fixture::map=292;events::fire(events::Kind::MapChanged);events::fire(events::Kind::BattleEnd);
    expect(fixture::step&&fixture::step()&&g_active,"late Opera's boss arena is not final completion");events::fire(events::Kind::MapChanging);fixture::map=290;events::fire(events::Kind::MapChanged);fixture::flags.scenario[434]=1;expect(fixture::step&&fixture::step()&&g_active,"early434 cannot finish before the full outside scene and autosave");fixture::ready=true;expect(!fixture::step()&&!g_active,"only full ready outside434 finishes the later Opera sequence");fixture::finish_scenario=-1;
    // Actual Lethe choices use the existing native open scalar. No worker,
    // actor/cursor/party snapshot or new hook is required during the movie.
    stop(false);queries::define("BattleStateHelper.IsInBattle",[]{return fixture::battle;});fixture::map=345;fixture::choice=true;fixture::ready=false;fixture::flags.scenario[50]=0;
    const auto lethe_reads=fixture::state_reads,lethe_routes=fixture::requests,lethe_scalars=fixture::scalar_reads;
    expect(queries::is("GameModule.HasPuzzleGuide")&&queries::is("GameModule.PuzzleGuideReadyDuringEvent")&&fixture::scalar_reads==lethe_scalars,"actual initial raft choice advertises direct help with no completion read");
    const auto lethe_lines=fixture::speech.size();next(true);next(true);
    expect(fixture::speech.size()==lethe_lines+2&&fixture::speech.back()==L"Choose the first option."&&fixture::scalar_reads==lethe_scalars+2,"Yes/Straight repeats each explicit request and reads only one completion bit per request");
    expect(fixture::state_reads==lethe_reads&&fixture::requests==lethe_routes&&fixture::jobs.empty()&&g_active&&g_finish_on_scenario==50,"native raft choice performs no snapshot/worker/route and arms only full50 completion");
    fixture::choice=false;events::fire(events::Kind::MapChanging);fixture::map=347;fixture::choice=true;next(true);
    expect(fixture::speech.back()==L"Choose the second option.","second river chooses Left instead of the Up loop");
    fixture::nav_guidance=false;fixture::other_guidance=true;const auto disabled_reads=fixture::scalar_reads;next(true);
    expect(!g_active&&fixture::scalar_reads==disabled_reads&&fixture::speech.back()==L"Navigation puzzle guidance is off","disabled navigation cannot expose the raft answer or read completion");
    fixture::nav_guidance=true;fixture::other_guidance=false;next(true);expect(g_active&&fixture::speech.back()==L"Choose the second option.","non-navigation setting remains independent of raft navigation");fixture::other_guidance=true;
    fixture::battle=true;expect(!lethe_choice_option(),"battle context cannot inherit a river choice");fixture::battle=false;
    fixture::choice=false;const auto waiting=fixture::speech.size();const auto waiting_reads=fixture::scalar_reads;next(true);
    expect(fixture::speech.size()==waiting&&fixture::scalar_reads==waiting_reads&&fixture::jobs.empty(),"blocked river motion without an actual choice produces no stale answer or work");
    events::fire(events::Kind::MapChanging);fixture::map=584;events::fire(events::Kind::MapChanged);expect(fixture::step&&fixture::step()&&g_active,"scenario arrival alone cannot finish the raft before50");fixture::flags.scenario[50]=1;
    expect(fixture::step&&fixture::step()&&g_active,"even full50 waits for the remaining native message/autosave");fixture::ready=true;expect(!fixture::step()&&!g_active&&fixture::speech.back()==L"Puzzle guide finished","only ready50 at scenario selection finishes the complete raft journey");
    fixture::map=347;fixture::choice=true;fixture::ready=false;next(true);expect(!g_active&&fixture::speech.back()==L"Choose the second option.","completed replay offers its current choice without arming a premature finish");fixture::flags.scenario[50].reset();next(true);expect(!g_active,"unknown completion cannot arm a full-journey completion");
    fixture::map=346;const auto tutorial_lines=fixture::speech.size();const auto tutorial_reads=fixture::scalar_reads;next(true);expect(fixture::speech.size()==tutorial_lines&&fixture::scalar_reads==tutorial_reads&&!queries::is("GameModule.PuzzleGuideReadyDuringEvent"),"save-point tutorial choices on rest floors are not intercepted by raft help");fixture::choice=false;fixture::ready=true;
    expect(fixture::wrong_owner==0,"worker owns no native API, pointer, speech or map-name operation");
    std::printf("FF6 story runtime: %d failures\n",failures);return failures?1:0;
}
