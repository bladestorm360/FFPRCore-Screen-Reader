// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual baked planner snapshot on a worker. Every fake native API flags worker use.
#include <windows.h>
#include <atomic>
#include <cstdio>
#include <thread>
#include <fstream>
#include <chrono>
#include <mutex>
#include "../src/core/field/baked_nav.cpp"
#include "../src/core/field/entity_filter.h"
namespace fixture {
const DWORD native_owner = GetCurrentThreadId();
std::atomic<int> wrong_owner{0};
int native_reads = 0;
bool capture_ready=false;
bool capture_movement=false;
struct MovementProperty {int type=2,steps=1;} movement_property;
struct MovementActor {MovementProperty* property=&movement_property;} movement_actor;
// Owner-only instruction-controller observations. Lists/objects are fixture
// owned and never supplied to planning workers; named fields match native RE.
struct RoutineInstruction {int kind=3;};
struct RoutineList {std::vector<void*> values;int count_override=INT_MIN;};
RoutineInstruction routine_move{1},routine_lerp{7},routine_animation{3};
RoutineList routine_controllers,routine_instructions;
struct RoutineController {
    void* actor=&movement_actor;void* current=&routine_move;
    RoutineList* instructions=&routine_instructions;int index=0;bool loop=true;
} routine_controller;
struct RoutineManager {RoutineList* controllers=&routine_controllers;} routine_manager;
struct RoutineHandle {RoutineManager* manager=&routine_manager;} routine_handle;
bool capture_routine=false;
bool capture_companion=false;
bool capture_entity_alive=false,capture_entity_active=false,capture_entity_vehicle_hidden=false;
// Optional IL2CPP-shaped value-list owner for actual transportation capture.
// Other planner fixtures keep their old null-property/native-free behavior.
struct TransportArray {uint8_t header[24]{};uintptr_t count=0;int32_t values[8]{};};
struct TransportList {uint8_t header[16]{};TransportArray* items=nullptr;int32_t count=0;};
struct TransportProperty {int32_t type=19,id=0;TransportList* targets=nullptr;} transport_property;
struct TransportActor {TransportProperty* property=&transport_property;bool enabled=true;int32_t collision=1;bool power=true;} transport_actor;
bool capture_transport=false;
std::wstring capture_raw_name;
int current_map=0;
unity::Vec3 current_position;
std::vector<int> auto_groups;
std::vector<std::function<void()>> queued,completed;
std::vector<std::string> scheduled;
std::mutex completion_lock;
game::Id game_id = game::Id::FF2;
int filter_mode=-1;bool legacy_filter=false;
void native() { if (GetCurrentThreadId() != native_owner) ++wrong_owner; else ++native_reads; }
json::Value records;
}
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
void* Method::ptr() { fixture::native(); return nullptr; }
il2cpp::Method* Method::get() { fixture::native(); return nullptr; }
il2cpp::Class* Class::get() { fixture::native(); return nullptr; }
bool Class::is(void*p) { fixture::native();
    if(fixture::capture_routine) {
        const std::string cls=name;
        if(cls=="FieldController")return p==&fixture::routine_handle;
        if(cls=="FieldNonPlayerRouteMoveInstructionController")return p==&fixture::routine_controller;
        if(cls=="MoveToInstruction")return p==&fixture::routine_move;
        if(cls=="MoveLerpInstruction")return p==&fixture::routine_lerp;
    }
    if(fixture::capture_transport&&p==&fixture::transport_actor) {
        if(std::strcmp(name,"FieldColliderEntity")==0)return fixture::transport_property.type==19;
        // Match the actual inheritance: RandomEvent and SwitchEvent are also
        // EventTriggerEntity instances. Otherwise the host would conceal a
        // production type-filter omission instead of exercising it.
        if(std::strcmp(name,"EventTriggerEntity")==0)return fixture::transport_property.type==2||fixture::transport_property.type==28||fixture::transport_property.type==30;
    }
    return fixture::capture_entity_alive&&std::string(name)=="FieldEntity" || (fixture::capture_companion||fixture::capture_movement)&&std::string(name)=="FieldNonPlayer" || fixture::capture_movement&&p==&fixture::movement_property&&std::string(name)=="PropertyNpc";
}
int32_t Field::get() { fixture::native();
    if(fixture::capture_routine) {
        const std::string field=name;
        if(field=="nonPlayerControllerManager")return offsetof(fixture::RoutineHandle,manager);
        if(field=="nonPlayerControllerList")return offsetof(fixture::RoutineManager,controllers);
        if(field=="nonPlayerEntity")return offsetof(fixture::RoutineController,actor);
        if(field=="currentInstruction")return offsetof(fixture::RoutineController,current);
        if(field=="instructionList")return offsetof(fixture::RoutineController,instructions);
        if(field=="instructionIndex")return offsetof(fixture::RoutineController,index);
        if(field=="loop")return offsetof(fixture::RoutineController,loop);
    }
    if(fixture::capture_transport) {
        if(std::strcmp(name,"<Property>k__BackingField")==0)return offsetof(fixture::TransportActor,property);
        if(std::strcmp(name,"<TargetTransportationIdList>k__BackingField")==0)return offsetof(fixture::TransportProperty,targets);
        if(std::strcmp(name,"<EntityId>k__BackingField")==0)return offsetof(fixture::TransportProperty,id);
        if(std::strcmp(name,"enable")==0)return offsetof(fixture::TransportActor,enabled);
        if(std::strcmp(name,"collisionId")==0)return offsetof(fixture::TransportActor,collision);
        if(std::strcmp(name,"<TriggerPower>k__BackingField")==0)return offsetof(fixture::TransportActor,power);
    }
    return fixture::capture_movement&&std::strcmp(name,"<Property>k__BackingField")==0?offsetof(fixture::MovementActor,property):std::strcmp(name,"<MoveType>k__BackingField")==0?offsetof(fixture::MovementProperty,type):std::strcmp(name,"<MoveStep>k__BackingField")==0?offsetof(fixture::MovementProperty,steps):-1;
}
std::wstring str(void*) { fixture::native(); return {}; }
std::wstring rd_str(void*,Field&) {fixture::native();return fixture::capture_transport?fixture::capture_raw_name:std::wstring();}
void call_failed_note(const char*) {}
void* new_str(const std::wstring&) { fixture::native(); return nullptr; }
void* instance(Class&) { fixture::native(); return nullptr; }
}
namespace field::party_entities {bool is_native(void*){fixture::native();return false;}int native_slot(void*){fixture::native();return -1;}bool is_companion(void*){fixture::native();return fixture::capture_companion;}}
namespace il2cpp {
Method* find_method_overload_param(Class*, const char*, int, Class*, int) { fixture::native(); return nullptr; }
void* method_pointer(Method*) { fixture::native(); return nullptr; }
Method* virtual_method(void*, Method*) { fixture::native(); return nullptr; }
Class* object_class(void*) { fixture::native(); return nullptr; }
}
namespace unity {
bool alive(void*) { fixture::native(); return fixture::capture_entity_alive; }
void* game_object(void*) { fixture::native(); return nullptr; }
void* transform(void* p) { fixture::native(); return fixture::capture_ready?p:nullptr; }
Vec3 local_position(void*) { fixture::native(); return fixture::current_position; }
int list_count(void* p) { fixture::native();
    if(fixture::capture_routine&&(p==&fixture::routine_controllers||p==&fixture::routine_instructions)) {
        const auto& list=*static_cast<fixture::RoutineList*>(p);
        return list.count_override==INT_MIN?int(list.values.size()):list.count_override;
    }
    return fixture::capture_transport&&p?static_cast<fixture::TransportList*>(p)->count:0;
}
void* list_at(void* p, int i) { fixture::native();
    if(fixture::capture_routine&&(p==&fixture::routine_controllers||p==&fixture::routine_instructions)) {
        const auto& list=*static_cast<fixture::RoutineList*>(p);
        if(i>=0&&i<int(list.values.size()))return list.values[i];
    }
    return nullptr;
}
int array_length(void* p) { fixture::native(); return fixture::capture_transport&&p?static_cast<int>(static_cast<fixture::TransportArray*>(p)->count):0; }
}
namespace log_ { void write(const char*, ...) {} }
namespace hooks { void note_fault(const char*) { std::abort(); } }
namespace frame {
double now() { return GetTickCount64() / 1000.0; }
void post(Task task) { std::lock_guard lock(fixture::completion_lock);fixture::completed.push_back(std::move(task)); }
int every_frame(Step, const char* label, int) { fixture::native();fixture::scheduled.push_back(label); return 0; }
void cancel_label(const char*) { fixture::native(); }
}
namespace background { bool post(std::function<void()> work, Lane) { fixture::native();if(!fixture::capture_ready)return false;fixture::queued.push_back(std::move(work));return true; } }
namespace ffpr { void say(std::wstring_view, bool) { fixture::native(); } }
namespace ffpr { void say_sequence(std::vector<std::wstring>, bool) { fixture::native(); } }
namespace settings {
bool get_bool(const char* key, bool value) { return std::string(key)==ui::prefs::kPathfindingFilter?fixture::legacy_filter:value; }
int get_int(const char* key,int value){return std::string(key)==ui::prefs::kPathfindingFilterMode?fixture::filter_mode:value;}
}
namespace ffpr::system { bool is_field_navigation_ready() { fixture::native(); return fixture::capture_ready; } }
namespace game {
Id current() { return fixture::game_id; }
const Flags& flags() {
    // Match the real navigation feature in games/ff3..ff6/module.cpp. A default
    // false flag made private-room/passage proofs differ from production. Keep
    // immutable per-game rows so concurrent fixture workers never mutate them.
    static const auto rows=[] {std::array<Flags,7> table{};for(size_t i=3;i<table.size();++i)table[i].has_hidden_passages=true;return table;}();
    const auto index=static_cast<size_t>(fixture::game_id);return rows[index<rows.size()?index:0];
}
const char* key() { return "ff2"; }
}
namespace modtext {
LocaleScope::LocaleScope(std::wstring) {}
LocaleScope::~LocaleScope() {}
const std::wstring& lang() { fixture::native(); static std::wstring value = L"en"; return value; }
std::wstring T(std::wstring_view value) { return std::wstring(value); }
std::wstring TF(std::wstring_view value, std::initializer_list<std::wstring> args) {
    auto out = std::wstring(value); int i = 0;
    for (const auto& arg : args) { const auto token = L"{" + std::to_wstring(i++) + L"}"; const auto at = out.find(token); if (at != out.npos) out.replace(at, token.size(), arg); }
    return out;
}
}
namespace data {
void prepare_game_json() {}
std::string game_file(const char*) { return {}; }
const json::Value& game_json(const char*) { return fixture::records; }
}
namespace field {
std::wstring format_steps(float distance) { return std::to_wstring(static_cast<int>(distance/16))+L".0 steps"; }
void prepare_entity_translations() {}
std::wstring translate_entity_name_snapshot(const std::wstring& value) { return value; }
std::wstring translate_entity_name_snapshot(const std::wstring& value,int) { return value; }
npc_names::Label npc_display_label_snapshot(const std::wstring& value) { return npc_names::parse(value); }
int current_map_id() { fixture::native(); return fixture::current_map; }
void* map_handle() { fixture::native(); return fixture::capture_routine?&fixture::routine_handle:nullptr; }
void* player() { fixture::native(); return fixture::capture_ready?reinterpret_cast<void*>(1):nullptr; }
unity::Vec3 player_position() { fixture::native(); return {}; }
VehicleState current_vehicle() { fixture::native(); return {}; }
std::vector<int> owned_auto_vehicle_groups() { fixture::native(); return fixture::auto_groups; }
std::vector<VehicleOnMap> vehicles_on_map() { fixture::native(); return {}; }
std::vector<SavedWorldVehicle> saved_world_vehicles() {fixture::native();return {};}
std::wstring vehicle_display_name(int, int, const std::wstring&) { fixture::native(); return {}; }
bool EntityCursor::next(void*&) { fixture::native(); return false; }
bool native_position_walkable(void*, unity::Vec3) { fixture::native(); return false; }
bool entity_is_active(void*) { fixture::native(); return fixture::capture_entity_active; }
bool entity_is_present(void*) { fixture::native(); return fixture::capture_entity_active||fixture::capture_entity_vehicle_hidden; }
bool entity_is_scenery(void*) { fixture::native(); return false; }
int object_type_of(void*) { fixture::native(); return fixture::capture_transport?fixture::transport_property.type:0; }
std::wstring resolve_map_name_by_id(int) { fixture::native(); return {}; }
}
namespace field::map_state {
Resolver::Resolver() : Resolver(Flags{}, fixture::records) { fixture::native(); }
}

