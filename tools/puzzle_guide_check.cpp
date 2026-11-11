// check-sources: json.cpp text.cpp
// Actual guide controller, real FF2 guide data and native-phase callback contract.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include "../src/core/field/puzzle_guide.cpp"
namespace fixture {
json::Value puzzles;
bool ready=true,blocked=false,module_active=false;
int verbosity=-1;std::vector<bool> interruptions;
bool nav_guidance=true,other_guidance=true;
std::map<std::string,actions::Fn> commands;
bool choice=false,dialogue=false,battle=false,module_has=false,keyword=false,event_item=false;
bool module_during=false,native_menu=false,module_ready=false,module_repeat=false;
std::map<std::string,std::function<bool()>> query_definitions;
int flag_reads=0,module_guides=0,module_stops=0;
game::Id running_game=game::Id::FF2; int module_moves=0;
int map=185,requests=0,retired=0;
std::vector<field::baked_nav::GuideTarget> guide_targets;
field::baked_route::Cell cell{0,15,27};
std::vector<std::wstring> spoken;
std::function<void(field::PathInfo)> route;
frame::Step step;
std::wstring scene=L"Field";
std::map<events::Kind,events::Handler> handlers;
std::map<int,std::optional<int>> flags;
std::map<int,std::optional<bool>> words;
int word_reads=0;
std::optional<int> local=0;
std::optional<bool> hidden=false;
int hidden_reads=0,storage_map=386;
field::baked_nav::MapGoal goal;
field::TargetKind kind=field::TargetKind::Infer;
}
namespace field::map_state {
std::optional<int> scenario_flag(int n){++fixture::flag_reads;auto i=fixture::flags.find(n);return i==fixture::flags.end()?std::optional<int>(0):i->second;}
std::optional<bool> keyword_learned(int n){++fixture::word_reads;auto i=fixture::words.find(n);return i==fixture::words.end()?std::optional<bool>(false):i->second;}
std::optional<int> local_value(int map,int){++fixture::flag_reads;return map==fixture::storage_map?fixture::local:std::nullopt;}
std::optional<bool> entity_hidden(int map,int entity,const std::wstring& raw){++fixture::hidden_reads;if(map!=256||entity!=13||raw!=L"スイッチ")std::abort();return fixture::hidden;}
}
namespace game {Id current(){return fixture::running_game;}}
namespace menu_state {bool is_active(const char* name){return std::string(name)==EVENT_ITEM_SELECT&&fixture::event_item;}bool any_active(){return fixture::native_menu;}}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace hooks {void note_fault(const char*){std::abort();} }
namespace log_ {void write(const char*,...){} }
namespace data {bool game_data_ready(){return true;} const json::Value& game_json(const char*){return fixture::puzzles;} }
namespace actions {void define(const char* n,Fn fn){fixture::commands[n]=std::move(fn);} bool defined(const char* n){return fixture::commands.count(n)!=0;} bool run(const char* n){const std::string name(n);if(auto i=fixture::commands.find(name);i!=fixture::commands.end()){i->second();return true;}if(name=="GameModule.PuzzleGuideMove")++fixture::module_moves;else if(name=="GameModule.PuzzleGuide")++fixture::module_guides;else if(name=="GameModule.StopPuzzleGuide"){++fixture::module_stops;fixture::module_active=false;}return true;} }
namespace queries {
bool is(const char*n){
    const std::string name(n);const auto custom=fixture::query_definitions.find(name);
    if(custom!=fixture::query_definitions.end())return custom->second();
    return name=="GameStatePatches.IsFieldNavigationReady"?fixture::ready:name=="GameStatePatches.IsInputTransitionBlocked"?fixture::blocked:
        name=="GameModule.PuzzleGuideActive"?fixture::module_active:name=="GameModule.PuzzleGuideDuringEvent"?fixture::module_during:name=="GameModule.PuzzleGuideReadyDuringEvent"?fixture::module_ready:name=="GameModule.PuzzleGuideRepeatDuringEvent"?fixture::module_repeat:
        name=="GameModule.HasPuzzleGuide"?fixture::module_has:name=="MessageChoice.IsOpen"?fixture::choice:
        name=="MessageWindowPatches.IsInDialogue"?fixture::dialogue:name=="KeywordMenuState.IsActive"?fixture::keyword:
        name=="BattleStateHelper.IsInBattle"?fixture::battle:false;
}
void define(const char*name,std::function<bool()> fn){fixture::query_definitions[name]=std::move(fn);}
}
namespace events {void on(Kind kind,Handler fn){fixture::handlers[kind]=std::move(fn);} }
namespace frame {int every_frame(Step s,const char*,int){fixture::step=std::move(s);return 1;} void cancel_label(const char*){fixture::step={};} }
namespace ffpr {void say(std::wstring_view s,bool interrupt){fixture::spoken.emplace_back(s);fixture::interruptions.push_back(interrupt);} void say_sequence(std::vector<std::wstring> parts,bool interrupt){for(auto& s:parts){fixture::spoken.push_back(std::move(s));fixture::interruptions.push_back(interrupt);interrupt=false;}} }
namespace modtext {std::wstring T(std::wstring_view s){return s==L"FF2 keyword Wyverns"?L"Wyverns":s==L"FF2 item Wyvern Egg"?L"Wyvern Egg":std::wstring(s);} std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring> args){auto out=std::wstring(s);int i=0;for(const auto& a:args){auto token=L"{"+std::to_wstring(i++)+L"}";auto at=out.find(token);if(at!=out.npos)out.replace(at,token.size(),a);}return out;} }
namespace settings {bool get_bool(const char* key,bool v){return std::string(key)==ui::prefs::kNavigationPuzzleGuidance?fixture::nav_guidance:std::string(key)==ui::prefs::kOtherPuzzleGuidance?fixture::other_guidance:v;} int get_int(const char* key,int v){return std::string(key)==ui::prefs::kPuzzleGuideVerbosity&&fixture::verbosity>=0?fixture::verbosity:v;} }
namespace field {int current_map_id(){return fixture::map;} void* map_handle(){return nullptr;} void* player(){return nullptr;} }
namespace field {const std::wstring& scene_name(){return fixture::scene;}}
namespace field::baked_nav {
std::wstring map_name_of(int id){return id==187 ? L"Deist Cavern B5":L"Deist Cavern";}
bool player_cell(void*,void*,baked_route::Cell& out){out=fixture::cell;return true;}
void clear_preview_destination(const MapGoal&){++fixture::retired;}
void request_route(MapGoal goal,TargetKind kind,std::wstring,std::function<void(PathInfo)> fn,bool keep){if(keep)std::abort();fixture::guide_targets.clear();fixture::goal=std::move(goal);fixture::kind=kind;++fixture::requests;fixture::route=std::move(fn);}
void request_guidance(std::vector<GuideTarget> targets,std::function<void(PathInfo)> fn,const map_state::Flags*,const DefenseContext*) {
    request_route(targets.front().goal,targets.front().kind,L"",std::move(fn),false);
    fixture::guide_targets=std::move(targets);
}
}
int main(){
    using namespace field::puzzle_guide;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    int failures=0;auto expect=[&](bool v,const char*s){if(!v){++failures;std::printf("FAIL %s\n",s);}};
    std::ifstream file("data/ff2/puzzles.json");const std::string bytes((std::istreambuf_iterator<char>(file)),{});
    expect(json::parse(bytes,fixture::puzzles),"real Deist guide JSON loads");
    for(int flag=41;flag<=45;++flag)fixture::flags[flag]=1;fixture::words[927]=true; // native prerequisites already completed
    install();
    fixture::blocked=true;start_or_repeat();
    expect(!active() && !fixture::requests && fixture::spoken.empty(),"guide key cannot load or plan during transition");
    fixture::blocked=false;start_or_repeat();
    expect(active() && fixture::requests==1,"Deist guide starts and owns its route preview");
    field::PathInfo info;info.success=true;info.description=L"3 West";
    fixture::blocked=true;const auto before=fixture::spoken.size();fixture::route(info);
    expect(fixture::spoken.size()==before,"guide callback cannot speak during transition");
    fixture::blocked=false;announce_step(false);auto old=fixture::route;stop();start_or_repeat();
    const auto restarted=fixture::spoken.size();old(info);
    expect(fixture::spoken.size()==restarted,"stopped/restarted same guide cannot revive the old callback");
    fixture::map=187;fixture::cell={0,9,8};on_move();
    expect(active()&&fixture::spoken.size()==restarted,"matching destination XY on another logical layer cannot complete the Deist guide");
    fixture::map=185;
    fixture::ready=false;on_map_changed();const int requests=fixture::requests;
    expect(fixture::step && fixture::step() && fixture::requests==requests,"guide waits for native readiness without route work");
    fixture::map=187;fixture::cell={1,9,8};fixture::ready=true;
    expect(!fixture::step() && active() && fixture::goal.entity_id==41 && fixture::retired>=2,
        "native B5 arrival must retain the real Life Spring operation until the egg is placed");
    fixture::flags[46]=1;start_or_repeat();
    expect(!active(),"actual native egg-placement receipt completes Deist and retires its preview");
    expect(fixture::spoken.back()==L"Puzzle guide finished","completion is queued after the map announcement");
    fixture::map=185;start_or_repeat();fixture::scene=L"Title";
    fixture::handlers[events::Kind::SceneLoaded]();
    expect(!active(),"return to title retires the guide before another game load");
    fixture::module_active=true;fixture::running_game=game::Id::FF3;on_move();
    expect(fixture::module_moves==1,"FFIII active guide receives movement dispatch");
    fixture::running_game=game::Id::FF6;on_move();expect(fixture::module_moves==1,"FFVI story guide receives no circuit movement dispatch");
    fixture::running_game=game::Id::FF3;fixture::blocked=true;on_move();expect(fixture::module_moves==1,"blocked FFIII movement is not dispatched");
    fixture::running_game=game::Id::FF5;on_move();expect(fixture::module_moves==1,"blocked FFV movement is not dispatched");
    fixture::blocked=false;on_move();expect(fixture::module_moves==2,"FFV active Library guide receives movement dispatch");
    fixture::blocked=false;fixture::module_active=false;
    const auto load_game=[&](int n) {
        delete g_puzzles;g_puzzles=nullptr;g_loaded=false;g_active=-1;g_step=0;g_arrived_step=-1;++g_context;
        fixture::flags.clear();fixture::words.clear();fixture::local=0;fixture::hidden=false;fixture::event_item=fixture::module_during=false;fixture::running_game=static_cast<game::Id>(n);
        std::ifstream f("data/ff"+std::to_string(n)+"/puzzles.json");const std::string data((std::istreambuf_iterator<char>(f)),{});
        fixture::puzzles={}; // production parses each game into a fresh resource object
        expect(json::parse(data,fixture::puzzles),"current real recipe loads");load();
    };
    int controls=0;
    // Exercise actual late-added story recipes:the old notice limit32 would
    // silently suppress every FFVI story entry once the catalog exceeded32.
    load_game(6);fixture::flags.clear();fixture::module_has=true;
    expect(fixture::puzzles.get(L"story_guides")->arr.size()>32,"entry regression uses the expanded FFVI story catalog");
    for(int map:{334,343,290,295}) {
        fixture::flags[297]=map<300?1:0;
        fixture::map=map;fixture::spoken.clear();on_map_changed();
        expect(fixture::step&&!fixture::step()&&fixture::spoken==std::vector<std::wstring>{L"Puzzle guide active"},"current Mt. Kolts and later Opera story entries retain Minimal notice");
    }
    load_game(5);fixture::module_has=true;fixture::map=424;
    for(int state:{0,1,-1}) {
        fixture::flags[137]=state<0?std::optional<int>{}:std::optional<int>{state};fixture::spoken.clear();on_map_changed();
        expect(fixture::step&&!fixture::step()&&(state==0?fixture::spoken==std::vector<std::wstring>{L"Puzzle guide active"}:fixture::spoken.empty()),"Pyramid entry notice requires a known unfinished tablet receipt");
    }
    for(int map:{284,285,270}) {
        fixture::map=map;fixture::nav_guidance=true;fixture::other_guidance=false;fixture::spoken.clear();on_map_changed();
        expect(fixture::step&&!fixture::step()&&fixture::spoken==std::vector<std::wstring>{L"Puzzle guide active"},"Jachol/Ship typed floor entry retains the Minimal availability notice with Other off");
        fixture::nav_guidance=false;fixture::other_guidance=true;fixture::spoken.clear();on_map_changed();
        expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"Jachol/Ship entry notice respects independent Navigation disable");
    }
    fixture::map=118;
    for(int state:{0,1,-1}) {
        fixture::nav_guidance=true;fixture::other_guidance=false;fixture::flags[299]=state<0?std::optional<int>{}:std::optional<int>{state};fixture::spoken.clear();on_map_changed();
        expect(fixture::step&&!fixture::step()&&(state==0?fixture::spoken==std::vector<std::wstring>{L"Puzzle guide active"}:fixture::spoken.empty()),"Istory Minimal entry notice requires known unfinished299 and survives Other disable");
    }
    fixture::flags[299]=0;fixture::nav_guidance=false;fixture::other_guidance=true;fixture::spoken.clear();on_map_changed();
    expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"Istory notice respects independent Navigation disable");
    fixture::map=1;fixture::nav_guidance=true;fixture::spoken.clear();on_map_changed();
    expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"Istory transit context never creates a blanket world-map entry notice");
    for(int map:{485,490,494})for(int state:{0,1,-1}) {
        fixture::map=map;fixture::nav_guidance=false;fixture::other_guidance=true;fixture::flags[166]=state<0?std::optional<int>{}:std::optional<int>{state};fixture::spoken.clear();on_map_changed();
        expect(fixture::step&&!fixture::step()&&(state==0?fixture::spoken==std::vector<std::wstring>{L"Puzzle guide active"}:fixture::spoken.empty()),"Gogo entry notice requires known unfinished166 and Other ownership independently from Navigation");
    }
    fixture::map=494;fixture::flags[166]=0;fixture::nav_guidance=true;fixture::other_guidance=false;fixture::spoken.clear();on_map_changed();
    expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"Other disable independently suppresses the Gogo entry notice");
    fixture::nav_guidance=fixture::other_guidance=true;fixture::module_has=false;
    for(int game=1;game<=6;++game) {
        load_game(game);
        constexpr std::array<size_t,6> expected_recipes{2,6,0,2,0,2};
        expect(g_puzzles&&g_puzzles->size()==expected_recipes[static_cast<size_t>(game-1)],"all authored recipes survive receipt validation");
        for(size_t pi=0;pi<g_puzzles->size();++pi) {
            const auto& p=(*g_puzzles)[pi];
            for(size_t si=0;si<p.steps.size();++si) {
                const auto& s=p.steps[si];if(s.done_scenario<0&&s.done_local<0&&!s.done_hidden&&s.done_keyword<0)continue;
                ++controls;g_active=int(pi);g_step=int(si);g_arrived_step=-1;++g_context;
                fixture::map=s.map_id;fixture::cell=s.cell;fixture::cell.x-=s.examine?1:0;
                fixture::flags.clear();fixture::words.clear();fixture::local=0;fixture::hidden=false;
                if(s.before_scenario>=0)fixture::flags[s.before_scenario]=0;
                announce_step(true);const auto pending=fixture::route;
                expect(fixture::goal.entity_id==s.entity_id&&fixture::goal.raw_name==s.raw_name&&fixture::goal.cell.layer==s.cell.layer&&fixture::kind==(s.examine?field::TargetKind::Interact:field::TargetKind::Spot),"control routes retain exact identity, layer and action kind");
                expect(fixture::goal.allow_world_transit==s.allow_world_transit,"only explicitly authored fetch/return steps enable world transit");
                on_move();expect(active()==&p&&g_step==int(si),"adjacency or foot-trigger position without native receipt never completes an operation");
                if(s.done_keyword>=0)fixture::words[s.done_keyword]={};
                else if(s.done_scenario>=0)fixture::flags[s.done_scenario]={};
                else if(s.done_local>=0)fixture::local={};else fixture::hidden={};
                start_or_repeat();expect(active()==&p&&g_step==int(si),"unknown operation state cannot complete a control");
                if(s.done_keyword>=0)fixture::words[s.done_keyword]=true;
                else if(s.done_scenario>=0)fixture::flags[s.done_scenario]=1;
                else if(s.done_local>=0)fixture::local=3;else fixture::hidden=true;
                if(s.done_hidden) {
                    const int reads=fixture::hidden_reads;
                    for(int j=0;j<100;++j)on_move();
                    expect(fixture::hidden_reads==reads&&g_step==int(si),"flagless native actor lookup is absent from repeated movement callbacks");
                }
                if(s.done_keyword>=0) {
                    const int reads=fixture::word_reads;for(int j=0;j<100;++j)on_move();
                    expect(fixture::word_reads==reads&&g_step==int(si),"movement never scans the native learned-word list; explicit repeat owns it");
                }
                start_or_repeat();expect(!active()||g_step>int(si),"explicit repeat consumes the actual native operation receipt");
                const auto speech=fixture::spoken.size();pending(info);
                expect(fixture::spoken.size()==speech,"completed control rejects its old route publication");
            }
        }
    }
    expect(controls==45,"all active static control operations are exercised");
    const std::array<std::vector<int>,6> ordinary_maps{{
        {115}, {21,41,118}, {43,153,305,306,336}, {4,5,16,24,25,29,42,47,84,109,125,220,268}, {256}, {162,472,478,488,491}
    }};
    for(int game=1;game<=6;++game)for(int map:ordinary_maps[static_cast<size_t>(game-1)]) {
        load_game(game);fixture::map=map;fixture::spoken.clear();
        const int before=fixture::requests;on_map_changed();
        const bool silent=fixture::step&&!fixture::step()&&fixture::spoken.empty();
        if(!silent)std::printf("Ordinary entry FF%d map%d spoke %ls\n",game,map,fixture::spoken.empty()?L"<none>":fixture::spoken.back().c_str());
        expect(silent,"ordinary interaction produces no puzzle availability notice");
        start_or_repeat();
        const bool absent=!active()&&fixture::requests==before&&fixture::spoken==std::vector<std::wstring>{L"No puzzle guide for this map"};
        if(!absent)std::printf("Ordinary request FF%d map%d spoke %ls\n",game,map,fixture::spoken.empty()?L"<none>":fixture::spoken.back().c_str());
        expect(absent,
            "manual Shift+G cannot start a route to a retired ordinary interaction");
    }
    load_game(3);
    fixture::module_active=fixture::module_during=true;
    expect(queries::is("PuzzleGuide.CanRepeatDuringEvent"),"shared input query admits the active FFIII form-guide owner");
    fixture::module_has=fixture::blocked=true;
    const int module_repeat=fixture::module_guides,module_flags=fixture::flag_reads;start_or_repeat();
    expect(fixture::module_guides==module_repeat+1&&fixture::flag_reads==module_flags,
        "blocked shared Shift+G delegates to the qualified FFIII owner without native observation");
    fixture::running_game=game::Id::FF5;const auto library_repeats=fixture::module_guides;
    expect(queries::is("PuzzleGuide.CanRepeatDuringEvent"),"shared input query admits the active FFV Library owner");start_or_repeat();
    expect(fixture::module_guides==library_repeats+1&&fixture::flag_reads==module_flags,
        "blocked shared Shift+G delegates FFV repeat without native observation");
    fixture::module_has=fixture::blocked=false;
    fixture::running_game=game::Id::FF4;
    expect(!queries::is("PuzzleGuide.CanRepeatDuringEvent"),"FFIII form-menu permission does not broaden another game's contexts");
    fixture::module_active=fixture::module_during=false;
    fixture::running_game=game::Id::FF6;fixture::module_has=fixture::module_active=fixture::module_ready=true;fixture::blocked=true;
    const int roof_delegates=fixture::module_guides,roof_reads=fixture::flag_reads;
    expect(queries::is("PuzzleGuide.CanRepeatDuringEvent"),"only an already pending FFVI readiness request owns its animation context");
    start_or_repeat();expect(fixture::module_guides==roof_delegates+1&&fixture::flag_reads==roof_reads,"roof readiness delegates during animation without native progress reads");
    fixture::module_ready=false;expect(!queries::is("PuzzleGuide.CanRepeatDuringEvent"),"ordinary FFVI field guides do not inherit the pending-ready exception");
    fixture::module_has=fixture::module_active=fixture::blocked=false;
    load_game(4);fixture::map=100;start_or_repeat();fixture::route(info);
    expect(active()&&fixture::goal.entity_id==22,"Magnetic Cave opens at the actual Twin Harp owner");
    fixture::native_menu=fixture::blocked=true;const int ff4_reads=fixture::flag_reads,ff4_requests=fixture::requests;
    const auto ff4_speech=fixture::spoken.size();start_or_repeat();
    expect(queries::is("PuzzleGuide.CanRepeatDuringEvent")&&fixture::spoken.size()==ff4_speech+1&&fixture::flag_reads==ff4_reads&&fixture::requests==ff4_requests,
        "FFIV item/equipment-menu repeat uses owned text with zero native observation or route work");
    stop();fixture::native_menu=fixture::blocked=false;
    expect(!active(),"FFIV menu cancellation retires the guide");
    load_game(4);fixture::map=192;fixture::flags[34]=1;start_or_repeat();
    expect(active()&&g_step==1&&fixture::goal.entity_id==5,"magnetic cave first encounter uses actual pre-harp boss5");
    auto pre_harp=fixture::route;fixture::blocked=true;fixture::map=100;on_map_changed();
    const int harp_requests=fixture::requests;expect(fixture::step&&fixture::step()&&fixture::requests==harp_requests,"automatic harp detour remains blocked without new routes or early step advancement");
    fixture::flags[97]=1;fixture::map=201;fixture::blocked=false;on_map_changed();
    if(fixture::step)fixture::step();
    expect(active()&&g_step==2&&fixture::goal.entity_id==4,"native harp97 and actual return advance to re-equipped boss4");
    const auto harp_speech=fixture::spoken.size();pre_harp(info);expect(fixture::spoken.size()==harp_speech,"pre-harp route publication cannot overwrite the new operation");
    fixture::flags[36]=1;start_or_repeat();expect(active()&&g_step==3&&fixture::goal.entity_id==3,"native victory advances to the actual animated crystal");
    fixture::flags[98]=1;on_move();expect(!active(),"magnetic guide ends at crystal98, not battle36");
    for(const auto map:{191}) {
        load_game(2);fixture::map=map;const int requests=fixture::requests;
        start_or_repeat();
        expect(!active()&&fixture::requests==requests&&fixture::spoken.back()==L"Story gated",
            "additional FFII guides respect native party/story availability before requesting a route");
    }
    load_game(2);fixture::map=179;fixture::flags[41]=fixture::flags[42]=1;start_or_repeat();
    fixture::route(info); // the ready instruction is published with its asynchronous route
    expect(active()&&g_step==2&&fixture::goal.entity_id==28&&fixture::spoken.back().find(L"learn Wyverns")!=std::wstring::npos,
        "fresh Deist guide reaches actual keyword teacher and substitutes the owned official term");
    fixture::keyword=fixture::blocked=true;
    const int word_requests=fixture::requests,word_flags=fixture::flag_reads,word_reads=fixture::word_reads;
    auto word_speech=fixture::spoken.size();start_or_repeat();
    expect(fixture::spoken.size()==word_speech+1&&fixture::spoken.back().find(L"learn Wyverns")!=std::wstring::npos,
        "Shift+G repeats the owned instruction while FFII's native keyword menu is active");
    expect(fixture::requests==word_requests&&fixture::flag_reads==word_flags&&fixture::word_reads==word_reads,
        "keyword-menu repeat does not route, read flags or scan native learned words");
    stop();expect(!active(),"Ctrl+G cancels while the native keyword menu blocks the field");fixture::keyword=fixture::blocked=false;
    load_game(2);fixture::map=43;const auto gated_requests=fixture::requests;start_or_repeat();
    expect(!active()&&fixture::requests==gated_requests&&fixture::spoken.back()==L"Story gated",
        "Fynn guide does not target an absent boss/queen before the native retaking episode");
    fixture::flags[51]=fixture::flags[52]=1;fixture::words[924]=true;start_or_repeat();fixture::route(info);
    expect(active()&&g_step==2&&fixture::spoken.back().find(L"Ask about FF2 keyword Mysidia and learn FF2 keyword Mask")!=std::wstring::npos,
        "Fynn resolves both owned Ask/Learn arguments and skips the completed boss and first word");
    fixture::keyword=fixture::blocked=true;const int pair_words=fixture::word_reads,pair_requests=fixture::requests;
    const auto pair_speech=fixture::spoken.size();start_or_repeat();
    expect(fixture::spoken.size()==pair_speech+1&&fixture::word_reads==pair_words&&fixture::requests==pair_requests,
        "two-term keyword-menu repeat remains owned information with zero route or learned-word reads");
    stop();fixture::keyword=fixture::blocked=false;
    fixture::flags[56]=1;fixture::words[924]={};fixture::words[930]={};fixture::words[931]={};
    const auto end_words=fixture::word_reads;start_or_repeat();
    expect(!active()&&fixture::spoken.back()==L"Puzzle guide finished"&&fixture::word_reads==end_words,
        "native opened-wall end receipt certifies the completed family without rereading unavailable prerequisite words");
    load_game(1);fixture::map=93;start_or_repeat();
    expect(active()&&g_step==0&&fixture::goal.entity_id==8,"fresh Citadel visit guides the actual elder before the maze");stop();
    load_game(1);fixture::map=94;fixture::flags[44]=1;start_or_repeat();
    expect(active()&&fixture::goal.map_id==95&&fixture::goal.cell==field::baked_route::Cell{2,35,32},"Citadel guides stable native3F entry rather than the automatic return warp");
    fixture::map=95;fixture::cell={0,29,31};on_move();
    expect(active()&&g_step==2&&fixture::goal.entity_id==9&&fixture::goal.cell==field::baked_route::Cell{0,27,29}&&fixture::kind==field::TargetKind::Interact,
        "3F arrival advances to the actual Rat Tail chest instead of finishing the trial");
    const auto chest_route=fixture::route;
    fixture::cell={0,28,29};on_move();
    expect(active()&&g_step==2,"standing beside the final chest never certifies the reward");
    fixture::flags[45]={};start_or_repeat();
    expect(active()&&g_step==2,"unknown Rat Tail reward state keeps the chest objective active");
    fixture::flags[45]=0;fixture::battle=true;fixture::handlers[events::Kind::BattleEnd]();
    expect(active()&&g_step==2,"a guardian battle without the chest reward never finishes the trial");
    fixture::battle=false;fixture::handlers[events::Kind::BattleEnd]();
    expect(active()&&g_step==2,"returning from battle still requires opening the chest");
    fixture::flags[45]=1;fixture::handlers[events::Kind::DialogueClosed]();
    expect(!active()&&fixture::spoken.back()==L"Puzzle guide finished","native Rat Tail acquisition45 finishes the full trial");
    const auto chest_speech=fixture::spoken.size();chest_route(info);
    expect(fixture::spoken.size()==chest_speech,"completed reward rejects an old chest route publication");
    for(int resume:{93,94,95}) {
        load_game(1);fixture::map=resume;fixture::flags[45]=1;const int requests=fixture::requests;
        start_or_repeat();
        expect(!active()&&fixture::requests==requests&&fixture::spoken.back()==L"Puzzle guide finished",
            "completed reward persists on every trial floor without inventory or earlier elder state");
    }
    load_game(1);fixture::map=95;fixture::flags[44]=1;start_or_repeat();
    expect(active()&&g_step==2&&fixture::goal.entity_id==9,"fresh guide on3F resumes directly at the unopened reward chest");stop();
    load_game(1);fixture::map=78;
    for(int flag:{16,17,18,19})fixture::flags[flag]=1;
    start_or_repeat();
    expect(active()&&g_step==4&&fixture::goal.entity_id==52&&fixture::goal.allow_world_transit,
        "Rod guide skips all four native prerequisite receipts on a resumed slab visit");stop();
    load_game(1);fixture::map=78;fixture::flags[16]=1;start_or_repeat();
    expect(active()&&g_step==1&&fixture::goal.entity_id==22,
        "Vampire victory alone cannot skip the actual Ruby chest acquisition");stop();
    load_game(4);fixture::map=102;fixture::cell={0,12,11};
    const std::array<int,4> treasury_switches{41,43,45,44};
    const std::array<int,4> treasury_receipts{274,275,276,279};
    actions::run("PuzzleGuide.Next");
    for(int stage=0;stage<4;++stage) {
        expect(active()&&g_step==stage&&fixture::goal.entity_id==treasury_switches[stage],
            "Troia starts at the entrance switch and advances to the reachable contact after each native receipt");
        stop();actions::run("PuzzleGuide.Next");
        expect(active()&&g_step==stage&&fixture::goal.entity_id==treasury_switches[stage],
            "Restarting Troia from partial progress selects the same next contact");
        expect(fixture::goal.avoid_other_events&&(stage==0?fixture::guide_targets.empty():
            fixture::guide_targets.size()==2&&fixture::guide_targets[1].goal.entity_id==41&&fixture::guide_targets[1].goal.avoid_other_events),
            "Troia offers only the intended contacts, with the entrance switch as the alternate after reload");
        if(stage>0) {
            auto alternate=info;alternate.guidance_index=1;alternate.step_count=3;fixture::route(alternate);
            expect(fixture::spoken.back()==L"Step on the entrance switch. 3 West","Troia announces the selected entrance fallback and its directions");
            alternate.step_count=0;alternate.description=L"No movement needed";fixture::route(alternate);
            expect(fixture::spoken.back()==L"Step off and back onto the switch","Troia directs retriggering when already standing on the entrance switch");
        }
        fixture::cell=fixture::goal.cell;
        fixture::flags[treasury_receipts[stage]]=1;
        actions::run("PuzzleGuide.OnMove");
    }
    expect(!active()&&fixture::spoken.back()==L"Puzzle guide finished","Troia completes on the fourth actual switch operation");
    const int treasury_requests=fixture::requests;
    actions::run("PuzzleGuide.Next");
    expect(!active()&&fixture::requests==treasury_requests&&fixture::spoken.back()==L"Puzzle guide finished",
        "A completed Troia sequence can be repeated without routing to a retired switch");
    load_game(6);fixture::map=517;fixture::module_has=fixture::module_active=false;start_or_repeat();
    expect(active()&&g_step==0&&fixture::goal.map_id==517&&fixture::goal.entity_id==2,"castle starts with the actual enabling statue, not the hidden switch");
    fixture::cell={1,14,12};on_move();expect(g_step==0,"standing beside the statue is not its native428 receipt");
    fixture::flags[428]=1;fixture::blocked=true;on_move();expect(g_step==0,"first statue movie retains its step until field readiness");fixture::blocked=false;on_move();
    expect(active()&&g_step==1&&fixture::goal.entity_id==12,"actual428 enables the hidden-switch objective");
    fixture::flags[429]=1;fixture::map=518;fixture::blocked=true;fixture::handlers[events::Kind::MapChanged]();
    expect(fixture::step&&fixture::step()&&g_step==1,"stair movie/map changes do not skip native readiness");fixture::blocked=false;expect(!fixture::step(),"stair refresh finishes when normal field control returns");
    expect(active()&&g_step==2&&fixture::goal.map_id==519&&fixture::goal.entity_id==10,"opened stairs route to the real final statue instead of construction scenery");
    fixture::map=519;fixture::cell={1,12,10};on_move();expect(active()&&g_step==2,"final-statue proximity cannot finish the objective");
    fixture::flags[432]={};start_or_repeat();expect(active()&&g_step==2,"unknown final receipt cannot finish or restart the castle sequence");
    const auto castle_route=fixture::route;fixture::flags[432]=1;fixture::blocked=true;on_move();expect(active(),"final transformation waits for native animation/dialogue readiness");fixture::blocked=false;fixture::handlers[events::Kind::DialogueClosed]();
    expect(!active()&&fixture::spoken.back()==L"Puzzle guide finished","actual432 and normal field readiness finish the complete castle objective");const auto castle_end=fixture::spoken.size();castle_route(info);expect(fixture::spoken.size()==castle_end,"finished castle rejects a stale route callback");
    for(int map:{517,518,519}){load_game(6);fixture::map=map;fixture::flags[432]=1;const int requests=fixture::requests;start_or_repeat();expect(!active()&&fixture::requests==requests,"completed castle receipt persists on every supported floor");}
    load_game(6);fixture::map=519;fixture::flags[429]=1;start_or_repeat();expect(active()&&g_step==0&&fixture::goal.map_id==517&&fixture::goal.entity_id==2,"entering the final chamber without the enabling statue guides its real prerequisite");stop();
    load_game(6);fixture::map=387;fixture::storage_map=387;fixture::local=3;
    start_or_repeat();expect(active()&&g_step==0&&fixture::goal.map_id==386&&fixture::goal.entity_id==11,"Zozo selects actual solution clock and does not borrow the clue room's local3");stop();
    load_game(6);fixture::map=387;fixture::storage_map=387;start_or_repeat();
    expect(active()&&g_step==0,"real Zozo guide owns its current instruction");
    const auto pending_choice=fixture::route;
    fixture::blocked=true;fixture::choice=true;fixture::module_has=true;
    const int choice_requests=fixture::requests,choice_flags=fixture::flag_reads,choice_modules=fixture::module_guides;
    auto spoken_count=fixture::spoken.size();start_or_repeat();
    expect(fixture::spoken.size()==spoken_count+1&&fixture::spoken.back().find(L"Set the clock to 6:10:50")!=std::wstring::npos,"Shift+G repeats the owned answer while the native choice blocks the field");
    expect(fixture::requests==choice_requests&&fixture::flag_reads==choice_flags&&fixture::module_guides==choice_modules,"choice repeat performs zero route/native-state work and cannot misroute to Opera");
    fixture::blocked=false;start_or_repeat();fixture::blocked=true;
    expect(fixture::requests==choice_requests&&fixture::flag_reads==choice_flags,"owned native choice outranks a still-ready field flag and performs no puzzle capture");
    fixture::choice=false;fixture::dialogue=true;start_or_repeat();
    expect(fixture::requests==choice_requests&&fixture::flag_reads==choice_flags,"dialogue repeat also uses only owned authored text");
    fixture::dialogue=false;spoken_count=fixture::spoken.size();start_or_repeat();
    expect(fixture::spoken.size()==spoken_count,"ordinary loading cannot repeat or restart guidance");
    stop();expect(!active()&&fixture::spoken.back()==L"Puzzle guide stopped","Ctrl+G retires a static guide during a blocked native scene");
    fixture::blocked=false;fixture::module_has=false;spoken_count=fixture::spoken.size();pending_choice(info);
    expect(fixture::spoken.size()==spoken_count,"blocked-scene cancellation rejects later ready route callbacks");
    fixture::module_active=true;fixture::blocked=true;const int stops=fixture::module_stops;stop();
    expect(fixture::module_stops==stops+1&&!fixture::module_active,"module-owned guide cancellation also dispatches during blocked scenes");
    fixture::blocked=false;
    load_game(6);fixture::map=387;fixture::module_active=true;const int old_module_stops=fixture::module_stops;
    start_or_repeat();
    expect(active()&&!fixture::module_active&&fixture::module_stops==old_module_stops+1,"starting a static guide retires a prior module continuation");
    fixture::module_active=true;fixture::blocked=true;stop();
    expect(!active()&&!fixture::module_active,"one blocked-scene cancel retires both guide owners if integration left overlapping state");fixture::blocked=false;
    fixture::map=466;fixture::module_active=fixture::module_has=fixture::module_repeat=true;fixture::ready=false;
    const int repeat_reads=fixture::flag_reads,repeat_requests=fixture::requests,repeat_dispatches=fixture::module_guides;
    expect(queries::is("PuzzleGuide.CanRepeatDuringEvent"),"native epitaph choices permit the module's owned instruction repeat");start_or_repeat();
    expect(fixture::module_guides==repeat_dispatches+1&&fixture::requests==repeat_requests&&fixture::flag_reads==repeat_reads,"blocked native choice repeat dispatches without flags or routing");
    fixture::module_repeat=fixture::module_has=false;fixture::ready=true;stop();
    load_game(2);fixture::map=185;fixture::cell={0,15,27};fixture::ready=true;fixture::blocked=false;
    fixture::module_active=fixture::module_has=fixture::module_during=fixture::module_repeat=false;
    fixture::scene=L"Field";fixture::choice=fixture::dialogue=fixture::native_menu=fixture::battle=false;
    for(int flag=41;flag<=45;++flag)fixture::flags[flag]=1;fixture::words[927]=true;
    fixture::verbosity=-1;fixture::spoken.clear();fixture::interruptions.clear();start_or_repeat();
    expect(fixture::spoken.empty(),"default Minimal starts routing without an activation announcement");
    fixture::route(info);
    expect(!fixture::spoken.empty()&&fixture::spoken.back().find(L"3 West")!=std::wstring::npos&&fixture::spoken.back().find(L"Step ")==std::wstring::npos,"Minimal retains the actual step instruction and directions without a numbered caption");
    expect(fixture::interruptions.back(),"removing the activation header preserves the first request's speech interruption");
    stop();fixture::verbosity=1;fixture::spoken.clear();fixture::interruptions.clear();start_or_repeat();fixture::route(info);
    expect(fixture::spoken.size()>=2&&fixture::spoken.front().find(L"Puzzle guide:")==0&&fixture::spoken.back().find(L"Step ")==0,"Verbose explicitly restores activation and numbered step announcements");
    expect(fixture::interruptions.front()&&!fixture::interruptions.back(),"Verbose still interrupts once for the activation and queues its step");
    stop();fixture::verbosity=-1;
    for(int n=1;n<=6;++n) {
        load_game(n);fixture::flags.clear();fixture::module_has=false;
        const auto& static_recipes=fixture::puzzles.get(L"puzzles")->arr;
        if(static_recipes.empty()) {
            fixture::map=n==3?169:284; // native form and Jachol guide owners
            fixture::module_has=true;
        } else {
            const auto& recipe=static_recipes.front();
            fixture::map=static_cast<int>(recipe.get(L"maps")->arr.front().n);
            if(const auto* prerequisite=recipe.get(L"requires_scenario"))fixture::flags[static_cast<int>(prerequisite->n)]=1;
        }
        fixture::spoken.clear();fixture::blocked=true;on_map_changed();
        expect(fixture::step&&fixture::step()&&fixture::spoken.empty(),"entry notice waits for actual field readiness in every game");
        fixture::blocked=false;const int requests=fixture::requests;
        const bool noticed=fixture::step&&!fixture::step()&&fixture::spoken==std::vector<std::wstring>{L"Puzzle guide active"};
        if(!noticed)std::printf("Complex entry FF%d map%d spoke %ls\n",n,fixture::map,fixture::spoken.empty()?L"<none>":fixture::spoken.back().c_str());
        expect(noticed,"Minimal announces a short availability notice on all-six authored puzzle entries");
        expect(!active()&&fixture::requests==requests&&!fixture::interruptions.empty()&&!fixture::interruptions.back(),"entry notice queues after the map name without activating a guide or requesting directions");
        fixture::spoken.clear();on_map_changed();const auto stale=fixture::step;++g_entry_context;
        expect(stale&&!stale()&&fixture::spoken.empty(),"a superseded same-map entry cannot announce from an old callback");
    }
    load_game(6);fixture::map=1;fixture::flags.clear();fixture::spoken.clear();on_map_changed();
    expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"registered shared world map stays silent when its story guide is not selected");
    fixture::flags[50]=fixture::flags[51]=1;on_map_changed();
    expect(fixture::step&&!fixture::step()&&fixture::spoken==std::vector<std::wstring>{L"Puzzle guide active"},"selected FFVI world episode advertises its guide using real selection flags");
    fixture::spoken.clear();fixture::flags[51].reset();on_map_changed();
    expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"unknown story selection cannot advertise the optional shared-map episode");
    load_game(6);fixture::module_active=fixture::module_has=false;fixture::flags[47]=1;
    for(int map:{345,346,348}){fixture::map=map;fixture::spoken.clear();on_map_changed();expect(fixture::step&&!fixture::step()&&fixture::spoken==std::vector<std::wstring>{L"Puzzle guide active"},"Minimal advertises each eligible actual Lethe landing without starting a route");}
    fixture::flags[50]=1;fixture::spoken.clear();on_map_changed();expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"completed raft50 does not advertise a new journey");fixture::flags[50]=0;fixture::flags[47].reset();fixture::spoken.clear();on_map_changed();expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"unknown Banon eligibility cannot advertise a premature raft guide");
    for(int game=1;game<=6;++game) {
        policy_changed();load_game(game);fixture::flags.clear();fixture::module_has=false;
        fixture::hidden=false;fixture::local=0;
        const auto& static_recipes=fixture::puzzles.get(L"puzzles")->arr;
        if(static_recipes.empty())continue;
        const auto& row=static_recipes.front();
        fixture::map=int(row.get(L"maps")->arr.front().n);
        if(const auto* start=row.get(L"requires_scenario"))fixture::flags[int(start->n)]=1;
        const auto type=field::puzzle_policy::kind(fixture::puzzles,row.get(L"id")->s);
        const bool nav=type==field::puzzle_policy::Kind::Navigation;
        fixture::nav_guidance=!nav;fixture::other_guidance=nav;
        const auto calls=fixture::requests;fixture::spoken.clear();start_or_repeat();
        expect(!active()&&fixture::requests==calls&&fixture::spoken.back().find(L"guidance is off")!=std::wstring::npos,"disabled type cannot activate or route any game's static recipe");
        fixture::spoken.clear();on_map_changed();expect(fixture::step&&!fixture::step()&&fixture::spoken.empty(),"disabled-only puzzle entry has no availability announcement");
        fixture::nav_guidance=nav;fixture::other_guidance=!nav;
        start_or_repeat();expect(active()&&fixture::requests>calls,"the independent enabled type still starts and routes");
        auto stale=fixture::route;fixture::nav_guidance=fixture::other_guidance=false;
        ui::prefs::notify_changed(ui::prefs::kNavigationPuzzleGuidance);fixture::spoken.clear();if(stale)stale(info);
        expect(!active()&&fixture::spoken.empty(),"changing a type quietly cancels a held real route publication");
        const auto reads=fixture::flag_reads;start_or_repeat();
        expect(fixture::requests==calls+1&&fixture::flag_reads==reads&&fixture::spoken.back()==L"Puzzle guidance is off","both types off returns immediately without a route or native state read");
        fixture::nav_guidance=fixture::other_guidance=true;
    }
    std::printf("Static operation coverage: %d controls in 12 active recipes\n",controls);
    std::printf("Puzzle guide check: %d failures\n",failures);return failures?1:0;
}
