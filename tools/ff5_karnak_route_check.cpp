// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace karnak_routes {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;
using EscapeRoute = ff5::karnak::Route;

map_state::Flags flags(int map, int encountered, bool timer = true, bool escaped = false, bool explosion = false) {
    auto value = exdeath_routes::flags(map,0,0);
    value.current_area_id = map == 1 ? 1 : map == 581 ? 101 : 24;
    value.scenario[359] = 0;
    value.scenario[353] = 1; // The wall blast precedes the timer and ordinary castle access.
    value.scenario[42] = value.scenario[43] = 1;
    value.scenario[46] = timer;
    value.scenario[245] = encountered;
    value.scenario[47] = escaped;
    value.scenario[992] = explosion;
    return value;
}

ff5::library::Plan advice(int map, Cell at, const map_state::Flags& state,
                         std::wstring guide = {}, puzzle_policy::Policy policy = {true,true},
                         std::optional<int> seconds = {}, EscapeRoute mode = EscapeRoute::EquipmentThenElixirs) {
    ff5::mechanics::Snapshot snapshot;
    snapshot.map = map;
    snapshot.player = at;
    snapshot.flags = state;
    snapshot.guide_id = std::move(guide);
    snapshot.locale = L"en";
    snapshot.policy = policy;
    snapshot.karnak_seconds = seconds;
    snapshot.karnak_route = mode;
    return ff5::mechanics::plan(snapshot,fixture::guide_recipes);
}

struct Position { int map; Cell cell; };
constexpr Position timer_start{102,{1,4,7}};

// Feed each exact native landing back to the production planner.
int traverse(Position from, const MapGoal& goal, TargetKind kind, Position& finish,
             map_state::Flags states, bool require_facing = false) {
    int steps = 0;
    for (int floors=0; floors<20; ++floors) {
        states.current_map_id = from.map;
        auto leg = route(from.map,from.cell,states,goal,kind);
        if (!leg.path.success || leg.path.walk_phases.empty()) return -1;
        steps += leg.path.step_count + (leg.path.next_map_id ? 4 : 0);
        if (!leg.path.next_map_id) {
            const auto& last = leg.path.walk_phases.back();
            if (from.map != goal.map_id || last.cells.empty() ||
                (require_facing && (last.arrival_direction < 0 || last.manual_after))) return -1;
            finish = {from.map,last.cells.back()};
            return steps;
        }
        const auto& hop = leg.pending.incoming_transition;
        if (!hop) return -1;
        from = {hop->destination,hop->arrival};
    }
    return -1;
}