// Real-map counter coverage owns its frame separately from the large planner suite.
__declspec(noinline) int check_counter_routes(int number, const field::map_state::Flags& chest_flags, const json::Value& chest_records) {
    using namespace field;
    using namespace field::baked_nav;
    int failures=0;
    auto expect=[&](bool ok,const char* text){if(!ok){++failures;std::printf("FAIL %s\n",text);}};
        // Every game's real one-cell counters use geometric range extension,
        // including pairs whose IDs differ or are zero. No invented test map.
        int counter_cases=0,zero_links=0;
        for (int index=0;index<field::baked::submap_count();++index) {
            const auto& map=*field::baked::submap_at(index);
            if(map.world)continue;
            const auto& grid=field::baked::grids(map);
            baked_route::Request movement;movement.submap=&map;
            for(const auto& npc:grid.objects) {
                if(npc.type!=baked::ObjType::NPC&&npc.type!=baked::ObjType::ShopNPC)continue;
                const auto group=baked::str(npc.group);
                if(!group.empty()&&group!=L"entity_default"&&group!=L"default")continue;
                for(const auto& relay:grid.objects) {
                    if(relay.type!=baked::ObjType::RelayInteraction||relay.tw!=1||relay.th!=1||relay.layer!=npc.layer)continue;
                    const auto rg=baked::str(relay.group);
                    if(!rg.empty()&&rg!=L"entity_default"&&rg!=L"default")continue;
                    const int dx=relay.tx-npc.tx,dy=relay.ty-npc.ty;
                    if(std::abs(dx)+std::abs(dy)!=1)continue;
                    const int side=dy<0?0:dx>0?1:dy>0?2:3;
                    if(npc.b>0&&!(npc.b&(1<<side)))continue;
                    const Cell stand{npc.layer,npc.tx+2*dx,npc.ty+2*dy};
                    if(!baked_route::open(movement,grid,stand))continue;
                    auto goals=baked_route::goals_for(movement,{npc.layer,npc.tx,npc.ty},baked_route::GoalKind::Interact,&npc);
                    expect(std::find(goals.cells.begin(),goals.cells.end(),stand)!=goals.cells.end(),"real counter approach exists independently of link IDs");
                    expect(baked_route::facing_direction(stand,goals)==(side+2)%4,"counter arrival faces the actual NPC");
                    ++counter_cases;if(npc.a==0)++zero_links;
                }
            }
        }
        expect(counter_cases>0,"each game has real counter approach regression coverage");
        std::printf("FF%d native one-cell counter approaches: %d (%d zero-link NPCs)\n",number,counter_cases,zero_links);
        if(number==3) {
            const auto* inn=baked::submap_by_map_id(6);
            const auto& grid=baked::grids(*inn);
            const auto clerk=std::find_if(grid.objects.begin(),grid.objects.end(),[](const auto& o){return o.type==baked::ObjType::ShopNPC&&o.entity_id()==10;});
            expect(clerk!=grid.objects.end()&&clerk->tx==12&&clerk->ty==18&&clerk->a==0,"Ur's actual zero-link clerk resolves");
            baked_route::Request movement;movement.submap=inn;movement.start={1,8,18};movement.transportation_id=1;
            map_state::Resolver state(chest_flags,chest_records);map_state::add_relays(state,*inn,movement);
            auto goals=baked_route::goals_for(movement,{1,12,18},baked_route::GoalKind::Interact,&*clerk);
            movement.goals=goals.cells;const auto route=baked_route::find(movement);
            expect(route.found&&route.cells.back()==Cell{1,12,20}&&route.steps()==6,"Ur clerk: pub doorway reaches the six-step counter approach");
            expect(baked_route::facing_direction(route.cells.back(),goals)==0,"Ur clerk: face north across the closed counter");
            const auto reach=baked_route::flood(movement);
            expect(any_reachable(reach,goals.cells),"Ur clerk remains in the reachable-entity list");
            const auto prior_map=fixture::current_map;fixture::current_map=6;
            auto published=std::make_shared<QuickGrid>();published->map_id=6;published->movement=movement;published->reach=reach;
            const auto prior_grid=g_quick_grid;g_quick_grid=published;
            field::PathInfo quick;
            const auto prior_jobs=fixture::queued.size();
            expect(quick_query(world_of_cell(*inn,movement.start),world_of_cell(*inn,{1,12,18}),nullptr,nullptr,TargetKind::Infer,false,quick)&&quick.success&&!quick.pending,
                "actual current-floor quick filter uses the published active relay geometry");
            expect(fixture::queued.size()==prior_jobs,"published native-foot relay geometry needs no asynchronous refresh");
            g_quick_grid=prior_grid;fixture::current_map=prior_map;
            movement.relays.clear();auto absent=baked_route::goals_for(movement,{1,12,18},baked_route::GoalKind::Interact,&*clerk);
            expect(!any_reachable(reach,absent.cells),"inactive counter cannot grant a false interaction route");
            RouteSnapshot snapshot(map_state::Resolver(chest_flags,chest_records));snapshot.submap=inn;snapshot.map_id=6;snapshot.layer=1;snapshot.locale=L"en";
            snapshot.position=world_of_cell(*inn,movement.start);snapshot.goal.map_id=6;snapshot.goal.cell={1,12,18};snapshot.goal.entity_id=10;snapshot.goal.raw_name=baked::object_identifier(*clerk);snapshot.kind=TargetKind::Interact;
            NativeObstacle actor;actor.type=17;actor.id=10;actor.npc=true;actor.cell={1,12,18};actor.raw_name=snapshot.goal.raw_name;snapshot.obstacles.push_back(actor);
            Planned plan;std::thread worker([&]{plan=plan_snapshot(snapshot);});worker.join();
            expect(plan.path.success&&plan.path.step_count==6,"actual immutable worker planner routes to Ur's clerk");
        }
    return failures;
}

