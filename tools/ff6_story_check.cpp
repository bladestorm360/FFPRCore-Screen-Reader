// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/entity_translator.cpp field/npc_names.cpp json.cpp text.cpp
// Actual owned rule model, all authored targets and real FF6 state/bake data.
// Translating an unknown Japanese name must never touch a native map on a worker.
#include "../src/games/ff6/story_model.cpp"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <thread>
namespace fixture {json::Value puzzles,translations;std::wstring locale=L"en";}
namespace data {
bool game_data_ready(){return true;}
const json::Value& game_json(const char* kind){return std::string(kind)=="translation"?fixture::translations:fixture::puzzles;}
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
    field::prepare_entity_translations();
    Snapshot state;state.locale=L"en";
    for(auto& v:state.flags.scenario)v=0;
    for(auto& v:state.flags.global)v=0;
    for(auto& v:state.flags.area)v=0;
    for(auto& v:state.flags.local)v=0;
    for(const auto& guide:fixture::puzzles.get(L"story_guides")->arr) {
        for(const auto& map:guide.get(L"maps")->arr)expect(supported_map(static_cast<int>(map.n)),"every recipe map is registered");
        for(const auto& rule:guide.get(L"rules")->arr){
            if(const auto* at=rule.get(L"at")){expect(target(*at).has_value(),"hold rule identifies exactly one physical switch");++identities;}
            if(const auto* list=rule.get(L"targets"))for(const auto& row:list->arr){
                expect(target(row).has_value(),"guide target resolves one exact bake identity");++identities;
            }
        }
    }
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
        state.flags.scenario[402]=1;out=plan(state);++plans;
        expect(!out.targets.empty(),"latched twin door releases the hold instruction");
        for(const auto& rule:fixture::puzzles.get(L"story_guides")->arr[1].get(L"rules")->arr) {
            const auto* release=rule.get(L"release");if(!release)continue;
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
        expect(!out.anticipated_release&&out.hold_note.empty()&&alternate,
            "persistent rock toggles provide the alternate control without a false stay-on-switch instruction");
        for(int map:{549,565,570}) {
            state.map=map;state.flags.global[5]=3;
            for(const auto& rule:fixture::puzzles.get(L"story_guides")->arr[2].get(L"rules")->arr) {
                const auto* at=rule.get(L"at");if(!at||integer(*at,L"map")!=map)continue;
                state.player=target(*at)->route.goal.cell;out=plan(state);++plans;
                expect(out.targets.empty()&&out.note==L"Hold switch. Switch parties.","each tower plate preserves the active party's hold");
            }
        }
        state.map=115;state.player={1,0,0};state.flags.scenario[167]=1;
        out=plan(state);++plans;bool correct=false,wrong=false;
        for(const auto& t:out.targets)if(t.goal.map_id==115){correct|=t.goal.entity_id==6;wrong|=t.goal.entity_id!=6;}
        expect(correct&&!wrong,"Opera guide offers only the far-right lever");
        state.flags.scenario[168]=1;out=plan(state);++plans;
        correct=false;for(const auto& t:out.targets)correct|=t.goal.map_id==115;
        expect(!correct,"native lever completion removes that objective");
        state.flags.scenario[177]=1;out=plan(state);++plans;expect(out.finished,"native Opera completion ends the guide");
        state.map=318;state.flags.scenario[13].reset();out=plan(state);++plans;
        expect(out.unknown&&!out.finished&&out.targets.empty(),"missing flags never certify sequence completion");
        std::printf("Story model: %d state plans, %.3f ms aggregate offline\n",plans,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
    });worker.join();
    std::printf("FF6 story check: %d physical identities, %d failures\n",identities,failures);return failures?1:0;
}
