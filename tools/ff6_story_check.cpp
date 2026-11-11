// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/entity_translator.cpp field/npc_names.cpp json.cpp text.cpp
// Actual owned rule model, all authored targets and real FF6 state/bake data.
// Translating an unknown Japanese name must never touch a native map on a worker.
#include "../src/games/ff6/story_model.cpp"
#include "../src/games/ff6/dream_train_maze.cpp"
#include "../src/games/ff6/zone_eater_roof_model.cpp"
#include "../src/games/ff6/sealed_gate_model.cpp"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <thread>
namespace fixture {json::Value puzzles,translations,records;std::wstring locale=L"en";}
namespace data {
bool game_data_ready(){return true;}
const json::Value& game_json(const char* kind){return std::string(kind)=="mapstate"?fixture::records:std::string(kind)=="translation"?fixture::translations:fixture::puzzles;}
}
namespace modtext {
LocaleScope::LocaleScope(std::wstring s){previous_=fixture::locale;fixture::locale=std::move(s);}
LocaleScope::~LocaleScope(){fixture::locale=*previous_;}
const std::wstring& lang(){return fixture::locale;}
std::wstring T(std::wstring_view s){return std::wstring(s);}
std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring> args){
    std::wstring result(s);int index=0;
    for(const auto& arg:args){const auto token=L"{"+std::to_wstring(index++)+L"}";size_t at=0;
        while((at=result.find(token,at))!=result.npos){result.replace(at,token.size(),arg);at+=arg.size();}}
    return result;
}
}
namespace log_ {void write(const char*,...){} }
namespace field {
std::wstring current_map_name(){std::abort();}
std::wstring safe_event_label(const std::wstring& raw,const std::wstring&,int,int,int){return raw;}
}
std::string read(const char* path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(){
    using namespace ffpr::ff6::story;
    int failures=0,identities=0,plans=0;
    auto expect=[&](bool pass,const char* name){if(!pass){++failures;std::printf("FAIL %s\n",name);}};
    auto bytes=read("data/ff6/maps.bin");expect(field::baked::load(bytes.data(),bytes.size()),"FF6 bake loads");
    expect(json::parse(read("data/ff6/puzzles.json"),fixture::puzzles),"authored recipes load");
    expect(json::parse(read("data/ff6/translation.json"),fixture::translations),"real translations load");
    expect(json::parse(read("data/ff6/mapstate.json"),fixture::records),"real projected wall state loads");
    field::prepare_entity_translations();
    Snapshot state;state.locale=L"en";
    for(auto& v:state.flags.scenario)v=0;
    for(auto& v:state.flags.global)v=0;
    for(auto& v:state.flags.area)v=0;
    for(auto& v:state.flags.local)v=0;
    state.flags.treasure.fill(0);
    for(const auto& guide:fixture::puzzles.get(L"story_guides")->arr) {
        for(const auto& map:guide.get(L"maps")->arr)expect(supported_map(static_cast<int>(map.n)),"every recipe map is registered");
        for(const auto* key:{L"maze_targets",L"pattern_targets"})if(const auto* list=guide.get(key))for(const auto& row:list->arr) {
            expect(target(row).has_value(),"maze and chest controls resolve exact physical identities");++identities;
        }
        for(const auto& rule:guide.get(L"rules")->arr){
            if(const auto* at=rule.get(L"at")){expect(target(*at).has_value(),"hold rule identifies exactly one physical switch");++identities;}
            if(const auto* list=rule.get(L"targets"))for(const auto& row:list->arr){
                expect(target(row).has_value(),"guide target resolves one exact bake identity");++identities;
            }
        }
    }
    const auto has_target=[](const Plan& p,int map,int entity) {
        return std::any_of(p.targets.begin(),p.targets.end(),[&](const auto& t){return t.goal.map_id==map&&t.goal.entity_id==entity;});
    };
    std::thread worker([&]{
        const auto began=std::chrono::steady_clock::now();
        state.map=318;state.flags.scenario[13]=1;state.player={1,13,15};
        auto out=plan(state);++plans;
        expect(out.supported&&!out.finished&&out.targets.size()==7,"Terra defense retains six contact enemies and its commander");
        expect(out.targets.front().goal.entity_id==9&&out.defense&&out.defense->enemies.size()==6,"commander is the completion objective, with live threats handled separately");
        const auto* defense_map=field::baked::submap_by_map_id(318);
        state.flags.global[4]=1;
        state.parties[0].map=318;state.parties[0].position=unity::Vec3{(11-defense_map->width*.5f)*16,(defense_map->height*.5f-14)*16,0};
        state.parties[1].map=318;state.parties[1].position=unity::Vec3{0,0,0};
        state.parties[2].map=318;state.parties[2].serialized=L"{\"position\":{\"x\":"+std::to_wstring((15-defense_map->width*.5)*16)+L",\"y\":"+std::to_wstring((defense_map->height*.5-16)*16)+L"},\"direction\":2}";
        out=plan(state);++plans;
        expect(out.defense&&out.defense->parties[0]==field::baked_route::Cell{1,11,14}&&
            out.defense->parties[1]==state.player&&out.defense->parties[2]==field::baked_route::Cell{1,15,16},
            "native saved poses and serialized fallback preserve each slot; the current party uses its live cell");
        state.parties[2].serialized=L"{}";out=plan(state);++plans;
        expect(out.defense&&!out.defense->parties[2],"missing saved position stays unknown");
        state.flags.scenario[14]=1;out=plan(state);++plans;
        expect(out.finished&&!out.defense,"Terra completion removes defense advice");state.flags.scenario[14]=0;
        state.map=19;state.flags.scenario[137]=1;state.flags.scenario[138]=0;out=plan(state);++plans;
        expect(out.defense&&out.defense->protected_actor.entity_id==64&&out.targets.front().goal.entity_id==25&&out.defense->enemies.size()==12,
            "imperial army defense protects Banon and ends with Kefka");
        state.flags.scenario[138]=1;out=plan(state);++plans;
        expect(out.finished&&!out.defense,"Banon defense completion removes danger advice");
        // Unknown names take the same central resolver without native tracking.
        expect(!field::translate_entity_name_snapshot(L"未収録の検証用名称").empty(),"worker unknown-name fallback is owned text");
        state.map=498;state.flags.scenario[13]=0;state.flags.global[5]=2;
        const auto plate=target(*fixture::puzzles.get(L"story_guides")->arr[1].get(L"rules")->arr[0].get(L"at"));
        expect(plate.has_value(),"Phoenix plate location resolves");
        if(plate)state.player=plate->route.goal.cell;
        out=plan(state);++plans;
        expect(out.targets.empty()&&out.note==L"Hold switch. Switch parties.","party on a pending twin plate is briefly told to hold and switch");
        expect(!out.finish_on_exit,"Phoenix cannot arm reunion completion before the native twin latch");
        state.flags.scenario[402]=1;out=plan(state);++plans;
        expect(!out.targets.empty(),"latched twin door releases the hold instruction");
        expect(out.finish_on_exit==262,"native twin completion arms only the reunion's last map");
        state.flags.scenario[399]=1;
        for(const auto& rule:fixture::puzzles.get(L"story_guides")->arr[1].get(L"rules")->arr) {
            const auto* release=rule.get(L"release");if(!release)continue;
            if(release->arr[0].s!=L"area")continue;
            const int index=static_cast<int>(release->arr[1].n);
            state.map=498;state.flags.area.fill(0);state.flags.area[static_cast<size_t>(index)]=1;
            state.player=target(*rule.get(L"at"))->route.goal.cell;out=plan(state);++plans;
            expect(out.anticipated_release&&out.anticipated_release->area[static_cast<size_t>(index)]==0&&
                state.flags.area[static_cast<size_t>(index)]==1&&!out.hold_note.empty(),
                "each temporary plate supplies an owned release prediction without changing native/observed state");
            expect(!out.targets.empty()&&out.targets[0].goal.map_id==499&&out.targets[0].goal.entity_id==50,
                "story completion is preferred over cycling temporary switches already passed");
        }
        state.map=499;state.flags.scenario[406]=1;state.flags.scenario[407]=0;
        state.player=field::baked_route::Cell{1,0,0};out=plan(state);++plans;
        bool alternate=false;for(const auto& t:out.targets)alternate|=t.goal.map_id==499&&t.goal.entity_id==55;
        expect(!out.anticipated_release&&out.note.empty()&&alternate,
            "persistent rock toggles provide the alternate control without a false stay-on-switch instruction");
        state.flags.area.fill(0);state.flags.scenario[399]=state.flags.scenario[402]=0;state.flags.scenario[405]=1;
        state.flags.scenario[406]=state.flags.scenario[407]=0;
        out=plan(state);++plans;
        expect(has_target(out,499,58)&&!has_target(out,499,55),"initial stepping-stone completion requests the first persistent rock toggle");
        state.flags.scenario[406]=1;out=plan(state);++plans;
        expect(!has_target(out,499,58)&&has_target(out,499,55),"the other persistent rock toggle is available before draining");
        state.flags.scenario[406]=0;state.flags.scenario[407]=1;out=plan(state);++plans;
        expect(!has_target(out,499,58)&&!has_target(out,499,55),"the completed lower handoff does not oscillate between mutually resetting controls");
        state.map=498;state.flags.scenario[399]=state.flags.scenario[400]=1;state.flags.scenario[401]=0;
        state.flags.area[6]=1;state.player={1,10,32};out=plan(state);++plans;
        expect(out.anticipated_release&&out.anticipated_release->scenario[400]==0&&state.flags.scenario[400]==1&&
            has_target(out,498,44)&&!has_target(out,498,40),
            "a premature final plate releases its exact native flag and recovers through the missing rock handoff");
        state.player={1,23,19};out=plan(state);++plans;
        expect(out.targets.empty()&&!out.anticipated_release&&out.note==L"Hold switch. Switch parties.",
            "the supporting rock holder waits while the other party recovers to7");
        state.flags.area[7].reset();out=plan(state);++plans;
        expect(out.unknown&&out.targets.empty()&&!out.anticipated_release&&out.note==L"Story guidance is waiting for current game state.",
            "an unknown occupied-plate handoff cannot inherit a permissive later release rule");
        state.flags.area.fill(0);
        state.map=549;state.flags.scenario[483]=1;state.flags.scenario[486]=state.flags.scenario[487]=0;
        for(const auto& row:fixture::puzzles.get(L"story_guides")->arr[2].get(L"rules")->arr)
            if(row.get(L"leave_switch")) {
                const auto plate=target(*row.get(L"at"));state.player=plate->route.goal.cell;
                out=plan(state);++plans;
                expect(out.targets.size()==1&&out.targets[0].goal.entity_id==-1&&
                    out.targets[0].goal.cell==field::baked_route::Cell{state.player.layer,state.player.x,state.player.y+1}&&
                    out.targets[0].goal.avoid_other_events&&!out.targets[0].goal.allow_scripted_crossings,
                    "each outer Tower party leaves south of the weight landing instead of getting trapped to its north");
            }
        state.flags.scenario[483]=0;
        state.map=565;state.flags.scenario[488]=1;state.flags.scenario[491]=0;
        state.flags.scenario[486]=state.flags.scenario[487]=0;
        for(const auto& row:fixture::puzzles.get(L"story_guides")->arr[2].get(L"rules")->arr)
            if(const auto* at=row.get(L"at");at&&integer(*at,L"map")==565&&integer(*at,L"entity")==40) {
                state.player=target(*at)->route.goal.cell;break;
            }
        out=plan(state);++plans;
        expect(out.anticipated_release&&out.anticipated_release->scenario[488]==0&&
            state.flags.scenario[488]==1&&!out.targets.empty(),
            "central Tower party may leave its later plate to push weights, projecting the exact native release");
        state.flags.scenario[486]=state.flags.scenario[487]=1;
        out=plan(state);++plans;
        expect(!out.anticipated_release&&out.targets.empty()&&out.note==L"Hold switch. Switch parties.",
            "after both weights fall, the central party holds for the remaining two parties");
        for(int map:{549,565,570}) {
            state.map=map;state.flags.global[5]=3;
            for(const auto& rule:fixture::puzzles.get(L"story_guides")->arr[2].get(L"rules")->arr) {
                const auto* at=rule.get(L"at");if(!at||integer(*at,L"map")!=map)continue;
                state.player=target(*at)->route.goal.cell;out=plan(state);++plans;
                expect(out.targets.empty()&&out.note==L"Hold switch. Switch parties.","each tower plate preserves the active party's hold");
            }
        }
        state.map=566;state.flags.scenario[492]=1;state.flags.scenario[491]=1;
        for(int tutorial_seen:{0,1})for(int y:{14,12,11})for(int layer:{0,1}) {
            state.flags.scenario[7]=tutorial_seen;state.player={layer,13,y};out=plan(state);++plans;
            bool waypoint=false;for(const auto& t:out.targets)waypoint|=t.goal.map_id==566&&t.goal.entity_id==51;
            expect(waypoint==(layer==1&&y>12),
                "Guardian's save-point waypoint is explicit only on its south approach, regardless of earlier tutorial state");
        }
        Snapshot opera;opera.locale=L"en";opera.map=113;
        opera.flags.scenario.fill(0);opera.flags.global.fill(0);opera.flags.area.fill(0);opera.flags.local.fill(0);
        for(int bit:{157,158,159,165})opera.flags.scenario[static_cast<size_t>(bit)]=1;
        out=plan(opera);++plans;
        expect(has_target(out,113,13)&&!has_target(out,113,7)&&out.finish_on_scenario==177,
            "first-attempt success165 with no failure160 guides the sabotage letter and arms actual Opera completion");
        opera.flags.scenario[165]=0;opera.flags.scenario[160]=opera.flags.scenario[161]=1;
        out=plan(opera);++plans;
        expect(has_target(out,113,12)&&!has_target(out,113,7),"the first retry prepares Celes before enabling the reused stage event");
        opera.flags.scenario[164]=1;out=plan(opera);++plans;
        expect(has_target(out,113,7)&&!has_target(out,113,12),"native retry164 reuses the correct stage event despite failure160");
        opera.map=114;opera.flags.scenario[165]=opera.flags.scenario[166]=1;
        out=plan(opera);++plans;
        expect(has_target(out,114,10),"reading the first letter leads to the Impresario who starts the timed sabotage phase");
        opera.map=112;opera.flags.scenario[166]=0;opera.flags.scenario[174]=1;
        out=plan(opera);++plans;
        expect(has_target(out,112,17)&&!has_target(out,113,13),"native sabotage failure selects the actual exterior retry letter");
        opera.map=114;opera.flags.scenario[175]=1;out=plan(opera);++plans;
        expect(has_target(out,114,10),"the retry letter reaches the live Impresario whose native script selects the restart scene");
        opera.flags.scenario[157]=0;out=plan(opera);++plans;
        expect(out.finished&&out.note==L"No puzzle guide for this map"&&out.finish_on_scenario<0,
            "before Opera's native story prerequisite, an inactive guide does not announce completion");
        opera.flags.scenario[177]=1;out=plan(opera);++plans;
        expect(out.finished&&out.note==L"This story sequence is complete.","actual Opera177 classifies completed independently of earlier prerequisites");
        for(const auto [map,start,done]:{std::tuple{318,13,14},std::tuple{19,137,138}}) {
            Snapshot inactive;inactive.map=map;inactive.locale=L"en";inactive.flags.scenario.fill(0);
            auto phase=plan(inactive);++plans;
            expect(phase.finished&&phase.note==L"No puzzle guide for this map"&&phase.finish_on_scenario<0,
                "inactive defense phases do not falsely announce a completed story or arm new callbacks");
            inactive.flags.scenario[static_cast<size_t>(done)]=1;phase=plan(inactive);++plans;
            expect(phase.finished&&phase.note==L"This story sequence is complete.","defense completion requires its actual native completion flag");
        }
        state.map=115;state.player={1,0,0};state.flags.scenario[157]=1;state.flags.scenario[167]=1;
        out=plan(state);++plans;bool correct=false,wrong=false;
        for(const auto& t:out.targets)if(t.goal.map_id==115){correct|=t.goal.entity_id==6;wrong|=t.goal.entity_id!=6;}
        expect(correct&&!wrong,"Opera guide offers only the far-right lever");
        state.flags.scenario[168]=1;out=plan(state);++plans;
        correct=false;for(const auto& t:out.targets)correct|=t.goal.map_id==115;
        expect(!correct,"native lever completion removes that objective");
        state.flags.scenario[177]=1;out=plan(state);++plans;expect(out.finished,"native Opera completion ends the guide");
        state.map=318;state.flags.scenario[13].reset();out=plan(state);++plans;
        expect(out.unknown&&!out.finished&&out.targets.empty(),"missing flags never certify sequence completion");
        state.map=320;state.flags.scenario[67]=1;state.flags.scenario[297]=0;state.flags.scenario[559]=0;
        const int light_ids[]{17,20,33,29,28,30,24,23,18,19,22,35,31,32,4};
        for(int progress=0;progress<=14;++progress) {
            state.flags.local[1]=progress;out=plan(state);++plans;
            expect(out.supported&&!out.finished&&!out.unknown&&out.targets.size()==1&&
                out.targets[0].goal.map_id==320&&out.targets[0].goal.entity_id==light_ids[progress]&&out.targets[0].goal.avoid_other_events,
                "each native light stage selects exactly its demonstration/checkpoint/exit");
            expect(out.finish_on_exit==(progress==14?317:0),"only native stage14 arms actual northern-exit completion");
            expect(out.hold_note==L"Leave and re-enter this room to restart the light path.","a genuinely blocked light route has explicit restart feedback");
        }
        state.flags.scenario[559]=1;out=plan(state);++plans;
        expect(out.targets.empty()&&out.note==L"Press a direction to resolve the light trap, then use Shift+G."&&!out.finish_on_exit,
            "a native failure gives trap feedback rather than another memory route");
        state.flags.scenario[559]=0;
        for(int progress:{99,100}){state.flags.local[1]=progress;out=plan(state);++plans;
            expect(out.targets.empty()&&!out.finished&&!out.note.empty(),"both native escaped-failure values request a real restart");}
        state.flags.local[1]=1;state.flags.scenario[559].reset();out=plan(state);++plans;
        expect(out.unknown&&out.targets.empty()&&!out.finished&&out.note==L"Story guidance is waiting for current game state.",
            "unknown failure state waits audibly without certifying a safe memory route");
        state.flags.scenario[559]=0;state.flags.scenario[67]=0;out=plan(state);++plans;
        expect(out.finished&&out.note==L"No puzzle guide for this map","an inactive story phase does not claim puzzle completion");
        state.map=525;state.flags.current_map_id=525;state.player={1,29,13};state.flags.scenario[442]=1;
        state.flags.scenario[445]=0;state.flags.scenario[446]=1;state.flags.scenario[447]=0;state.flags.scenario[448]=0;
        for(int mask=0;mask<64;++mask) {
            for(int i=0;i<6;++i)state.flags.local[i+1]=(mask>>i)&1;
            out=plan(state);++plans;
            int expected=10;
            for(int i=0;i<6;++i)if(((mask>>i)&1)!=((28>>i)&1)){expected=61+i;break;}
            expect(out.supported&&!out.unknown&&!out.finished&&out.targets.size()==1&&out.targets[0].goal.entity_id==expected,
                "all64 chest states select exactly the first correction or the checking lever");
        }
        state.flags.local[1].reset();out=plan(state);++plans;
        expect(out.unknown&&out.targets.empty()&&!out.finished,"missing chest state cannot certify a completed pattern");
        state.flags.local[1]=0;state.flags.scenario[446]=0;
        state.cancelled=[]{return true;};out=plan(state);++plans;
        expect(out.targets.empty()&&!out.finished,"cancelled maze work emits no objective");state.cancelled={};
        std::printf("Story model: %d state plans, %.3f ms aggregate offline\n",plans,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
    });worker.join();
    // Keep these large owned snapshots/results out of the already substantial
    // preserved worker frame. Sequential default-stack workers, no larger RAM
    // or stack allowance and no concurrent model calls.
    std::thread car_worker([&]{
        const auto began=std::chrono::steady_clock::now();const int prior=plans;
        Plan out;
        Snapshot car;car.map=524;car.locale=L"en";car.player={1,32,10};
        car.flags.scenario.fill(0);car.flags.global.fill(0);car.flags.local.fill(0);car.flags.area.fill(0);car.flags.treasure.fill(0);
        car.flags.current_map_id=524;car.flags.scenario[442]=1;
        auto next_car=[&]{++plans;return plan(car);};
        out=next_car();
        expect(out.targets.size()==1&&has_target(out,524,26)&&out.targets[0].instruction.find(L"four-second")!=std::wstring::npos,
            "before pull one, the guide explains the complete two-pull timer-stop sequence");
        car.flags.local[0]=1;out=next_car();
        expect(out.targets.size()==1&&has_target(out,524,26)&&out.targets[0].instruction.find(L"again now")!=std::wstring::npos,
            "running timer selects an immediate second pull rather than a movement race");
        car.flags.local[0]=0;out=next_car();
        expect(out.targets.size()==1&&has_target(out,524,26),"native timeout returns to the actual initial lever objective");
        for(int local:{2,3}) {
            car.flags.local[0]=local;out=next_car();
            expect(out.targets.size()==1&&has_target(out,524,9),"both native locked/success states select the real crate chest");
        }
        car.flags.treasure[200]=1;car.flags.local[0]=0;out=next_car();
        expect(out.targets.size()==1&&has_target(out,524,7),"opened persistent crate reward survives map-local reset without redoing the lever");
        car.flags.scenario[444]=1;out=next_car();
        expect(out.targets.size()==1&&has_target(out,524,107),"native item-acquired receipt selects the moving interaction's exact identity");
        car.flags.scenario[443]=1;out=next_car();
        expect(out.targets.size()==1&&has_target(out,524,8),"native feeding receipt unlocks the guarded chest objective");
        car.flags.treasure[199]=1;out=next_car();
        expect(out.targets.size()==1&&out.targets[0].goal.map_id==525&&out.targets[0].goal.cell==field::baked_route::Cell{1,29,13},
            "first-car receipts guide the actual next-car arrival, not an unobserved future lever state");
        car.flags.scenario[448]=1;out=next_car();
        expect(out.targets.size()==1&&out.targets[0].goal.map_id==527&&out.targets[0].goal.cell==field::baked_route::Cell{2,8,12},
            "completed maze selects the real Upper locomotive arrival");
        for(int map:{522,523,526}) {
            car.map=map;out=next_car();
            expect(out.targets.size()==1&&out.targets[0].goal.map_id==527,"each intermediate train car continues to the engine after puzzle receipts");
        }
        car.map=522;car.flags.scenario[442]=0;out=next_car();
        expect(out.targets.size()==1&&has_target(out,522,1),"initial exterior story uses its native flashback trigger");
        car.map=527;out=next_car();
        expect(out.targets.size()==1&&has_target(out,527,8)&&out.finish_on_exit==528,
            "locomotive guide selects the real departure event and actual arrival completion, never decorative levers");
        car.flags.scenario[449]=1;out=next_car();
        expect(out.finished&&out.targets.empty(),"native intro449 is the complete-train story receipt");
        car.map=524;car.flags.scenario[449]=0;car.flags.scenario[442]=1;car.flags.treasure[200].reset();
        out=next_car();
        expect(out.unknown&&out.targets.empty()&&!out.finished,"unknown crate reward cannot select a later puzzle or certify completion");
        const auto* dream=fixture::puzzles.get(L"story_guides");
        bool typed_chest=false;
        for(const auto& guide:dream->arr)if(string(guide,L"id")==L"dream_train")for(const auto& r:guide.get(L"rules")->arr)
            if(const auto* list=r.get(L"targets"))for(const auto& row:list->arr)if(integer(row,L"map")==524&&integer(row,L"entity")==9)
                if(auto identity=target(row))typed_chest=identity->label==L"Treasure Chest";
        expect(typed_chest,"story treasure labels remain type-only before collection");
        std::printf("Train model: %d state plans, %.3f ms aggregate offline\n",plans-prior,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
    });car_worker.join();
    std::thread crossing_worker([&]{
        Snapshot s;s.locale=L"en";s.flags.scenario.fill(0);s.flags.global.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);s.flags.treasure.fill(0);
        const auto check=[&](int map,int target_map,int entity){s.map=map;s.flags.current_map_id=map;++plans;auto p=plan(s);
            expect(p.supported&&!p.unknown&&!p.finished&&p.targets.size()==1&&p.targets[0].goal.map_id==target_map&&(entity<0||p.targets[0].goal.entity_id==entity),"complete Zone Eater state selects its exact next native objective");
            expect(p.targets.size()==1&&p.targets[0].goal.avoid_other_events&&p.targets[0].goal.allow_scripted_crossings,"crossings allow reviewed jumps but never unrelated trigger shortcuts");return p;};
        check(500,501,-1);check(501,501,6);check(502,502,8);
        s.flags.local[1]=1;check(502,502,2);s.flags.local[1].reset();++plans;auto p=plan(s);
        expect(p.unknown&&p.targets.empty(),"unknown recovery local cannot invent an opened return");s.flags.local[1]=0;
        check(503,504,-1);check(505,506,-1);check(506,506,25);
        s.flags.scenario[409]=1;check(506,506,2);s.flags.scenario[409].reset();++plans;p=plan(s);
        expect(p.unknown&&p.targets.empty(),"unknown footing receipt never skips its switch");s.flags.scenario[409]=1;
        p=check(507,507,1);expect(p.finish_on_scenario==410&&!p.finished,"reaching Gogo is not native joining completion");
        s.flags.scenario[410]=1;++plans;p=plan(s);expect(p.finished&&p.targets.empty(),"actual joining receipt completes the story guide");
        s.flags.scenario[410].reset();++plans;p=plan(s);expect(p.unknown&&p.targets.empty()&&!p.finished,"missing joining state cannot claim completion");
    });crossing_worker.join();
    std::thread phantom_worker([&]{
        Snapshot s;s.locale=L"en";s.flags.scenario.fill(0);s.flags.global.fill(0);s.flags.local.fill(0);s.flags.area.fill(0);s.flags.treasure.fill(0);
        const auto check=[&](int map,int goal_map,int entity){s.map=map;s.flags.current_map_id=map;++plans;const auto p=plan(s);
            expect(p.supported&&!p.unknown&&!p.targets.empty()&&has_target(p,goal_map,entity),"ordinary train native receipts select the next real stage");return p;};
        check(355,355,1);s.flags.scenario[107]=1;check(355,355,2);
        s.flags.scenario[108]=1;check(357,357,2);s.flags.scenario[109]=1;check(357,361,8);
        s.flags.scenario[519]=1;check(361,361,6);s.flags.scenario[110]=1;check(624,624,1);
        s.flags.scenario[111]=1;check(624,624,2);s.flags.scenario[112]=1;check(624,624,3);
        s.flags.scenario[113]=1;check(362,362,20);s.flags.scenario[114]=1;check(624,624,22);check(362,362,20);
        s.flags.scenario[115]=1;
        for(int mask=0;mask<8;++mask){for(int i=0;i<3;++i)s.flags.scenario[120+i]=(mask>>i)&1;
            s.map=371;++plans;const auto p=plan(s);expect(!p.targets.empty()&&!p.unknown,"all eight engine states retain an actionable native objective");
            if(mask==5)expect(p.targets.size()==1&&has_target(p,356,5),"only original1/0/1 opens the boss objective");}
        s.flags.scenario[123]=1;const auto done=check(372,372,1);expect(done.finish_on_exit==1&&!done.finished,"final conversation arms only actual world1 readiness");
        s.flags.scenario[123].reset();s.map=372;++plans;const auto unknown=plan(s);expect(unknown.unknown&&unknown.targets.empty(),"unknown final receipt cannot skip the scene or invent departure");
    });phantom_worker.join();
    std::thread darill_worker([&]{
        Snapshot s;s.locale=L"en";s.flags.scenario.fill(0);s.flags.global.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);s.flags.treasure.fill(0);
        const auto check=[&](int map,int goal_map,int entity){s.map=map;s.flags.current_map_id=map;++plans;auto p=plan(s);expect(p.supported&&!p.finished&&!p.unknown&&p.targets.size()==1&&has_target(p,goal_map,entity),"Darill receipt selects the next complete puzzle/story objective");return p;};
        check(456,458,1);s.flags.scenario[341]=1;check(458,459,1);s.flags.scenario[342]=1;
        for(int bit=346;bit<=349;++bit){const auto p=check(464,464,0);expect(p.targets[0].goal.raw_name==L"石碑を調べる"+std::to_wstring(bit-345),"all four distinct epitaph actors retain their exact identities");s.flags.scenario[bit]=1;}
        const auto answer=check(466,466,0);expect(answer.targets[0].instruction==L"Examine the gravestone. Choose Yes, then options 4, 1, 2, 1.","actual shrinking menus use native4/1/2/1, not a guessed reversed order");
        s.flags.scenario[350]=1;check(466,461,9);s.flags.treasure[169]=1;check(461,463,24);
        s.flags.scenario[343]=1;check(463,463,1);s.flags.scenario[344]=1;check(463,463,22);
        s.flags.area[1]=1;check(463,468,4);s.flags.scenario[352]=1;check(468,468,5);
        check(577,577,2);s.flags.scenario[528]=1;const auto departure=check(577,577,2);expect(departure.finish_on_exit==2,"memory scene arms actual ready world2, not movie-only world1");
        s.flags.scenario[353]=1;s.map=456;++plans;auto p=plan(s);expect(p.finished&&!p.unknown&&p.targets.empty(),"story353, epitaph350 and chest169 together complete the whole guide");
        s.flags.treasure[169]=0;check(456,461,9);s.flags.treasure[169].reset();++plans;p=plan(s);expect(p.unknown&&!p.finished&&p.targets.empty(),"unknown treasure cannot invent full puzzle completion");
        s.flags.treasure[169]=1;s.flags.scenario[350]=0;check(466,466,0);
        s.flags.scenario[350].reset();++plans;p=plan(s);expect(p.unknown&&!p.finished&&p.targets.empty(),"unknown epitaph receipt cannot certify completion");
        s.flags.scenario[350]=0;s.flags.scenario[346]=0;check(464,464,0);
        s.flags.scenario[350]=1;s.flags.scenario[353]=0;s.flags.area[1]=0;s.flags.scenario[352]=0;s.flags.scenario[346]=1;check(463,463,22);
        s.flags.area[1].reset();++plans;p=plan(s);expect(p.unknown&&!p.finished&&p.targets.empty(),"unknown native turtle bank never invents a ride or a remote boss route");
    });darill_worker.join();
    std::thread cultists_worker([&]{
        Snapshot s;s.locale=L"en";s.flags.scenario.fill(0);s.flags.global.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);s.flags.treasure.fill(0);s.flags.current_area_id=59;s.flags.current_map_id=476;
        s.flags.scenario[392]=1;auto projection=field::map_state::project(*fixture::records.get(L"Map_30700"),s.flags);
        expect(projection.known&&!projection.groups.count(L"ev_e_0776")&&!projection.groups.count(L"ev_e_0001"),"completed recruitment preserves common barrier absence across unknown membership");
        s.flags.scenario[392]=0;projection=field::map_state::project(*fixture::records.get(L"Map_30700"),s.flags);
        expect(!projection.known&&projection.groups.count(L"ev_e_0001")&&!projection.groups.count(L"ev_e_0776"),"pre-recruit remote membership stays unknown rather than inventing or denying the conditional event");
        for(int map=476;map<=487;++map){
            s.map=s.flags.current_map_id=map;++plans;
            expect(!plan(s).supported,"Cultists Tower's ordinary switch and treasure routes do not start a guide");
        }
    });cultists_worker.join();
    std::thread owzer_worker([&]{
        Snapshot s;s.locale=L"en";s.flags.scenario.fill(0);s.flags.global.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);s.flags.treasure.fill(0);s.flags.current_area_id=36;s.flags.scenario[298]=1;s.player={1,25,27};
        const auto check=[&](int map,int target_map,int entity){s.map=map;s.flags.current_map_id=map;++plans;auto p=plan(s);expect(p.supported&&!p.finished&&!p.unknown&&p.targets.size()==1&&has_target(p,target_map,entity),"Owzer exact native progress selects one next operation");return p;};
        for(const auto pair:std::vector<std::pair<int,int>>{{379,19},{577,20},{380,18},{381,15},{382,13}}){check(442,442,pair.second);s.flags.scenario[pair.first]=1;}
        check(443,444,2);s.flags.scenario[384]=1;check(444,445,-1);check(445,445,88);s.flags.local[2]=1;check(445,445,-1);
        s.player={1,23,23};++plans;auto p=plan(s);expect(p.owzer_door_wait&&p.targets.empty()&&!p.note.empty(),"timed-door staging offers a preview, never an already stale route through a closed door");
        s.flags.local[2]=2;check(445,445,56);s.flags.area[1]=1;s.player={1,6,24};
        for(const auto pair:std::vector<std::pair<int,int>>{{385,43},{386,45},{387,47},{388,49}}){check(445,445,pair.second);s.flags.scenario[pair.first]=1;}
        check(445,445,1);s.flags.scenario[389]=1;check(445,446,7);s.flags.scenario[390]=1;s.player={1,11,14};check(446,446,8);
        s.player={1,11,35};check(446,446,9);s.player={1,11,18};check(446,446,8);
        s.flags.scenario[391]=1;++plans;p=plan(s);expect(p.finished&&p.targets.empty(),"bookshelf391, not boss390, owns the actual story endpoint");
        s.flags.scenario[385]=0;check(446,445,-1);s.flags.area[1]=0;p=check(445,445,56);expect(p.finish_on_scenario<0,"late unfinished map puzzle never arms an already-set story flag");s.flags.area[1]=1;check(445,445,43);s.flags.scenario[385]=1;
        s.flags.scenario[391]=0;s.flags.scenario[298]=0;s.flags.scenario[378]=1;const auto final=check(446,446,8);expect(final.finish_on_scenario==391,"only the actual bookshelf objective arms native391 completion");
        s.flags.scenario[378]=0;++plans;p=plan(s);expect(p.targets.empty()&&p.note==L"Story gated","native298 OR378 eligibility never becomes an AND or a guessed unlock");
        s.flags.scenario[298].reset();++plans;p=plan(s);expect(p.unknown&&!p.finished&&p.targets.empty(),"unknown native eligibility cannot invent a painting route");
    });owzer_worker.join();
    std::thread clock_worker([&]{
        Snapshot s;s.locale=L"en";s.map=386;s.flags.scenario.fill(0);s.flags.global.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);s.flags.treasure.fill(0);s.flags.current_area_id=38;s.flags.current_map_id=386;s.player={1,7,23};
        for(int score=0;score<=3;++score){s.flags.local[1]=score;++plans;auto p=plan(s);expect(p.supported&&!p.finished&&!p.unknown&&p.targets.size()==2&&has_target(p,386,11)&&has_target(p,386,14),"clock and actual reward remain explicit physical operations; solving the clock alone never finishes the guide");}
        s.map=387;s.flags.current_map_id=387;++plans;auto p=plan(s);expect(p.supported&&!p.finished&&p.targets.size()==1&&has_target(p,386,-1),"hint clock routes to the native solution-floor entrance, not its clue object");
        for(int map:{386,387}){s.map=map;s.flags.treasure[115]=1;++plans;p=plan(s);expect(p.finished&&p.targets.empty(),"actual native chest115 completes the guide on both floors despite local resets");}
        s.flags.treasure[115].reset();++plans;p=plan(s);expect(p.unknown&&!p.finished&&p.targets.empty(),"unknown treasure state never invents clock-puzzle completion");
    });clock_worker.join();
    std::thread sealed_worker([&]{
        using Cell=field::baked_route::Cell;Snapshot s;s.locale=L"en";s.map=407;
        s.flags.scenario.fill(0);s.flags.treasure.fill(0);s.flags.local.fill(0);s.flags.area.fill(0);s.flags.global.fill(0);
        s.flags.current_map_id=407;s.flags.current_area_id=63;
        const json::Value* guide=nullptr;for(const auto& g:fixture::puzzles.get(L"story_guides")->arr)if(g.get(L"id")->s==L"sealed_gate")guide=&g;
        expect(guide!=nullptr,"complete Sealed Gate recipe exists");if(!guide)return;
        for(int mode:{1,2})for(int held:{0,1}){
            s.flags.local[1]=mode;s.flags.local[mode==1?3:2]=held;
            const auto* bridge=guide->get(L"bridges")->get(std::to_wstring(mode));
            for(const auto& row:bridge->get(L"legs")->arr){
                const auto& points=row.get(L"cells")->arr;s.player={1,int(points[0].arr[0].n),int(points[0].arr[1].n)};
                ++plans;auto p=plan(s);expect(p.supported&&!p.unknown&&p.bridge&&p.bridge->phase==int(row.get(L"phase")->n)&&p.bridge->cells.front()==s.player&&p.bridge->cells.size()==points.size(),"both retained native states select the exact next cardinal bridge crossing");
                if(points.size()>2){
                    s.flags.local[mode==1?2:3]=int(row.get(L"phase")->n);s.player={1,int(points[1].arr[0].n),int(points[1].arr[1].n)};++plans;p=plan(s);
                    bool stable=false;for(const auto& at:bridge->get(L"safe_cells")->arr)stable|=s.player==Cell{1,int(at.arr[0].n),int(at.arr[1].n)};
                    expect(stable?(!p.unknown&&(p.bridge||!p.note.empty()||!p.targets.empty())):(p.bridge&&p.bridge->immediate&&p.bridge->cells.front()==s.player),"only a native-proven stable intermediate tile may wait; temporary tiles get immediate suffixes");
                }
            }
            for(const auto& row:bridge->get(L"recoveries")->arr){
                const auto& points=row.get(L"cells")->arr;s.player={1,int(points[0].arr[0].n),int(points[0].arr[1].n)};
                ++plans;const auto p=plan(s);expect(p.supported&&!p.unknown&&(p.bridge||!p.note.empty()||!p.targets.empty()),"qualified stable-cell recovery has bounded advice without engine access");
                if(p.bridge)expect(p.bridge->cells.front()==s.player&&p.bridge->cells.size()<=7&&!p.bridge->immediate,"stable recovery previews a short crossing and requires deliberate readiness");
            }
        }
        s.flags.local.fill(0);s.player={1,7,8};++plans;auto p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.cell==Cell{1,8,4},"native fall reset routes back to the actual bridge start");
        s.flags.local[1]=1;s.player={1,15,4};++plans;p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.entity_id==9,"completed first bridge reaches the native second-bridge activation");
        s.flags.local[1]=2;s.player={1,18,13};++plans;p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.entity_id==3,"completed second bridge routes through the actual floor exit");
        s.flags.local[1].reset();++plans;p=plan(s);expect(p.unknown&&!p.bridge,"unknown bridge mode cannot invent safe timing");
        s.map=411;s.flags.local.fill(0);s.flags.scenario[225]=1;++plans;p=plan(s);expect(!p.finished&&!p.targets.empty(),"story225 cannot erase unfinished cave map puzzles");
        for(int flag:{218,219,220,221,222,223,224})s.flags.scenario[flag]=1;s.flags.treasure[140]=1;++plans;p=plan(s);expect(p.finished,"all persistent puzzle receipts and the native story scene complete the full guide");
        s.flags.scenario[225]=0;++plans;p=plan(s);expect(p.finish_on_scenario==225,"only fully qualified final story stage arms native completion");
        auto saved=fixture::puzzles;const_cast<json::Value*>(fixture::puzzles.get(L"story_guides"))->arr.resize(49);++plans;p=plan(s);expect(!p.supported,"finite48-guide capacity still rejects oversized input");fixture::puzzles=std::move(saved);
    });sealed_worker.join();
    const auto floating_cases=[&]{
        json::Value cases;expect(json::parse(read("tools/floating_continent_cases.json"),cases),"factual Floating Continent journeys load");
        const auto apply=[](field::map_state::Flags& flags,const json::Value* values){
            if(!values)return;
            const auto bank=[&](const wchar_t* name,auto& into){if(const auto* rows=values->get(name))for(const auto& row:rows->arr)into.at(size_t(row.arr[0].n))=int(row.arr[1].n);};
            bank(L"scenario",flags.scenario);bank(L"area",flags.area);bank(L"local",flags.local);bank(L"treasure",flags.treasure);bank(L"global",flags.global);
        };
        for(const auto& row:cases.arr){
            Snapshot s;s.locale=L"en";s.map=row.get(L"map")?int(row.get(L"map")->n):431;
            s.flags.scenario.fill(0);s.flags.treasure.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);s.flags.global.fill(0);s.flags.scenario[286]=1;
            const auto& start=row.get(L"start")->arr;s.player={int(start[0].n),int(start[1].n),int(start[2].n)};apply(s.flags,row.get(L"initial"));
            for(const auto& step:row.get(L"steps")->arr){
                const int id=int(step.get(L"entity")->n);++plans;const auto p=plan(s);
                expect(p.supported&&!p.unknown&&!p.finished,"Floating Continent native receipt state has valid unfinished guidance");
                bool found=false;for(const auto& target:p.targets)found|=target.goal.entity_id==id;
                expect(found,"every full journey/resume includes its actual next native objective");
                if(id==74)expect(!has_target(p,s.map,155),"exact native decline landing retires the otherwise receipt-free airship fallback");
                if(id==10)expect(!has_target(p,s.map,21)&&!has_target(p,s.map,22),"escape chest must be collected before either irreversible collapse trigger");
                if(id==-1)expect(p.targets.size()==1&&p.targets[0].goal.map_id==431&&p.targets[0].goal.cell==field::baked_route::Cell{2,90,50},"save floor uses actual431 PointIn3 and final native landing");
                if(id==11)expect(p.finish_on_exit==191&&p.finish_through.size()==11,"escape edge arms final ready191 through the exact catastrophe chain");
                apply(s.flags,step.get(L"after"));if(const auto* at=step.get(L"landing"))s.player={int(at->arr[0].n),int(at->arr[1].n),int(at->arr[2].n)};
            }
        }
        Snapshot s;s.locale=L"en";s.map=631;s.flags.scenario.fill(0);s.flags.treasure.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);s.flags.global.fill(0);s.flags.scenario[292]=1;s.flags.area[1]=10;
        s.flags.treasure[152].reset();++plans;auto p=plan(s);expect(p.unknown&&!p.finished&&!has_target(p,631,21),"unknown escape treasure cannot permit an irreversible collapse early");
        s.flags.treasure[152]=0;s.flags.area[1]=11;++plans;p=plan(s);expect(has_target(p,631,17)&&!has_target(p,631,10),"missed chest behind actual collapse walls cannot strand the resumed escape");
        s.flags.scenario[295]=s.flags.scenario[296]=s.flags.scenario[298]=1;++plans;p=plan(s);expect(!p.finished&&has_target(p,631,11),"Shadow's arrival alone cannot prematurely finish the native catastrophe sequence");
        s.flags.scenario[297]=1;++plans;p=plan(s);expect(p.finished,"actual final native story receipt completes the Floating Continent journey");
    };std::thread floating_worker(floating_cases);floating_worker.join();
    std::thread type_worker([&] {
        Snapshot s;s.locale=L"en";s.map=466;s.flags.scenario.fill(0);s.flags.treasure.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);s.flags.global.fill(0);
        for(int bit:{341,342,346,347,348,349})s.flags.scenario[bit]=1;
        s.guidance={true,false};auto p=plan(s);
        expect(p.targets.empty()&&p.note==L"Non-navigation puzzle guidance is off"&&!p.finished,"a disabled epitaph answer cannot fall through to the next unsolved stage");
        s.guidance={false,true};p=plan(s);
        expect(p.targets.size()==1&&p.targets[0].instruction.find(L"4, 1, 2, 1")!=std::wstring::npos&&p.puzzle_types==2,"epitaph solution remains available independently of navigation hints");
        s.flags.scenario[350]=1;p=plan(s);
        expect(p.targets.empty()&&p.note==L"Navigation puzzle guidance is off","after a real answer receipt, the disabled traversal stage stays withheld");
        s.guidance={true,false};p=plan(s);
        expect(!p.targets.empty()&&p.puzzle_types==1,"a player who solves the answer manually can resume enabled traversal");
        s.map=113;s.flags.scenario.fill(0);for(int bit:{157,158,159,165})s.flags.scenario[bit]=1;
        p=plan(s);expect(!p.targets.empty()&&p.puzzle_types==1,"Opera letter and catwalk guidance survives disabling lyric solutions");
        s.guidance={false,true};p=plan(s);expect(p.targets.empty()&&p.note==L"Navigation puzzle guidance is off","Opera cannot skip disabled navigation to a later story stage");
        s.map=386;s.guidance={true,false};s.flags.scenario.fill(0);p=plan(s);
        expect(p.targets.empty()&&p.note==L"Non-navigation puzzle guidance is off","clock solution is classified independently from its route directions");
        s.guidance={false,false};s.cancelled=[] {return false;};p=plan(s);
        expect(p.targets.empty()&&p.note==L"Puzzle guidance is off","disabled types produce no worker target or timing plan");
    });type_worker.join();
    std::thread mobliz_worker([&] {
        Snapshot s;s.locale=L"en";s.map=90;s.flags.scenario.fill(0);s.flags.treasure.fill(0);s.flags.global.fill(0);s.flags.area.fill(0);s.flags.local.fill(0);
        auto p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.entity_id==9,"letters start with the actual wounded soldier");s.flags.scenario[502]=1;
        for(const auto row:{std::array{503,504,505,8},std::array{506,507,508,7},std::array{509,510,511,6},std::array{512,513,514,5},std::array{515,516,-1,4}}) {
            p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.map_id==90&&p.targets[0].goal.entity_id==row[3]&&p.puzzle_types==2,"each native letter is its exact current animated target");
            s.flags.scenario[size_t(row[0])]=1;p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.map_id==88&&p.targets[0].instruction==L"Choose the first option.","read receipt leads to the actual postman/first-option payment");
            const auto unchanged=plan(s);expect(unchanged.targets.size()==1&&unchanged.targets[0].goal.entity_id==10,"refusal or failed payment cannot advance without a native paid receipt");
            s.flags.scenario[size_t(row[1])]=1;
            if(row[2]>=0){p=plan(s);expect(p.targets.size()==4&&p.targets[0].instruction.find(L"close the shop menu")!=std::wstring::npos,"waiting reply is obtained from real shop-close callbacks");s.flags.scenario[size_t(row[2])]=1;}
        }
        p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.entity_id==9&&!p.finished&&p.finish_on_scenario==517,"five payments still require the real soldier reward scene");
        s.flags.scenario[517]=1;expect(plan(s).finished,"only actual517 completes letters");
        s.map=207;s.flags.scenario.fill(0);p=plan(s);expect(!p.supported&&p.targets.empty()&&p.note==L"No puzzle guide for this map","hidden-house guide does not open before native airship353");
        s.map=176;p=plan(s);expect(!p.supported&&p.note==L"No puzzle guide for this map","pre-invitation Palace explicit request retains a useful no-guide response");
        s.map=207;s.flags.scenario[353]=1;
        for(auto [receipt,expected]:{std::pair{316,57},std::pair{317,46},std::pair{318,66},std::pair{319,44},std::pair{320,43}}){p=plan(s);expect(!p.targets.empty()&&p.targets[0].goal.entity_id==expected,"later hidden-house guide retains a genuinely missed first-visit prerequisite");s.flags.scenario[size_t(receipt)]=1;}
        s.flags.area[1]=1;p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.map_id==207&&p.targets[0].goal.entity_id==6,"gift area-local guard requires a real village exit");
        s.map=2;p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.map_id==207&&p.targets[0].goal.entity_id<0,"world return routes the actual village entrance");
        s.map=207;s.flags.area[1]=0;p=plan(s);expect(p.targets.size()==3&&p.targets[0].goal.map_id==210,"actual re-entry permits the dog passage regions");
        s.flags.scenario[354]=1;p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.map_id==213&&p.targets[0].goal.entity_id==1,"dog receipt leads through the secret passage to discovery");
        s.flags.scenario[355]=1;p=plan(s);expect(p.targets.size()==1&&p.targets[0].goal.entity_id==9&&!p.finished&&p.finish_on_scenario==356,"discovery alone cannot finish before the native final battle scene");
        s.flags.scenario[356]=1;expect(plan(s).finished,"final356 completes full hidden-house journey");
    });mobliz_worker.join();
    std::printf("FF6 story check: %d physical identities, %d failures\n",identities,failures);return failures?1:0;
}
