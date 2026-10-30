// check-sources: text.cpp ui/navigation_buffer.cpp
// Actual native landing-guide detours; no game/engine/device is launched.
#define main retained_party_fixture_main
#include "party_esper_check.cpp"
#undef main
#include "../src/core/fieldnav/vehicle_landing.cpp"

namespace field {
VehicleState fixture_vehicle;
int fixture_map=4;
fixture::Node fixture_player;
int current_map_id(){fixture::native();return fixture_map;}
void* player(){fixture::native();return &fixture_player;}
std::optional<int> fixture_move;
std::optional<bool> fixture_asset;
VehicleState current_vehicle(){fixture::native();return fixture_vehicle;}
std::optional<int> player_movement_state(){fixture::native();return fixture_move;}
std::optional<bool> player_uses_vehicle_asset(){fixture::native();return fixture_asset;}
std::wstring vehicle_game_name(int,const std::wstring&){fixture::native();return {};}
}
namespace ffpr::fieldnav::move_state {
bool is_on_foot(){return true;} // deliberate stale cache during boarding/loaded saves
void set_on_foot(){}void set_vehicle_state(int,const std::wstring&){}void reset(){}
}
namespace landing_fixture {
int originals=0;
int attribute=4,attribute_reads=0;
bool attribute_throws=false;
unity::Vec3 cell{3,4,0};
unity::Vec3 to_cell(void*,unity::Vec3,const void*){fixture::native();return cell;}
int get_attribute(void*,unity::Vec2 cell,const void*){
    fixture::native();++attribute_reads;
    if(attribute_throws)throw 1;
    return cell.x==3&&cell.y==4?attribute:0;
}
bool ready=true,blocked=false;
void show(void* self,bool,const void*){
    ++originals;static_cast<fixture::Node*>(fixture::get(*static_cast<fixture::Node*>(self),"landingGuide"))->active=true;
}
void change(void*,bool,const void*){++originals;}
void hide(void* self,const void*){
    ++originals;static_cast<fixture::Node*>(fixture::get(*static_cast<fixture::Node*>(self),"landingGuide"))->active=false;
}
}
namespace unity {Vec3 position(void*){fixture::native();return {16,32,0};}}
namespace ffpr::system {
bool is_field_navigation_ready(){fixture::native();return landing_fixture::ready;}
bool input_transition_blocked(){fixture::native();return landing_fixture::blocked;}
}
int main(){
    using namespace fixture;using namespace ffpr::fieldnav::vehicle_landing;
    int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    o_ShowLandingGuide=&landing_fixture::show;o_SwitchLandable=&landing_fixture::change;o_HideLandingGuide=&landing_fixture::hide;
    for(int n=1;n<=6;++n){
        fixture::game=static_cast<game::Id>(n);Node manager,guide,other,hidden;
        guide.active=hidden.active=false;set(manager,"landingGuide",&guide);set(other,"landingGuide",&hidden);
        landing_fixture::ready=true;landing_fixture::blocked=false;
        field::fixture_vehicle={true,3,3,{}};
        reset_landing_state();spoken.clear();landing_fixture::originals=0;
        d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken==std::vector<std::wstring>{L"Can land"},"native Show path speaks with stale on-foot cache; no Switch callback required");
        expect(last_announcement_time()>=0,"shared FF6 priority timestamp follows the actual spoken edge");
        d_ShowLandingGuide(&manager,true,nullptr);d_SwitchLandable(&manager,true,nullptr);
        expect(spoken.size()==1,"both native producers share one state edge, not repeated text suppression");
        d_SwitchLandable(&other,false,nullptr);d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.size()==1,"another hidden guide cannot reset the active guide's edge");
        d_ShowLandingGuide(&manager,false,nullptr);d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.size()==2,"leaving and re-entering landable terrain announces the new edge");
        d_HideLandingGuide(&manager,nullptr);d_SwitchLandable(&manager,true,nullptr);
        expect(spoken.size()==2,"hidden guide updates remain silent");
        d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.size()==3,"a new visible landing guide after disembarking can announce again");
        reset_landing_state();d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.size()==4,"map/battle context reset permits the fresh visible guide");
        expect(landing_fixture::originals==11,"every native operation runs exactly once");

        reset_landing_state();spoken.clear();
        const double before_preview=last_announcement_time();
        for(int type:{0,1,4,5,13,-1}) {
            field::fixture_vehicle={true,2,type,{}};
            d_ShowLandingGuide(&manager,true,nullptr);
            d_SwitchLandable(&manager,false,nullptr);d_SwitchLandable(&manager,true,nullptr);
        }
        field::fixture_vehicle={};d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.empty()&&last_announcement_time()==before_preview,"foot, automatic canoe, markers, invalid and unknown vehicles stay silent without claiming speech priority");
        for(int type:{2,3,6,7,8,9,10,11,12}) {
            field::fixture_vehicle={true,20+type,type,{}};
            d_ShowLandingGuide(&manager,true,nullptr);
            const auto count=spoken.size();
            d_SwitchLandable(&manager,true,nullptr);
            expect(spoken.size()==count,"same mounted model retains its guide edge");
        }
        expect(spoken.size()==9,"native manual mounted vehicles each establish a fresh landing context");
        field::fixture_vehicle={true,1,1,{}};d_ShowLandingGuide(&manager,true,nullptr);
        field::fixture_vehicle={true,3,3,{}};d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.size()==10,"boarding after an on-foot preview cannot inherit its landable state");

        // Reproduce the own-log sequence: guide initialization between native
        // ChangeMap1 and FieldReady2, then Player3 without a movement edge.
        reset_landing_state();spoken.clear();landing_fixture::originals=0;
        const double prior_speech=last_announcement_time();
        landing_fixture::ready=false;
        d_ShowLandingGuide(&manager,true,nullptr);d_SwitchLandable(&manager,true,nullptr);
        expect(spoken.empty()&&last_announcement_time()==prior_speech,"loading a landable guide is silent and cannot claim FF6 speech priority");
        landing_fixture::ready=true;
        d_ShowLandingGuide(&manager,true,nullptr);d_SwitchLandable(&manager,true,nullptr);
        expect(spoken.empty(),"control returning does not replay the loading-time state");
        d_ShowLandingGuide(&manager,false,nullptr);d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken==std::vector<std::wstring>{L"Can land"},"a real landability edge after loading still announces");
        expect(landing_fixture::originals==6,"loading suppression preserves each native operation exactly once");

        // Ready Player/Transport state alone is insufficient during a fade or
        // queued transition. The shared input-context gate owns those cases.
        for(bool initial:{false,true}) {
            reset_landing_state();spoken.clear();landing_fixture::originals=0;
            landing_fixture::blocked=true;
            d_ShowLandingGuide(&manager,initial,nullptr);
            d_ShowLandingGuide(&manager,true,nullptr);
            expect(spoken.empty(),"native initialization/changes during fades remain silent");
            landing_fixture::blocked=false;d_ShowLandingGuide(&manager,true,nullptr);
            expect(spoken.empty(),"fade completion cannot replay an already observed landable state");
            d_ShowLandingGuide(&manager,false,nullptr);d_SwitchLandable(&manager,true,nullptr);
            expect(spoken==std::vector<std::wstring>{L"Can land"},"Switch producer still speaks a new live edge after a fade");
            expect(landing_fixture::originals==5,"fade suppression never skips the native producer");
        }
        for(bool initial:{false,true}) {
            reset_landing_state();spoken.clear();landing_fixture::ready=false;
            d_ShowLandingGuide(&manager,initial,nullptr);
            landing_fixture::ready=true;d_ShowLandingGuide(&manager,initial,nullptr);
            expect(spoken.empty(),"initial unavailable or landable baseline stays silent after load/menu/battle return");
            d_ShowLandingGuide(&manager,false,nullptr);d_ShowLandingGuide(&manager,true,nullptr);
            expect(spoken==std::vector<std::wstring>{L"Can land"},"subsequent live movement keeps landing guidance in either initial state");
        }
    }
    {
        fixture::game=game::Id::FF3;field::fixture_vehicle={true,9,3,{}};
        field::fixture_map=4;Node manager,guide,map,controller;
        guide.active=true;set(manager,"landingGuide",&guide);set(map,"fieldController",&controller);
        owners["FieldMap"]=&map;
        extra_methods["ConvertWorldPositionToCellPosition"]=reinterpret_cast<void*>(&landing_fixture::to_cell);
        extra_methods["GetCellAttribute"]=reinterpret_cast<void*>(&landing_fixture::get_attribute);
        landing_fixture::ready=true;landing_fixture::blocked=false;landing_fixture::attribute=4;
        reset_landing_state();spoken.clear();d_ShowLandingGuide(&manager,true,nullptr);
        landing_fixture::attribute=16;d_ShowLandingGuide(&manager,true,nullptr);d_SwitchLandable(&manager,true,nullptr);
        expect(spoken==std::vector<std::wstring>{L"Can land",L"Can dive"},"Nautilus changes land to dive while the native guide stays available, with one action edge across both producers");
        landing_fixture::attribute=14;d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken==std::vector<std::wstring>{L"Can land",L"Can dive",L"Can land"},"Nautilus changes dive back to ordinary landing without a false unavailable edge");
        d_ShowLandingGuide(&manager,false,nullptr);landing_fixture::attribute=16;d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.back()==L"Can dive"&&spoken.size()==4,"a newly available native dive zone announces Can dive");
        reset_landing_state();spoken.clear();landing_fixture::ready=false;d_ShowLandingGuide(&manager,true,nullptr);
        landing_fixture::ready=true;d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.empty(),"loading-time dive availability is seeded silently and never replayed at control return");
        d_ShowLandingGuide(&manager,false,nullptr);d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken==std::vector<std::wstring>{L"Can dive"},"later live dive availability remains audible after loading");
        reset_landing_state();spoken.clear();landing_fixture::attribute_throws=true;
        d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken.empty(),"failed native terrain observation cannot claim either a dive or a landing");
        landing_fixture::attribute_throws=false;
        landing_fixture::cell={-253,-252,0};
        expect(available_action(field::fixture_vehicle)==LandingAction::Dive,"wrapped native coordinates identify the same actual dive terrain");
        landing_fixture::cell={NAN,4,0};const auto malformed_reads=landing_fixture::attribute_reads;
        expect(available_action(field::fixture_vehicle)==LandingAction::Unknown&&landing_fixture::attribute_reads==malformed_reads,"invalid conversion cannot enter native attribute wrapping or fabricate dive availability");
        landing_fixture::cell={3,4,0};
        const auto began=std::chrono::steady_clock::now();double peak=0;
        for(int i=0;i<1000;++i){const auto one=std::chrono::steady_clock::now();
            expect(available_action(field::fixture_vehicle)==LandingAction::Dive,"bounded copied native dive observation remains stable");
            peak=std::max(peak,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-one).count());}
        std::printf("Dive observer1000 fake-owner samples: %.3fms aggregate/%.3fms peak; real native/frame/audio unmeasured\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count(),peak);
        const auto reads=landing_fixture::attribute_reads;
        for(int n:{1,2,4,5,6}) {fixture::game=static_cast<game::Id>(n);reset_landing_state();d_ShowLandingGuide(&manager,true,nullptr);}
        fixture::game=game::Id::FF3;field::fixture_vehicle.id=8;reset_landing_state();d_ShowLandingGuide(&manager,true,nullptr);
        expect(landing_fixture::attribute_reads==reads,"other games and unupgraded Nautilus add no terrain query or dive capability");
        fixture::game=game::Id::FF3;field::fixture_vehicle={true,14,6,{}};field::fixture_map=3;
        reset_landing_state();spoken.clear();d_ShowLandingGuide(&manager,true,nullptr);d_SwitchLandable(&manager,true,nullptr);
        expect(spoken==std::vector<std::wstring>{L"Can surface"},"underwater Nautilus uses Can surface once across both native guide producers");
        d_ShowLandingGuide(&manager,false,nullptr);d_ShowLandingGuide(&manager,true,nullptr);
        expect(spoken==std::vector<std::wstring>{L"Can surface",L"Can surface"},"leaving and returning to a native surface zone announces a new surface opportunity");
        fixture::game=game::Id::FF5;
        for(int id:{8,13}){field::fixture_vehicle={true,id,6,{}};reset_landing_state();spoken.clear();d_ShowLandingGuide(&manager,true,nullptr);
            expect(spoken==std::vector<std::wstring>{L"Can surface"},"both actual FFV submarine models use the same surface action term");}
        expect(landing_fixture::attribute_reads==reads,"submarine surfacing uses native guide availability without a terrain query");
        owners.erase("FieldMap");extra_methods.erase("ConvertWorldPositionToCellPosition");extra_methods.erase("GetCellAttribute");
    }
    expect(wrong==0,"native view reads remain on their owner");
    fixture::game=game::Id::FF3;spoken.clear();field::fixture_move=0;field::fixture_asset=false;
    on_get_on(5);
    expect(spoken.empty(),"restored canoe context cannot announce boarding while the local player still has the walking asset");
    field::fixture_asset=true;on_get_on(5);
    expect(spoken==std::vector<std::wstring>{L"On canoe"},"actually applied canoe asset permits native boarding feedback");
    std::printf("All-six landing-guide actual-source detours: %d failures\n",failures);return failures?1:0;
}
