// check-sources: fieldnav/move_state.cpp
// Actual explicit vehicle query with fake ready native transportation snapshots.
#include <cstdio>
#include "core.h"
#include "field/field_api.h"
#include "fieldnav/move_state.h"
#include "fieldnav/watercraft.h"
#include "frame.h"
#include <array>
namespace fixture {
bool ready=false;
int reads=0;
int map=95;
field::VehicleState vehicle;
std::wstring name;
std::optional<int> movement;
std::optional<bool> applied_vehicle;
int (*install)()=nullptr;
std::array<std::vector<events::Handler>,static_cast<size_t>(events::Kind::Count)> handlers;
std::vector<frame::Task> tasks;
std::vector<std::wstring> spoken;
void event(events::Kind kind) { for(auto& handler:handlers[static_cast<size_t>(kind)])handler(); }
void tick() { auto pending=std::move(tasks);tasks.clear();for(auto& task:pending)task(); }
}
namespace core { Install::Install(const char* n,int (*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){fixture::install=f;} }
namespace events { void on(Kind kind,Handler handler){fixture::handlers[static_cast<size_t>(kind)].push_back(std::move(handler));} }
namespace frame { void next_frame(Task task,const char*) { fixture::tasks.push_back(std::move(task)); } }
namespace queries {
void define(const char*,std::function<bool()>) {}
void define_text(const char*,std::function<std::wstring()>) {}
}
namespace ffpr::system { bool is_field_navigation_ready() { return fixture::ready; } }
namespace field {
VehicleState current_vehicle() { ++fixture::reads; return fixture::vehicle; }
int current_map_id() { return fixture::map; }
std::optional<int> player_movement_state() { return fixture::movement; }
std::optional<bool> player_uses_vehicle_asset() { return fixture::applied_vehicle; }
std::wstring vehicle_display_name(int id,int type,const std::wstring& message) {
    if(id!=fixture::vehicle.id || type!=fixture::vehicle.type || message!=fixture::vehicle.message_id) std::abort();
    return fixture::name;
}
}
namespace modtext { std::wstring T(std::wstring_view value) { return std::wstring(value); }
std::wstring TF(std::wstring_view,std::initializer_list<std::wstring>) { return {}; } }
namespace ffpr { void say(std::wstring_view line,bool interrupt) { if(interrupt)std::abort();fixture::spoken.emplace_back(line); } }
int main() {
    using namespace ffpr::fieldnav::move_state;
    int failures=0;
    auto expect=[&](bool ok,const char* reason) { if(!ok) { ++failures; std::printf("FAIL %s\n",reason); } };
    using namespace ffpr::fieldnav::landing_spots;
    for(int game:{1,2,3,5}) expect(watercraft(game,4,2),"freeform ships supported in FF1/2/3/5");
    for(int game:{1,2,3}) expect(watercraft(game,2,5),"actual canoe master supported in FF1/2/3");
    expect(!watercraft(2,5,5)&&!watercraft(3,3,3)&&!watercraft(4,4,2)&&!watercraft(6,4,2),"snowcraft, aircraft and FF4/6 excluded");
    expect(legal_shore(true,false,true,true)&&legal_shore(true,false,false,false),"valid dock and canoe shore");
    expect(!legal_shore(false,false,true,true)&&!legal_shore(true,false,false,true)&&!legal_shore(true,true,true,true),"wall, forbidden ship shore and same water rejected");
    reset(); fixture::ready=true; fixture::vehicle={true,3,2,L"ShipName"}; fixture::name=L"Ship";
    expect(current_name()==L"Ship" && fixture::reads==1,"loaded boarded ship is named without a GetOn event");
    fixture::vehicle={true,5,5,L""}; fixture::name=L"Snowcraft";
    expect(current_name()==L"Snowcraft","actual transportation id/name survives a shared canoe type");
    fixture::vehicle={true,9,3,L"AirshipName"}; fixture::name=L"Airship";
    expect(current_name()==L"Airship","loaded airship uses the current native vehicle");
    set_vehicle_state(2,L"Old ship"); fixture::vehicle={true,1,1,L""}; fixture::name.clear();
    expect(current_name()==L"Walking","loaded native player row overrides an earlier boarding cache");
    fixture::vehicle={true,3,2,L""}; fixture::name.clear();
    expect(current_name()==L"Ship","unnamed current vehicle retains the known movement-type fallback");
    fixture::ready=false; int reads=fixture::reads;
    expect(current_name()==L"Old ship" && fixture::reads==reads,"unready/menu context performs no native transport read and retains existing behavior");
    fixture::ready=true; fixture::vehicle={};
    expect(current_name()==L"Old ship","missing native ownership preserves the prior fallback without manufacturing state");
    // A newly constructed Temple player can walk while the world transport is
    // remembered or absent. Native movement, rather than that cache, owns V.
    fixture::movement=0;
    expect(current_name()==L"Walking","constructed walking player replaces cached Enterprise when transport is absent");
    fixture::vehicle={true,13,2,L"Enterprise",2};fixture::name=L"Enterprise";fixture::applied_vehicle=false;
    expect(current_name()==L"Walking","actual foot mode overrides a retained world transportation row");
    fixture::movement=1;expect(current_name()==L"Dashing","native dash remains on foot");
    fixture::movement=3;expect(current_name()==L"Enterprise","native sailing still names the Enterprise");
    fixture::movement=0;fixture::vehicle={true,2,5,L"",95};fixture::name=L"Canoe";fixture::applied_vehicle=true;
    expect(current_name()==L"Canoe","same-map automatic Canoe uses native Walk mode without becoming foot");
    fixture::vehicle={true,7,6,L"",95};fixture::name=L"Submarine";
    expect(current_name()==L"Submarine","native submarine Walk mode retains its current craft");
    fixture::vehicle={true,2,5,L"",4};fixture::name=L"Canoe";fixture::applied_vehicle=true;
    expect(current_name()==L"Canoe","applied automatic canoe survives map entry with its saved world location");
    fixture::vehicle={true,13,2,L"Enterprise",95};fixture::name=L"Enterprise";fixture::applied_vehicle=false;
    expect(current_name()==L"Walking","actual player asset overrides remembered same-map craft");
    fixture::applied_vehicle.reset();
    expect(current_name()==L"Enterprise","unknown applied asset cannot manufacture a dismount from the saved map location");
    fixture::vehicle={true,13,2,L"Enterprise",2};fixture::name=L"Enterprise";
    fixture::install();fixture::movement=0;fixture::applied_vehicle=false;
    fixture::event(events::Kind::MapChanged);
    expect(fixture::tasks.size()==1&&fixture::spoken.empty(),"map event defers once beyond the native state-change stack");
    fixture::tick();
    expect(is_on_foot()&&fixture::spoken==std::vector<std::wstring>{L"On foot"},"map entry reconciles and announces the missing GetOff edge once");
    fixture::event(events::Kind::MapChanged);fixture::tick();
    expect(fixture::spoken.size()==1,"another walking floor cannot repeat the dismount");
    set_vehicle_state(2,L"Enterprise");fixture::event(events::Kind::MapChanged);
    fixture::event(events::Kind::BattleEntering);fixture::tick();
    expect(!is_on_foot()&&fixture::spoken.size()==1,"battle cancels obsolete map reconciliation");
    fixture::event(events::Kind::MapChanged);fixture::event(events::Kind::MapChanging);fixture::tick();
    expect(!is_on_foot(),"another transition cancels the prior map's read");
    fixture::movement.reset();fixture::event(events::Kind::MapChanged);fixture::tick();
    expect(!is_on_foot(),"unknown native player cannot manufacture a foot state");
    fixture::movement=0;fixture::vehicle={true,2,5,L"",4};fixture::name=L"Canoe";fixture::applied_vehicle=true;
    fixture::event(events::Kind::MapChanged);fixture::tick();
    expect(vehicle_name()==L"Canoe"&&!is_on_foot()&&fixture::spoken.size()==1,"automatic canoe on a riding map cannot announce a false dismount");
    fixture::movement=3;fixture::vehicle={true,13,2,L"Enterprise",2};fixture::name=L"Enterprise";
    fixture::event(events::Kind::MapChanged);fixture::tick();
    expect(vehicle_name()==L"Enterprise"&&fixture::spoken.size()==1,"native riding map seeds the named vehicle without repeating GetOn");
    fixture::ready=false;fixture::movement=0;fixture::event(events::Kind::MapChanged);fixture::tick();
    expect(!is_on_foot()&&fixture::spoken.size()==1,"unready field performs no map reconciliation");
    std::printf("Vehicle query: boarded loads, native walking replacement, shared type/name and readiness; %d failures\n",failures);
    return failures?1:0;
}
