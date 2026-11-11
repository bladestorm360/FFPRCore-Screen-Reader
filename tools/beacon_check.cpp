// check-sources: audio/custom_cues.cpp
// Production audio-loop target selection, mode action, gates and ping parameters.
// Native owners and the route service boundary are fake; no game/device is opened.
#include <cstdio>
#include <map>
#include <limits>
#include "../src/core/fieldnav/audio_loops.cpp"
namespace fixture {
uint32_t custom_mask=0;bool hrtf=false;
std::map<std::string,int> values;
std::map<std::string,actions::Fn> actions;
std::map<events::Kind,std::vector<events::Handler>> handlers;
std::vector<std::wstring> speech;
unity::Vec3 player{};
field::Target target;
field::baked::Submap submap{};
ffpr::fieldnav::nav_target::Kind kind=ffpr::fieldnav::nav_target::Kind::Entity;
ffpr::fieldnav::waypoints::EntityPtr waypoint;
bool ready=true,focus=true,field=true,suppress=false,dialogue=false,fading=false;
bool map_available=true,route_found=true,arrived=false,route_pending=false;
int pings=0,landing_pings=0,follow=0,across=0,quick=0,map_reads=0;float pan=0,volume=0;bool south=false,low=false;
int planned_entity=-1;
double at=10;
unity::Vec3 corner{0,80,0};
void reset() {pings=follow=across=quick=map_reads=0;speech.clear();ffpr::fieldnav::audio_loops::g_last_beacon_played_at=0;}
}
namespace core { Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace actions {
void define(const char* name,Fn fn){fixture::actions[name]=std::move(fn);}
bool defined(const char* name){return fixture::actions.contains(name);}
bool run(const char* name){if(!defined(name))return false;fixture::actions[name]();return true;}
}
namespace settings {
int get_int(const char* key,int fallback){auto i=fixture::values.find(key);return i==fixture::values.end()?fallback:i->second;}
bool get_bool(const char* key,bool fallback){return get_int(key,fallback?1:0)!=0;}
void set_int(const char* key,int value){fixture::values[key]=value;}
void set_bool(const char* key,bool value){set_int(key,value?1:0);}
}
namespace queries { bool is(const char* name){
    const std::string n=name;
    if(n=="ControllerRouter.IsFieldActive")return fixture::field;
    if(n=="ControllerRouter.SuppressGameInput")return fixture::suppress;
    if(n=="MessageWindowPatches.IsInDialogue")return fixture::dialogue;
    return false;
} }
namespace frame {double now(){return fixture::at;}void subscribe(void(*)(),const char*){} }
namespace events {void on(Kind kind,Handler handler){fixture::handlers[kind].push_back(std::move(handler));} void fire(Kind kind){for(auto& handler:fixture::handlers[kind])handler();} }
namespace hooks {void note_fault(const char*){std::abort();} }
namespace input {bool game_window_focused(){return fixture::focus;} }
namespace unity {void* transform(void* p){return p;}Vec3 local_position(void*){return fixture::player;} }
namespace ffpr {void say(std::wstring_view text,bool){fixture::speech.emplace_back(text);} }
namespace ffpr::system {bool is_field_navigation_ready(){return fixture::ready;} }
namespace modtext {std::wstring T(std::wstring_view key){return std::wstring(key);} }
namespace field {
Target selected(){return fixture::target;}void* player(){return reinterpret_cast<void*>(1);}
void* map_handle(){++fixture::map_reads;return reinterpret_cast<void*>(2);}int current_map_id(){return 1;}
const std::wstring& scene_name(){static std::wstring name=L"Field";return name;}
WallDistances nearby_walls(void*){return {};}
std::vector<unity::Vec3> map_exit_positions(){return {};}
bool is_direction_near_map_exit(unity::Vec3,unity::Vec3,const std::vector<unity::Vec3>&){return false;}
PathInfo find_path_to_quick(unity::Vec3,unity::Vec3,void*,void*,TargetKind,bool){++fixture::quick;PathInfo result;result.success=fixture::route_found;return result;}
}
namespace field::baked {const Submap* submap_by_map_id(int){return &fixture::submap;} }
namespace field::baked_nav {
bool available(void*){return fixture::map_available;}
bool follow(void*,void*,unity::Vec3,TargetKind,Corner& out,int entity,int){++fixture::follow;fixture::planned_entity=entity;out.point=fixture::corner;out.arrived=fixture::arrived;out.pending=fixture::route_pending;return fixture::route_found;}
bool follow_goal(void*,void*,const MapGoal& goal,TargetKind,Corner& out){++fixture::across;fixture::planned_entity=goal.entity_id;out.point=fixture::corner;out.arrived=fixture::arrived;out.pending=fixture::route_pending;return fixture::route_found;}
}
namespace ffpr::fieldnav {
float distance(unity::Vec3 a,unity::Vec3 b){return std::sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z));}
namespace nav_target {Kind last_kind(){return fixture::kind;} }
namespace waypoints::nav {EntityPtr selected(){return fixture::waypoint;} }
namespace map_transition {bool is_screen_fading(){return fixture::fading;} }
namespace landing_spots {bool riding_watercraft(){return false;}void nearby(void*,std::vector<sound::Direction>& out){out.clear();} }
}
namespace sound {
bool wall_tone_playing(){return false;}void stop_wall_tone(){}
void play_wall_tones_looped(const std::vector<Direction>&){}void play_landing_ping(float,float,int){++fixture::landing_pings;}
void play_beacon(float east,float north,float volume,bool low){++fixture::pings;fixture::south=north<0;fixture::pan=std::clamp(east/100.f,-1.f,1.f);fixture::volume=volume;fixture::low=low;}
}
namespace sound::runtime {
bool hrtf_active(){return fixture::hrtf;}
uint32_t custom_available_mask(){return fixture::custom_mask;}
}
int main(){
    using namespace ffpr::fieldnav::audio_loops;
    int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    install();fixture::values["AudioBeacons"]=1;fixture::values["Soundscape"]=1;fixture::values["HRTF"]=1;
    fixture::target.valid=true;fixture::target.identity=reinterpret_cast<void*>(3);fixture::target.position={320,0,0};fixture::target.entity_id=0;
    expect(ui::prefs::beacon_mode()==0,"existing users default to route mode");
    fixture::reset();beacon_iteration(10);
    expect(fixture::pings==1&&fixture::follow==1&&fixture::pan==0&&!fixture::south&&!fixture::low,"route mode aims at northward route corner, not eastward object");
    expect(fixture::planned_entity==0,"route beacon preserves a valid zero-valued native event identity");
    const float corner_volume=fixture::volume;
    fixture::reset();fixture::route_found=false;beacon_iteration(10.5);
    expect(fixture::pings==1&&fixture::follow==1&&fixture::low,"route mode preserves its blocked-route lower ping");
    fixture::route_found=true;
    for(int hrtf=0;hrtf<2;++hrtf){
        fixture::values["HRTF"]=hrtf;
        for(int step=0;step<80;++step){
            fixture::player={float(step*16),0,0};fixture::corner={float(step*16),80,0};
            fixture::reset();beacon_iteration(20+step*2+hrtf*200);
            expect(fixture::pings==1&&!fixture::low,"continuous movement retains ordinary beacon cadence in both audio modes");
        }
        fixture::route_found=false;fixture::route_pending=true;
        fixture::player.x+=16;fixture::reset();beacon_iteration(400+hrtf*2);
        expect(fixture::pings==1&&!fixture::low,"pending replan continues the known corner rather than silence/blocked pitch");
        fixture::route_pending=false;fixture::route_found=true;
    }
    fixture::player={};fixture::corner={0,80,0};fixture::values["HRTF"]=1;
    actions::run("Mod.ToggleBeaconMode");
    expect(ui::prefs::beacon_mode()==1&&fixture::values["AudioBeacons"]==1&&fixture::values["Soundscape"]==1&&fixture::values["HRTF"]==1,
        "mode action persists Object and preserves beacon enable/soundscape/HRTF settings");
    expect(fixture::speech.back()==L"Beacon mode: Object location"&&g_next_beacon_time==0,"mode action speaks new choice and restarts its cadence");
    fixture::reset();fixture::route_found=false;beacon_iteration(11);
    expect(fixture::pings==1&&fixture::pan==1&&!fixture::low&&fixture::volume<corner_volume&&
        fixture::follow==0&&fixture::across==0&&fixture::quick==0&&fixture::map_reads==0,"object mode pings actual location through walls without any planner/map-accessor call");
    fixture::target.position={-160,-80,0};fixture::reset();beacon_iteration(12);
    expect(fixture::pings==1&&fixture::pan==-1&&fixture::south,"object position updates control stereo/HRTF source direction");
    fixture::player=fixture::target.position;fixture::reset();beacon_iteration(13);
    expect(fixture::pings==0&&g_beacon_silenced,"object mode stops within the existing arrival radius");
    fixture::player={0,0,0};fixture::reset();beacon_iteration(14);
    expect(fixture::pings==1&&!g_beacon_silenced,"object beacon resumes when moving away");
    fixture::target.remote_map_id=2;fixture::reset();restart_beacon();beacon_iteration(15);
    expect(fixture::pings==0&&fixture::follow==0&&fixture::across==0&&fixture::speech.size()==1,"remote direct target explains current-floor requirement once per explicit request");
    fixture::values["AnnounceOnBeaconRestart"]=1;
    actions::define("Mod.AnnounceEntityOnly",[]{fixture::speech.push_back(L"Selected entity");});
    fixture::reset();actions::run("Mod.RestartEntityBeacon");
    expect(fixture::speech==std::vector<std::wstring>{L"Object beacon requires a destination on the current floor"},"optional destination announcement cannot overwrite remote-object explanation");
    fixture::reset();beacon_iteration(16);expect(fixture::speech.empty(),"audio loop never repeats remote-target speech");
    actions::run("Mod.ToggleBeaconMode");fixture::route_found=true;fixture::reset();beacon_iteration(17);
    expect(fixture::pings==1&&fixture::across==1,"returning to route mode restores cross-floor routing");
    expect(fixture::planned_entity==0,"other-floor beacon keeps the same event identity across its legs");
    fixture::target.remote_map_id=0;fixture::arrived=true;fixture::reset();beacon_iteration(18);
    expect(fixture::pings==0,"route mode retains route-arrival silence");fixture::arrived=false;
    actions::run("Mod.ToggleBeaconMode");fixture::target.position={16,0,0};fixture::player={1584,0,0};
    fixture::submap.world=true;fixture::submap.width=fixture::submap.height=100;fixture::reset();beacon_iteration(19);
    expect(fixture::pings==1&&fixture::pan>0&&fixture::pan<1,"object world-edge bearing uses nearest wrapped image");
    fixture::submap.world=false;fixture::player={0,0,0};fixture::kind=ffpr::fieldnav::nav_target::Kind::Waypoint;
    auto waypoint=std::make_shared<ffpr::fieldnav::waypoints::Entity>();waypoint->position={-160,0,0};fixture::waypoint=waypoint;
    fixture::reset();beacon_iteration(20);expect(fixture::pings==1&&fixture::pan==-1&&fixture::follow==0,"waypoint direct bearing uses the same mode");
    fixture::kind=ffpr::fieldnav::nav_target::Kind::Entity;fixture::target.position.y=std::numeric_limits<float>::quiet_NaN();fixture::reset();beacon_iteration(21);
    expect(fixture::pings==0&&fixture::follow==0,"invalid target position emits no audio or route work");fixture::target.position={160,0,0};
    for(int gate=0;gate<6;++gate){
        fixture::reset();fixture::at+=2;g_next_beacon_time=0;
        fixture::ready=gate!=0;fixture::focus=gate!=1;fixture::field=gate!=2;fixture::suppress=gate==3;fixture::dialogue=gate==4;
        fixture::values["NavigationAudio"]=gate!=5;beacon_step();
        expect(fixture::pings==0&&fixture::follow==0&&fixture::quick==0,"all existing context/navigation master gates apply to object mode");
    }
    fixture::ready=fixture::focus=fixture::field=true;fixture::suppress=fixture::dialogue=false;fixture::values["NavigationAudio"]=1;
    fixture::values["AudioBeacons"]=0;const auto before=fixture::values;actions::run("Mod.ToggleBeaconMode");
    expect(fixture::values["AudioBeacons"]==0&&fixture::values.size()==before.size(),"switching modes while off never enables beacon or creates other settings");
    g_wall_running=g_beacon_running=g_landing_running=true;
    g_last_beacon_target.keep=waypoint;g_last_beacon_target.ptr=waypoint.get();
    events::fire(events::Kind::BattleEntering);fixture::reset();tick();
    expect(!g_wall_running&&!g_beacon_running&&!g_landing_running &&
        g_last_beacon_target.keep==waypoint && fixture::follow==0 && fixture::map_reads==0,
        "battle prefix suspends every audio navigation loop without queries or destroying its retained target");
    events::fire(events::Kind::BattleStart);
    expect(!g_last_beacon_target.keep,"native opening ready releases the retained audio target");
    fixture::values["AudioBeacons"]=1;fixture::values["BeaconMode"]=1;
    fixture::kind=ffpr::fieldnav::nav_target::Kind::Entity;fixture::target.valid=true;fixture::target.remote_map_id=0;
    fixture::target.position={48,0,0};fixture::player={};fixture::map_available=true;g_beacon_silenced=false;
    fixture::custom_mask=0;fixture::reset();beacon_iteration(900);
    const double original_interval=g_next_beacon_time-900;
    const auto& custom_beacon=sound::custom::keys(sound::cues::Id::Beacon);
    fixture::values[custom_beacon.repeat]=5000;fixture::custom_mask=uint32_t(1)<<size_t(sound::cues::Id::Beacon);
    fixture::reset();beacon_iteration(910);
    expect(fixture::pings==1&&std::abs(g_next_beacon_time-915)<.001,"actual beacon producer uses custom5s interval");
    fixture::values[custom_beacon.enabled]=0;fixture::reset();beacon_iteration(920);
    expect(fixture::pings==1&&std::abs(g_next_beacon_time-920-original_interval)<.001,"disabled custom beacon restores original producer timing");
    fixture::hrtf=true;fixture::values["BeaconMode"]=0;fixture::values[custom_beacon.enabled]=1;
    fixture::values[sound::custom::keys(sound::cues::Id::BeaconBlocked).repeat]=200;
    fixture::route_found=true;fixture::route_pending=false;beacon_iteration(929);
    fixture::route_pending=true;fixture::reset();beacon_iteration(930);
    expect(fixture::pings==1&&std::abs(g_next_beacon_time-935)<.001,"active HRTF pending beacon takes normal Beacon file timing");
    fixture::route_pending=false;fixture::route_found=false;fixture::reset();beacon_iteration(936);
    expect(fixture::pings==1&&fixture::low&&std::abs(g_next_beacon_time-941)<.001,"active HRTF blocked beacon uses normal Beacon file interval");
    fixture::hrtf=false;fixture::route_found=true;
    fixture::values["LandingPings"]=1;fixture::at=940;g_landing_next_check=1000;g_landing_mask=1;g_landing_next_ping=0;
    const auto& custom_landing=sound::custom::keys(sound::cues::Id::Landing);
    fixture::values[custom_landing.repeat]=200;fixture::custom_mask=uint32_t(1)<<size_t(sound::cues::Id::Landing);
    landing_ping_step();expect(fixture::landing_pings==1&&std::abs(g_landing_next_ping-940.2)<.001,"actual landing producer uses custom200ms interval");
    fixture::at=940.1;landing_ping_step();expect(fixture::landing_pings==1,"landing producer does not fire early");
    fixture::values[custom_landing.enabled]=0;fixture::at=941;landing_ping_step();
    expect(fixture::landing_pings==2&&std::abs(g_landing_next_ping-941.25)<.001,"default landing producer restores250ms cadence");
    fixture::dialogue=true;fixture::at=942;landing_ping_step();expect(!g_landing_mask&&fixture::landing_pings==2,"dialogue cancels custom landing production");
    std::printf("%s beacon modes: real loop/action, route/object/remote/waypoint/wrap/arrival/gates; %d failures\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
