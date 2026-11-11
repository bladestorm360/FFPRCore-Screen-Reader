// check-sources: text.cpp
#include <cstdio>
#include <cstdlib>
#include <map>
#include <chrono>
#include "../src/games/ff3/chocobo_circuit.cpp"
namespace fixture {
bool ready=true,blocked=false,battle=false,menu=false; int map=1,vehicle=12,reads=0,flags=0,requests=0,clears=0,verbosity=-1;
std::optional<int> flag=0; field::baked_route::Cell cell{1,20,93};
std::vector<std::wstring> spoken; std::vector<std::function<void(field::PathInfo)>> callbacks;
std::map<events::Kind,events::Handler> handlers; std::wstring scene=L"Field";
}
namespace ff3::puzzles {
bool supported(int){return false;}bool active(){return false;}bool repeatable(){return false;}
void next(){}void move(){}void stop(bool){}
}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace hooks {void note_fault(const char*){std::abort();}}
namespace menu_state {bool any_active(){return fixture::menu;}}
namespace actions {void define(const char*,Fn){} bool run(const char*){return false;}}
namespace queries {bool is(const char*n){std::string s(n);return s=="GameStatePatches.IsFieldNavigationReady"?fixture::ready:s=="GameStatePatches.IsInputTransitionBlocked"?fixture::blocked:s=="BattleStateHelper.IsInBattle"?fixture::battle:false;}void define(const char*,std::function<bool()>){} }
namespace events {void on(Kind k,Handler f){fixture::handlers[k]=std::move(f);} }
namespace ffpr {void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}void say_sequence(std::vector<std::wstring> a,bool){for(auto& s:a)fixture::spoken.push_back(s);}}
namespace modtext {std::wstring T(std::wstring_view s){return std::wstring(s);}std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring> args){auto out=std::wstring(s);int i=0;for(auto& a:args){auto t=L"{"+std::to_wstring(i++)+L"}";auto pos=out.find(t);if(pos!=out.npos)out.replace(pos,t.size(),a);}return out;}}
namespace settings {bool get_bool(const char*,bool v){return v;}int get_int(const char* key,int v){return std::string(key)==ui::prefs::kPuzzleGuideVerbosity&&fixture::verbosity>=0?fixture::verbosity:v;}}
namespace field {int current_map_id(){return fixture::map;}void* map_handle(){return nullptr;}void* player(){return nullptr;}VehicleState current_vehicle(){++fixture::reads;return {true,fixture::vehicle,9,{}};}const std::wstring& scene_name(){return fixture::scene;}}
namespace field::map_state {std::optional<int> scenario_flag(int n){++fixture::flags;if(n!=192)std::abort();return fixture::flag;}}
namespace field::baked_nav {bool player_cell(void*,void*,baked_route::Cell& c){c=fixture::cell;return true;}void clear_preview_destination(const MapGoal&){++fixture::clears;}void request_route(MapGoal,TargetKind k,std::wstring,std::function<void(PathInfo)> f,bool keep){if(k!=TargetKind::Spot||keep)std::abort();++fixture::requests;fixture::callbacks.push_back(std::move(f));}}
int main(){using namespace ff3::chocobo_circuit;int failures=0;auto expect=[&](bool b,const char*s){if(!b){++failures;std::printf("FAIL %s\n",s);}};
 install();next();expect(active&&fixture::requests==1&&fixture::flags==1,"start requests first route and one flag");
 auto stale=fixture::callbacks.back();field::PathInfo result;result.success=true;result.description=L"East";
 expect(fixture::spoken.empty(),"default Minimal omits circuit activation");stale(result);
 expect(fixture::spoken.back()==L"Go to the circuit starting point. East","Minimal circuit retains current instruction and directions without step caption");
 auto silent=fixture::spoken.size();fixture::menu=true;stale(result);fixture::menu=false;fixture::blocked=true;stale(result);fixture::blocked=false;fixture::battle=true;stale(result);fixture::battle=false;expect(fixture::spoken.size()==silent,"menu transition and battle callbacks stay silent");
 stop();auto before=fixture::spoken.size();stale(result);expect(fixture::spoken.size()==before,"stopped callback silent");
 next();fixture::blocked=true;const int stop_reads=fixture::reads,stop_flags=fixture::flags;stop();
 expect(!active&&fixture::reads==stop_reads&&fixture::flags==stop_flags,"blocked native scene can retire the owned circuit without engine/flag reads");fixture::blocked=false;
 fixture::flag=1;next();expect(!active&&fixture::spoken.back()==L"Chocobo circuit recorded","already recorded does not start lap");
 fixture::flag=0;fixture::cell=points[0];next();expect(active&&step==1,"start cell advances immediately");
 auto start=std::chrono::steady_clock::now();const int reads=fixture::reads,requests=fixture::requests;
 for(int i=0;i<1000;++i)move();auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
 expect(fixture::reads==reads&&fixture::requests==requests,"stationary inputs skip vehicle and jobs");
 fixture::cell=points[1];next();expect(step==2,"repeat advances a reached waypoint without a movement callback");
 for(int i=2;i<14;++i){fixture::cell=points[i];move();}expect(!active&&fixture::spoken.back()==L"Circuit route finished, but the game has not recorded it","arrival does not assert completion");
 fixture::flag={};fixture::cell=points[0];next();for(int i=1;i<14;++i){fixture::cell=points[i];move();}expect(fixture::spoken.back()==L"Circuit completion could not be checked","unknown flag reported");
 fixture::flag=0;next();auto old=fixture::callbacks.back();fixture::handlers[events::Kind::MapChanging]();before=fixture::spoken.size();old(result);expect(!active&&fixture::spoken.size()==before,"map change cancels callback");
 next();fixture::vehicle=1;fixture::cell={1,22,93};move();expect(!active&&fixture::spoken.back()==L"Ride a chocobo on this world map to use this guide","dismount cancels");
 fixture::vehicle=12;fixture::flag=0;fixture::cell={1,20,93};next();
 const int repeat_start=fixture::requests;start=std::chrono::steady_clock::now();
 for(int i=0;i<1000;++i)next();stop();
 const auto repeat_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
 expect(fixture::requests-repeat_start==1000,"explicit repeats each request current directions");
 before=fixture::spoken.size();for(auto& callback:fixture::callbacks)callback(result);
 expect(fixture::spoken.size()==before,"all queued repeat callbacks remain silent after cancellation");
 std::printf("1000 rapid repeats plus cancel: %.3f ms controller fixture, 1000 jobs requested; fake native owners, no worker scheduling measured\n",repeat_ms);
 fixture::flag=0;fixture::cell=points[0];next();fixture::flag=1;
 for(int i=1;i<14;++i){fixture::cell=points[i];move();}
 expect(fixture::spoken.back()==L"Chocobo circuit recorded","final native true confirms recording");
 fixture::flag=0;next();fixture::handlers[events::Kind::BattleEntering]();expect(!active,"battle cancels state");
 next();fixture::handlers[events::Kind::TitleScreen]();expect(!active,"title cancels state");
 fixture::verbosity=1;fixture::cell={1,20,93};fixture::spoken.clear();next();fixture::callbacks.back()(result);
 expect(fixture::spoken.size()==2&&fixture::spoken.front()==L"Chocobo circuit guide"&&fixture::spoken.back().find(L"Step 1 of 14:")==0,"Verbose circuit restores activation and numbered step");stop();
 std::printf("Circuit controller: 1000 stationary inputs %.3f ms; %d failures\n",ms,failures);return failures?1:0;}