// Keep copied-state fixture work out of the large legacy main frame. Snapshots
// are heap-owned just as production jobs are; do not increase the host stack.
__declspec(noinline) int check_looped_ship(const field::baked::Submap& world, const field::baked::Submap& shop,
    field::baked_route::Cell boat, field::baked_route::Cell shore, const field::baked_nav::RouteSnapshot& initial) {
    using namespace field::baked_nav;
    int failures=0;
    auto expect=[&](bool ok,const char* text){if(!ok){++failures;std::printf("FAIL %s\n",text);}};
    auto target=std::make_unique<RouteSnapshot>(initial);
    Planned planned;BrowsePlanned browsed;
    // The current Canoe is an automatically used Content transport.
    // A parked Ship remains a normal selected destination from it.
    target->vehicle={true,2,5,L""};
    target->goal.entity_id=0;target->goal.raw_name=L"Ship";
    target->goal.transportation_id=target->vehicles.front().transportation_id;
    NativeObstacle ship_actor;ship_actor.npc=true;ship_actor.id=0;ship_actor.raw_name=L"Ship";
    ship_actor.cell={-1,boat.x,boat.y};target->obstacles.push_back(ship_actor);
    std::thread canoe_worker([&]{planned=plan_snapshot(*target);browsed=plan_browse_snapshot(*target);});canoe_worker.join();
    expect(planned.path.success&&browsed.route.reason.find(L"board the Ship")!=std::wstring::npos,
        "current native Canoe state preserves selected Ship boarding directions and browse action");

    // A looped world's live transform may occupy another copy of the same
    // map. Use the public cell conversion used by entity browsing/directions.
    for(const auto period:{Cell{1,1,0},Cell{1,-1,0},Cell{1,0,1},Cell{1,0,-1},Cell{1,2,-2}}) {
        auto copied=std::make_unique<RouteSnapshot>(*target);
        auto point=world_of_cell(world,boat);
        point.x+=period.x*world.width*16.0f;
        point.y+=period.y*world.height*16.0f;
        copied->position.x+=period.x*world.width*16.0f;
        copied->position.y+=period.y*world.height*16.0f;
        copied->vehicles.front().pos=point;
        Planned looped;BrowsePlanned list;
        std::thread loop_worker([&]{
            {SnapshotScope scope(*copied);expect(cell_at(nullptr,point,copied->goal.cell),"looped live Ship cell is available");}
            looped=plan_snapshot(*copied);list=plan_browse_snapshot(*copied);
        });loop_worker.join();
        expect(copied->goal.cell==boat&&looped.start==shore,
            "live world copies retain canonical Ship goal and player origin");
        expect(looped.path.success&&looped.path.full_description==planned.path.full_description&&
            list.route.reason==browsed.route.reason,
            "looped current-Canoe to Ship route matches the canonical legal route and browse action");
    }
    // Reproduce the rejected destination generated by the old unwrapped
    // converter, without changing terrain or claiming an unavailable target.
    auto old=std::make_unique<RouteSnapshot>(*target);
    old->goal.transportation_id=0;old->goal.entity_id=-1;old->goal.raw_name.clear();old->obstacles.clear();
    old->goal.cell.x+=world.width;
    Planned rejected;BrowsePlanned rejected_list;
    std::thread old_worker([&]{rejected=plan_snapshot(*old);rejected_list=plan_browse_snapshot(*old);});old_worker.join();
    expect(!rejected.path.success&&rejected.path.reason.empty()&&!rejected_list.route.direct&&rejected_list.route.reason.empty(),
        "unwrapped live Ship destination reproduces the generic no-path and unreachable browse result");
    const auto indoor_outside=cell_of(shop,world_of_cell(shop,{0,-1,shop.height}),0);
    expect(indoor_outside.x==-1&&indoor_outside.y==shop.height,
        "indoor invalid coordinates never wrap onto unrelated cells");
    expect(cell_of(world,{1.e30f,0,0},1).x==-1,
        "invalid oversized observations cannot wrap into a valid world tile");
    expect(cell_of(world,{std::nanf(""),0,0},1).x==-1&&cell_of(world,{0,HUGE_VALF,0},1).y==-1,
        "nonfinite live observations are rejected before integer conversion");
    std::printf("FF2 looped Ship: native Canoe, five map copies, old-goal rejection and indoor/invalid bounds; %d failures\n",failures);
    return failures;
}

// Compare the new immutable route index with the former exact suffix semantics.
// Every query remains useful while the player changes tile; no worker is needed
// for a warm published route, including corners, revisits and vehicle phases.
__declspec(noinline) int check_beacon_index(){
    using namespace field;using namespace field::baked_nav;
    int failures=0;size_t compared=0;
    auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL beacon index %s\n",why);}};
    for(int world=0;world<2;++world)for(int variant=0;variant<32;++variant){
        baked::Submap sm{};sm.width=sm.height=64;sm.world=world!=0;
        std::vector<Phase> phases(4);Cell at{1,2,2};
        for(int pi=0;pi<4;++pi){
            if(pi==1&&(variant&4))continue;
            phases[pi].cells.push_back(at);
            for(int i=0;i<50;++i){
                const int d=(i/7+variant+pi)%4;
                if(d==0)++at.x;else if(d==1)++at.y;else if(d==2)--at.x;else --at.y;
                if(world){at.x=(at.x+64)%64;at.y=(at.y+64)%64;}
                if((variant&8)&&i%11==0)at.layer=at.layer==1?2:1;
                phases[pi].cells.push_back(at);
                if((variant&16)&&i%13==0)phases[pi].cells.push_back(at);
            }
        }
        phases.back().wait_for_npc=(variant&2)!=0;const bool partial=(variant&1)!=0;
        auto index=build_beacon_corner_index(sm,phases,partial,&world_of_cell);
        for(const auto& phase:phases)for(const auto& cell:phase.cells){
            std::vector<Phase> rest;expect(route_progress::remaining(phases,cell,rest),"reference suffix exists");
            Corner expected;corner_of(sm,rest,cell,expected);if(partial)expected.arrived=false;
            if(rest.back().wait_for_npc&&!rest.back().cells.empty()&&cell==rest.back().cells.back())expected.found=false;
            const auto* actual=index.find(cell);
            expect(actual&&actual->found==expected.found&&actual->arrived==expected.arrived&&actual->steps_left==expected.steps_left&&
                actual->steps_to_corner==expected.steps_to_corner&&actual->point.x==expected.point.x&&actual->point.y==expected.point.y,"matches exact former corner for moving/revisited/layer/phase/wrapped cells");++compared;
        }
        expect(!index.find({2,500,500}),"off-route cell is missing, never a stale corner");
        expect(build_beacon_corner_index(sm,phases,partial,&world_of_cell,[]{return true;}).entries.empty(),"cancelled index never publishes partial guidance");
    }
    std::printf("Beacon index: %zu exact suffix/corner comparisons, %d failures\n",compared,failures);
    return failures;
}

