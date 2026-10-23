// check-sources: text.cpp
// Real guide native-owner controller and epochs, held worker/route completions.
#include "../src/games/ff6/story_guidance.cpp"
#include "../src/games/ff6/defense_watch.cpp"
#include <cstdio>
#include <thread>
#include <chrono>
namespace fixture {
DWORD owner=GetCurrentThreadId();int wrong_owner=0,party_events=0,retired=0,requests=0;
bool ready=true;int map=498;std::wstring scene=L"Field";
bool held_plate=false;int predicted=0;
bool released_refresh=false;int first_target=-1;
struct Actor {int id=0;unity::Vec3 position;};
int updates=0;
void update(void*,const void*){++updates;}
field::map_state::Flags flags;
std::vector<std::function<void()>> jobs,posts;
frame::Step step;
std::map<events::Kind,std::vector<events::Handler>> handlers;
std::map<std::string,actions::Fn> actions;
std::vector<std::wstring> speech;
std::function<void(field::PathInfo)> route;
std::map<std::string,int> offsets;
void native(){if(GetCurrentThreadId()!=owner)++wrong_owner;}
void flush(){auto q=std::move(jobs);jobs.clear();std::thread worker([&]{for(auto& fn:q)fn();});worker.join();auto p=std::move(posts);posts.clear();for(auto& fn:p)fn();}
void original_party(void*,int32_t slot,const void*){flags.global[4]=slot;}
}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
void* Method::ptr(){fixture::native();return nullptr;}
il2cpp::Method* Method::get(){fixture::native();return nullptr;}
int32_t Field::get(){fixture::native();auto at=fixture::offsets.find(name);return at==fixture::offsets.end()?-1:at->second;}
bool Class::is(void* p){fixture::native();return p!=nullptr;}
void call_failed_note(const char*){}
std::wstring rd_str(void*,Field&){fixture::native();return {};}
std::wstring rd_str_bounded(void*,Field&,size_t){fixture::native();return {};}
}
namespace hooks {
void note_fault(const char*){std::abort();}
bool attach_raw(ref::Method&,void*,void** original){*original=reinterpret_cast<void*>(&fixture::original_party);return true;}
}
namespace actions {
void define(const char*n,Fn f){fixture::actions[n]=std::move(f);}
bool run(const char*n){fixture::native();if(auto at=fixture::actions.find(n);at!=fixture::actions.end()){at->second();return true;}return false;}
}
namespace queries {
void define(const char*,std::function<bool()>){ }
bool is(const char*n){fixture::native();return std::string(n)=="GameStatePatches.IsFieldNavigationReady"?fixture::ready:false;}
}
namespace menu_state {bool any_active(){fixture::native();return false;} }
namespace events {
void on(Kind kind,Handler h){fixture::handlers[kind].push_back(std::move(h));}
void fire(Kind kind){fixture::native();if(kind==Kind::PartyChanging)++fixture::party_events;for(auto& h:fixture::handlers[kind])h();}
}
namespace frame {
double now(){return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}
int every_frame(Step s,const char*,int){fixture::native();fixture::step=std::move(s);return 1;}
void cancel_label(const char*){fixture::native();fixture::step={};}
void post(Task t){fixture::posts.push_back(std::move(t));}
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
namespace settings {bool get_bool(const char*,bool v){return v;} }
namespace log_ {void write(const char*,...){} }
namespace unity {int array_length(void*){fixture::native();return 0;} void* array_at(void*,int){fixture::native();return nullptr;} void* transform(void* p){fixture::native();return p;} Vec3 local_position(void* p){fixture::native();return static_cast<fixture::Actor*>(p)->position;} }
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
namespace field::map_state {Flags snapshot(){fixture::native();return fixture::flags;} }
namespace field::scanner {void invalidate_map(){fixture::native();} }
namespace field::baked_nav {
struct ExitSnapshot {std::array<std::optional<baked_route::Cell>,64> cells;};
std::shared_ptr<ExitSnapshot> begin_exit_capture(bool,bool,bool){fixture::native();auto out=std::make_shared<ExitSnapshot>();out->cells=ffpr::ff6::defense_watch::cells;return out;}
void capture_exit_obstacle(ExitSnapshot&,void*){fixture::native();}
void finish_exit_capture(ExitSnapshot&){fixture::native();}
DefenseAdvice defense_advice(ExitSnapshot& snapshot,const DefenseContext& context,std::function<bool()>){DefenseAdvice out;for(const auto& goal:context.enemies)if(snapshot.cells[static_cast<size_t>(goal.entity_id)]){out.intercept=goal;out.intercept->cell=*snapshot.cells[static_cast<size_t>(goal.entity_id)];out.warning=L"Terra in danger. Party 2 closest.";out.closest_party=1;break;}return out;}
bool cell_at(void*,unity::Vec3 at,baked_route::Cell& out){fixture::native();out={1,static_cast<int>(at.x),static_cast<int>(at.y)};return true;}
bool prepared(){return true;}void invalidate(){fixture::native();}void clear_pending(bool){fixture::native();}
void clear_preview_destination(const MapGoal&){fixture::native();++fixture::retired;}
bool player_cell(void*,void*,baked_route::Cell& out){fixture::native();out={1,2,3};return true;}
void request_guidance(std::vector<GuideTarget> candidates,std::function<void(PathInfo)> fn,const map_state::Flags* release,const DefenseContext*){fixture::native();fixture::first_target=candidates.front().goal.entity_id;if(release&&release->area[1]==0)++fixture::predicted;++fixture::requests;fixture::route=std::move(fn);}
}
namespace ffpr::fieldnav::audio_loops {void stop_all(){fixture::native();}void start_if_enabled(){fixture::native();} }
namespace ffpr::ff6::story {
bool supported_map(int map){return map==498||map==318;}
Plan plan(const Snapshot& input){Plan out;out.map=input.map;out.supported=true;out.introduction=L"Do not repeat this introduction";field::baked_nav::GuideTarget target;target.goal.map_id=498;target.goal.cell={1,4,5};target.goal.entity_id=42;target.instruction=L"Guide";out.targets.push_back(target);if(input.map==318){out.defense.emplace();for(int id:{18,19}){field::baked_nav::MapGoal goal;goal.map_id=318;goal.entity_id=id;out.defense->enemies.push_back(goal);}}if(fixture::released_refresh){target.goal.entity_id=108;out.targets.insert(out.targets.begin(),target);}if(fixture::held_plate){out.anticipated_release=input.flags;out.anticipated_release->area[1]=0;out.hold_note=L"Stay on this switch";}return out;}
}
int main(){
    using namespace ffpr::ff6::story;int failures=0;
    auto expect=[&](bool v,const char* name){if(!v){++failures;std::printf("FAIL %s\n",name);}};
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
    expect(fixture::wrong_owner==0,"worker owns no native API, pointer, speech or map-name operation");
    std::printf("FF6 story runtime: %d failures\n",failures);return failures?1:0;
}
