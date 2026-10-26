// check-sources: text.cpp ui/navigation_buffer.cpp
// Actual native landing-guide detours; no game/engine/device is launched.
#define main retained_party_fixture_main
#include "party_esper_check.cpp"
#undef main
#include "../src/core/fieldnav/vehicle_landing.cpp"

namespace field {
VehicleState fixture_vehicle;
VehicleState current_vehicle(){fixture::native();return fixture_vehicle;}
std::wstring vehicle_game_name(int,const std::wstring&){fixture::native();return {};}
}
namespace ffpr::fieldnav::move_state {
bool is_on_foot(){return true;} // deliberate stale cache during boarding/loaded saves
void set_on_foot(){}void set_vehicle_state(int,const std::wstring&){}void reset(){}
}
namespace landing_fixture {
int originals=0;
bool ready=true,blocked=false;
void show(void* self,bool,const void*){
    ++originals;static_cast<fixture::Node*>(fixture::get(*static_cast<fixture::Node*>(self),"landingGuide"))->active=true;
}
void change(void*,bool,const void*){++originals;}
void hide(void* self,const void*){
    ++originals;static_cast<fixture::Node*>(fixture::get(*static_cast<fixture::Node*>(self),"landingGuide"))->active=false;
}
}
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
    expect(wrong==0,"native view reads remain on their owner");
    std::printf("All-six landing-guide actual-source detours: %d failures\n",failures);return failures?1:0;
}
