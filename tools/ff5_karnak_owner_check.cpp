// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
#define FFPR_LIBRARY_OWNER_MAIN retained_karnak_library_owner_main
#include "ff5_library_owner_check.cpp"

namespace karnak_owner {
void scene(int map, field::baked_route::Cell at, std::initializer_list<int> bits) {
    owner::flags(bits);
    fixture::storage.map_id = map;
    fixture::user.area_id = map == 1 ? 1 : map == 581 ? 101 : 24;
    owner::native_scene(map);
    owner::position = at;
}
void step() {
    actions::run("GameModule.PuzzleGuide");
    owner::drain();
}
}

int main() {
    if (retained_karnak_library_owner_main()) return 1;
    using namespace owner;
    using namespace karnak_owner;
    const auto bytes = fixture::read("data/ff5/maps.bin");
    if (!field::baked::load(bytes.data(),bytes.size())) return 2;
    ff5::library::stop(false);
    scene(102,{1,4,7},{42,43,46});
    const auto initial_reads = fixture::native_control_reads;
    step();
    check(targets.size() == 1 && targets[0].goal.entity_id == 29,"the native-ready countdown starts the actual escape route");
    respond();

    ready = false;
    battle = true;
    event(events::Kind::BattleEntering);
    scene(95,{1,23,35},{42,43,46,245});
    const auto before_battle = routes;
    for (int i = 0; i < 20; ++i) actions::run("GameModule.FieldStateChanged");
    for (int i = 0; i < 60; ++i) pulse();
    check(routes == before_battle && tasks.size() == 1,"early encounter receipt cannot publish before native battle return");
    battle = false;
    ready = true;
    event(events::Kind::BattleEnd);
    pulse();
    drain();
    check(targets.size() == 3 && targets[0].goal.entity_id == 54,"battle return selects the actual timed exits");
    respond();

    event(events::Kind::MapChanging);
    scene(1,{2,84,78},{42,43,46,245});
    event(events::Kind::MapChanged);
    const auto before_scene = routes;
    pulse();
    drain();
    check(routes == before_scene && queries::is("GameModule.PuzzleGuideActive"),"first world scene can neither finish nor route back");
    ready = false;
    event(events::Kind::MapChanging);
    scene(581,{2,19,8},{42,43,46,245,992});
    event(events::Kind::MapChanged);
    for (int i = 0; i < 60; ++i) pulse();
    check(queries::is("GameModule.PuzzleGuideActive"),"explosion scene retains the native continuation");
    event(events::Kind::MapChanging);
    scene(1,{2,83,78},{42,43,46,47,245,992});
    event(events::Kind::MapChanged);
    for (int i = 0; i < 60; ++i) pulse();
    check(queries::is("GameModule.PuzzleGuideActive"),"final receipt still waits for the full native-ready world return");
    ready = true;
    pulse();
    drain();
    check(!queries::is("GameModule.PuzzleGuideActive") && spoken.back() == L"Puzzle guide finished",
          "full native-ready world callback completes the timed escape");
    check(fixture::native_control_reads == initial_reads && fixture::wrong_owner == 0,
          "all Karnak stages reuse copied state without extra actor reads");
    std::printf("Karnak native owner: %d cases, %d failures\n",cases,fixture::failures);
    return fixture::failures ? 3 : 0;
}