void verify_loot() {
    const auto chests = ff5::mechanics::karnak_chests(fixture::guide_recipes);
    check(chests.size()==15,"all 15 distinct treasure flags have exact native objects");
    if (chests.size()!=15) return;
    auto state = flags(timer_start.map,0);
    Position at=timer_start;
    int distance=0;
    constexpr std::array expected{86,87,88,89,99,92,93,96,97,98,94,95,100,90,91};
    for (const int receipt : expected) {
        const auto plan=advice(at.map,at.cell,state,L"karnak_escape",{true,true},600,EscapeRoute::AllChests);
        check(plan.karnak_treasure_flag==receipt && plan.targets.size()==1,
              "full countdown selects the shortest room-grouped chest sequence");
        if (plan.karnak_treasure_flag!=receipt || plan.targets.size()!=1) return;
        const auto& next=plan.targets.front();
        Position reached;
        const int steps=traverse(at,next.goal,next.kind,reached,state,true);
        check(steps>=0 && reached.cell==next.goal.cells.front(),
              "every chest route reaches the planned exact facing stance");
        if (steps<0 || reached.cell!=next.goal.cells.front())
            std::printf("Chest %d reached L%d (%d,%d), planned L%d (%d,%d), route cost %d\n",
                        receipt,reached.cell.layer,reached.cell.x,reached.cell.y,next.goal.cells.front().layer,
                        next.goal.cells.front().x,next.goal.cells.front().y,steps);
        if (steps<0) return;
        distance+=steps;
        state.treasure[receipt]=1;
        at=reached;
    }
    const auto exit=advice(at.map,at.cell,state,L"karnak_escape",{true,true},181,EscapeRoute::AllChests);
    check(exit.targets.size()==1 && exit.targets.front().goal.entity_id==29,
          "all collected chests hand off to the native courtyard fight");
    if (exit.targets.size()==1) {
        Position outside;
        const auto steps=traverse(at,exit.targets.front().goal,exit.targets.front().kind,outside,state);
        check(steps>=0 && distance+steps==674,"full loot mode matches the measured physical route cost");
    }
    state=flags(timer_start.map,0);at=timer_start;
    for (const int flag : {88,99,100}) {
        const auto plan=advice(at.map,at.cell,state,L"karnak_escape",{true,true},350);
        const auto equipment_only=advice(at.map,at.cell,state,L"karnak_escape",{true,true},350,EscapeRoute::EquipmentOnly);
        check(plan.karnak_treasure_flag==flag && equipment_only.karnak_treasure_flag==flag,
              "both equipment modes route to Ribbon, Elven Mantle and Main Gauche first");
        if(plan.targets.empty())return;
        const auto& target=plan.targets.front();
        Position reached;
        check(traverse(at,target.goal,target.kind,reached,state,true)>=0,"equipment leg uses the native castle route");
        if (reached.map!=target.goal.map_id) return;
        state.treasure[flag]=1;
        at=reached;
    }
    const auto gear_exit=advice(at.map,at.cell,state,L"karnak_escape",{true,true},320,EscapeRoute::EquipmentOnly);
    check(gear_exit.karnak_treasure_flag<0 && gear_exit.targets.size()==1 && gear_exit.targets.front().goal.entity_id==29,
          "equipment-only mode leaves as soon as all three items are collected");
    for (int i=0;i<6;++i) {
        const auto next=advice(at.map,at.cell,state,L"karnak_escape",{true,true},181);
        const auto flag=next.karnak_treasure_flag;
        check(flag==87||flag==93||flag==94||flag==96||flag==97||flag==98,
              "equipment-then-Elixirs selects only an unopened Elixir at 3:01");
        if (flag<0 || next.targets.empty()) return;
        const auto& goal=next.targets.front().goal;
        state.treasure[flag]=1;at={goal.map_id,goal.cells.front()};
    }
    const auto done=advice(at.map,at.cell,state,L"karnak_escape",{true,true},300);
    check(done.karnak_treasure_flag<0&&done.targets.size()==1&&done.targets.front().goal.entity_id==29,
          "default mode leaves after the final Elixir even with time available");
    for (const auto mode : {EscapeRoute::EquipmentThenElixirs,EscapeRoute::EquipmentOnly,EscapeRoute::AllChests}) {
        const auto hurry=advice(108,{1,4,7},flags(108,0),L"karnak_escape",{true,true},180,mode);
        check(hurry.karnak_treasure_flag<0 && hurry.targets.size()==1 && hurry.targets.front().goal.entity_id==29 &&
              hurry.targets.front().instruction.find(L"Time is short")!=std::wstring::npos,
              "at exactly three minutes every mode goes directly to the courtyard fight");
    }
    const auto separate = [&](int opened, int closed) {
        auto two_gil=flags(105,0);
        two_gil.treasure[opened]=1;
        map_state::Resolver resolver(two_gil,fixture::records);
        const auto& floor=*baked::submap_by_map_id(105);
        const auto& objects=baked::grids(floor).objects;
        for (const int flag : {opened,closed}) {
            const auto& goal=chests[size_t(flag-86)].target.goal;
            const auto object=std::find_if(objects.begin(),objects.end(),[&](const auto& item) {
                return item.type==baked::ObjType::TreasureBox && item.entity_id()==0 &&
                       baked::object_identifier(item)==goal.raw_name;
            });
            check(object!=objects.end() && resolver.treasure_opened(floor,*object)==(flag==opened),
                  "the two ID-zero gil chests keep independent opened states");
        }
    };
    separate(92,95); separate(95,92);
}

