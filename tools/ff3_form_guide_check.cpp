// check-sources: json.cpp text.cpp
// Actual FFIII guide and circuit registration, with native phase boundaries fake.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include "../src/games/ff3/puzzle_model.cpp"
#include "../src/games/ff3/puzzle_guide.cpp"
#include "../src/games/ff3/chocobo_circuit.cpp"
namespace fixture {
bool ready=true,blocked=false,battle=false,menu=false,event_item=false;
int verbosity=-1;
bool nav_guidance=true;
int map=169,vehicle=1,flag_reads=0,form_reads=0,vehicle_reads=0,requests=0,clears=0;
std::optional<int> mini=2,frog=0;
std::map<int,std::optional<int>> flags;
std::map<std::string,actions::Fn> commands;
std::map<std::string,std::function<bool()>> registered_queries;
std::map<events::Kind,std::vector<events::Handler>> event_handlers;
std::map<std::string,frame::Step> frames;
std::vector<std::wstring> spoken;
std::vector<std::function<void(field::PathInfo)>> callbacks;
field::baked_nav::MapGoal goal;
field::baked_route::Cell cell{1,20,93};
json::Value data;std::wstring scene=L"Field";
void emit_fixture_event(events::Kind event){for(const auto& fn:event_handlers[event])fn();}
void flush(){for(int i=0;i<4&&!frames.empty();++i){auto pending=frames;frames.clear();for(auto& [label,fn]:pending)if(fn())frames[label]=fn;}}
void command(const char* name){commands.at(name)();}
}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace hooks {void note_fault(const char*){std::abort();}}
namespace menu_state {bool any_active(){return fixture::menu||fixture::event_item;}}
namespace actions {void define(const char*n,Fn fn){fixture::commands[n]=std::move(fn);}bool run(const char*n){auto i=fixture::commands.find(n);if(i==fixture::commands.end())return false;i->second();return true;}}
namespace queries {
void define(const char*n,std::function<bool()> fn){fixture::registered_queries[n]=std::move(fn);}
bool is(const char*n){const std::string name(n);auto i=fixture::registered_queries.find(name);if(i!=fixture::registered_queries.end())return i->second();return name=="GameStatePatches.IsFieldNavigationReady"?fixture::ready:name=="GameStatePatches.IsInputTransitionBlocked"?fixture::blocked:name=="BattleStateHelper.IsInBattle"?fixture::battle:name=="EventItemSelectState.IsActive"?fixture::event_item:false;}
}
namespace events {void on(Kind k,Handler fn){fixture::event_handlers[k].push_back(std::move(fn));}}
namespace frame {int every_frame(Step fn,const char*n,int){fixture::frames[n]=std::move(fn);return 1;}void cancel_label(const char*n){fixture::frames.erase(n);}}
namespace data {bool game_data_ready(){return true;}const json::Value& game_json(const char*){return fixture::data;}}
namespace ffpr {void say(std::wstring_view text,bool){fixture::spoken.emplace_back(text);}void say_sequence(std::vector<std::wstring> lines,bool){for(auto& line:lines)fixture::spoken.push_back(std::move(line));}}
namespace modtext {std::wstring T(std::wstring_view s){return std::wstring(s);}std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring> args){auto result=std::wstring(s);int i=0;for(const auto& arg:args){const auto token=L"{"+std::to_wstring(i++)+L"}";auto at=result.find(token);if(at!=result.npos)result.replace(at,token.size(),arg);}return result;}}
namespace settings {bool get_bool(const char* key,bool value){return std::string(key)==ui::prefs::kNavigationPuzzleGuidance?fixture::nav_guidance:value;}int get_int(const char* key,int value){return std::string(key)==ui::prefs::kPuzzleGuideVerbosity&&fixture::verbosity>=0?fixture::verbosity:value;}}
namespace field {
int current_map_id(){return fixture::map;}void* map_handle(){return nullptr;}void* player(){return nullptr;}
VehicleState current_vehicle(){++fixture::vehicle_reads;return {true,fixture::vehicle,fixture::vehicle,{}};}
const std::wstring& scene_name(){return fixture::scene;}
}
namespace field::map_state {
std::optional<int> scenario_flag(int n){++fixture::flag_reads;auto i=fixture::flags.find(n);return i==fixture::flags.end()?std::optional<int>(0):i->second;}
std::optional<int> party_condition_state(int n){++fixture::form_reads;return n==401?fixture::mini:fixture::frog;}
}
namespace field::baked_nav {
bool player_cell(void*,void*,baked_route::Cell& out){out=fixture::cell;return true;}
void clear_preview_destination(const MapGoal&){++fixture::clears;}
void request_route(MapGoal goal,TargetKind,std::wstring,std::function<void(PathInfo)> callback,bool keep){if(keep)std::abort();++fixture::requests;fixture::goal=std::move(goal);fixture::callbacks.push_back(std::move(callback));}
}
int main() {
    using namespace fixture;using ff3::puzzles::Action;
    int failures=0,cases=0;
    const auto expect=[&](bool ok,const char* text){++cases;if(!ok){++failures;std::printf("FAIL %s\n",text);}};
    std::ifstream file("data/ff3/puzzles.json");const std::string bytes((std::istreambuf_iterator<char>(file)),{});
    if(!json::parse(bytes,fixture::data))return 2;
    ff3::puzzles::install();ff3::chocobo_circuit::install();
    command("GameModule.PuzzleGuide");
    expect(ff3::puzzles::active()&&requests==0&&form_reads==1&&spoken.back().find(L"Cast FF3 spell Mini")!=std::wstring::npos,"mixed Mini starts at owned transformation instruction");
    expect(spoken.back().find(L"only on members who do not yet have")!=std::wstring::npos,"mixed-party instructions target only missing forms instead of toggling everyone");
    expect(spoken.back().find(L"Step ")==std::wstring::npos,"unset FFIII verbosity gives only the current instruction");
    int reads=form_reads,bits=flag_reads;
    for(int i=0;i<1000;++i)command("GameModule.PuzzleGuideMove");
    expect(form_reads==reads&&flag_reads==bits,"waiting for transformation performs zero movement-time native reads");
    menu=blocked=true;const auto speech=spoken.size();command("GameModule.PuzzleGuide");
    expect(form_reads==reads&&flag_reads==bits&&requests==0&&spoken.size()==speech+1,"native-menu repeat uses owned instruction only");
    verbosity=1;command("GameModule.PuzzleGuide");
    expect(spoken.back().find(L"Step ")==0&&form_reads==reads&&flag_reads==bits&&requests==0,"Verbose native-menu repeat adds its real step caption without a game read");
    verbosity=0;command("GameModule.PuzzleGuide");
    expect(spoken.back().find(L"Step ")==std::wstring::npos&&spoken.back().find(L"only on members who do not yet have")!=std::wstring::npos,"changing back to Minimal also updates the owned native-menu repeat");
    command("GameModule.StopPuzzleGuide");expect(!ff3::puzzles::active(),"cancel works inside native menu");menu=blocked=false;
    mini=1;command("GameModule.PuzzleGuide");
    expect(goal.map_id==169&&goal.entity_id==8,"all Mini targets actual small opening");
    auto old=callbacks.back();emit_fixture_event(events::Kind::MapChanging);map=170;flags[31]=1;emit_fixture_event(events::Kind::MapChanged);flush();
    expect(goal.map_id==173&&goal.entity_id==6,"native entrance arrival advances to actual guardian");
    map=173;flags[32]=1;command("GameModule.PuzzleGuide");
    expect(goal.map_id==173&&goal.entity_id==5,"defeated-before-pickup resume targets exact interactive Eye5");
    field::PathInfo path;path.success=true;const auto before=spoken.size();old(path);
    expect(spoken.size()==before,"old-floor route publication is rejected");
    reads=form_reads;emit_fixture_event(events::Kind::BattleEntering);flags[33]=1;emit_fixture_event(events::Kind::BattleEnd);flush();
    expect(form_reads==reads&&goal.map_id==169&&goal.entity_id==7,"guardian victory returns Eye without rereading form or targeting off-map Eye prop");
    event_item=blocked=true;reads=form_reads;bits=flag_reads;const int pending_requests=requests;command("GameModule.PuzzleGuide");
    expect(queries::is("GameModule.PuzzleGuideDuringEvent")&&form_reads==reads&&flag_reads==bits&&requests==pending_requests,"exact FFIII event-item menu repeats owned Eye instruction with no route/native state reads");
    old=callbacks.back();command("GameModule.StopPuzzleGuide");const auto canceled=spoken.size();old(path);
    expect(!ff3::puzzles::active()&&spoken.size()==canceled,"cancel retires pending worker publication in item menu");event_item=blocked=false;
    flags[34]=1;reads=form_reads;command("GameModule.PuzzleGuide");
    expect(!ff3::puzzles::active()&&form_reads==reads&&spoken.back()==L"Puzzle guide finished","Nepto end receipt skips all earlier form observations");
    flags.clear();map=174;frog=2;command("GameModule.PuzzleGuide");
    expect(ff3::puzzles::last==Action::CastFrog,"mixed Frog at basement requires all frogs");
    frog=1;command("GameModule.PuzzleGuide");expect(goal.map_id==174&&goal.entity_id==10,"all Frog targets waterway");
    reads=form_reads;emit_fixture_event(events::Kind::MapChanging);flags[39]=1;map=175;emit_fixture_event(events::Kind::MapChanged);flush();
    expect(form_reads==reads&&ff3::puzzles::last==Action::RemoveFrog,"entering prompts return to normal without movement-time form scan");
    emit_fixture_event(events::Kind::MenuClosed);flush();frog=0;command("GameModule.PuzzleGuide");
    expect(goal.map_id==175&&goal.entity_id==7,"normal-party repeat starts actual first clue");
    for(const auto step:std::vector<std::pair<int,int>>{{40,9},{41,10},{42,7},{225,4}}) {
        flags[step.first]=1;command("GameModule.PuzzleGuideMove");flush();
        expect(goal.entity_id==step.second,"each exact native clue/control receipt advances its frontier");
    }
    reads=form_reads;emit_fixture_event(events::Kind::BattleEntering);flags[43]=1;emit_fixture_event(events::Kind::BattleEnd);flush();
    expect(form_reads==reads&&goal.map_id==184&&goal.entity_id==9,"victory moves to engine with no obsolete form requirement");
    flags[44]=1;emit_fixture_event(events::Kind::MapChanging);map=1;emit_fixture_event(events::Kind::MapChanged);flush();
    expect(!ff3::puzzles::active()&&spoken.back()==L"Puzzle guide finished","native engine receipt survives world-return map change");
    flags.clear();flags[39]=flags[225]=1;map=174;frog=0;command("GameModule.PuzzleGuide");
    expect(ff3::puzzles::last==Action::CastFrog,"basement revisit ignores historical entered/wall flags for current Frog requirement");
    frog=1;command("GameModule.PuzzleGuide");reads=form_reads;bits=flag_reads;
    for(int i=0;i<1000;++i)command("GameModule.PuzzleGuideMove");
    expect(form_reads==reads&&flag_reads==bits,"old entrance receipt cannot produce repeated automatic refreshes while awaiting transfer");
    map=175;emit_fixture_event(events::Kind::MapChanged);flush();frog=0;command("GameModule.PuzzleGuide");
    expect(goal.map_id==179&&goal.entity_id==10,"opened-wall resume still guides mandatory fifth-floor boss-spawn event");
    flags[42]=1;command("GameModule.PuzzleGuideMove");flush();
    expect(goal.map_id==184&&goal.entity_id==4,"native42 admits the guardian while earlier optional clues stay skipped");
    command("GameModule.StopPuzzleGuide");map=1;vehicle=12;command("GameModule.PuzzleGuide");
    expect(!ff3::puzzles::active()&&ff3::chocobo_circuit::active&&goal.map_id==1,"single registration preserves native chocobo circuit ownership");
    command("GameModule.StopPuzzleGuide");map=162;
    expect(!queries::is("GameModule.HasPuzzleGuide"),"unrelated maps keep the existing static-guide owner");
    flags.clear();map=45;mini=1;command("GameModule.PuzzleGuide");
    expect(goal.map_id==46&&goal.entity_id==25,"Tozus selects actual Antidote recipient");
    flags[29]=1;command("GameModule.PuzzleGuideMove");flush();
    expect(!ff3::puzzles::active(),"Tozus uses native healed-patient receipt");
    flags.clear();map=194;mini=0;command("GameModule.PuzzleGuide");
    expect(goal.map_id==194&&goal.entity_id==9,"Hein begins at actual soldier help");
    flags[55]=1;command("GameModule.PuzzleGuideMove");flush();
    expect(ff3::puzzles::last==Action::CastMini,"soldier receipt advances to prison transformation");
    mini=1;command("GameModule.PuzzleGuide");expect(goal.map_id==194&&goal.entity_id==18,"Hein uses exact escape hole");
    flags[56]=1;emit_fixture_event(events::Kind::MapChanging);map=195;emit_fixture_event(events::Kind::MapChanged);flush();
    expect(ff3::puzzles::last==Action::RemoveMini,"Hein landing prompts restoration before further exploration");
    mini=2;command("GameModule.PuzzleGuide");
    expect(spoken.back().find(L"only on members who still have")!=std::wstring::npos,"mixed restoration targets only remaining transformed members");
    mini=0;command("GameModule.PuzzleGuide");expect(!ff3::puzzles::active(),"Hein finishes only at actual escape plus normal party");
    flags.clear();map=84;frog=0;command("GameModule.PuzzleGuide");expect(goal.map_id==90&&goal.entity_id==5,"lake starts with the real waterway approach");
    map=90;emit_fixture_event(events::Kind::MapChanged);flush();expect(ff3::puzzles::last==Action::CastFrog,"lake arrival uses cached explicit form observation without a new scan");
    frog=1;command("GameModule.PuzzleGuide");flags[46]=1;emit_fixture_event(events::Kind::MapChanging);map=185;emit_fixture_event(events::Kind::MapChanged);flush();
    expect(ff3::puzzles::last==Action::RemoveFrog,"lake entry prompts normal form");
    frog=0;command("GameModule.PuzzleGuide");expect(goal.map_id==188&&goal.entity_id==9,"lake routes to actual horn guardian");
    flags[48]=1;command("GameModule.PuzzleGuideMove");flush();expect(goal.map_id==85&&goal.entity_id==21,"recovered horn guides the dwarf conversation");
    flags[49]=1;command("GameModule.PuzzleGuideMove");flush();expect(goal.map_id==85&&goal.entity_id==27,"dwarf receipt guides actual altar item use");
    flags[50]=1;command("GameModule.PuzzleGuideMove");flush();expect(!ff3::puzzles::active(),"native altar receipt ends horn placement");
    flags.clear();map=131;mini=0;command("GameModule.PuzzleGuide");expect(goal.map_id==131&&goal.entity_id==10,"Manor begins with actual native Doga event");
    flags[77]=1;command("GameModule.PuzzleGuideMove");flush();expect(goal.map_id==133&&goal.entity_id==11,"Manor keeps existing passage control");
    flags[198]=1;command("GameModule.PuzzleGuideMove");flush();expect(ff3::puzzles::last==Action::CastMini,"Manor leads to Mini after opening passage");
    mini=1;command("GameModule.PuzzleGuide");expect(goal.map_id==133&&goal.entity_id==18&&goal.cell.layer==2,"Manor keeps the actual Upper-layer hole");
    flags[195]=1;emit_fixture_event(events::Kind::MapChanging);map=238;emit_fixture_event(events::Kind::MapChanged);flush();
    expect(goal.map_id==241&&goal.entity_id==3,"magic cave targets actual upgrade event");
    flags[78]=1;emit_fixture_event(events::Kind::MapChanging);map=4;emit_fixture_event(events::Kind::MapChanged);flush();
    expect(!ff3::puzzles::active(),"native upgrade survives Surface World return");
    map=133;flags[198]=0;expect(!queries::is("GameModule.HasPuzzleGuide"),"completed Manor retains independent optional switch guide when198 was missed");
    flags.clear();map=169;command("GameModule.PuzzleGuide");auto late_type_route=callbacks.back();
    nav_guidance=false;command("GameModule.PuzzleGuidePolicyChanged");const auto lines=spoken.size();
    if(late_type_route){field::PathInfo p;p.success=true;p.description=L"Old solution";late_type_route(p);}
    expect(!ff3::puzzles::active()&&spoken.size()==lines,"navigation toggle quietly cancels form guidance and held output");
    const auto old_requests=requests,old_reads=form_reads;command("GameModule.PuzzleGuide");
    expect(requests==old_requests&&form_reads==old_reads&&spoken.back()==L"Navigation puzzle guidance is off","disabled form guidance performs no new party scan or route");
    nav_guidance=true;
    std::printf("FFIII form guide: %d controller/menu/cancel/receipt/circuit cases, %d failures\n",cases,failures);
    return failures?1:0;
}
