// check-sources: text.cpp
// Actual movement/input detours with bounded native owners; no game is launched.
#include <map>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include "../src/core/text.h"
#include "../src/core/fieldnav/autowalk.cpp"
#include "../src/core/fieldnav/route_readout.cpp"
#include "../src/core/audio/movement_sounds.cpp"
#include "../src/core/input/input_passthrough.cpp"
namespace fixture {
struct Node { void* player=nullptr; void* controller=nullptr; bool enabled=true, airship=false, moving=false, up=false, down=false; float angle=0; unity::Vec2 cached{}; int mode=0;float acceleration=0,left_scale=1,right_scale=1;bool translation=false; };
Node player, controller, map, field_controller;
uint64_t tick=10; double seconds=1;
bool ready=true, focus=true, menu=false, manual=false, edge_open=true;
bool automatic_readout=false, transition_blocked=false, pose_ready=true;
int pose_reads=0, obstacle_queries=0;
unity::Vec3 attempted_destination{};
game::Id game=game::Id::FF1;
field::VehicleState vehicle{true,1,1,L""};
field::baked_route::Cell cell{1,1,1};
field::Target selected_target;
field::baked_nav::MapGoal last_requested;
int facing = -1;
int map_id=1,precision_queries=0;unity::Vec3 position{};
unity::Vec2 native{};
unity::Vec2 moved{};int calls=0, airship_calls=0, key_calls=0, cleaned=0; bool shoulders=false;
std::map<events::Kind,std::vector<events::Handler>> handlers;
std::vector<frame::Step> waits;int requests=0;
std::vector<frame::Task> wall_checks;
std::vector<std::function<void(field::PathInfo)>> route_results;
std::vector<std::wstring> spoken;
void step() { ++tick; seconds+=.016; }
bool moving(void* o,const void*) { return static_cast<Node*>(o)->moving; }
float angle(void* o,const void*) { return static_cast<Node*>(o)->angle; }
void on_key(void* o,int k,const void*) { ++key_calls; if(k==2)static_cast<Node*>(o)->up=true; if(k==3)static_cast<Node*>(o)->down=true; }
void on_key_up(void* o,int k,const void*) { ++key_calls; if(k==2)static_cast<Node*>(o)->up=false; if(k==3)static_cast<Node*>(o)->down=false; }
void touch(void* o,unity::Vec2 value,const void*) {static_cast<Node*>(o)->cached=value;}
void flight(void* o,const void*) { ++airship_calls;auto& n=*static_cast<Node*>(o);shoulders=n.up&&n.down;
 const float radians=field_controller.angle*.017453292519943295f;
 moved={n.cached.x*std::cos(radians)-n.cached.y*std::sin(radians),n.cached.x*std::sin(radians)+n.cached.y*std::cos(radians)};
 // Recovered native phase order: consume stored axis, then observe next input.
 n.cached=ffpr::fieldnav::autowalk::d_InputAxis(nullptr,nullptr);
}
unity::Vec2 native_axis(void*,const void*) { ++calls;return native; }
bool native_key(void*,int,const void*) { ++calls;return false; }
bool listener(void*,void*,int,bool,bool,const void*) { ++calls;return false; }
}
namespace core { Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
bool Class::is(void* p){
 if(!p || !game::in(games))return false;auto*n=static_cast<fixture::Node*>(p);const std::string type=name;
 if(type=="FieldPlayerKeyAirshipController")return n->airship;
 if(type=="FieldPlayerKeyController")return !n->airship&&(n->mode==0||n->mode==6);
 if(type=="FieldPlayerTouchBaseController")return n->mode==1||n->mode==2||n->mode==4||n->mode==5;
 if(type=="FieldPlayerTouchAirshipController")return n->mode==5;
 if(type=="FieldPlayerKeyGroundBirdController")return n->mode==3;
 if(type=="FieldPlayerTouchGroundBirdController")return n->mode==4;
 if(type=="FieldPlayerKeyTwoStepController")return n->mode==7;
 return type=="FieldPlayerController";
}
int32_t Field::get(){std::string n=name;if(n=="fieldPlayer")return offsetof(fixture::Node,player);if(n=="fieldController")return offsetof(fixture::Node,controller);if(n=="<InputEnable>k__BackingField")return offsetof(fixture::Node,enabled);if(n=="isDownShiftUp")return offsetof(fixture::Node,up);if(n=="isDownShiftDown")return offsetof(fixture::Node,down);if(n=="rotateAccelerator")return offsetof(fixture::Node,acceleration);if(n=="leftFastRotateScale")return offsetof(fixture::Node,left_scale);if(n=="rightFastRotateScale")return offsetof(fixture::Node,right_scale);if(n=="isTranslationMode")return offsetof(fixture::Node,translation);return -1;}
void* Method::ptr(){std::string n=name;if(n=="get_IsMoving")return reinterpret_cast<void*>(&fixture::moving);if(n=="GetZAxisRotateBirdCamera")return reinterpret_cast<void*>(&fixture::angle);if(n=="OnTouchPadCallback")return reinterpret_cast<void*>(&fixture::touch);if(n=="OnKey")return reinterpret_cast<void*>(&fixture::on_key);if(n=="OnKeyUp")return reinterpret_cast<void*>(&fixture::on_key_up);return nullptr;}
il2cpp::Method* Method::get(){return reinterpret_cast<il2cpp::Method*>(ptr());}
std::wstring rd_str(void*,Field&){return L"Action";}
void call_failed_note(const char*){std::abort();}
}
namespace unity {bool alive(void*p){return p!=nullptr;}bool active_in_hierarchy(void*p){return p!=nullptr;}void*transform(void*p){return p;}Vec3 local_position(void*){return fixture::position;}}
namespace log_ {void write(const char*,...){} }
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return true;}void note_fault(const char*){std::abort();}}
namespace game {Id current(){return fixture::game;}}
namespace frame {uint64_t count(){return fixture::tick;}double now(){return fixture::seconds;}void cancel_label(const char*){}int every_frame(Step fn,const char*,int){fixture::waits.push_back(std::move(fn));return 1;}}
namespace frame {void after_seconds(double,Task fn,const char*){fixture::wall_checks.push_back(std::move(fn));}}
namespace actions {void define(const char*,Fn){} bool run(const char*){return false;}bool defined(const char*){return false;} }
namespace events {void on(Kind kind,Handler handler){fixture::handlers[kind].push_back(std::move(handler));} void fire(Kind kind){for(auto& handler:fixture::handlers[kind])handler();} }
namespace queries {
void define(const char*, std::function<bool()>) {}
bool is(const char* name) {
    const std::string_view key = name;
    if (key == "GameStatePatches.IsFieldNavigationReady") return fixture::ready;
    if (key == "ControllerRouter.IsFieldActive") return fixture::ready;
    if (key == "MoveStateHelper.IsOnFoot") return fixture::vehicle.type==1;
    return key == "NavigationTargetTracker.IsEntity" && fixture::selected_target.valid;
}
}
namespace settings {int get_int(const char*,int d){return d;} bool get_bool(const char*k,bool d){const std::string_view key=k;if(key=="Autowalk")return true;if(key==ui::prefs::kAutoRouteReadout)return fixture::automatic_readout;return d;}void set_bool(const char*,bool){} }
namespace menu_state {bool any_active(){return fixture::menu;} }
namespace ffpr::system {
bool is_field_navigation_ready(){return fixture::ready;}
bool input_transition_blocked(){return fixture::transition_blocked;}
}
namespace field::cache {void*get(ref::Class&t){return std::string(t.name)=="FieldMap"?&fixture::map:&fixture::controller;}void*get_or_refresh(ref::Class&t){return get(t);} }
namespace field {
bool confirm_blocked_move(void*,unity::Vec3 destination){++fixture::obstacle_queries;fixture::attempted_destination=destination;return !fixture::edge_open;}
void*player_controller(){return &fixture::controller;}
void*player(){return &fixture::player;}void*map_handle(){return &fixture::map;}int current_map_id(){return fixture::map_id;}bool face_player(void*,int direction){fixture::facing=direction;return true;}Target selected(){return fixture::selected_target;}
VehicleState current_vehicle(){return fixture::vehicle;}const std::wstring&scene_name(){static std::wstring s=L"Field";return s;}
namespace baked_nav {bool player_cell(void*,void*,baked_route::Cell&c){++fixture::pose_reads;c=fixture::cell;return fixture::pose_ready;}bool cell_at(void*,unity::Vec3 position,baked_route::Cell&c){c={fixture::cell.layer,static_cast<int>(position.x),static_cast<int>(position.y)};return true;}bool walk_edge(void*,void*,const baked_route::Cell&,const baked_route::Cell&,bool,route_progress::VehicleAction,int){return fixture::edge_open;}
unity::Vec3 world_of(void*,const baked_route::Cell& cell){return {cell.x*16.f,-cell.y*16.f,0};}
bool walk_precision_edge(void*,void*,const route_progress::PrecisionPoint&,const route_progress::PrecisionPoint&){++fixture::precision_queries;return fixture::edge_open;}
void remember_destination(const MapGoal&,TargetKind,const std::wstring&){}void clear_pending(bool){++fixture::cleaned;}void cancel_requests(){++fixture::cleaned;}void request_route(MapGoal goal,TargetKind,std::wstring,std::function<void(PathInfo)> callback,bool){fixture::last_requested=std::move(goal);++fixture::requests;fixture::route_results.push_back(std::move(callback));} }
}
namespace modtext {std::wstring T(std::wstring_view s){return std::wstring(s);}std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring>a){return text::format(s,a);} }
namespace ffpr {void say(std::wstring_view value,bool){fixture::spoken.emplace_back(value);} }
namespace sound {void play_wall_bump(){} void play_footstep(){}}
namespace input {
bool game_window_focused(){return fixture::focus;}bool suppress_game_input(){return false;}bool field_active(){return true;}bool any_key_pressed(){return fixture::manual;}bool key_pressed(Key k){return fixture::manual && k==Key::W;}bool key_in_use(Key){return false;}
bool shift_held(){return false;}bool ctrl_held(){return false;}bool alt_held(){return false;}float left_trigger(){return 0;}float right_trigger(){return 0;}float left_stick_x(){return 0;}float left_stick_y(){return 0;}float right_stick_x(){return 0;}float right_stick_y(){return 0;}
bool button_pressed(int){return false;}bool button_held(int){return false;}bool button_released(int){return false;}bool gamepad_available(){return false;}
namespace router {bool is_button_consumed(int){return false;}bool is_stick_pulse_down(int){return false;}bool is_stick_pulse_up(int){return false;}}
namespace gamepad {bool lstick_up_pressed(){return false;}bool lstick_down_pressed(){return false;}bool lstick_left_pressed(){return false;}bool lstick_right_pressed(){return false;}bool lstick_up_held(){return false;}bool lstick_down_held(){return false;}bool lstick_left_held(){return false;}bool lstick_right_held(){return false;}}
}
namespace input::bindings {
bool remapped_keyboard_in_use(){return false;}
bool event_guide_keyboard_in_use(){return false;}
bool battle_party_keyboard_in_use(){return false;}
bool consumed(int){return false;}
bool navigation_request_pressed(){return false;}
bool navigation_controller_input(int){return false;}
bool keyboard_in_use(Context c){
    for(Key k:{Key::UpArrow,Key::DownArrow,Key::LeftArrow,Key::RightArrow,Key::I})if(key_in_use(k))return true;
    if(c==Context::Results)for(Key k:{Key::W,Key::A,Key::S,Key::D,Key::F7,Key::F8,Key::F5})if(key_in_use(k))return true;
    return false;
}
}