int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0); // retain the decisive fixture if a native-style fault terminates the host
    using namespace field::baked_nav;
    int failures = check_beacon_index();
    auto expect = [&](bool ok, const char* line) { if (!ok) { ++failures; std::printf("FAIL %s\n", line); } };
    int retired = 0;
    std::vector<worker_job::Owner*> owners{&g_route_owner,&g_follow_owner,&g_quick_owner,&g_browse_owner};
    std::vector<uint64_t> epochs;
    for (auto* owner : owners) { epochs.push_back(owner->epoch()->load()); owner->retain([&] { ++retired; }); }
    const int reads_before_interrupt = fixture::native_reads;
    interrupt_requests();
    bool interrupted = true;
    for (size_t i=0;i<owners.size();++i) interrupted &= owners[i]->epoch()->load() == epochs[i]+1;
    expect(interrupted && retired == 0 && fixture::native_reads == reads_before_interrupt,
        "battle entry cancels all four real route worker epochs without native APIs, callback destruction or frame-queue cleanup");
    cancel_requests(); cancel_quick_work(); cancel_browse_route();
    expect(retired == 4, "ordinary lifecycle cleanup retires all four interrupted owners once");
    const int blocked_reads=fixture::native_reads;
    bool blocked_publish=false;
    request_route({},field::TargetKind::Infer,L"",[&](field::PathInfo){blocked_publish=true;});
    expect(!blocked_publish&&fixture::native_reads==blocked_reads+2&&!request_pending(),
        "transition-blocked direction request only cancels its owned label and checks readiness, with no map/entity capture");
    std::ifstream maps("data/ff2/maps.bin", std::ios::binary);
    static std::string bytes((std::istreambuf_iterator<char>(maps)), {});
    expect(field::baked::load(bytes.data(), bytes.size()), "real FF2 bake loads");
    for (int i = 0; i < field::baked::submap_count(); ++i) field::baked::grids(*field::baked::submap_at(i));
    g_loaded = true; g_prepared = true;
    const auto* shop = field::baked::submap_by_name(L"Map_20011_11");
    expect(shop != nullptr, "known Fynn shop bake exists"); if (!shop) return 1;
    RouteSnapshot snapshot(field::map_state::Resolver(field::map_state::Flags{}, fixture::records));
    snapshot.submap = shop; snapshot.map_id = shop->first_map_id; snapshot.layer = 0;
    snapshot.position = world_of_cell(*shop, {0, 8, 15});
    snapshot.goal.map_id = shop->first_map_id; snapshot.goal.cell = {0, 8, 8};
    snapshot.kind = field::TargetKind::Spot; snapshot.locale = L"en";
    Planned result;
    std::thread worker([&] { result = plan_snapshot(snapshot); }); worker.join();
    expect(fixture::wrong_owner == 0, "planner never resolves/reads engine objects or APIs on worker");
    expect(result.path.success && result.path.step_count == 7, "owned snapshot preserves Fynn shop seven-cell route");
    expect(result.path.description == L"7 North", "worker formats concise directions");
    expect(result.active && result.active->valid && result.path.walk_phases.size() == 1, "owned active plan and autowalk phases returned");
    if(result.active){
        g_published_route=result.active;g_active=g_published_route.get();
        CornerSnapshot corner_input{result.active,shop,snapshot.goal.cell,snapshot.goal.cell,snapshot.map_id,snapshot.map_id};
        std::thread index_worker([&]{g_corner=build_corner(corner_input);});index_worker.join();
        const auto prior=fixture::queued.size();
        for(const auto& phase:result.active->phases)for(const auto& cell:phase.cells){
            Corner moving;expect(cached_corner(shop,snapshot.map_id,cell,snapshot.goal.cell,snapshot.map_id,moving)==CornerState::Ready,
                "actual cached corner remains ready on every moving tile without idle publication");
        }
        expect(fixture::queued.size()==prior,"walking a known route submits no per-footstep corner worker jobs");
        const auto started=std::chrono::steady_clock::now();size_t moving_reads=0;
        for(int repeat=0;repeat<300;++repeat)for(const auto& phase:result.active->phases)for(const auto& cell:phase.cells){
            Corner moving;moving_reads+=cached_corner(shop,snapshot.map_id,cell,snapshot.goal.cell,snapshot.map_id,moving)==CornerState::Ready;
        }
        const double milliseconds=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
        std::printf("Beacon moving episode: %zu actual warm cache reads, zero worker jobs, %.3f ms cumulative owner compute\n",moving_reads,milliseconds);
        expect(fixture::queued.size()==prior&&milliseconds<25,"combined repeated warm beacon work stays below the cumulative lag blocker");

        cancel_follow_work();g_published_route.reset();g_active=nullptr;
    }

    {
        ExitSnapshot landing(false, field::map_state::Resolver(field::map_state::Flags{}, fixture::records));
        landing.route.submap = shop; landing.route.map_id = shop->first_map_id; landing.route.layer = 0;
        landing.route.position = snapshot.position;
        const auto target = world_of_cell(*shop, {0, 8, 10});
        bool clear = false, occupied = true, invalid = true;
        std::thread clearance([&] {
            clear = teleport_landing_safe(landing, target);
            NativeObstacle npc; npc.npc = true; npc.cell = {0, 8, 10};
            landing.route.obstacles.push_back(npc);
            occupied = teleport_landing_safe(landing, target);
            invalid = teleport_landing_safe(landing, {1.e30f, 0, 0});
        }); clearance.join();
        expect(clear && !occupied && !invalid, "real Fynn landing accepts clear terrain, rejects copied NPC occupancy and oversized coordinates");
        expect(fixture::wrong_owner == 0, "production landing clearance never reads native objects on its worker");
    }
    const auto* world = field::baked::submap_by_name(L"Map_10010");
    expect(world != nullptr, "world bake exists");
    if (world) {
        RouteSnapshot long_route(field::map_state::Resolver(field::map_state::Flags{}, fixture::records));
        long_route.submap = world; long_route.map_id = world->first_map_id; long_route.layer = 1;
        long_route.position = world_of_cell(*world, {1, 124, 36});
        long_route.auto_groups = {2}; // Semitt Falls requires the owned canoe
        long_route.goal.map_id = world->first_map_id; long_route.goal.cell = {1, 107, 43};
        long_route.kind = field::TargetKind::Transition; long_route.locale = L"en";
        double route_ms = 0, flood_ms = 0;
        std::thread full([&] {
            const auto started = std::chrono::steady_clock::now();
            result = plan_snapshot(long_route);
            route_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
            field::baked_route::Request reach;
            reach.submap = world; reach.start = {1, 123, 34}; reach.auto_groups = {2}; reach.ignore_hazards = true;
            const auto flood_started = std::chrono::steady_clock::now();
            auto grid = field::baked_route::flood(reach);
            flood_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - flood_started).count();
            expect(grid.reachable({1, 107, 44}), "world flood preserves canoe reachability");
        }); full.join();
        std::printf("FF2 world worker: route %.3f ms, reachability %.3f ms (offline computation)\n", route_ms, flood_ms);
        expect(result.path.success && result.path.step_count >= 26, "worker routes real world map without native distance cap");
        if(result.active&&!result.active->phases.empty()&&result.active->phases[0].cells.size()>2) {
            g_published_route=result.active;g_active=g_published_route.get();
            fixture::capture_ready=true;fixture::current_map=world->first_map_id;fixture::auto_groups={2};fixture::scheduled.clear();
            const auto& cells=result.active->phases[0].cells;
            fixture::current_position=world_of_cell(*world,cells[1]);
            int published=0;field::PathInfo answer;
            const auto began=std::chrono::steady_clock::now();
            request_route(long_route.goal,long_route.kind,L"",[&](field::PathInfo path){++published;answer=std::move(path);});
            fixture::current_position=world_of_cell(*world,cells[2]);
            request_route(long_route.goal,long_route.kind,L"",[&](field::PathInfo path){++published;answer=std::move(path);});
            const double dispatch_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
            expect(fixture::queued.size()==2&&fixture::scheduled.empty(),"small real direction requests submit their bounded capture immediately without waiting a frame");
            auto jobs=std::move(fixture::queued);fixture::queued.clear();
            std::thread worker([&]{for(auto& job:jobs)job();});worker.join();
            auto publications=std::move(fixture::completed);fixture::completed.clear();for(auto& publication:publications)publication();
            expect(published==1&&answer.success&&answer.step_count==result.path.step_count-2&&!request_pending(),
                "rapid repeated actual requests cancel stale work and publish only the latest moved-player route");
            std::printf("Actual warm direction pipeline: two requests, latest-only answer, native dispatch %.3f ms; total %.3f ms (fake native owners, worker launch/join, immediate owner delivery; excludes game-frame/Prism timing)\n",
                dispatch_ms,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
            fixture::capture_ready=false;fixture::current_map=0;fixture::auto_groups.clear();
            g_published_route.reset();g_active=nullptr;clear_pending(true);
        }
        {
            SnapshotScope scope(long_route);
            Phase approach,ride,landing;
            for(int y=32;y>=16;--y)approach.cells.push_back({1,16,y});
            ride.cells={{1,16,16},{1,16,15}};for(int x=15;x>=1;--x)ride.cells.push_back({1,x,15});
            ride.action_before=L"board the Ship North";ride.vehicle_before=field::route_progress::VehicleAction::WalkOn;
            ride.vehicle_direction=0;ride.action_cell=ride.cells.front();
            landing.cells={{1,1,15},{1,1,16}};landing.action_before=L"land South";
            landing.vehicle_before=field::route_progress::VehicleAction::WalkOff;landing.vehicle_direction=2;landing.action_cell=landing.cells.front();
            expect(describe({approach,ride,landing},true)==L"16 North, board the Ship North, 15 West, land South",
                "vehicle action speech includes shore headings without counting the boarding/landing step twice");
        }
        // User's actual FF2 onboard failures: map 1, ship at (126,129),
        // land entrances (110,98) and (218,43). Existing bakes contain group 4.
        const auto* ship = field::baked::transport(4);
        expect(ship && ship->type==2 && ship->group==4, "actual FF2 ship transportation rules are baked");
        {
            const auto& grid=field::baked::grids(*world);
            auto foot=walk_request(*world,{1,0,0});foot.auto_groups={2};
            auto plain=foot;plain.auto_groups.clear();
            auto ship_grid=foot;ship_grid.vehicle_group=4;
            Cell boat,shore;bool found=false;
            for(int y=0;y<grid.height&&!found;++y)for(int x=0;x<grid.width&&!found;++x){
                Cell water{1,x,y};
                if(!field::baked_route::open(ship_grid,grid,water)||field::baked_route::open(foot,grid,water))continue;
                for(auto side:{Cell{1,0,-1},Cell{1,1,0},Cell{1,0,1},Cell{1,-1,0}}){
                    Cell bank{1,(x+side.x+grid.width)%grid.width,(y+side.y+grid.height)%grid.height};
                    if(field::baked_route::open(foot,grid,bank)&&!field::baked_route::open(plain,grid,bank)){boat=water;shore=bank;found=true;break;}
                }
            }
            expect(found,"real FF2 Canoe-only river reaches an owned Ship boarding edge");
            if(found){
                RouteSnapshot target(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                target.submap=world;target.map_id=world->first_map_id;target.layer=1;target.locale=L"en";
                target.position=world_of_cell(*world,shore);target.auto_groups={2};
                field::VehicleOnMap vehicle;vehicle.transportation_id=4;vehicle.type=2;vehicle.pos=world_of_cell(*world,boat);
                target.vehicles={vehicle};target.vehicle_names[4]=L"Ship";
                target.goal.map_id=target.map_id;target.goal.cell=boat;target.kind=field::TargetKind::Spot;
                Planned planned;BrowsePlanned browsed;
                std::thread board_worker([&]{planned=plan_snapshot(target);browsed=plan_browse_snapshot(target);});board_worker.join();
                expect(planned.path.success&&planned.path.walk_phases.size()==2,"selected parked Ship is reached without a fictional landing round trip");
                if(planned.path.walk_phases.size()==2)expect(planned.path.walk_phases.back().cells.back()==boat&&
                    planned.path.walk_phases.back().vehicle_before==field::route_progress::VehicleAction::WalkOn,
                    "selected Ship goal preserves its exact water tile and native boarding edge");
                expect(browsed.route.reason.find(L"board the Ship")!=std::wstring::npos,
                    "list tracker and explicit directions share the specific valid Ship boarding action");

                failures+=check_looped_ship(*world,*shop,boat,shore,target);
            }
        }

        for (auto target : {Cell{1,110,98}, Cell{1,218,43}}) {
            RouteSnapshot onboard(field::map_state::Resolver(field::map_state::Flags{}, fixture::records));
            onboard.submap=world; onboard.map_id=world->first_map_id; onboard.layer=1;
            onboard.position=world_of_cell(*world,{1,126,129}); onboard.vehicle={true,4,2,L"MSG_SYSTEM_200"};
            onboard.auto_groups={2}; onboard.goal.map_id=onboard.map_id; onboard.goal.cell=target;
            onboard.kind=field::TargetKind::Transition; onboard.locale=L"en";
            std::thread ship_worker([&] {
                field::baked_route::Request riding;
                riding.submap=world; riding.start={1,126,129}; riding.vehicle_group=4;
                riding.vehicle_type=2; riding.landing_group=ship->landing_group; riding.auto_groups={2}; riding.ignore_hazards=true;
                auto foot=riding; foot.vehicle_group=0;
                auto goals=field::baked_route::goals_for(foot,target,field::baked_route::GoalKind::Transition);
                riding.goals=goals.cells;
                expect(!field::baked_route::find(riding).found, "old ship-only search reproduces the logged no-path failure");
                auto reach=field::baked_route::flood(riding);
                auto land=land_after_ride(riding,reach);
                if (!any_reachable(land,goals.cells)) {
                    int riding_count=0, land_count=0;
                    for(const auto distance:reach.dist[0]) if(distance!=0xFFFF) ++riding_count;
                    for(const auto distance:land.dist[0]) if(distance!=0xFFFF) ++land_count;
                    std::printf("FF2 ship target (%d,%d): ride cells %d, land cells %d, target attribute %d, %zu goals\n",target.x,target.y,riding_count,land_count,field::baked::attribute(field::baked::grids(*world),target.x,target.y),goals.cells.size());
                    for(const auto& cell:goals.cells) std::printf("goal (%d,%d) foot-open %d, land-distance %d\n",cell.x,cell.y,field::baked::vehicle_open(field::baked::grids(*world),1,cell.x,cell.y),land.distance(cell));
                }
                expect(any_reachable(land,goals.cells), "onboard filter includes land destinations through legal shores");
                const auto started=std::chrono::steady_clock::now();
                result=plan_snapshot(onboard);
                const double elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
                std::printf("FF2 onboard route (%d,%d): %.3f ms, %zu phases\n",target.x,target.y,elapsed,result.path.walk_phases.size());
                expect(result.path.success && result.path.walk_phases.size()==2, "actual ship position reaches the reported land entrance using ride and walk phases");
                if(result.path.walk_phases.size()==2) {
                    const auto& ride_phase=result.path.walk_phases[0]; const auto& walk_phase=result.path.walk_phases[1];
                    expect(ride_phase.action_before.empty() && walk_phase.action_before==L"land "+std::wstring(direction_key(walk_phase.vehicle_direction)),
                        "already-boarded route speaks the exact shore heading without boarding again");
                    expect(ride_phase.cells.back()==walk_phase.cells.front(), "ship and walking phases meet at the same legal coastal stop");
                    for(const auto& cell:ride_phase.cells) expect(field::baked::vehicle_open(field::baked::grids(*world),4,cell.x,cell.y), "riding leg remains on the ship grid");
                    const auto& shore=walk_phase.cells[1];
                    const auto heading=field::baked_route::legs({ride_phase.cells.back(),shore},world->width,world->height);
                    expect(heading.size()==1&&heading[0].dir==walk_phase.vehicle_direction,
                        "spoken landing heading matches the actual planned water-to-shore step, including wrapping");
                    expect(field::baked::group_lands(ship->landing_group,field::baked::attribute(field::baked::grids(*world),shore.x,shore.y)), "walking leg starts through the native ship landing policy");
                    double worst=0;
                    const auto saved=std::make_shared<ActiveRoute>(*result.active);
                    const auto& cells=saved->phases[0].cells;
                    for(size_t step=1;step<std::min<size_t>(cells.size(),21);++step) {
                        auto warm=onboard;warm.previous_active=saved;warm.position=world_of_cell(*world,cells[step]);
                        const auto begin=std::chrono::steady_clock::now();
                        const auto answer=plan_snapshot(warm);
                        worst=std::max(worst,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());
                        expect(answer.path.success&&answer.path.step_count==result.path.step_count-static_cast<int>(step),
                            "repeated directions after ship movement reuse the valid remaining route and retain landing/walk phases");
                    }
                    std::printf("FF2 warm ship target (%d,%d): worst %.3f ms over up to20 movement requests (owned offline compute)\n",target.x,target.y,worst);
                }
                riding.cancelled=[] { return true; };
                expect(!land_after_ride(riding,reach).submap, "cancelled onboard reachability never publishes land results");
            }); ship_worker.join();
        }
        // A one-cell wrapped ship step must retain native cardinal direction.
        field::baked_route::Request edge;
        edge.submap=world; edge.vehicle_group=4; edge.ignore_hazards=true; edge.goals_open=false;
        const auto& grid=field::baked::grids(*world);
        bool wrap_case=false;
        for(int y=0; y<grid.height; ++y) if(field::baked::vehicle_open(grid,4,0,y) && field::baked::vehicle_open(grid,4,grid.width-1,y)) {
            edge.start={1,0,y}; edge.goals={{1,grid.width-1,y}};
            auto wrapped=field::baked_route::find(edge);
            auto directions=field::baked_route::legs(wrapped.cells,grid.width,grid.height);
            expect(wrapped.found && wrapped.steps()==1 && directions.size()==1 && directions[0].dir==3, "world left edge wraps west in A* and spoken directions");
            Phase phase; phase.cells=wrapped.cells; Corner corner;
            corner_of(*world,{phase},edge.start,corner);
            expect(corner.point.x < world_of_cell(*world,edge.start).x && corner.steps_to_corner==1, "wrapped beacon corner points along the westward route leg");
            wrap_case=true; break;
        }
        expect(wrap_case, "actual ship grids supply a wrapped-edge regression");
        field::baked_route::Request cancelled;
        cancelled.submap = world; cancelled.start = {1, 123, 34}; cancelled.goals = {{1, 107, 44}};
        cancelled.cancelled = [] { return true; };
        expect(!field::baked_route::find(cancelled).found && field::baked_route::flood(cancelled).submap == nullptr,
               "retired contexts cancel A* and reachability computation");
        RouteSnapshot refresh(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
        refresh.submap=world;refresh.map_id=world->first_map_id;refresh.layer=1;
        refresh.position=world_of_cell(*world,{1,126,129});refresh.vehicle={true,4,2,L"MSG_SYSTEM_200"};
        refresh.cancelled=[] {return true;};
        std::thread obsolete([&] {
            SnapshotScope scope(refresh);
            auto walking=walk_request(*world,{1,126,129});world_mode(walking);walking.cancelled=&planning_cancelled;
            std::vector<Phase> ignored;
            const auto begin=std::chrono::steady_clock::now();
            expect(!boarding_route(*world,field::baked::grids(*world),walking,{{1,110,98}},ignored),
                "destination refresh cancellation reaches nested vehicle planning, not only its first flood");
            std::printf("Cancelled FF2 vehicle refresh: %.3f ms tail (offline)\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());
        });obsolete.join();
    }
    int vehicle_cases=0, no_landing=0, parked_cases=0;
    int world_exit_cases=0, world_exit_reachable=0, world_exit_pruned=0;
    double worst_exit_ms=0;
    double worst_vehicle_ms=0, worst_vehicle_filter_ms=0;
    for(int number=1; number<=6; ++number) {
        bool parked_checked=false;
        fixture::game_id=static_cast<game::Id>(number);
        std::ifstream source("data/ff"+std::to_string(number)+"/maps.bin",std::ios::binary);
        std::string vehicle_bytes((std::istreambuf_iterator<char>(source)),{});
        expect(field::baked::load(vehicle_bytes.data(),vehicle_bytes.size()), "each game's actual vehicle grids load");
        std::ifstream state_file("data/ff"+std::to_string(number)+"/mapstate.json",std::ios::binary);
        std::string state_bytes((std::istreambuf_iterator<char>(state_file)),{});
        json::Value chest_records; expect(json::parse(state_bytes,chest_records), "all-six actual saved chest records load");
        field::map_state::Flags chest_flags;
        for(auto& v:chest_flags.scenario) v=0;
        for(auto& v:chest_flags.treasure) v=0;
        for(auto& v:chest_flags.global) v=0;
        failures += check_counter_routes(number,chest_flags,chest_records);
        if(number==6) {
            const auto* room=field::baked::submap_by_map_id(318);
            expect(room!=nullptr,"Terra defense uses the actual Narshe mines room 318");
            auto flags=chest_flags;flags.scenario[13]=1;
            RouteSnapshot defense(field::map_state::Resolver(flags,chest_records));
            defense.submap=room;defense.map_id=318;defense.layer=1;defense.locale=L"en";
            defense.position=world_of_cell(*room,{1,13,15});
            const auto& grid=field::baked::grids(*room);
            std::vector<field::baked::Object> encounters;
            for(const auto& object:grid.objects) {
                if(field::baked::str(object.group)!=L"ev_e_0018")continue;
                NativeObstacle actor;actor.type=static_cast<int>(object.type);actor.id=object.entity_id();
                actor.raw_name=field::baked::object_identifier(object);actor.cell={object.layer,object.tx,object.ty};
                actor.npc=object.type==field::baked::ObjType::NPC;
                defense.obstacles.push_back(actor);
                if(actor.npc&&(object.d&4))encounters.push_back(object);
            }
            expect(encounters.size()==7,"six contact encounters and Guard Leader retain distinct identities");
            {
                SnapshotScope scope(defense);
                const auto blockers=live_blockers(*room,grid,{});
                expect(blockers.gates.size()==2,"both ID-zero defense barriers actually enter the current collision snapshot");
            }
            auto moved_defense=defense;
            for(const auto& object:encounters) {
                GuideTarget candidate;candidate.goal.map_id=318;candidate.goal.cell={object.layer,object.tx,object.ty};
                candidate.goal.entity_id=object.entity_id();candidate.goal.raw_name=field::baked::object_identifier(object);
                candidate.kind=field::TargetKind::Interact;
                defense.candidates.push_back(candidate);
                if(object.entity_id()!=18)continue;
                moved_defense.goal=candidate.goal;moved_defense.kind=candidate.kind;
                for(auto& actor:moved_defense.obstacles)if(actor.id==18&&actor.npc)actor.cell={1,13,22};
                Planned result;
                std::thread moved([&]{result=plan_snapshot(moved_defense);});moved.join();
                expect(result.path.success&&result.goal.cell==Cell{1,13,22},"moving encounter rebinds to its captured current cell instead of its baked home");
                expect(!result.path.walk_phases.empty()&&result.path.walk_phases.back().arrival_direction>=0,
                    "moved encounter retains a facing interaction approach");
                moved_defense.obstacles.erase(std::remove_if(moved_defense.obstacles.begin(),moved_defense.obstacles.end(),[](const auto& actor){return actor.npc&&actor.id==18;}),moved_defense.obstacles.end());
                std::thread missing([&]{result=plan_snapshot(moved_defense);});missing.join();
                expect(!result.path.success&&result.path.reason==L"Destination is no longer available","defeated contact enemy cannot be resurrected from its baked candidate");
            }
            std::stable_sort(defense.candidates.begin(),defense.candidates.end(),[](const auto& a,const auto& b){return (a.goal.entity_id==9)<(b.goal.entity_id==9);});
            // Native encounters themselves occupy choke points. Clear a usable
            // enemy before asking for one behind it; never walk through the exit
            // barriers or pretend every enemy is initially accessible at once.
            int completed=0;
            for(int phase=0;phase<7;++phase) {
                Planned result;
                std::thread route([&]{result=plan_snapshot(defense);});route.join();
                expect(result.path.success,"each defense phase supplies a reachable encounter with every remaining enemy and both barriers intact");
                if(!result.path.success)break;
                ++completed;
                if(!result.path.walk_phases.empty()&&!result.path.walk_phases.back().cells.empty())
                    defense.position=world_of_cell(*room,result.path.walk_phases.back().cells.back());
                const int defeated=result.goal.entity_id;
                defense.obstacles.erase(std::remove_if(defense.obstacles.begin(),defense.obstacles.end(),[defeated](const auto& actor){return actor.npc&&actor.id==defeated;}),defense.obstacles.end());
            }
            expect(completed==7,"guidance reaches all six moving encounters and the story-advancing Guard Leader in a legal sequence");
            std::printf("FF6 Terra defense: seven real encounters, moving/defeated state and native exit barriers checked\n");
            int crossings=0,directional=0;
            for(int index=0;index<field::baked::submap_count();++index) {
                const auto* floor=field::baked::submap_at(index);
                for(const auto& crossing:field::baked::grids(*floor).objects) {
                    if(crossing.type!=field::baked::ObjType::Warp||(crossing.d&255)!=2)continue;
                    ++crossings;auto request=walk_request(*floor,{crossing.layer,crossing.tx,crossing.ty});
                    request.blocked={{crossing.layer,crossing.tx,crossing.ty}};
                    const auto goals=field::baked_route::goals_for(request,request.start,field::baked_route::GoalKind::Interact,&crossing);
                    if(goals.cells.empty())std::printf("Crossing without baked approach: %ls entity %d at %d,%d L%d mask %d; %ls\n",
                        field::baked::name_of(*floor).c_str(),crossing.entity_id(),crossing.tx,crossing.ty,crossing.layer,(crossing.d>>8)&15,
                        field::baked::object_identifier(crossing).c_str());
                    expect(!goals.cells.empty(),"reviewed manual crossing has an actual native approach");
                    auto reach=field::baked_route::flood(request,goals.cells);
                    expect(distance_to_object(request,reach,crossing,crossing.layer,false)==0,
                        "solid crossing uses its interaction approach as a graph edge, never its blocked actor cell");
                    const int mask=(crossing.d>>8)&15;
                    if(mask){++directional;for(const auto& c:goals.cells){
                        const int dx=c.x-crossing.tx,dy=c.y-crossing.ty;
                        const int side=dy<0?0:dx>0?1:dy>0?2:3;
                        expect((mask&(1<<side))!=0,"direction-dispatched crossing cannot use an unreviewed side");
                    }}
                    bool landing=false;
                    for(const auto& p:field::baked::grids(*floor).objects)
                        landing|=p.type==field::baked::ObjType::PointIn&&p.a==crossing.b;
                    expect(landing,"every reviewed same-floor crossing resolves its own factual landing");
                }
            }
            expect(crossings==87&&directional==69,"Phoenix, Zozo, cave bridges and tower ladder travel are present");
            std::printf("FF6 reviewed travel: %d solid interactions, %d approach restrictions checked\n",crossings,directional);
            const auto* phoenix=field::baked::submap_by_map_id(498);
            const auto& cave_record=*chest_records.get(field::baked::name_of(*phoenix));
            for(int index=1;index<=7;++index) {
                auto held=chest_flags;held.current_map_id=498;held.current_area_id=static_cast<int>(cave_record.get(L"area_id")->n);
                held.area.fill(0);held.local.fill(0);held.area[static_cast<size_t>(index)]=1;
                auto released=held;released.area[static_cast<size_t>(index)]=0;
                RouteSnapshot departing(field::map_state::Resolver(released,chest_records));
                departing.submap=phoenix;departing.map_id=498;departing.anticipated_release=std::make_shared<const field::map_state::Flags>(released);
                NativeObstacle npc;npc.npc=true;npc.type=6;npc.id=777;npc.cell={1,2,3};
                departing.obstacles.push_back(npc);
                Blockers projected;
                std::thread work([&]{SnapshotScope scope(departing);projected=live_blockers(*phoenix,field::baked::grids(*phoenix),{});});work.join();
                auto expected=walk_request(*phoenix,{1,2,3});field::map_state::Resolver state(released,chest_records);
                field::map_state::add_obstacles(state,*phoenix,expected,true,true);
                expect(projected.gates==expected.blocked&&projected.moving_npcs.size()==1,
                    "plate departure restores exact released-state gates while retaining captured NPC positions");
                expect(held.area[static_cast<size_t>(index)]==1,"counterfactual departure does not modify observed/native state");
            }
            std::printf("FF6 Phoenix: seven exact plate-release geometry predictions checked\n");
            const auto* rafters=field::baked::submap_by_map_id(117);
            auto opera_flags=chest_flags;opera_flags.scenario[167]=1;opera_flags.scenario[168]=1;
            RouteSnapshot opera(field::map_state::Resolver(opera_flags,chest_records));
            opera.submap=rafters;opera.map_id=117;opera.layer=1;opera.locale=L"en";
            bool entry=false;
            for(const auto& o:field::baked::grids(*rafters).objects) {
                if(o.type==field::baked::ObjType::PointIn&&o.a==1){opera.layer=o.layer;opera.position=world_of_cell(*rafters,{o.layer,o.tx,o.ty});entry=true;}
                if(o.type!=field::baked::ObjType::NPC||!((o.entity_id()>=9&&o.entity_id()<=13)||
                    (o.entity_id()==5&&field::baked::str(o.group)==L"ev_e_0344")))continue;
                NativeObstacle raw;raw.type=6;raw.id=o.entity_id();raw.npc=true;raw.cell={o.layer,o.tx,o.ty};raw.raw_name=field::baked::object_identifier(o);
                opera.obstacles.push_back(raw);GuideTarget candidate;candidate.goal={117,raw.cell};
                candidate.goal.entity_id=raw.id;candidate.goal.raw_name=raw.raw_name;candidate.kind=field::TargetKind::Interact;opera.candidates.push_back(candidate);
            }
            std::stable_sort(opera.candidates.begin(),opera.candidates.end(),[](const auto& a,const auto& b){return (a.goal.entity_id==5)<(b.goal.entity_id==5);});
            expect(entry&&opera.candidates.size()==6,"real Opera rafters entry, five contact rats and Ultros are captured");
            int cleared=0;
            for(int stage=0;stage<6;++stage) {
                Planned result;std::thread work([&]{result=plan_snapshot(opera);});work.join();
                expect(result.path.success,"rafters maze supplies a legal next encounter through remaining native rats");
                if(!result.path.success)break;
                ++cleared;
                if(!result.path.walk_phases.empty()&&!result.path.walk_phases.back().cells.empty())opera.position=world_of_cell(*rafters,result.path.walk_phases.back().cells.back());
                const auto id=result.goal.entity_id;
                opera.obstacles.erase(std::remove_if(opera.obstacles.begin(),opera.obstacles.end(),[id](const auto& npc){return npc.id==id;}),opera.obstacles.end());
            }
            expect(cleared==6,"rafters route reaches the native story-advancing Ultros encounter after the rats");
            std::printf("FF6 Opera rafters: %d encounter routes checked\n",cleared);
        }
        bool remote_checked=false;
        for(int index=0;index<field::baked::submap_count()&&!remote_checked;++index){
            const auto* here=field::baked::submap_at(index);if(here->world||here->first_map_id<=0)continue;
            std::vector<RemoteObject> closed;
            field::map_state::Resolver closed_state(chest_flags,chest_records);
            std::thread projection([&]{closed=connected_objects_snapshot(here->first_map_id,closed_state);});projection.join();
            for(const auto& target:closed){
                if(target.type!=field::baked::ObjType::TreasureBox||!target.opened.has_value())continue;
                const auto* floor=field::baked::submap_by_map_id(target.map_id);if(!floor)continue;
                const auto* record=chest_records.get(field::baked::name_of(*floor));
                const auto* records=record?record->get(L"treasures"):nullptr;if(!records)continue;
                int bit=-1;
                for(const auto& object:field::baked::grids(*floor).objects){
                    if(object.type!=target.type||object.tx!=target.cell.x||object.ty!=target.cell.y||field::baked::str(object.group)!=target.group)continue;
                    for(const auto& row:records->arr){const auto& r=row.arr;if(r.size()>=3&&static_cast<int>(r[0].n)==object.entity_id()&&r[1].s==target.group)bit=static_cast<int>(r[2].n);}
                    break;
                }
                if(bit<0||bit>=1024)continue;
                auto updated=chest_flags;updated.treasure[static_cast<size_t>(bit)]=1;
                field::map_state::Resolver open_state(updated,chest_records);
                std::vector<RemoteObject> opened;
                std::thread reprojection([&]{opened=connected_objects_snapshot(here->first_map_id,open_state);});reprojection.join();
                bool retained=false;
                for(const auto& row:opened)if(row.map_id==target.map_id&&row.cell==target.cell&&row.type==target.type&&row.group==target.group){retained=row.opened==std::optional<bool>(true);break;}
                expect(!*target.opened&&retained,"Current map retains opened other-floor chests and copies the saved state on its worker");
                remote_checked=true;break;
            }
        }
        expect(remote_checked,"each game exercises an actual connected other-floor chest");
        for(int index=0; index<field::baked::submap_count(); ++index) {
            const auto* map=field::baked::submap_at(index);
            if(!map->world) continue;
            const auto& grid=field::baked::grids(*map);
            if (!parked_checked) for(const auto& transport:field::baked::transports()) {
                if(transport.type==1||transport.type==5||transport.group<=0||transport.group>static_cast<int>(grid.transport.size()))continue;
                Cell shore,parking;bool found=false;
                field::baked_route::Request walking;walking.submap=map;
                for(int y=0;y<grid.height&&!found;++y)for(int x=0;x<grid.width&&!found;++x) {
                    const Cell at{1,x,y};if(!field::baked_route::open(walking,grid,at))continue;
                    for(const auto& side:std::vector<Cell>{{0,0,-1},{0,1,0},{0,0,1},{0,-1,0}}) {
                        Cell beside{1,(x+side.x+grid.width)%grid.width,(y+side.y+grid.height)%grid.height};
                        if(field::baked::vehicle_open(grid,transport.group,beside.x,beside.y)&&
                            field::baked::group_crosses(transport.group,field::baked::attribute(grid,beside.x,beside.y))) {
                            shore=at;parking=beside;found=true;break;
                        }
                    }
                }
                if(!found)continue;
                ExitSnapshot capture(true,field::map_state::Resolver(chest_flags,chest_records));
                auto& parked=capture.route;parked.submap=map;parked.map_id=map->first_map_id;parked.layer=1;
                parked.position=world_of_cell(*map,shore);
                field::VehicleOnMap vehicle;vehicle.transportation_id=transport.id;vehicle.type=transport.type;vehicle.pos=world_of_cell(*map,parking);parked.vehicles={vehicle};
                std::vector<ExitTarget> exits;std::vector<const field::baked::Object*> sources;
                for(const auto& object:grid.objects)if(object.type==field::baked::ObjType::GotoMap) {
                    exits.push_back({world_of_cell(*map,{1,object.tx,object.ty}),0,object.a,{1,object.tx,object.ty}});sources.push_back(&object);
                }
                if(exits.empty())continue;
                std::thread parked_worker([&] {
                    auto copy=vehicle.pos;
                    copy.x+=map->width*16.0f;copy.y-=map->height*16.0f;
                    expect(cell_of(*map,copy,1)==parking,
                        "all-six parked vehicles share canonical cells across both world-map axes");
                    const auto began=std::chrono::steady_clock::now();
                    const auto distances=exit_distances(capture,exits,{});
                    const double bulk_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                    std::vector<StoryTarget> story_targets;
                    for(const auto& exit:exits){MapGoal target;target.map_id=parked.map_id;target.cell=exit.cell;story_targets.push_back({target,field::TargetKind::Transition});}
                    const auto mask=story_gate_mask(capture,story_targets,{});
                    expect(std::none_of(mask.begin(),mask.end(),[](bool hidden){return hidden;}),
                        "owned parked-vehicle access is never misclassified as a story gate on any bake");
                    SnapshotScope scope(parked);
                    auto req=walk_request(*map,shore);const auto live=live_blockers(*map,grid,{});
                    req.blocked=live.hard();req.directional=live.directional;req.live_hazards=true;req.hazards=live.triggers;req.live_traps=true;req.traps=live.traps;
                    const auto reachable=field::baked_route::flood(req);
                    const auto reference_began=std::chrono::steady_clock::now();
                    for(size_t i=0;i<sources.size();++i) {
                        int reference=-1;
                        for(const auto& object:grid.objects) {
                            if(object.type!=field::baked::ObjType::GotoMap||object.a!=exits[i].destination||
                                !object.contains(sources[i]->tx,sources[i]->ty)||!parked.state.active(*map,object))continue;
                            auto request=req;
                            for(int y=object.ty;y<object.ty+object.th;++y)for(int x=object.tx;x<object.tx+object.tw;++x)
                                if(field::baked_route::transition_enterable(request,grid,x,y))request.goals.push_back({1,x,y});
                            reference=field::baked_route::transition_distance(request,reachable);
                            if(reference<0&&!request.goals.empty()) {
                                std::vector<Phase> route;
                                if(boarding_route(*map,grid,request,request.goals,route)) {
                                    reference=1;
                                    for(const auto& phase:route)if(phase.vehicle_before==field::route_progress::VehicleAction::WalkOn || phase.vehicle_before==field::route_progress::VehicleAction::WalkOff) {
                                        expect(phase.cells.size()>1&&phase.vehicle_direction==field::baked_route::direction_towards(phase.cells[0],phase.cells[1],map->width,map->height)&&
                                            phase.action_before.ends_with(direction_key(phase.vehicle_direction)),
                                            "all-six watercraft route actions speak their exact physical boarding/landing direction");
                                    }
                                }
                            }
                            if(reference>=0)break;
                        }
                        expect((distances[i]>=0)==(reference>=0),"parked-vehicle bulk distances retain exactly the exits usable by full route construction");
                    }
                    const double reference_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-reference_began).count();
                    std::printf("FF%d parked vehicle %d: %zu exits, bulk %.3f ms, previous per-exit route reference %.3f ms (offline)\n",number,transport.id,exits.size(),bulk_ms,reference_ms);
                    int checks=0;parked.cancelled=[&]{return ++checks>500;};req.cancelled=&planning_cancelled;
                    const auto cancel_began=std::chrono::steady_clock::now();
                    const auto cancelled=parked_vehicle_reach(req,reachable);
                    expect(!cancelled.submap&&checks<=510,"parked-vehicle refresh cancellation reaches its inner riding/landing search promptly");
                    std::printf("FF%d parked cancellation: %.3f ms, %d token checks\n",number,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-cancel_began).count(),checks);
                    parked.cancelled={};
                });parked_worker.join();
                parked_checked=true;++parked_cases;break;
            }
            {
                using namespace field::filter_worker;
                FilterInput input; input.world=true; input.exits=true; input.sort=true;
                input.navigation=std::make_shared<ExitSnapshot>(false,field::map_state::Resolver(field::map_state::Flags{},chest_records));
                auto& copied=input.navigation->route;
                copied.submap=map; copied.map_id=map->first_map_id; copied.layer=1;
                bool start=false;
                for (int y=0;y<grid.height&&!start;++y) for (int x=0;x<grid.width&&!start;++x)
                    if (field::baked::vehicle_open(grid,1,x,y)) { copied.position=world_of_cell(*map,{1,x,y}); start=true; }
                input.player=copied.position;
                for (const auto& object:grid.objects) {
                    if (object.type!=field::baked::ObjType::GotoMap) continue;
                    FilterRow row; row.index=input.rows.size(); row.kind=field::Kind::MapExit;
                    row.category=field::Category::MapExits; row.destination=object.a;
                    row.position=row.numbering_position=world_of_cell(*map,{1,object.tx,object.ty});
                    row.cell_x=object.tx; row.cell_y=object.ty; row.cell_layer=1;
                    input.rows.push_back(std::move(row));
                }
                Categories categories;
                std::thread exit_worker([&] {
                    const auto began=std::chrono::steady_clock::now(); categories=build_categories(input);
                    worst_exit_ms=std::max(worst_exit_ms,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
                }); exit_worker.join();
                auto movement=walk_request(*map,cell_of(*map,copied.position,1));
                std::map<int,int> minimum;
                for (const auto& row:input.rows) {
                    ++world_exit_cases;
                    if(row.route_distance<0) { ++world_exit_pruned; continue; }
                    ++world_exit_reachable;
                    expect(field::baked_route::transition_enterable(movement,grid,row.cell_x,row.cell_y),"every retained world exit is on enterable terrain");
                    auto found=minimum.find(row.destination);
                    if(found==minimum.end()||row.route_distance<found->second) minimum[row.destination]=row.route_distance;
                }
                std::unordered_set<int> seen;
                for(size_t index:categories.views[0].order) {
                    const auto& row=input.rows[index];
                    expect(row.route_distance>=0&&minimum[row.destination]==row.route_distance&&seen.insert(row.destination).second,
                        "all-six world grouping keeps exactly the nearest reachable trigger per destination");
                    RouteSnapshot route(field::map_state::Resolver(field::map_state::Flags{},chest_records));
                    route.submap=map;route.map_id=map->first_map_id;route.layer=1;route.position=copied.position;
                    route.goal.map_id=route.map_id;route.goal.cell={1,row.cell_x,row.cell_y};route.kind=field::TargetKind::Transition;route.locale=L"en";
                    std::thread route_worker([&]{result=plan_snapshot(route);});route_worker.join();
                    expect(result.path.success,"retained grouped world exit is usable by the production route planner");
                }
                expect(seen.size()==minimum.size(),"group filter preserves every reachable world destination");
                input.exits=false; const auto ungrouped=build(input);
                expect(ungrouped.order.size()==static_cast<size_t>(std::count_if(input.rows.begin(),input.rows.end(),[](const auto& row){return row.route_distance>=0;})),
                    "grouping off retains each reachable trigger and still prunes invalid world tiles");
                if(!categories.views[0].order.empty()) {
                    const auto index=categories.views[0].order.front(); const auto& row=input.rows[index];
                    NativeObstacle gate;gate.type=19;gate.cell={-1,row.cell_x,row.cell_y};gate.collider=true;gate.collision=1;
                    copied.obstacles.push_back(gate);
                    StoryTarget target;target.goal.map_id=copied.map_id;target.goal.cell={1,row.cell_x,row.cell_y};target.kind=field::TargetKind::Transition;
                    std::vector<bool> story_mask;
                    std::thread story([&]{story_mask=story_gate_mask(*input.navigation,{target},{});});story.join();
                    expect(story_mask==std::vector<bool>({true}),"all-six world story filter proves a captured gate without confusing world layers");
                    std::thread gated([&]{categories=build_categories(input);});gated.join();
                    expect(input.rows[index].route_distance<0,"copied native story collider vetoes a world transition before grouping");
                }
            }
            for(const auto& row:field::baked::transports()) {
                if(row.type==0 || row.type==1 || row.type==5 || row.group<=0 || row.group>static_cast<int>(grid.transport.size())) continue;
                Cell start; bool found=false;
                for(int y=0; y<grid.height && !found; ++y) for(int x=0; x<grid.width; ++x)
                    if(field::baked::vehicle_open(grid,row.group,x,y)) { start={1,x,y}; found=true; break; }
                if(!found) { ++no_landing; continue; }
                field::baked_route::Request riding;
                riding.submap=map; riding.start=start; riding.vehicle_group=row.group;
                riding.vehicle_type=row.type; riding.landing_group=row.landing_group; riding.ignore_hazards=true;
                for(const auto& owned:field::baked::transports()) if(owned.type==5) riding.auto_groups.push_back(owned.group);
                field::baked_route::Reach ride, landed;
                std::thread reach_worker([&] {
                    const auto started=std::chrono::steady_clock::now();
                    ride=field::baked_route::flood(riding); landed=land_after_ride(riding,ride);
                    worst_vehicle_filter_ms=std::max(worst_vehicle_filter_ms,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count());
                }); reach_worker.join();
                auto foot=riding; foot.vehicle_group=0;
                Cell target; bool destination=false;
                for(const auto& object:grid.objects) {
                    if(object.type!=field::baked::ObjType::GotoMap) continue;
                    Cell cell{1,object.tx,object.ty};
                    const auto goals=field::baked_route::goals_for(foot,cell,field::baked_route::GoalKind::Transition);
                    if(any_reachable(landed,goals.cells)) { target=cell; destination=true; break; }
                }
                if(!destination) { ++no_landing; continue; }
                RouteSnapshot onboard(field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                onboard.submap=map; onboard.map_id=map->first_map_id; onboard.layer=1; onboard.position=world_of_cell(*map,start);
                onboard.vehicle={true,row.id,row.type,field::baked::str(row.message)}; onboard.auto_groups=riding.auto_groups;
                onboard.goal.map_id=onboard.map_id; onboard.goal.cell=target; onboard.kind=field::TargetKind::Transition; onboard.locale=L"en";
                // Exercise the actual exit-filter path while already aboard,
                // rather than inferring its behavior from a successful route.
                {
                    SnapshotScope scope(onboard);
                    const auto inferred=goals_for(*map,target,field::TargetKind::Infer);
                    const auto explicit_goal=goals_for(*map,target,field::TargetKind::Transition);
                    expect(!inferred.cells.empty()&&inferred.cells==explicit_goal.cells,
                        "all-six mounted filter inference agrees with explicit destination footprint");
                }
                for(bool with_vehicles : {false,true}) {
                    ExitSnapshot filtered(with_vehicles,field::map_state::Resolver(field::map_state::Flags{},fixture::records));
                    filtered.route.submap=map;filtered.route.map_id=onboard.map_id;filtered.route.layer=1;
                    filtered.route.position=onboard.position;filtered.route.vehicle=onboard.vehicle;filtered.route.auto_groups=onboard.auto_groups;
                    std::vector<ExitTarget> exits;
                    for(const auto& object:grid.objects) if(object.type==field::baked::ObjType::GotoMap)
                        exits.push_back({world_of_cell(*map,{1,object.tx,object.ty}),0,object.a,{1,object.tx,object.ty}});
                    std::vector<int> distances;
                    std::thread filter([&]{distances=exit_distances(filtered,exits,{});});filter.join();
                    if(number==6 && (row.type==3||row.type==8))
                        expect(std::any_of(distances.begin(),distances.end(),[](int d){return d>=0;}),
                            "both FF6 floor and vehicles filters retain destinations reachable from an already flying airship");
                }
                std::thread vehicle_worker([&] {
                    const auto started=std::chrono::steady_clock::now(); result=plan_snapshot(onboard);
                    worst_vehicle_ms=std::max(worst_vehicle_ms,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count());
                }); vehicle_worker.join();
                if(!result.path.success) std::printf("FAIL FF%d world %d vehicle %d type %d group %d: onboard destination (%d,%d)\n",number,index,row.id,row.type,row.group,target.x,target.y);
                expect(result.path.success, "representative boarded vehicles reach their world-map land destination"); ++vehicle_cases;
                for(const auto& phase:result.path.walk_phases) expect(phase.action_before.find(L"board")==std::wstring::npos, "already-boarded plans never require boarding again");
            }
        }
    }
    expect(vehicle_cases>=20, "all-six boarded-vehicle regression has representative coverage");
    expect(parked_cases==6,"all six games exercise walking plus a parked vehicle during destination refresh");
    std::printf("All-six boarded vehicles: %d world/transport cases; %d without eligible start/land destination; worst route %.3f ms, filter %.3f ms (offline)\n",vehicle_cases,no_landing,worst_vehicle_ms,worst_vehicle_filter_ms);
    std::printf("All-six world exits: %d candidates, %d reachable, %d pruned from representative starts; worst six-category preparation %.3f ms (offline)\n",world_exit_cases,world_exit_reachable,world_exit_pruned,worst_exit_ms);
    expect(fixture::wrong_owner == 0, "world search/cancellation also avoid all native APIs");
    std::printf("Route worker check: %d failures\n", failures); return failures ? 1 : 0;
}
