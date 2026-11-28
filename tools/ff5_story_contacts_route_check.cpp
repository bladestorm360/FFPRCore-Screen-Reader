// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main
#include "../src/core/field/target_kind.h"

namespace story_contacts {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

const baked::Object& object(int map, int entity) {
    const auto& objects = baked::grids(*baked::submap_by_map_id(map)).objects;
    const auto actor = std::find_if(objects.begin(),objects.end(),[entity](const auto& row) {
        return row.entity_id() == entity;
    });
    if (actor == objects.end()) std::abort();
    return *actor;
}

bool active(const map_state::Flags& state, int entity) {
    map_state::Resolver resolver(state,fixture::records);
    const auto& room = *baked::submap_by_map_id(state.current_map_id);
    return resolver.active(room,object(state.current_map_id,entity));
}

bool reach(const map_state::Flags& state, Cell from, int entity, TargetKind kind, int vehicle = 1) {
    const auto& actor = object(state.current_map_id,entity);
    MapGoal goal;
    goal.map_id = state.current_map_id;
    goal.cell = {actor.layer,actor.tx,actor.ty};
    goal.entity_id = entity;
    goal.raw_name = baked::object_identifier(actor);
    const auto result = route(goal.map_id,from,state,goal,kind,false,vehicle);
    if (!result.path.success) std::printf("Contact %d/%d: %ls / %ls\n",goal.map_id,entity,
                                          result.path.error.c_str(),result.path.reason.c_str());
    return result.path.success && !result.path.required_trap;
}

void verify_walse_inventory(map_state::Flags flags)
{
    using namespace filter_worker;
    const auto& world = *baked::submap_by_map_id(1);
    const auto& entrance = object(1,64);
    check(entrance.type == baked::ObjType::GotoMap && entrance.a == 234,
          "the reported world row is the original Tower of Walse entrance");
    for (const auto sunk : {std::optional<int>{0}, std::optional<int>{1}, std::optional<int>{}}) {
        flags.scenario[991] = sunk;
        auto state = std::make_shared<map_state::Resolver>(flags,fixture::records);
        const bool exists = sunk != 1;
        check(state->active(world,entrance) == exists,"only the known sinking removes the original tower edge");
        for (bool reachable : {true,false}) {
            FilterInput input;
            input.current_map = 1; input.world = true; input.reachable_only = reachable; input.state = state;
            for (const auto& [id,dest,cell] : {std::tuple{64,234,Cell{1,196,88}},
                                               std::tuple{48,244,Cell{1,204,84}}}) {
                FilterRow row;
                row.index = input.rows.size(); row.entity_id = id; row.destination = dest;
                row.category = Category::MapExits; row.kind = Kind::MapExit;
                row.position = row.numbering_position = world_of_cell(world,cell);
                row.route_distance = 1; // positive ranking cannot resurrect a retired destination
                input.rows.push_back(std::move(row));
            }
            const auto categories = build_categories(input);
            const std::vector<size_t> expected = exists ? std::vector<size_t>{0,1} : std::vector<size_t>{1};
            check(categories.views[0].order == expected &&
                    categories.views[static_cast<size_t>(Category::MapExits)].order == expected &&
                    categories.views[0].exits.size() == expected.size() && categories.audio_order == expected,
                  "all world inventory modes omit the sunk tower without hiding the meteorite");
        }
    }
}

void verify_meteorite_journey()
{
    auto flags = exdeath_routes::flags(1, 0, 0);
    flags.current_area_id = 1;
    for (int receipt : {37,38,39,40,43,44,45,46,47,991,992}) flags.scenario[receipt] = 1;
    const auto& world = *baked::submap_by_map_id(1);
    const auto& exits = baked::grids(world).objects;
    const auto find_exit = [&](int id) {
        const auto found = std::find_if(exits.begin(), exits.end(), [id](const auto& o) {
            return o.type == baked::ObjType::GotoMap && o.entity_id() == id;
        });
        if (found == exits.end()) std::abort();
        return MapGoal{1, {1,found->tx,found->ty}, false, {}, 0, id, baked::object_identifier(*found)};
    };
    const auto tule = find_exit(9);
    const Cell escaped_castle{1,84,78}, walse_world{1,205,84}, parked_drake{1,204,83};
    const MapGoal karnak{1,escaped_castle};
    const auto replay = [&](Cell at, const MapGoal& goal, TargetKind kind, std::initializer_list<int> maps,
                           std::optional<Cell> vehicle = {}) {
        int current = 1;
        for (const int expected : maps) {
            auto state = flags;
            state.current_map_id = current;
            const auto result = route(current, at, state, goal, kind, false, 1, {}, {}, {}, vehicle);
            const auto& hop = result.pending.incoming_transition;
            if (!result.path.success || result.path.next_map_id != expected || !hop || hop->destination != expected) {
                std::printf("Meteorite leg %d/%d,%d expected %d: %ls / %ls (next %d)\n", current, at.x, at.y,
                    expected, result.path.reason.c_str(), result.path.error.c_str(), result.path.next_map_id);
                return false;
            }
            const auto pad = current == 247 && expected == 246 ? std::optional<Cell>{Cell{1,15,14}} :
                current == 246 && expected == 247 ? std::optional<Cell>{Cell{1,14,14}} : std::nullopt;
            if (pad && hop->arrival != *pad) {
                std::printf("Meteorite leg %d -> %d landed at %d,%d L%d\n", current, expected,
                    hop->arrival.x, hop->arrival.y, hop->arrival.layer);
                return false;
            }
            current = expected;
            at = hop->arrival;
            if (current == 1) at.layer = 1; // World movement uses one layer even for native Upper PointIn.
        }
        const auto last = route(current, at, flags, goal, kind, false, 1, {}, {}, {}, vehicle).path;
        const bool complete = current == 1 && last.success && !last.next_map_id && !last.walk_phases.empty() &&
            !last.walk_phases.back().cells.empty() && last.walk_phases.back().cells.back() == goal.cell &&
            (!vehicle || std::any_of(last.walk_phases.begin(), last.walk_phases.end(), [](const auto& phase) {
                return phase.transportation_id == 4;
            }));
        if (!complete) std::printf("Meteorite final leg from %d/%d,%d: %ls / %ls (next %d)\n",
            current, at.x, at.y, last.reason.c_str(), last.error.c_str(), last.next_map_id);
        return complete;
    };
    check(replay(escaped_castle, tule, TargetKind::Transition, {249,248,247,246,245,244,1}, parked_drake),
        "Tule uses the actual meteorite journey followed by the previously unreachable Wind Drake");
    check(replay(walse_world, karnak, TargetKind::Spot, {244,245,246,247,248,249,1}),
        "The same pads provide the complete foot journey back to Karnak");

    const auto& pad_room = *baked::submap_by_map_id(247);
    const auto& actors = baked::grids(pad_room).objects;
    const auto pad = std::find_if(actors.begin(), actors.end(), [](const auto& o) {
        return o.type == baked::ObjType::Warp && (o.d & baked::kMeteoriteTransfer);
    });
    if (pad == actors.end()) std::abort();
    flags.current_map_id = 247;
    const Cell standing{1,14,14};
    const auto back = route(247, standing, flags, tule, TargetKind::Transition, false, 1, {}, {}, {}, parked_drake).path;
    const bool reentry = back.success && back.next_map_id == 246 && back.step_count == 2 &&
        !back.walk_phases.empty() && !back.walk_phases.back().cells.empty() &&
        back.walk_phases.back().cells.front() == standing && back.walk_phases.back().cells.back() == standing;
    if (!reentry) std::printf("Meteorite reentry: %ls / %ls (next %d, %d steps)\n",
        back.reason.c_str(), back.error.c_str(), back.next_map_id, back.step_count);
    check(reentry,
        "Standing on a landing pad requires stepping away and back to use it again");
    for (const auto disabled : {std::optional<int>(1), std::optional<int>{}}) {
        flags.scenario[74] = disabled;
        map_state::Resolver state(flags, fixture::records);
        check(!state.active(pad_room, *pad), "A closed or unknown story phase has no meteorite travel arc");
    }
    flags.scenario[74] = 0;
    flags.current_map_id = 1;
    const auto flying = route(1, parked_drake, flags, tule, TargetKind::Transition, false, 4).path;
    check(flying.success && !flying.next_map_id, "An already usable Wind Drake remains a direct route");
    check(!route(1, escaped_castle, flags, tule, TargetKind::Transition).path.success,
        "The meteorite journey does not invent a route across the water when no Wind Drake is available");
    MapGoal drake{1,parked_drake,false,{},4};
    const auto parked = route(1, escaped_castle, flags, drake, TargetKind::Spot, false, 1, {}, {}, drake.cell).path;
    check(parked.success && parked.next_map_id == 249, "A far-side parked Wind Drake can be approached on foot through the meteorites");
}

int verify() {
    verify_meteorite_journey();
    auto walse = exdeath_routes::flags(1,0,0);
    walse.current_area_id = 1;
    verify_walse_inventory(walse);
    MapGoal tower;
    tower.map_id = 1;
    tower.cell = {1,196,88};
    const auto approach_tower = route(1,{1,195,87},walse,tower,TargetKind::Transition);
    check(approach_tower.path.success && !approach_tower.path.walk_phases.empty() &&
          approach_tower.path.walk_phases.back().cells.back() == tower.cell,
          "the logged walking pose reaches the exact Walse Tower entrance before the sinking");
    const auto flight_tower = route(1,{1,214,116},walse,tower,TargetKind::Transition,false,4);
    check(flight_tower.path.success && !flight_tower.path.walk_phases.empty() &&
          flight_tower.path.walk_phases.back().cells.back() == tower.cell,
          "the Wind Drake can fly from the Walse settlement and land for the tower approach");
    MapGoal meteor;
    meteor.map_id = 1;
    meteor.cell = {1,204,84};
    walse.scenario[37] = walse.scenario[991] = 1;
    const auto approach_meteor = route(1,{1,205,84},walse,meteor,TargetKind::Transition);
    check(approach_meteor.path.success && !approach_meteor.path.walk_phases.empty() &&
          approach_meteor.path.walk_phases.back().cells.back() == meteor.cell,
          "the Walse Meteorite uses its exact scripted entrance after the tower sinks");
    const auto flight_meteor = route(1,{1,214,116},walse,meteor,TargetKind::Transition,false,4);
    check(flight_meteor.path.success && !flight_meteor.path.walk_phases.empty() &&
          flight_meteor.path.walk_phases.back().cells.back() == meteor.cell,
          "the Wind Drake can land on the surviving eastern shore for the meteorite");
    check(!route(1,{1,195,87},walse,tower,TargetKind::Transition).path.success,
          "the same tower entrance cannot be reached once its native foot ID becomes water");
    walse.scenario[990] = 1;
    check(route(1,{1,205,84},walse,meteor,TargetKind::Transition).path.success,
          "the post-meteor terrain conversion also permits its active entrance");
    walse.scenario[991].reset();
    check(!route(1,{1,195,87},walse,tower,TargetKind::Transition).path.success,
          "unknown sinking state does not invent a walkable tower entrance");

    auto karnak = exdeath_routes::flags(1,0,0);
    karnak.current_area_id = 1;
    const Cell between_castle_and_ship{1,84,78};
    MapGoal castle;
    castle.map_id = 1;
    castle.cell = {1,84,77};
    const auto castle_route = route(1,between_castle_and_ship,karnak,castle,TargetKind::Transition);
    check(castle_route.path.success && castle_route.path.step_count == 1 &&
          !castle_route.path.walk_phases.empty() && castle_route.path.walk_phases.back().cells.back() == castle.cell,
          "the logged Karnak world pose reaches the castle's exact northern contact");
    MapGoal fire_ship = castle;
    fire_ship.cell = {1,84,79};
    const auto ship_route = route(1,between_castle_and_ship,karnak,fire_ship,TargetKind::Transition);
    check(ship_route.path.success && ship_route.path.step_count == 1 &&
          !ship_route.path.walk_phases.empty() && ship_route.path.walk_phases.back().cells.back() == fire_ship.cell,
          "the same pose reaches the Fire-Powered Ship contact before it becomes water");
    karnak.scenario[989] = 1;
    check(!route(1,between_castle_and_ship,karnak,fire_ship,TargetKind::Transition).path.success,
          "the water at the original Fire-Powered Ship contact remains closed to foot travel");
    karnak.scenario[989].reset();
    check(!route(1,between_castle_and_ship,karnak,fire_ship,TargetKind::Transition).path.success,
          "unknown Fire-Powered Ship terrain does not invent a ground entrance");

    auto tower_state = exdeath_routes::flags(238,0,0);
    tower_state.current_area_id = 71;
    MapGoal circle;
    circle.map_id = 238;
    circle.cell = {1,7,8};
    circle.entity_id = 17;
    circle.raw_name = baked::object_identifier(object(238,17));
    const auto circle_kind = navigation_target_kind(Kind::SavePoint, Category::SavePoints);
    const auto at_circle = route(238,circle.cell,tower_state,circle,circle_kind);
    check(at_circle.path.success && at_circle.path.step_count == 0,
          "the occupied Walse save circle needs no movement");
    const auto near_circle = route(238,{1,7,10},tower_state,circle,circle_kind);
    check(near_circle.path.success && near_circle.path.step_count == 2 && !near_circle.path.event_on_route,
          "the nearby Walse save circle has a real walking approach");
    MapGoal walse_chest;
    walse_chest.map_id = 238;
    walse_chest.cell = {1,10,5};
    const auto through_circle = route(238,{1,7,10},tower_state,walse_chest,TargetKind::Interact);
    check(through_circle.path.success && !through_circle.path.event_on_route &&
          std::any_of(through_circle.path.walk_phases.begin(),through_circle.path.walk_phases.end(),[&](const auto& phase) {
              return std::find(phase.cells.begin(),phase.cells.end(),circle.cell) != phase.cells.end();
          }), "the Walse chest route walks through the active save tutorial without a required event");

    int jump_transfers = 0;
    for (int floor : {238,242}) {
        for (const auto& actor : baked::grids(*baked::submap_by_map_id(floor)).objects) {
            if (actor.type != baked::ObjType::Warp || !baked::object_identifier(actor).starts_with(L"sc_e_0212_")) continue;
            ++jump_transfers;
            check(!baked::is_trap(actor) && baked::transition_bits(actor) != 0,
                  "Walse jumps retain their real floor transfers without trap classification");
        }
    }
    check(jump_transfers == 4, "all four Walse jumps are present");

    auto pub = exdeath_routes::flags(92,0,0);
    pub.current_area_id = 25;
    const auto& pub_grid = baked::grids(*baked::submap_by_map_id(92));
    const auto& pub_contact = object(92,33);
    const auto pub_name = baked::object_identifier(pub_contact);
    for (int x : {18,22}) {
        const auto* exit = baked::world_exit_at(pub_grid,33,pub_name,x,4);
        check(exit && exit->a == 93 && exit->b == 1,
              "the exact native pub contacts are Map Exits to 2F PointIn 1");
    }
    MapGoal upstairs;
    upstairs.map_id = 92;
    upstairs.cell = {1,22,4};
    upstairs.entity_id = 33;
    upstairs.raw_name = pub_name;
    const auto pub_route = route(92,{1,22,5},pub,upstairs,TargetKind::Transition);
    check(pub_route.path.success && pub_route.path.step_count == 1,
          "Karnak Pub directions use the verified 2F entry from the adjacent return tile");

    auto valley = exdeath_routes::flags(577,0,0);
    valley.current_area_id = 4;
    valley.scenario[5] = valley.scenario[417] = 1;
    for (int entity : {13,17,18}) {
        const auto& room = *baked::submap_by_map_id(577);
        map_state::Resolver resolver(valley,fixture::records);
        check(active(valley,entity) && resolver.transportation_mask(room,object(577,entity)) == (1ull<<3),
              "the real Boko contact is enabled for the scene's transportation 3");
        check(reach(valley,{1,13,57},entity,TargetKind::Spot,3),"each equivalent jump contact has a native approach");
    }
    check(!active(valley,12),"the first goblin contact is retired once the battle receipt is present");
    valley.scenario[418] = 1;
    const auto projection = map_state::project(*fixture::records.get(L"Map_40001"),valley);
    check(projection.known && projection.entities.contains(8) && projection.entities.contains(9),
          "the copied native initializer resolves the two earthquake regions after the jumps");
    check(active(valley,14),"the northern goblin contact remains available after the jump");
    valley.scenario[419] = 1;
    check(!active(valley,14),"the northern contact is retired by its own battle receipt");

    auto forest = exdeath_routes::flags(129,0,0);
    forest.current_area_id = 32;
    check(!active(forest,96),"recruitment does not activate before the native forest prerequisite");
    forest.scenario[55] = 1;
    check(active(forest,96) && reach(forest,{1,17,28},96,TargetKind::Spot),
          "the forest entrance reaches the automatic recruitment scene");
    forest.scenario[415] = 1;
    check(active(forest,111) && reach(forest,{1,17,24},111,TargetKind::Interact),
          "after the scene the black chocobo can be approached for the native conversation");

    auto village = exdeath_routes::flags(136,0,0);
    village.current_area_id = 44;
    village.scenario[93] = 1;
    check(active(village,29) && reach(village,{0,17,26},29,TargetKind::Interact),
          "the moogle conversation is reachable during the native scene phase");
    MapGoal house;
    house.map_id = 139;
    house.cell = {1,7,11};
    check(!route(136,{0,34,16},village,house,TargetKind::Spot).path.success,
          "the third house remains gated before the conversation receipt");
    village.scenario[94] = 1;
    const auto opened = route(136,{0,34,16},village,house,TargetKind::Spot);
    check(!active(village,29) && opened.path.success && opened.pending.incoming_transition &&
          opened.pending.incoming_transition->destination == 139,
          "the completed conversation opens the correct house and retires the old moogle");
    village.current_map_id = 139;
    MapGoal outside;
    outside.map_id = 136;
    outside.cell = {0,34,15};
    const auto back = route(139,{1,7,11},village,outside,TargetKind::Spot);
    check(back.path.success && back.pending.incoming_transition && back.pending.incoming_transition->arrival == outside.cell,
          "the third house returns to its own entrance in the village");

    auto pirates = exdeath_routes::flags(191,0,0);
    pirates.current_area_id = 81;
    MapGoal cave;
    cave.map_id = 192;
    cave.cell = {1,16,19};
    const auto entrance = route(191,{1,30,47},pirates,cave,TargetKind::Spot);
    check(entrance.path.success && entrance.path.next_map_id == 192 && !entrance.path.required_trap &&
          route_warning(entrance.path,cave) == L"Event required",
          "first Pirate's Hideout entrance uses the playable scene transfer and warns of the event");
    check(entrance.path.description.find(L"warp") == std::wstring::npos,
          "the cave conversation's counted directions never describe a warp");

    pirates.current_map_id = 1;
    pirates.current_area_id = 1;
    for (const int flag : {7,8,9,12}) pirates.scenario[flag] = 1;
    MapGoal south_entrance;
    south_entrance.map_id = 1;
    south_entrance.cell = {1,165,115};
    const auto cave_return = route(1,{1,166,107},pirates,south_entrance,TargetKind::Transition);
    if (!cave_return.path.success)
        std::printf("Cave 3F return: %ls / %ls\n",cave_return.path.error.c_str(),cave_return.path.reason.c_str());
    check(cave_return.path.success && cave_return.path.next_map_id == 194 && !cave_return.path.required_trap,
          "the logged world position uses the Cave 3F connector to the scripted Cave Entrance without a vehicle");

    auto tule = exdeath_routes::flags(30,0,0);
    tule.current_area_id = 9;
    tule.scenario[21] = 1;
    MapGoal world_exit;
    world_exit.map_id = 30;
    world_exit.entity_id = 135;
    world_exit.cell = {1,32,51};
    const Cell scene_cell{1,32,49};
    auto check_tule_scene = [&](Cell from, const char* message,
                                std::shared_ptr<const ActiveRoute> previous = {}) {
        const auto result = route(30,from,tule,world_exit,TargetKind::Transition,false,1,std::move(previous));
        if (!result.path.success)
            std::printf("Tule: %ls / %ls\n",result.path.error.c_str(),result.path.reason.c_str());
        check(result.path.success && !result.path.walk_phases.empty() &&
              result.path.walk_phases.back().cells.back() == scene_cell &&
              result.path.warning == L"Event required" && !result.path.required_trap &&
              result.path.description.find(L"warp") == std::wstring::npos, message);
        return result;
    };
    check(active(tule,58), "native Tule initializer activates the pirate farewell");
    const auto first = check_tule_scene({1,21,38}, "the reported Tule route ends at the farewell that exits to the World Map");
    if (first.path.success && first.path.walk_phases.back().cells.size() > 1) {
        const auto next = first.path.walk_phases.back().cells[1];
        check_tule_scene(next, "a repeated request keeps the active scene endpoint and warning", first.active);
    }
    check_tule_scene({1,32,48}, "one step before the farewell the route still announces the required event");
    tule.scenario[22] = 1;
    const auto exit_after = route(30,{1,21,38},tule,world_exit,TargetKind::Transition,false,1,first.active);
    check(!active(tule,58) && exit_after.path.success && !exit_after.path.walk_phases.empty() &&
          exit_after.path.walk_phases.back().cells.back() == world_exit.cell && exit_after.path.warning.empty(),
          "the completed farewell restores the ordinary World Map exit without a warning");
    tule.scenario[21] = tule.scenario[22] = 0;
    tule.scenario[273] = 1;
    check(active(tule,100), "native Tule initializer activates the alternate Faris scene");
    check_tule_scene({1,21,38}, "the alternate Faris scene also fulfills the selected exit");
    std::printf("FFV story contacts: %d assertions, %d route legs, %d failures\n",
                exdeath_routes::assertions,exdeath_routes::legs,exdeath_routes::failures);
    return fixture::wrong_owner || exdeath_routes::failures ? 3 : 0;
}
}

int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read = [](const char* path) {
        std::ifstream stream(path,std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(stream),{});
    };
    const auto bytes = read("data/ff5/maps.bin");
    if (!field::baked::load(bytes.data(),bytes.size()) || !json::parse(read("data/ff5/mapstate.json"),fixture::records)) return 2;
    fixture::game_id = game::Id::FF5;
    field::baked_nav::g_loaded = field::baked_nav::g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = story_contacts::verify(); });
    worker.join();
    return result;
}