int verify() {
    const auto& basement = *baked::submap_by_map_id(101);
    const auto& objects = baked::grids(basement).objects;
    const auto wall = std::find_if(objects.begin(),objects.end(),[](const auto& object) {
        return object.type == baked::ObjType::Entity && object.entity_id() == 34;
    });
    if (wall == objects.end()) return 2;
    auto before_blast = flags(101,0,false);
    before_blast.scenario[42] = before_blast.scenario[43] = before_blast.scenario[353] = 0;
    map_state::Resolver closed(before_blast,fixture::records);
    map_state::Resolver opened(flags(101,0),fixture::records);
    check((wall->d & 1) && closed.active(basement,*wall) && !opened.active(basement,*wall),
          "the original blast wall blocks the basement before the native receipt and clears after it");

    int map = 102;
    Cell at{1,4,7};
    int stair_legs = 0;
    for (; stair_legs < 12; ++stair_legs) {
        const auto state = flags(map,0);
        const auto plan = advice(map,at,state);
        check(plan.supported && !plan.unknown && plan.targets.size() == 1 && plan.targets[0].goal.entity_id == 29,
              "countdown uses the actual exit encounter as its next destination");
        if (plan.targets.empty()) return 2;
        const auto& target = plan.targets[0];
        const auto result = route(map,at,state,target.goal,target.kind);
        check(result.path.success && !result.path.required_trap,"each actual escape leg uses ordinary stairs and terrain");
        if (!result.path.success) {
            std::printf("Escape map %d at %d,%d,%d: %ls / %ls\n",map,at.layer,at.x,at.y,
                        result.path.error.c_str(),result.path.reason.c_str());
            return 3;
        }
        if (map == 95) break;
        const auto& incoming = result.pending.incoming_transition;
        if (!incoming) return 4;
        map = incoming->destination;
        at = incoming->arrival;
    }
    check(map == 95 && stair_legs > 1 && stair_legs < 12,"the actual timer start connects through every intervening native stair");
    const auto normal = advice(102,{1,4,7},flags(102,0,false));
    check(!normal.supported && normal.targets.empty(),"the ordinary castle visit never starts the timed escape recipe");
    const auto after_battle = advice(95,{1,23,35},flags(95,1));
    check(after_battle.targets.size() == 3,"native encounter return selects all three actual escape contacts");
    for (const auto& target : after_battle.targets) {
        const auto result = route(95,{1,23,35},flags(95,1),target.goal,target.kind);
        check(result.path.success && !result.path.required_trap,"actual exit contact is usable without a false trap warning");
    }
    MapGoal world;
    world.map_id = 1;
    world.cell = {1,83,78};
    const auto ordinary = route(95,{1,23,35},flags(95,1),world,TargetKind::Spot);
    check(ordinary.path.success && !ordinary.path.required_trap,"ordinary routing can use the actual scripted exit");
    const auto& arrival = ordinary.pending.incoming_transition;
    check(arrival && arrival->destination == 1 && arrival->arrival.x == 83 && arrival->arrival.y == 78,
          "the world route resumes at the full native landing after the explosion");
    const auto before = route(95,{1,23,35},flags(95,0,false),world,TargetKind::Spot);
    check(before.path.success && !before.path.required_trap,"the normal courtyard exit remains available before the countdown");
    check(!advice(1,{2,84,78},flags(1,1),L"karnak_escape").finished &&
          advice(1,{2,84,78},flags(1,1),L"karnak_escape").targets.empty(),"first world arrival cannot finish or route back through the castle");
    check(!advice(581,{2,19,8},flags(581,1,true,false,true),L"karnak_escape").finished,
          "the explosion's own receipt does not end guidance inside the cinematic");
    check(advice(1,{2,83,78},flags(1,1,true,true,true),L"karnak_escape").finished,
          "only the full native world callback completes the escape");
    check(!advice(1,{2,83,78},flags(1,1,true,true,true)).supported,"ordinary world entry never activates the castle recipe");
    check(advice(102,{1,4,7},flags(102,0),{}, {false,true}).targets.empty(),"Navigation toggle independently controls the timed route");
    verify_loot();
    std::printf("Karnak production: %d assertions, %d route legs, %d stairs, %d failures\n",
                exdeath_routes::assertions,exdeath_routes::legs,stair_legs,exdeath_routes::failures);
    return fixture::wrong_owner || exdeath_routes::failures ? 5 : 0;
}

// Report shortest production routes among the save point, exact chest
// approaches and courtyard encounter. Ordinary room exits retain their
// native arrival poses on each leg.
int print_chest_distances() {
    const auto chests = ff5::mechanics::karnak_chests(fixture::guide_recipes);
    if (chests.size() != 15) return 2;
    const Position start=timer_start;
    const auto state=flags(start.map,0);
    std::vector<Position> origins{start};
    std::vector<MapGoal> goals;
    for (const auto& chest : chests) {
        Position at;
        auto goal = chest.target.goal;
        goal.cells.clear();
        if (traverse(start,goal,chest.target.kind,at,state) < 0) {
            std::printf("Chest flag %d cannot be reached\n",chest.flag);
            return 3;
        }
        origins.push_back(at);
        goal.cells = {at.cell};
        goals.push_back(std::move(goal));
        std::printf("Chest %d approach %d %d %d %d\n",chest.flag,at.map,at.cell.layer,at.cell.x,at.cell.y);
    }
    const auto exit = advice(start.map,start.cell,flags(start.map,0)).targets.front();
    std::printf("Karnak distance matrix rows (last column: courtyard)\n");
    for (const auto& origin : origins) {
        Position end;
        for (const auto& goal : goals) std::printf("%d,",traverse(origin,goal,TargetKind::Interact,end,state));
        std::printf("%d\n",traverse(origin,exit.goal,exit.kind,end,state));
    }
    return 0;
}
}

int main(int argc, char** argv) {
    using namespace karnak_routes;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read = [](const char* path) {
        std::ifstream stream(path,std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(stream),{});
    };
    const auto bytes = read("data/ff5/maps.bin");
    if (!baked::load(bytes.data(),bytes.size()) || !json::parse(read("data/ff5/mapstate.json"),fixture::records) ||
        !json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes)) return 2;
    fixture::game_id = game::Id::FF5;
    g_loaded = g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = argc==2 && std::string_view(argv[1])=="--matrix" ? print_chest_distances() : verify(); });
    worker.join();
    return result;
}
