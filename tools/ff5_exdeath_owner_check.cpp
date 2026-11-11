// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
#define FFPR_LIBRARY_OWNER_MAIN retained_exdeath_library_owner_main
#include "ff5_library_owner_check.cpp"

namespace exdeath_owner {
void scene(int map, field::baked_route::Cell position, int layout, int bridge, bool spinning = false) {
    owner::flags({});
    fixture::storage.map_id = map;
    fixture::user.area_id = 40;
    fixture::area.values.fill(0);
    fixture::area.values[0] = layout;
    fixture::area.values[1] = bridge;
    owner::native_scene(map);
    owner::position = position;
    if (spinning) for (auto& actor : owner::actors) {
        if (actor.property->id >= 26 && actor.property->id <= 33) actor.own_hidden = false;
    }
}

void step() {
    actions::run("GameModule.PuzzleGuide");
    owner::drain();
}

bool target(int map, int entity) {
    return owner::targets.size() == 1 && owner::targets.front().goal.map_id == map &&
           owner::targets.front().goal.entity_id == entity;
}
}

int main() {
    if (retained_exdeath_library_owner_main()) return 1;
    using namespace owner;
    using namespace exdeath_owner;
    const auto bytes = fixture::read("data/ff5/maps.bin");
    if (!field::baked::load(bytes.data(), bytes.size())) return 2;
    ff5::library::stop(false);
    scene(397, {2,6,17}, 0, 0);
    const auto initial_reads = fixture::native_control_reads;
    step();
    check(target(397,24) && fixture::native_control_reads == initial_reads,
          "south entry requests the actual start with no extra actor observation");
    respond();

    ready = false;
    for (int i = 0; i < 20; ++i) actions::run("GameModule.FieldStateChanged");
    for (int i = 0; i < 80; ++i) pulse();
    check(tasks.size() == 1 && fixture::native_control_reads == initial_reads,
          "scene events coalesce without reading the actor while native control is blocked");
    scene(397, {2,6,10}, 0, 0, true);
    ready = true;
    pulse();
    drain();
    check(target(397,27) && fixture::native_control_reads == initial_reads + 1,
          "one bounded actor observation distinguishes the actual moving floor from a reload");
    respond();
    const auto reads_after_start = fixture::native_control_reads;
    for (int i = 0; i < 200; ++i) actions::run("GameModule.PuzzleGuideMove");
    check(fixture::native_control_reads == reads_after_start && tasks.empty(),
          "movement never polls the stop contact or advances the solution");

    ready = false;
    battle = true;
    event(events::Kind::BattleEntering);
    scene(397, {2,6,9}, 1, 0);
    actions::run("GameModule.FieldStateChanged");
    const auto during_battle = routes;
    for (int i = 0; i < 80; ++i) pulse();
    check(routes == during_battle, "a stop receipt cannot publish during the encounter");
    battle = false;
    ready = true;
    event(events::Kind::BattleEnd);
    pulse();
    drain();
    check(target(397,24) && fixture::native_control_reads == reads_after_start,
          "unfavorable native outcome returns to the center after battle");
    respond();

    scene(397, {2,6,10}, 0, 0);
    const auto before_reload = routes;
    step();
    check(routes == before_reload && spoken.back().find(L"South once") != std::wstring::npos,
          "hidden stop on reload gives the exact rearming steps instead of a dead destination");
    scene(397, {2,6,9}, 5, 0);
    step();
    check(target(397,4), "a favorable stop uses the north stair without another timing request");
    respond();

    scene(401, {1,14,9}, 5, 0);
    step();
    check(target(401,14), "west arrival selects the actual collapse trigger");
    respond();
    scene(401, {1,18,9}, 5, 1);
    step();
    check(target(401,1), "collapsed bridge selects the recovery control");
    respond();
    scene(401, {1,22,13}, 5, 0);
    step();
    check(target(401,13), "restored bridge resumes from the actual control position");
    respond();

    ready = false;
    event(events::Kind::MapChanging);
    scene(402, {1,6,6}, 5, 0);
    event(events::Kind::MapChanged);
    for (int i = 0; i < 60; ++i) pulse();
    check(queries::is("GameModule.PuzzleGuideActive"), "final stair remains pending during loading");
    ready = true;
    pulse();
    drain();
    check(!queries::is("GameModule.PuzzleGuideActive") && spoken.back() == L"Puzzle guide finished",
          "the ready native floor transition finishes the guide");
    check(fixture::wrong_owner == 0, "native reads and speech preserve their actual owner");
    std::printf("Exdeath native owner: %d cases, %d failures\n", cases, fixture::failures);
    return fixture::failures ? 3 : 0;
}
