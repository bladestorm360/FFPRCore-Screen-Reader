// check-sources: field/storyline.cpp field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main
#include "../src/core/field/storyline.h"

namespace {
using namespace field;
using namespace field::storyline;
using exdeath_routes::check;

map_state::Flags progress_at(const CorneliaProgress& progress, int map)
{
    map_state::Flags flags;
    flags.scenario.fill(0);
    flags.treasure.fill(0);
    flags.global.fill(0);
    flags.area.fill(0);
    flags.local.fill(0);
    flags.current_map_id = map;
    flags.scenario[4] = progress.bridge;
    flags.scenario[9] = progress.ship;
    flags.scenario[10] = progress.crown;
    flags.scenario[11] = progress.eye;
    flags.scenario[8] = progress.met_matoya;
    flags.scenario[12] = progress.tonic;
    flags.scenario[13] = progress.key;
    flags.scenario[14] = progress.powder;
    flags.scenario[15] = progress.canal;
    flags.treasure[501] = progress.crown;
    flags.treasure[246] = progress.eye;
    flags.treasure[247] = progress.tonic;
    flags.treasure[248] = progress.key;
    flags.treasure[505] = progress.powder;
    for (const auto& [index, value] : std::array<std::pair<int, std::optional<int>>, 7>{{
         {0, progress.opening}, {1, progress.audience}, {2, progress.garland},
         {3, progress.returned}, {5, progress.lute}, {6, progress.bridge}, {7, progress.crossed}}})
        flags.scenario[index] = value;
    if (map == 1) flags.current_area_id = 1;
    else if (map == 4) flags.current_area_id = 17;
    else if (map == 12) flags.current_area_id = 28;
    else if (map == 13) flags.current_area_id = 29;
    else if (map == 21) flags.current_area_id = 24;
    else if (map == 32) flags.current_area_id = 8;
    else if (map == 33) flags.current_area_id = 25;
    else if (map == 72) flags.current_area_id = 11;
    else if (map >= 73 && map <= 75) flags.current_area_id = 26;
    else flags.current_area_id = 16;
    return flags;
}

void replay(const CorneliaProgress& progress, ObjectiveId expected, int origin, baked_route::Cell at,
            int goal_map, std::vector<int> contacts_expected, bool advice = false)
{
    auto objective = cornelia_objective(progress);
    if (advice && objective) objective = cornelia_advice(objective->id, progress);
    check(objective && objective->id == expected, "persistent receipts select the expected objective");
    if (!objective) return;
    const auto targets = contacts(*objective, origin);
    std::vector<int> actual;
    for (const auto& target : targets) actual.push_back(target.goal.entity_id);
    check(actual == contacts_expected, "exact baked placements and authored group are retained");
    if (targets.empty()) return;
    const auto& target = targets.front();
    check(target.goal.map_id == goal_map && target.goal.allow_world_transit,
          "the route leg targets the expected map and supports intervening world travel");
    const auto route = exdeath_routes::route(origin, at, progress_at(progress, origin), target.goal, target.kind);
    check(route.path.success && !route.path.required_trap, "the production worker plans a verified first movement leg");
    if (!route.path.success || route.path.required_trap) {
        std::printf("  map %d at %d/%d/%d, target %d at %d/%d/%d, %ls, %ls\n", origin, at.layer, at.x, at.y,
                    target.goal.map_id, target.goal.cell.layer, target.goal.cell.x, target.goal.cell.y,
                    route.path.error.c_str(), route.path.reason.c_str());
    }
}

void verify_after_canal()
{
    struct Leg {
        int flag, map, contact;
        ObjectiveId id;
        baked_route::Cell start;
    };
    using Id = ObjectiveId;
    // Starts are nearby passable approaches on the installed production bake.
    // Intermediate map traversal and item state are checked separately below.
    constexpr std::array itinerary{
        Leg{16, 78, 54, Id::Vampire, {0,31,42}},
        Leg{17, 78, 22, Id::StarRuby, {0,31,40}},
        Leg{18, 81, 11, Id::GiveRuby, {2,18,19}},
        Leg{19, 40, 9, Id::EarthRod, {2,30,22}},
        Leg{20, 78, 52, Id::EarthSlab, {0,31,40}},
        Leg{21, 80, 33, Id::Lich, {0,13,32}},
        Leg{22, 41, 57, Id::Canoe, {1,49,22}},
        Leg{23, 87, 33, Id::Marilith, {0,12,60}},
        Leg{24, 91, 8, Id::Levistone, {0,19,16}},
        Leg{25, 1, 82, Id::Airship, {1,212,232}},
        Leg{27, 59, 8, Id::BuyFairy, {1,11,10}},
        Leg{28, 60, 97, Id::ReleaseFairy, {1,63,60}},
        Leg{29, 60, 90, Id::Oxyale, {1,51,20}},
        Leg{30, 52, 67, Id::MeetMermaid, {1,53,37}},
        Leg{31, 103, 42, Id::RosettaStone, {0,40,14}},
        Leg{34, 96, 38, Id::Kraken, {0,20,14}},
        Leg{32, 34, 126, Id::LearnLufenian, {1,36,10}},
        Leg{33, 70, 131, Id::Chime, {1,30,30}},
        Leg{26, 104, 23, Id::WarpCube, {0,25,60}},
        Leg{35, 112, 3, Id::Tiamat, {1,16,17}},
        Leg{36, 72, 39, Id::EnterPast, {0,38,31}},
        Leg{37, 115, 9, Id::LuteSlab, {0,38,26}},
        Leg{38, 116, 20, Id::PastLich, {2,14,50}},
        Leg{39, 117, 52, Id::PastMarilith, {2,27,35}},
        Leg{40, 118, 69, Id::PastKraken, {2,11,6}},
        Leg{41, 119, 14, Id::PastTiamat, {2,11,11}},
        Leg{42, 120, 6, Id::Chaos, {0,27,26}},
    };
    Ff1Receipts flags;
    flags.fill(0);
    std::fill_n(flags.begin(), 16, 1);
    auto state = progress_at({1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, 1);
    for (const auto& leg : itinerary) {
        const auto stage = ff1_stage(flags, leg.map);
        check(stage.objective && stage.objective->id == leg.id && !stage.complete,
              "campaign chooses the next native receipt");
        if (!stage.objective) break;
        const auto targets = contacts(*stage.objective, leg.map);
        const bool found = !targets.empty() && targets.front().goal.entity_id == leg.contact &&
                           targets.front().goal.map_id == leg.map;
        check(found, "the chosen stage retains the exact native map contact");
        if (!found) std::printf("  no baked contact for map%d entity%d\n", leg.map, leg.contact);
        if (found) {
            const auto& target = targets.front();
            state.current_map_id = leg.map;
            const auto ok = exdeath_routes::route(leg.map, leg.start, state, target.goal, target.kind);
            check(ok.path.success, "a movement leg can reach the actual native contact");
            if (!ok.path.success)
                std::printf("  map%d entity%d: %ls / %ls\n", leg.map, leg.contact, ok.path.reason.c_str(), ok.path.error.c_str());
        }
        if (stage.advice)
            check(!contacts(*stage.advice, leg.map).empty(), "informational NPC retains an exact native contact");
        if (leg.flag == 31 || leg.flag == 37) {
            const auto entry = ff1_stage(flags, 1).objective;
            const auto reentry = leg.flag == 31 ? Id::BoardSubmarine : Id::ReturnToPast;
            check(entry && entry->id == reentry && !contacts(*entry, 1).empty(),
                  "resuming outside the area uses the real repeatable entrance");
        }
        flags[leg.flag] = 1;
        state.scenario[leg.flag] = 1;
        if (leg.flag == 17) state.treasure[508] = 1;
        if (leg.flag == 19) state.treasure[253] = 1;
    }
    check(ff1_stage(flags, 1).complete && !ff1_stage(flags, 1).objective,
          "the native ending receipt completes the campaign");
    flags[42] = 0;
    flags[41] = std::nullopt;
    check(!ff1_stage(flags, 120).objective && !ff1_stage(flags, 120).complete,
          "an unavailable victory receipt does not infer further progression");
}

int verify()
{
    const CorneliaProgress audience{1,0,0,0,0,0,0};
    const CorneliaProgress garland{1,1,0,0,0,0,0};
    const CorneliaProgress lute{1,1,1,1,0,0,0};
    const CorneliaProgress leave{1,1,1,1,1,0,0};
    const CorneliaProgress bridge{1,1,1,1,1,1,0};
    const CorneliaProgress ship{1,1,1,1,1,1,1,0};
    const CorneliaProgress crown{1,1,1,1,1,1,1,1,0};
    const CorneliaProgress keep{1,1,1,1,1,1,1,1,1,0};
    const CorneliaProgress intro{1,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
    const CorneliaProgress eye{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0};
    const CorneliaProgress tonic{1,1,1,1,1,1,1,1,1,1,1,1,0,0,0};
    const CorneliaProgress powder{1,1,1,1,1,1,1,1,1,1,1,1,1,0,0};
    const CorneliaProgress canal{1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};
    check(!cornelia_objective({0,0,0,0,0,0,0}), "opening must finish first");
    check(!cornelia_objective({1,1,1,0,0,0,0}), "rescue return transition withholds the next objective");
    check(!cornelia_objective({1,1,0,1,0,0,0}), "a missing earlier receipt is ambiguous");
    check(!cornelia_objective({1,1,1,1,1,1,{}}), "unavailable receipt stays unknown");
    check(!cornelia_objective({1,1,1,1,1,1,1}), "an unavailable ship receipt stays unknown");
    check(!cornelia_objective({1,1,1,1,1,1,1,1}), "an unavailable crown receipt stays unknown");
    check(!cornelia_objective({1,1,1,1,1,1,1,1,1}), "an unavailable crystal eye receipt stays unknown");
    check(!cornelia_objective({1,1,1,1,1,1,1,1,1,1}), "unavailable crystal eye handover progress stays unknown");
    replay(audience, ObjectiveId::Audience, 3, {2,20,17}, 3, {14});
    replay(audience, ObjectiveId::Audience, 2, {2,26,41}, 3, {14});
    replay(audience, ObjectiveId::Audience, 1, {1,145,156}, 3, {14});
    replay(garland, ObjectiveId::Garland, 72, {0,38,31}, 72, {32});
    replay(garland, ObjectiveId::Garland, 1, {1,145,156}, 72, {32});
    replay(garland, ObjectiveId::Garland, 2, {2,26,41}, 72, {32});
    replay(lute, ObjectiveId::Lute, 3, {0,20,9}, 3, {7});
    replay(leave, ObjectiveId::LeaveCastle, 2, {2,26,41}, 2, {45,44,46});
    replay(bridge, ObjectiveId::CrossBridge, 1, {1,145,146}, 1, {89});
    replay(bridge, ObjectiveId::CrossBridge, 1, {1,145,156}, 1, {89});
    replay(ship, ObjectiveId::GetShip, 1, {1,145,145}, 13, {38});
    replay(ship, ObjectiveId::GetShip, 13, {1,27,37}, 13, {38});
    replay(crown, ObjectiveId::FindCrown, 75, {2,13,9}, 75, {40});
    replay(crown, ObjectiveId::FindCrown, 74, {2,31,23}, 75, {40});
    replay(crown, ObjectiveId::FindCrown, 73, {2,30,32}, 75, {40});
    replay(crown, ObjectiveId::FindCrown, 1, {1,95,228}, 75, {40});
    replay(crown, ObjectiveId::FindCrown, 13, {1,13,12}, 13, {5});
    replay(keep, ObjectiveId::BringCrown, 33, {2,32,38}, 33, {105});
    replay(keep, ObjectiveId::BringCrown, 1, {1,95,228}, 33, {105});
    replay(keep, ObjectiveId::BringCrown, 75, {0,33,48}, 33, {105});
    replay(intro, ObjectiveId::MeetMatoya, 12, {2,24,17}, 12, {17});
    replay(eye, ObjectiveId::ReturnEye, 12, {0,17,15}, 12, {15});
    replay(tonic, ObjectiveId::WakePrince, 32, {2,27,47}, 32, {116});
    replay(powder, ObjectiveId::GetPowder, 2, {2,26,41}, 2, {10});
    replay(canal, ObjectiveId::OpenCanal, 21, {2,33,16}, 21, {45});
    replay(bridge, ObjectiveId::BridgeAdvice, 4, {2,24,30}, 4, {83}, true);
    replay(ship, ObjectiveId::ShipAdvice, 1, {1,145,145}, 4, {83}, true);
    auto early_crown = crown;
    early_crown.met_matoya = 0;
    replay(early_crown, ObjectiveId::MeetMatoya, 12, {2,24,17}, 12, {17}, true);
    auto later_crown = crown;
    later_crown.met_matoya = 1;
    replay(later_crown, ObjectiveId::CrownAdvice, 4, {2,24,30}, 4, {83}, true);
    replay(tonic, ObjectiveId::PrinceAdvice, 4, {2,24,30}, 4, {83}, true);
    replay(powder, ObjectiveId::PowderAdvice, 21, {2,33,16}, 21, {1}, true);

    const auto ship_quest = cornelia_objective(ship);
    const auto crown_quest = cornelia_objective(later_crown);
    if (!ship_quest || !crown_quest) return 3;
    auto hint = cornelia_advice(ship_quest->id, ship);
    Selection selection;
    check(selection.resolve(*ship_quest, hint, false).id == ObjectiveId::ShipAdvice, "advice is offered first");
    check(selection.resolve(*ship_quest, hint, true).id == ObjectiveId::GetShip, "the player can skip optional advice");
    check(selection.resolve(*ship_quest, hint, false).id == ObjectiveId::GetShip, "repeated requests keep the chosen destination");
    hint = cornelia_advice(crown_quest->id, later_crown);
    check(selection.resolve(*crown_quest, hint, false).id == ObjectiveId::CrownAdvice, "a new stage selects its own advice");
    verify_after_canal();
    std::printf("FF1 storyline: %d assertions, %d physical legs, %d failures\n",
                exdeath_routes::assertions, exdeath_routes::legs, exdeath_routes::failures);
    return exdeath_routes::failures || fixture::wrong_owner ? 3 : 0;
}
}

int main()
{
    const auto read = [](const char* path) {
        std::ifstream file(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(file), {});
    };
    const auto maps = read("data/ff1/maps.bin");
    if (!baked::load(maps.data(), maps.size()) || !json::parse(read("data/ff1/mapstate.json"), fixture::records)) return 2;
    fixture::game_id = game::Id::FF1;
    baked_nav::g_loaded = baked_nav::g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = verify(); });
    worker.join();
    return result;
}