template<class Expect> void check_route_refresh(Expect expect)
{
    namespace readout=ffpr::fieldnav::route_readout;
    using events::Kind;
    using field::baked_route::Cell;
    readout::install();
    fixture::waits.clear();
    fixture::ready=fixture::automatic_readout=fixture::pose_ready=true;
    fixture::menu=fixture::transition_blocked=false;
    fixture::map_id=261;
    fixture::cell={1,13,26};
    const auto pump=[] {
        auto steps=std::move(fixture::waits);
        fixture::waits.clear();
        for(auto& step:steps)if(step())fixture::waits.push_back(std::move(step));
    };
    int refreshes=0, chunks=0;
    Cell route_origin;
    const auto refresh=[&] {++refreshes;route_origin=fixture::cell;};
    const auto next_chunk=[&] {++chunks;};
    const auto walking=[] {
        field::PathInfo path;
        path.success=true;
        path.step_count=2;
        field::route_progress::Phase phase;
        phase.cells={{1,13,26},{1,13,25},{1,13,24}};
        path.walk_phases={phase};
        path.readout_cells=phase.cells;
        return path;
    };
    auto path=walking();
    auto token=readout::begin(refresh);
    readout::follow(path,token,next_chunk);
    readout::motion_input();
    const int before_battle=fixture::pose_reads;
    events::fire(Kind::BattleEntering);
    events::fire(Kind::BattleStart);
    fixture::cell={1,13,24};
    pump();
    expect(!readout::current(token)&&chunks==0&&fixture::pose_reads==before_battle,
           "battle entry retires the pre-battle chunk without sampling native movement");
    events::fire(Kind::BattleEnd);
    expect(refreshes==0&&fixture::waits.empty(),"battle result alone cannot queue field route capture");
    fixture::ready=false;
    readout::field_returned(false);
    readout::field_returned(false);
    pump();
    expect(refreshes==0&&fixture::waits.size()==1&&fixture::pose_reads==before_battle,
           "a repeated native field return waits for readiness without reading the player");
    fixture::ready=true;
    fixture::pose_ready=false;
    pump();
    expect(refreshes==0&&fixture::waits.size()==1,"the refresh waits for the placed field player");
    fixture::pose_ready=true;
    fixture::cell={1,13,22};
    pump();readout::field_returned(false);pump();
    expect(refreshes==1&&route_origin==Cell{1,13,22}&&!readout::battle_refresh_pending(),
           "the route refreshes once at the actual post-battle landing");

    readout::begin(refresh); // a new directions request can be interrupted before publication
    events::fire(Kind::BattleEntering);
    events::fire(Kind::BattleStart);
    events::fire(Kind::MapChanging);
    events::fire(Kind::MapChanged);
    expect(readout::battle_refresh_pending(),"battle refresh retains its destination through a scripted map handoff");
    fixture::map_id=270;
    readout::field_returned(false);pump();
    expect(refreshes==2&&!readout::battle_refresh_pending(),"the new map has one battle refresh owner");

    path=walking();
    path.readout_cells.clear();
    path.walk_phases.back().manual_after=true;
    path.walk_phases.back().action_after=L"Operate the switch.";
    token=readout::begin(refresh);
    readout::follow(path,token,next_chunk);
    expect(readout::expects_field_action(),"a fully spoken in-place switch records its interaction stance");
    readout::field_action_started(270,{1,13,25});
    readout::field_returned(true);pump();
    expect(refreshes==2,"an unrelated script elsewhere does not refresh the route");
    readout::field_action_started(270,{1,13,24});
    readout::field_returned(false);pump();
    expect(refreshes==2,"a switch needs its native script return");
    fixture::transition_blocked=true;
    readout::field_returned(true);pump();
    expect(refreshes==2&&fixture::waits.size()==1,"native action return waits for the field to settle");
    fixture::transition_blocked=false;
    fixture::cell={1,12,21};
    pump();readout::field_returned(true);pump();
    expect(refreshes==3&&route_origin==Cell{1,12,21},"a same-floor platform switch refreshes once from its new pose");

    path.readout_limited=true;
    readout::follow(path,readout::begin(refresh),next_chunk);
    expect(!readout::expects_field_action(),"an action beyond the spoken chunk is not armed");
    path.readout_limited=false;
    path.next_map_id=271;
    readout::follow(path,readout::begin(refresh),next_chunk);
    expect(!readout::expects_field_action(),"a map-changing interaction keeps its existing continuation");
    events::fire(Kind::BattleEntering);
    readout::field_returned(false);
    events::fire(Kind::TitleScreen);pump();
    expect(refreshes==3,"the title drops a queued refresh");
    fixture::automatic_readout=false;
    readout::follow(path,readout::begin(refresh),next_chunk);
    events::fire(Kind::BattleEntering);
    readout::field_returned(false);pump();
    expect(refreshes==3,"refresh remains silent with automatic readout off");

    namespace movement=ffpr::audio;
    movement::install();
    movement::o_OnTouchPadCallback=&fixture::touch;
    fixture::automatic_readout=true;
    fixture::vehicle={true,1,1,L""};
    fixture::cell={1,13,26};
    fixture::position={0,0,0};
    const auto motion=[](unity::Vec2 axis) {movement::d_OnTouchPadCallback(&fixture::controller,axis,nullptr);};
    const auto settle=[&] {
        auto checks=std::move(fixture::wall_checks);
        fixture::wall_checks.clear();
        for(auto& check:checks)check();
        pump();
    };
    const auto show_chunk=[&] {
        auto chunk=walking();
        chunk.readout_limited=true;
        chunk.step_count=3;
        chunk.walk_phases.back().cells={{1,13,26},{1,12,26},{1,11,26},{1,11,25}};
        chunk.readout_cells=chunk.walk_phases.back().cells;
        readout::follow(chunk,readout::begin(refresh),next_chunk);
    };
    show_chunk();
    motion({-1,0});
    fixture::cell={1,10,26};
    fixture::position={-48,0,0};
    settle();
    expect(refreshes==3&&chunks==0&&fixture::obstacle_queries==0,
           "overshooting west is silent and retains the destination for recovery");
    fixture::edge_open=false;
    motion({0,1});settle();
    expect(refreshes==4&&route_origin==Cell{1,10,26}&&fixture::obstacle_queries==1&&
           fixture::attempted_destination.x==-48&&fixture::attempted_destination.y==16,
           "a blocked north move refreshes from the actual tile and native attempted direction");
    show_chunk();
    motion({0,1});settle();motion({0,1});settle();
    expect(refreshes==4&&fixture::obstacle_queries==1,"holding against a wall stays quiet after the new readout");
    motion({0,0});motion({0,1});motion({0,0});settle();
    expect(refreshes==5&&fixture::obstacle_queries==2,"a released key can make a distinct obstacle hit, even on a short tap");
    show_chunk();
    fixture::edge_open=true;
    motion({0,-1});settle();
    expect(refreshes==5&&fixture::obstacle_queries==3,"a stationary but natively open edge is not an obstacle");
    motion({1,0});
    events::fire(Kind::BattleEntering);settle();
    expect(refreshes==5&&fixture::obstacle_queries==3,"battle entry retires a queued obstacle observation");

    path=walking();
    path.readout_cells.clear();
    fixture::cell={1,13,25};
    fixture::position={208,-400,0};
    readout::follow(path,readout::begin(refresh),next_chunk);
    motion({0,1});
    fixture::cell={1,13,24};
    fixture::position={208,-384,0};
    settle();
    expect(!readout::obstacle_token(),"arriving at the destination ends obstacle correction for the old route");
}

int main(){
 namespace a=ffpr::fieldnav::autowalk;int failures=0;auto expect=[&](bool b,const char*s){if(!b){++failures;std::printf("FAIL %s\n",s);}};
 a::install();
 fixture::controller.player=&fixture::player;fixture::map.controller=&fixture::field_controller;a::g_hooked=true;a::o_InputAxis=&fixture::native_axis;a::o_airship_update=&fixture::flight;
 input::o_GetKeyDown=input::o_GetKey=input::o_GetKeyUp=&fixture::native_key;input::o_listener_down=&fixture::listener;
 field::PathInfo p;p.success=true;p.map_width=p.map_height=8;
 a::Phase walk;walk.cells={{1,1,1}};a::Phase fly;fly.cells={{1,2,1},{1,3,1},{1,4,1}};fly.action_before=L"board";fly.vehicle_before=a::VehicleAction::ConfirmBoard;fly.transportation_id=3;fly.vehicle_direction=1;fly.action_cell=a::Cell{1,1,1};
 a::Phase land;land.cells={{1,4,1},{1,4,2}};land.action_before=L"land";land.vehicle_before=a::VehicleAction::ConfirmLand;land.transportation_id=3;land.action_cell=a::Cell{1,4,1};p.walk_phases={walk,fly,land};
 a::start(p);expect(a::active(),"valid route starts");auto axis=a::d_InputAxis(nullptr,nullptr);expect(axis.x==0&&axis.y==0&&input::d_GetKeyDown(nullptr,0,nullptr),"planned boarding queues native confirm without movement");
 expect(!input::d_GetKeyDown(nullptr,1,nullptr)&&input::d_listener_down(nullptr,nullptr,3,false,false,nullptr),"one pulse reaches manager and listener only for correct action");
 fixture::step();fixture::ready=false;fixture::controller.enabled=false;a::input_updated();expect(a::active()&&a::d_InputAxis(nullptr,nullptr).x==0,"boarding animation releases movement and preserves route");
 fixture::ready=true;fixture::controller.enabled=true;fixture::vehicle={true,3,3,L""};fixture::cell={1,2,1};fixture::step();axis=a::d_InputAxis(nullptr,nullptr);expect(axis.x==1&&a::active(),"matching boarded owner resumes riding leg");
 fixture::player.moving=true;expect(a::d_InputAxis(nullptr,nullptr).x==0,"native in-progress movement cannot advance route");fixture::player.moving=false;fixture::cell={1,3,1};fixture::step();expect(a::d_InputAxis(nullptr,nullptr).x==1,"next completed tile advances route");
 fixture::cell={1,4,1};fixture::step();a::d_InputAxis(nullptr,nullptr);expect(input::d_GetKeyDown(nullptr,0,nullptr),"landing uses native confirm");fixture::vehicle={true,1,1,L""};fixture::step();axis=a::d_InputAxis(nullptr,nullptr);expect(axis.y==-1&&a::active(),"native walking state resumes last leg");
 fixture::cell={1,4,2};fixture::step();a::d_InputAxis(nullptr,nullptr);expect(!a::active()&&!a::virtual_key(0),"arrival releases all synthetic input");
 for(int game_number:{3,6}) {
  fixture::game=static_cast<game::Id>(game_number);fixture::cell={1,1,1};fixture::vehicle={true,1,1,L""};
  a::Phase manual_board=fly;manual_board.vehicle_before=a::VehicleAction::None;p.walk_phases={walk,manual_board};a::start(p);fixture::step();a::d_InputAxis(nullptr,nullptr);
  expect(!a::active()&&!a::virtual_key(0),"manual airship boundary stops the walker without a confirm pulse in FF3/FF6");
 }
 fixture::game=game::Id::FF6;fixture::controller.airship=true;fixture::vehicle={true,3,3,L""};fixture::cell={1,2,1};fixture::field_controller.angle=90;fly.action_before.clear();fly.vehicle_before=a::VehicleAction::None;land.vehicle_before=a::VehicleAction::BackLand;p.walk_phases={fly,land};a::start(p);
 axis=a::d_InputAxis(nullptr,nullptr);expect(std::fabs(axis.x)<.00001&&std::fabs(axis.y+1)<.00001,"FF6 native camera rotation receives inverse-transformed cardinal axis");a::d_airship_update(&fixture::controller,nullptr);expect(fixture::shoulders&&!fixture::controller.up&&!fixture::controller.down,"FF6 holds both native shoulder inputs during flight and restores them afterwards");
 fixture::controller.up=true;a::d_airship_update(&fixture::controller,nullptr);expect(fixture::controller.up&&!fixture::controller.down,"native pre-existing shoulder hold survives synthetic flight");fixture::controller.up=false;
 fixture::cell={1,3,1};fixture::step();a::d_InputAxis(nullptr,nullptr);fixture::cell={1,4,1};fixture::step();a::d_InputAxis(nullptr,nullptr);expect(input::d_GetKeyDown(nullptr,1,nullptr)&&!input::d_GetKeyDown(nullptr,0,nullptr),"FF6 lands with Back rather than Confirm");
 a::cancel();fixture::cell={1,2,1};fixture::controller.cached={.75f,.75f};
 a::Phase corner;corner.cells={{1,2,1},{1,3,1},{1,3,2}};p.walk_phases={corner};a::start(p);fixture::step();a::d_airship_update(&fixture::controller,nullptr);
 expect(std::fabs(fixture::moved.x-1)<.00001&&std::fabs(fixture::moved.y)<.00001,"flight movement consumes current route axis rather than prior stored input");
 fixture::cell={1,3,1};fixture::step();a::d_airship_update(&fixture::controller,nullptr);
 expect(std::fabs(fixture::moved.x)<.00001&&std::fabs(fixture::moved.y+1)<.00001,"native stored-axis pipeline turns at the route corner without an extra straight move");
 fixture::cell={1,3,2};fixture::step();a::d_airship_update(&fixture::controller,nullptr);
 expect(!a::active()&&std::fabs(fixture::moved.x)<.00001&&std::fabs(fixture::moved.y)<.00001,"arrival releases cached native flight input before movement");
 fixture::cell={1,2,1};a::start(p);fixture::step();a::d_airship_update(&fixture::controller,nullptr);a::cancel();fixture::step();a::d_airship_update(&fixture::controller,nullptr);
 expect(std::fabs(fixture::moved.x)<.00001&&std::fabs(fixture::moved.y)<.00001,"explicit cancel releases the owned stored axis on the next native observation");
 a::start(p);fixture::native={.5f,0};fixture::step();a::d_airship_update(&fixture::controller,nullptr);expect(!a::active()&&fixture::controller.cached.x==.5f,"native manual axis takes priority and is preserved");fixture::native={};
 fixture::manual=true;a::input_updated();expect(!a::active()&&!a::virtual_key(1),"manual input cancels pending landing and releases pulse");fixture::manual=false;
 a::start(p);a::g_planning=true;a::g_key_action=0;a::g_key_frame=frame::count();
 const int cleaned_before=fixture::cleaned;
 events::fire(events::Kind::BattleEntering);
 expect(!a::active()&&!a::virtual_input_active()&&fixture::cleaned==cleaned_before,
  "battle prefix immediately releases virtual movement/keys/planning without route cleanup");
 events::fire(events::Kind::BattleStart);
 expect(fixture::cleaned==cleaned_before+1,"native opening ready retires the interrupted autowalk destination once");
 a::cancel();fixture::controller.airship=false;fixture::controller.mode=0;fixture::cell={1,2,1};
 for(int game=1;game<=6;++game) {
  fixture::game=game::Id(game);
  a::g_key_bird.hooked=a::g_touch_bird.hooked=true; // each game installs its applicable native hooks at boot
  for(int mode:{0,1,2,5,6}) {fixture::controller.mode=mode;expect(a::controller_ok(&fixture::player),"ordinary, touch and special-airship input profiles are supported across all six games");}
  if(game==3||game==5||game==6)for(int mode:{3,4}) {fixture::controller.mode=mode;expect(a::controller_ok(&fixture::player),"key/touch ground-bird controllers have their own native steering profile");}
  fixture::controller.mode=7;expect(!a::controller_ok(&fixture::player),"native up/down choice controller is not mistaken for free spatial movement");
  fixture::controller.mode=0;a::g_airship_hooked=false;
  a::start(p);expect(a::active(),"unavailable flight hook cannot disable ordinary autowalk");a::cancel();a::g_airship_hooked=true;
 }
 fixture::controller.mode=0;fixture::controller.enabled=false;field::baked_nav::MapGoal goal;goal.map_id=1;
 a::start(p,&goal);expect(a::g_planning&&!a::g_active&&!fixture::waits.empty(),"temporary input disable begins bounded readiness wait without movement");
 auto ready_wait=fixture::waits.back();fixture::controller.enabled=true;const int requests=fixture::requests;
 expect(!ready_wait()&&fixture::requests==requests+1&&a::g_planning&&!a::g_active,"input readiness requests a fresh route instead of moving on stale path cells");
 events::fire(events::Kind::BattleEntering);expect(!a::active()&&!ready_wait(),"battle cancels input readiness and cannot resume it later");events::fire(events::Kind::BattleStart);
 fixture::controller.enabled=false;a::start(p,&goal);auto expires=fixture::waits.back();fixture::seconds+=1;
 expect(!expires()&&!a::active(),"input readiness timeout retires the pending request");fixture::controller.enabled=true;
 a::start(p);fixture::manual=true;a::input_updated();expect(!a::active(),"fresh physical movement key cancels the active route");fixture::manual=false;
 // Exercise the actual native owner, not just the pure route follower. A failed
 // live edge schedules one worker request outside the input-axis callback.
 field::PathInfo detour=p;detour.step_count=2;detour.walk_phases.front().cells={{1,2,1},{1,2,2},{1,3,2}};
 field::PathInfo waiting=p;waiting.step_count=0;waiting.npc_approach=true;waiting.walk_phases.resize(1);
 waiting.walk_phases.front()={};waiting.walk_phases.front().cells={{1,2,1}};
 waiting.walk_phases.front().wait_for_npc=true;waiting.walk_phases.front().action_after=L"Wait for Black Mage to move.";
 for(int number=1;number<=6;++number) {
  fixture::game=game::Id(number);fixture::cell={1,2,1};fixture::edge_open=false;
  a::start(p,&goal);const int before=fixture::requests;
  auto paused=a::d_InputAxis(nullptr,nullptr);
  expect(a::active()&&a::g_replan_needed&&paused.x==0&&paused.y==0&&fixture::requests==before,
   "live NPC collision pauses without declaring blocked or capturing inside the axis hook in all six games");
  a::input_updated();expect(fixture::requests==before+1&&a::g_planning,"owner update admits exactly one fresh detour request");
  a::input_updated();a::d_InputAxis(nullptr,nullptr);
  expect(fixture::requests==before+1&&a::active(),"pending worker cannot accumulate duplicate reroutes or move on the old path");
  fixture::route_results.back()(detour);fixture::edge_open=true;auto rerouted=a::d_InputAxis(nullptr,nullptr);
  expect(rerouted.x==0&&rerouted.y==-1&&a::active(),"fresh detour resumes through native movement without walking into the old obstruction");
  a::cancel();a::start(waiting,&goal);a::d_InputAxis(nullptr,nullptr);a::input_updated();
  const auto notice_count=[&]{return std::count(fixture::spoken.begin(),fixture::spoken.end(),L"Wait for Black Mage to move.");};
  const auto notices=notice_count();expect(notices>0,"temporary doorway supplies its wait notice");
  fixture::route_results.back()(waiting);const int first=fixture::requests;
  fixture::seconds+=.25;a::input_updated();expect(fixture::requests==first,"temporary wait retries are rate bounded");
  fixture::seconds+=.3;a::input_updated();expect(fixture::requests==first+1,"temporary wait obtains a new actor snapshot after its retry interval");
  a::input_updated();expect(fixture::requests==first+1&&notice_count()==notices,"one in-flight retry and one semantic wait notice are retained");
  auto stale=fixture::route_results.back();fixture::manual=true;a::input_updated();fixture::manual=false;
  expect(!a::active(),"manual input cancels a doorway retry immediately");
  a::start(detour,&goal);stale(waiting);auto still_current=a::d_InputAxis(nullptr,nullptr);
  expect(still_current.y==-1&&!a::g_replan_needed,"cancelled wait callback cannot replace a newer walk");
  a::cancel();a::start(waiting,&goal);a::d_InputAxis(nullptr,nullptr);a::input_updated();
  fixture::route_results.back()(detour);fixture::edge_open=false;a::d_InputAxis(nullptr,nullptr);
  fixture::seconds+=31;a::input_updated();
  expect(!a::active(),"a fresh-looking route to the same native-vetoed edge cannot extend the bounded wait forever");
  fixture::edge_open=true;
  a::start(detour,&goal);a::d_InputAxis(nullptr,nullptr);fixture::seconds+=2.1;
  a::d_InputAxis(nullptr,nullptr);
  expect(a::active()&&a::g_replan_needed,"native movement stalled after an accepted edge also requests a fresh detour");
  a::cancel();
  a::start(waiting,&goal);a::d_InputAxis(nullptr,nullptr);a::input_updated();auto focus_stale=fixture::route_results.back();
  fixture::focus=false;a::input_updated();fixture::focus=true;focus_stale(detour);
  expect(!a::active(),"focus loss retires a pending obstacle route and its completion");
 }
 // Actual input producer must retain fractional values, wait for native
 // displacement and honor the same collision/focus ownership on this room.
 fixture::game=game::Id::FF1;fixture::map_id=91;fixture::controller.mode=0;fixture::controller.airship=false;
 fixture::vehicle={true,1,1,L""};fixture::ready=fixture::focus=fixture::edge_open=true;fixture::player.moving=false;
 field::PathInfo precision;precision.success=true;precision.map_width=precision.map_height=64;
 a::Phase gap;gap.cells={{0,18,18},{0,20,17}};gap.precision={{0,36,36},{0,37,36},{0,37,35},{0,38,35},{0,39,35},{0,40,35},{0,40,34}};
 a::Phase finish;finish.cells={{0,20,17},{0,20,16},{0,19,16}};precision.walk_phases={gap,finish};
 fixture::cell={0,18,18};fixture::position={288,-288,0};a::start(precision);
 auto fine_axis=a::d_InputAxis(nullptr,nullptr);
 expect(fine_axis.x==.5f&&fine_axis.y==0&&a::active(),"native producer takes the first cardinal half step east");
 const int query_count=fixture::precision_queries;fixture::player.moving=true;
 expect(a::d_InputAxis(nullptr,nullptr).x==0&&fixture::precision_queries==query_count,"moving precision input neither advances nor repeats collision queries");
 fixture::player.moving=false;
 for(size_t i=1;i+1<gap.precision.size();++i){const auto from=gap.precision[i],to=gap.precision[i+1];fixture::cell={from.layer,(from.x2+1)/2,(from.y2+1)/2};fixture::position={from.x2*8.f,-from.y2*8.f,0};fixture::step();fine_axis=a::d_InputAxis(nullptr,nullptr);
  expect(fine_axis.x==(to.x2-from.x2)*.5f&&fine_axis.y==(from.y2-to.y2)*.5f&&!(fine_axis.x&&fine_axis.y),"each actual native half-step input follows the ordered cardinal corner");}
 fixture::cell={0,20,17};fixture::position={320,-272,0};fixture::step();fine_axis=a::d_InputAxis(nullptr,nullptr);
 expect(fine_axis.x==0&&fine_axis.y==1,"precision completion resumes the ordinary pickup approach");a::cancel();
 fixture::cell={0,18,18};fixture::position={288,-288,0};a::start(precision);fixture::edge_open=false;
 fine_axis=a::d_InputAxis(nullptr,nullptr);expect(!fine_axis.x&&!fine_axis.y,"a native precision collision veto releases movement");a::cancel();
 fixture::edge_open=true;fixture::map_id=1;fixture::position={};
 {
    fixture::game = game::Id::FF4;
    fixture::cell = {1,79,41};
    fixture::vehicle = {true,7,10,L""};
    const field::baked_nav::MapGoal exit{1,{1,79,40}};
    a::Phase dismount;
    dismount.cells = {{1,79,41},{1,79,40}};
    dismount.action_cell = fixture::cell;
    dismount.vehicle_before = a::VehicleAction::ConfirmLand;
    dismount.transportation_id = 7;
    dismount.action_before = L"dismount";
    field::PathInfo path;
    path.success = path.wraps = true;
    path.step_count = 1;
    path.walk_phases = {dismount};
    a::start(path,&exit,field::TargetKind::Transition);
    const auto stopped = a::d_InputAxis(nullptr,nullptr);
    expect(!stopped.x && !stopped.y && !a::active() && !a::virtual_input_active() &&
           fixture::spoken.back() == L"Autowalk stopped, dismount",
           "the final landing by a world exit releases native movement and leaves dismount and entry to the player");
 }
 {
    fixture::map_id = 160;
    fixture::cell = {0,17,19};
    fixture::vehicle = {true,1,1,L""};
    fixture::selected_target.valid = fixture::selected_target.npc = true;
    fixture::selected_target.identity = &fixture::field_controller;
    fixture::selected_target.position = {17,18,0};
    field::baked_nav::MapGoal bird{160,{0,19,18},true,{},0,14,L"Black Chocobo"};
    a::Phase approach;
    approach.cells = {{0,17,19},{0,18,19},{0,19,19}};
    field::PathInfo path;
    path.success = true;
    path.step_count = 2;
    path.walk_phases = {approach};
    a::start(path,&bird,field::TargetKind::Interact);
    const auto held = a::d_InputAxis(nullptr,nullptr);
    expect(!held.x && !held.y && a::active() && a::g_replan_needed,
           "a moving selected NPC holds movement and keeps autowalk active");
    const int requests_before = fixture::requests;
    a::input_updated();
    a::input_updated();
    expect(fixture::requests == requests_before + 1 && fixture::last_requested.entity_id == 14 &&
           fixture::last_requested.cell == a::Cell{0,17,18},
           "the existing route worker receives one fresh request for the same NPC");
    approach.cells = {fixture::cell};
    approach.arrival_direction = 0;
    path.step_count = 0;
    path.walk_phases = {approach};
    fixture::route_results.back()(path);
    const auto arrived = a::d_InputAxis(nullptr,nullptr);
    expect(!arrived.x && !arrived.y && !a::active() && fixture::facing == 0 &&
           fixture::spoken.back() == L"Arrived",
           "fresh interaction range ends the walk and faces the NPC without walking onto its tile");
    fixture::selected_target = {};
 }
 std::printf("All-six live obstruction detour/wait/rate/cancellation/timeout checks complete\n");
 check_route_refresh(expect);
 std::printf("Autowalk native input, vehicle waits and route refresh: %d failures\n",failures);return failures?1:0;
}
