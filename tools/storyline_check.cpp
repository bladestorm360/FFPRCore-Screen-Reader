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
    const Stage selected_clue{selection.resolve(*ship_quest, hint, false), hint};
    check(selected_clue.objective->id == ObjectiveId::ShipAdvice, "advice is offered first");
    const Stage selected_destination{selection.resolve(*ship_quest, hint, true), hint};
    check(selected_destination.objective->id == ObjectiveId::GetShip, "the player can skip optional advice");
    check(selection_label_key(selected_clue, false) == L"Clue" &&
          selection_label_key(selected_destination, false) == L"Destination",
          "spoiler-free names distinguish the selected clue and destination");
    check(selection_label_key(selected_clue, true) == selected_clue.objective->label &&
          selection_label_key(selected_destination, true) == selected_destination.objective->label,
          "enabling spoilers exposes both selected names");
    check(selection_label_key({{}, {}, true}, false) == L"Storyline complete" &&
          selection_label_key({}, false) == L"No Storyline objective available",
          "spoiler-free selection preserves end and unavailable announcements");
    check(selection.resolve(*ship_quest, hint, false).id == ObjectiveId::GetShip, "repeated requests keep the chosen destination");
    hint = cornelia_advice(crown_quest->id, later_crown);
    check(selection.resolve(*crown_quest, hint, false).id == ObjectiveId::CrownAdvice, "a new stage selects its own advice");
    verify_after_canal();

    // Gaia and Onrac are on separate ground components. The first leg can
    // only prove Gaia's real exit until the world vehicle is actually loaded.
    Ff1Receipts travel;
    travel.fill(0);
    std::fill_n(travel.begin(), 30, 1);
    auto state = progress_at(tonic, 60);
    std::copy(travel.begin(), travel.end(), state.scenario.begin());
    const auto mermaid = ff1_stage(travel, 60).objective;
    if (mermaid) {
        const auto goals = contacts(*mermaid, 60);
        check(!goals.empty(), "the Onrac introduction remains the selected goal from Gaia");
        if (!goals.empty()) {
            auto goal = goals.front().goal;
            const auto kind = goals.front().kind;
            const auto leave = exdeath_routes::route(60, {1,63,60}, state, goal, kind);
            check(leave.path.success && leave.path.next_map_id == 1 &&
                  leave.path.description.find(L"Request directions again on the world map.") != std::wstring::npos,
                  "Storyline proves Gaia's real departure when it cannot project a parked craft");
            goal.storyline = false;
            const auto ordinary = exdeath_routes::route(60, {1,63,60}, state, goal, kind);
            check(!ordinary.path.success, "ordinary remote selection keeps its reachability contract");
            goal.storyline = true;
            state.current_map_id = 1;
            const auto flight = exdeath_routes::route(1, {1,214,22}, state, goal, kind, false, 3);
            check(flight.path.success && flight.path.next_map_id == 52,
                  "after native world recapture the mounted airship can approach Onrac");
        }
    } else check(false, "the Onrac introduction is selected after Oxyale");

    std::printf("FF1 storyline: %d assertions, %d physical legs, %d failures\n",
                exdeath_routes::assertions, exdeath_routes::legs, exdeath_routes::failures);
    return exdeath_routes::failures || fixture::wrong_owner ? 3 : 0;
}

int verify_ff2()
{
    Ff2Receipts flags;
    flags.fill(0);
    Ff2Keywords known;
    known.fill(false);
    check(!ff2_stage(flags, known).objective, "FFII opening owns the field until the native prologue finishes");
    flags[0] = 1;
    const auto unknown = ff2_stage(flags, {});
    check(!unknown.objective && !unknown.complete, "unknown keyword storage does not supply a quest");
    const int scenario_base = static_cast<int>(ObjectiveId::Ff2Scenario);
    const int keyword_base = static_cast<int>(ObjectiveId::Ff2Keyword);
    struct Arrival { int map, entity, vehicle; baked_route::Cell start; };
    constexpr std::array arrivals{
        Arrival{6, 49, 1, {0,13,16}},
        Arrival{27, 12, 1, {0,5,5}},
        Arrival{6, 29, 1, {0,13,16}},
        Arrival{1, 108, 4, {1,30,194}},
        Arrival{273, 5, 1, {0,22,28}},
        Arrival{1, 49, 3, {1,233,170}},
        Arrival{316, 46, 1, {0,22,25}}
    };
    auto state = progress_at({}, 1);
    int count = 0;
    for (; count < 90; ++count) {
        const auto stage = ff2_stage(flags, known);
        if (!stage.objective) {
            check(stage.complete && flags[82] == 1, "only the Pandaemonium ending receipt completes FFII");
            break;
        }
        const auto& objective = *stage.objective;
        const auto targets = contacts(objective, objective.map);
        check(!targets.empty() && targets.front().goal.map_id == objective.map &&
              targets.front().goal.entity_id == objective.contacts[0], "FFII step binds a native contact");
        if (targets.empty())
            std::printf("  FFII map%d/%d type%d %ls\n", objective.map, objective.contacts[0],
                        static_cast<int>(objective.object_type), objective.label.data());
        if (stage.advice)
            check(!contacts(*stage.advice, objective.map).empty(), "FFII optional advice binds a native conversation");
        if (!targets.empty()) {
            const auto arrival = std::find_if(arrivals.begin(), arrivals.end(), [&objective](const auto& candidate) {
                return objective.map == candidate.map && objective.contacts[0] == candidate.entity;
            });
            if (arrival != arrivals.end()) {
                std::copy(flags.begin(), flags.end(), state.scenario.begin());
                if (flags[32] == 1) state.scenario[110] = 1; // native Dreadnought pursuit continuation
                state.current_map_id = arrival->map;
                const auto& target = targets.front();
                const auto route = exdeath_routes::route(arrival->map, arrival->start, state, target.goal,
                                                         target.kind, false, arrival->vehicle);
                check(route.path.success, "FFII can reach the story contact from its native arrival");
                if (!route.path.success)
                    std::printf("  FFII approach map%d/%d: %ls / %ls\n", arrival->map, arrival->entity,
                                route.path.reason.c_str(), route.path.error.c_str());
            }
        }
        const int id = static_cast<int>(objective.id);
        if (id >= keyword_base) {
            check(id - keyword_base < static_cast<int>(known.size()), "learned keyword is bound to a real content ID");
            known.at(id - keyword_base) = true;
        } else {
            check(flags.at(id - scenario_base) == 0, "FFII replays the next unsatisfied native receipt once");
            flags.at(id - scenario_base) = 1;
        }
    }
    check(count >= 60 && count < 90, "FFII qualified story order reaches a finite ending");
    flags[82] = 0;
    flags[17] = flags[100] = 0;
    const auto resumed = ff2_stage(flags, {});
    check(resumed.objective && resumed.objective->map == 316,
          "a later save skips obsolete word prompts, the optional pass, and opened cell bars");
    flags[75] = 0;
    check(!ff2_stage(flags, known).complete, "the first Emperor victory cannot complete the campaign");
    std::printf("FFII Storyline: %d objectives, %d total failures\n", count, exdeath_routes::failures);
    return exdeath_routes::failures || fixture::wrong_owner ? 3 : 0;
}

int verify_ff3()
{
    Ff3Receipts flags;
    flags.fill(0);
    check(!ff3_stage(flags).objective, "the native Altar Cave introduction owns the field before gameplay");
    flags[0] = 1;
    const int base = static_cast<int>(ObjectiveId::Ff3Scenario);
    struct Resume { int conversation, continuation, next; };
    constexpr std::array resumed_stories{
        Resume{9,11,194}, Resume{10,11,194}, Resume{12,14,16},
        Resume{30,31,33}, Resume{55,56,59}, Resume{68,69,70}
    };
    for (const auto& saved : resumed_stories) {
        Ff3Receipts resumed;
        resumed.fill(1);
        resumed[32] = resumed[105] = resumed[saved.conversation] = resumed[saved.next] = 0;
        const auto after = ff3_stage(resumed);
        check(after.objective && static_cast<int>(after.objective->id) == base + saved.next && !after.advice,
              "FFIII resumes the current objective without restoring an obsolete conversation or clue");
        resumed[saved.continuation] = 0;
        const auto before = ff3_stage(resumed);
        check(before.objective && static_cast<int>(before.objective->id) == base + saved.conversation,
              "FFIII still offers the conversation before its verified continuation");
    }
    auto state = progress_at({}, 1);
    struct Arrival { int map, entity; baked_route::Cell start; };
    constexpr std::array arrivals{
        Arrival{153, 10, {1,15,14}},
        Arrival{155, 3, {1,10,20}},
        Arrival{155, 0, {1,10,15}},
        Arrival{164, 8, {1,35,8}},
        Arrival{327, 4, {1,9,19}},
        Arrival{335, 4, {1,11,11}}
    };
    struct Hint {
        int receipt;
        ObjectiveId id;
        int map, entity, origin, area, inside;
    };
    constexpr std::array hints{
        Hint{10, ObjectiveId::Ff3KazusAdvice, 5, 98, 5, 3, 14},
        Hint{29, ObjectiveId::Ff3TozusAdvice, 152, 3, 152, 11, 46},
        Hint{51, ObjectiveId::Ff3GutscoAdvice, 85, 19, 85, 22, 189},
        Hint{72, ObjectiveId::Ff3GoldorAdvice, 101, 10, 101, 30, 219},
        Hint{77, ObjectiveId::Ff3DalgAdvice, 110, 8, 104, 35, 131}
    };
    size_t hint_count = 0;
    int count = 0;
    for (; count < 90; ++count) {
        const auto stage = ff3_stage(flags);
        if (!stage.objective) {
            check(stage.complete && flags[105] == 1, "only the Cloud of Darkness ending receipt completes FFIII");
            break;
        }
        const auto& objective = *stage.objective;
        const int receipt = static_cast<int>(objective.id) - base;
        const auto targets = contacts(objective, objective.map);
        check(!targets.empty() && targets.front().goal.map_id == objective.map &&
              targets.front().goal.entity_id == objective.contacts[0], "FFIII step binds a native contact");
        if (targets.empty())
            std::printf("  FFIII map%d/%d type%d %ls\n", objective.map, objective.contacts[0],
                        static_cast<int>(objective.object_type), objective.label.data());
        const auto hint = std::find_if(hints.begin(), hints.end(), [receipt](const auto& candidate) {
            return candidate.receipt == receipt;
        });
        if (hint == hints.end()) {
            check(!stage.advice, "FFIII only offers hints during their verified story stages");
        } else {
            ++hint_count;
            const auto inside = ff3_stage(flags, hint->inside);
            check(inside.objective && inside.objective->id == objective.id && !inside.advice,
                  "entering the destination keeps the objective and removes the earlier NPC detour");
            check(stage.advice && stage.advice->id == hint->id, "FFIII selects the expected optional speaker");
            if (stage.advice) {
                Selection choice;
                check(choice.resolve(objective, stage.advice, false).id == hint->id &&
                      choice.resolve(objective, stage.advice, true).id == objective.id &&
                      choice.resolve(objective, stage.advice, false).id == objective.id &&
                      choice.resolve(objective, stage.advice, true).id == hint->id,
                      "the player can switch both ways without the hint advancing the save");
                const auto speakers = contacts(*stage.advice, hint->origin);
                check(speakers.size() == 1 && speakers.front().goal.map_id == hint->map &&
                      speakers.front().goal.entity_id == hint->entity && speakers.front().goal.examine &&
                      speakers.front().kind == TargetKind::Interact,
                      "each hint resolves the exact native conversation");
                const auto* map = baked::submap_by_map_id(hint->origin);
                check(map != nullptr, "the hint's starting map is present in the shipped map data");
                if (map && !speakers.empty()) {
                    const auto& objects = baked::grids(*map).objects;
                    const auto entry = std::find_if(objects.begin(), objects.end(), [](const auto& object) {
                        return object.type == baked::ObjType::PointIn && object.a == 1;
                    });
                    check(entry != objects.end(), "the hint replay starts at the native entry point");
                    if (entry != objects.end()) {
                        std::copy(flags.begin(), flags.end(), state.scenario.begin());
                        state.current_map_id = hint->origin;
                        state.current_area_id = hint->area;
                        const auto& speaker = speakers.front();
                        const auto planned = exdeath_routes::route(hint->origin, {entry->layer, entry->tx, entry->ty},
                                                                  state, speaker.goal, speaker.kind);
                        check(planned.path.success && !planned.path.required_trap &&
                              (hint->origin == hint->map || planned.path.next_map_id == hint->map),
                              "FFIII directions reach the clue or its native first door");
                        if (!planned.path.success)
                            std::printf("  FFIII clue %ls: %ls / %ls\n", stage.advice->label.data(),
                                        planned.path.reason.c_str(), planned.path.error.c_str());
                    }
                }
            }
        }
        if (receipt == 33) {
            auto interrupted = flags;
            interrupted[32] = 1;
            const auto recovery = ff3_stage(interrupted).objective;
            check(recovery && recovery->id == ObjectiveId::Ff3EyeRecovery &&
                  !contacts(*recovery, 173).empty(),
                  "a defeated Giant Rat with no Eye receipt uses the exposed Eye");
        }
        if (receipt == 351) {
            auto interrupted = flags;
            interrupted[351] = {};
            check(!ff3_stage(interrupted).objective, "the mirror story reads the actual second scenario bank");
        }
        if (!targets.empty()) {
            const auto arrival = std::find_if(arrivals.begin(), arrivals.end(), [&objective](const auto& candidate) {
                return objective.map == candidate.map && objective.contacts[0] == candidate.entity;
            });
            if (arrival != arrivals.end()) {
                std::copy(flags.begin(), flags.end(), state.scenario.begin());
                state.current_map_id = arrival->map;
                const auto& target = targets.front();
                const auto route = exdeath_routes::route(arrival->map, arrival->start, state, target.goal, target.kind);
                check(route.path.success, "FFIII can reach the story contact from its native approach");
                if (!route.path.success)
                    std::printf("  FFIII approach map%d/%d: %ls / %ls\n", arrival->map, arrival->entity,
                                route.path.reason.c_str(), route.path.error.c_str());
            }
        }
        check(flags.at(static_cast<size_t>(receipt)) == 0, "FFIII replays each unsatisfied native receipt once");
        flags.at(static_cast<size_t>(receipt)) = 1;
    }
    check(count >= 70 && count < 90, "FFIII story replay reaches a finite ending");
    check(hint_count == hints.size(), "all five FFIII clues were exercised by the complete campaign replay");
    flags[105] = 0;
    for (int index = 79; index <= 105; ++index) flags[index] = 0;
    const auto temple = ff3_stage(flags, 4).objective;
    const auto lute = ff3_stage(flags, 242).objective;
    check(temple && temple->map == 3 && !temple->instruction.empty() &&
          lute && lute->map == 252 && lute->id == temple->id,
          "FFIII approaches the Temple before routing inside to Noah's Lute on the same receipt");
    if (temple) {
        const auto targets = contacts(*temple, 4);
        check(targets.size() == 2, "both native Temple entrance tiles are available for guidance");
        if (!targets.empty()) {
            std::copy(flags.begin(), flags.end(), state.scenario.begin());
            state.current_map_id = 4;
            state.current_area_id = 1;
            const auto& goal = targets.front();
            const auto planned = exdeath_routes::route(4, {1,151,74}, state, goal.goal, goal.kind, false, 9);
            check(planned.path.success && planned.path.next_map_id == 3,
                  "the Temple approach takes the upgraded Nautilus to a dive with unopened interior doors");
            if (!planned.path.success) std::printf("  FFIII Temple: %ls / %ls\n", planned.path.reason.c_str(), planned.path.error.c_str());
        }
    }
    std::printf("FFIII Storyline: %d objectives, %zu optional clues, %d total failures\n",
                count, hint_count, exdeath_routes::failures);
    return exdeath_routes::failures || fixture::wrong_owner ? 3 : 0;
}

struct StoryArrival {
    int map, entity;
    baked_route::Cell start;
    int vehicle = 1;
    std::optional<baked_route::Cell> observed_actor{};
};

template<class Receipts>
int verify_campaign(const char* title, Stage (*next)(const Receipts&), ObjectiveId first,
                    int opened, int ending, int minimum, std::span<const StoryArrival> arrivals)
{
    Receipts flags;
    flags.fill(0);
    check(!next(flags).objective, "the prologue owns the field until the native opening is ready");
    flags[opened] = 1;
    auto state = progress_at({}, 1);
    int count = 0;
    for (; count < 256; ++count) {
        const auto stage = next(flags);
        if (!stage.objective) {
            check(stage.complete && flags[ending] == 1, "only the final ending receipt completes the campaign");
            break;
        }
        const auto& objective = *stage.objective;
        const int receipt = static_cast<int>(objective.id) - static_cast<int>(first);
        const auto targets = contacts(objective, objective.map);
        check(!targets.empty() && targets.front().goal.map_id == objective.map &&
              targets.front().goal.entity_id == objective.contacts[0], "the story objective binds an exact native contact");
        if (targets.empty())
            std::printf("  %s map%d/%d type%d %ls\n", title, objective.map, objective.contacts[0],
                        static_cast<int>(objective.object_type), objective.label.data());
        if (!targets.empty()) {
            const auto arrival = std::find_if(arrivals.begin(), arrivals.end(), [&objective](const auto& candidate) {
                return objective.map == candidate.map && objective.contacts[0] == candidate.entity;
            });
            if (arrival != arrivals.end()) {
                std::copy(flags.begin(), flags.end(), state.scenario.begin());
                state.current_map_id = arrival->map;
                const auto& target = targets.front();
                const auto route = exdeath_routes::route(arrival->map, arrival->start, state,
                                                         target.goal, target.kind, false, arrival->vehicle, {},
                                                         arrival->observed_actor);
                check(route.path.success, "the production planner reaches the contact from a native approach");
                if (!route.path.success)
                    std::printf("  %s approach map%d/%d: %ls / %ls\n", title, arrival->map, arrival->entity,
                                route.path.reason.c_str(), route.path.error.c_str());
            }
        }
        if (receipt < 0 || static_cast<size_t>(receipt) >= flags.size()) return 3;
        check(flags[receipt] == 0, "each unsatisfied receipt advances the story once");
        flags[receipt] = 1;
    }
    check(count >= minimum && count < 256, "the story replay reaches a finite ending");
    std::printf("%s Storyline: %d objectives, %d total failures\n", title, count, exdeath_routes::failures);
    return exdeath_routes::failures || fixture::wrong_owner ? 3 : 0;
}

int verify_ff4()
{
    Ff4Receipts rested;
    rested.fill(0);
    rested[0] = rested[3] = 1;
    const auto after_baron = ff4_stage(rested).objective;
    check(after_baron && after_baron->map == 162, "FFIV retires the optional Baron conversations after resting");
    rested.fill(1);
    rested[37] = rested[39] = rested[70] = 0;
    const auto after_troia = ff4_stage(rested).objective;
    check(after_troia && after_troia->map == 1 && after_troia->contacts[0] == 135,
          "FFIV keeps the Tower of Zot objective when the party skipped thanking Edward");
    constexpr std::array arrivals{
        StoryArrival{14, 5, {0,6,5}},
        StoryArrival{239, 10, {0,21,19}, 1, baked_route::Cell{0,21,16}},
        StoryArrival{357, 1, {0,2,12}},
        StoryArrival{153, 12, {0,15,25}},
        StoryArrival{161, 14, {0,2,12}},
        StoryArrival{317, 3, {1,15,23}}
    };
    Ff4Receipts lunar;
    lunar.fill(1);
    lunar[65] = lunar[70] = 0;
    const auto palace = ff4_stage(lunar).objective;
    check(palace && palace->map == 153, "the Lunar Whale's outbound receipt advances to FuSoYa");
    if (palace) {
        const auto targets = contacts(*palace, 3);
        if (!targets.empty()) {
            auto flags = progress_at({}, 3);
            std::copy(lunar.begin(), lunar.end(), flags.scenario.begin());
            flags.current_area_id = 3;
            const auto& goal = targets.front();
            const auto planned = exdeath_routes::route(3, {1,59,11}, flags, goal.goal, goal.kind, false, 11);
            check(planned.path.success && planned.path.next_map_id == 291,
                  "FuSoYa's actual route lands for the lower entrance of Western Lunar Path");
            if (!planned.path.success) std::printf("  FFIV Palace: %ls / %ls\n", planned.path.reason.c_str(), planned.path.error.c_str());
        }
    }
    return verify_campaign("FFIV", ff4_stage, ObjectiveId::Ff4Scenario, 0, 70, 65, arrivals);
}

void verify_ff5_karnak_resume()
{
    Ff5Receipts karnak;
    karnak.fill(0);
    std::fill_n(karnak.begin(), 44, 1);
    karnak[36] = karnak[39] = 0;
    for (int shard = 423; shard <= 427; ++shard) karnak[shard] = 1;

    auto walse = karnak;
    std::fill_n(walse.begin() + 40, 4, 0);
    const auto before_transfer = ff5_stage(walse).objective;
    check(before_transfer && before_transfer->map == 57 && before_transfer->contacts[0] == 7,
          "FFV Storyline still offers the Walse king before using the meteorite");
    walse[39] = 1;
    const auto after_talk = ff5_stage(walse).objective;
    check(after_talk && after_talk->map == 246 && after_talk->contacts[0] == 2,
          "Speaking to the king still leads to the Walse meteorite");

    const auto stage = ff5_stage(karnak);
    check(stage.objective && stage.objective->map == 250 && stage.objective->contacts[0] == 28,
          "The saved Karnak transfer retires the skipped Walse conversation and selects Cid's ship scene");
    if (!stage.objective) return;
    const auto targets = contacts(*stage.objective, 1);
    check(targets.size() == 1, "The active Karnak objective binds its exact scripted contact");
    if (targets.empty()) return;

    const auto& target = targets.front();
    auto state = progress_at({}, 1);
    std::copy(karnak.begin(), karnak.end(), state.scenario.begin());
    state.scenario[990] = state.scenario[991] = 1;
    const auto land = exdeath_routes::route(1, {1,84,78}, state, target.goal, target.kind);
    check(land.path.success && land.path.next_map_id == 250 && land.path.step_count == 1 &&
          !land.path.walk_phases.empty() && !land.path.walk_phases.back().cells.empty() &&
          land.path.walk_phases.back().cells.back() == baked_route::Cell{1,84,79},
          "Storyline reaches the ship entrance one step south of the logged position");
    state.current_map_id = 250;
    state.current_area_id = 26;
    const auto deck = exdeath_routes::route(250, {1,13,17}, state, target.goal, target.kind);
    check(deck.path.success && !deck.path.walk_phases.empty() && !deck.path.walk_phases.back().cells.empty() &&
          deck.path.walk_phases.back().cells.back() == target.goal.cell,
          "From the ship's native arrival Storyline reaches Cid's scene on deck");
    std::printf("FFV Karnak resume: world %ls, deck %ls\n", land.path.description.c_str(), deck.path.description.c_str());

    karnak[44] = 1;
    const auto onward = ff5_stage(karnak).objective;
    check(onward && onward->map == 271 && onward->contacts[0] == 1,
          "Completing the ship scene advances to Liquid Flame with the Walse conversation still skipped");
}

int verify_ff5()
{
    verify_ff5_karnak_resume();
    Ff5Receipts fleet;
    fleet.fill(1);
    fleet[105] = fleet[107] = fleet[125] = fleet[153] = fleet[161] = 0;
    const auto on_fleet = ff5_stage(fleet).objective;
    check(on_fleet && on_fleet->map == 351 && on_fleet->contacts[0] == 1,
          "FFV continues on Xezat's fleet when the party flew there without visiting Castle Surgate");
    fleet[106] = 0;
    const auto before_fleet = ff5_stage(fleet).objective;
    check(before_fleet && before_fleet->map == 153 && before_fleet->contacts[0] == 16,
          "FFV still offers the Castle Surgate guards before the fleet landing");
    constexpr std::array arrivals{
        StoryArrival{189, 8, {1,25,21}},
        StoryArrival{189, 12, {1,10,26}, 1, baked_route::Cell{1,16,21}},
        StoryArrival{205, 17, {1,14,15}},
        StoryArrival{88, 20, {1,18,10}},
        StoryArrival{95, 55, {1,23,35}},
        StoryArrival{12, -1, {1,206,110}},
        StoryArrival{19, 112, {1,175,138}, 7},
        StoryArrival{569, 3, {1,30,24}}
    };
    Ff5Receipts later;
    later.fill(0);
    later[0] = later[83] = later[125] = 1;
    const auto merged = ff5_stage(later).objective;
    check(merged && merged->map == 70, "the merged world retires early contacts from unavailable worlds");
    later[153] = 1;
    const auto rift = ff5_stage(later).objective;
    check(rift && rift->map == 525, "a save inside the Rift does not return to optional tablet routes");
    later.fill(1);
    later[139] = later[153] = later[161] = 0;
    const auto boarding = ff5_stage(later).objective;
    check(boarding && boarding->transportation_id == 17, "Lenna's return leads to the ship that recovers the airship");
    if (boarding) {
        const auto targets = contacts(*boarding, 19);
        check(targets.size() == 1 && targets.front().goal.transportation_id == 17,
              "the ship objective takes the live merged-world craft position");
        if (!targets.empty()) {
            auto state = progress_at({}, 19);
            std::copy(later.begin(), later.end(), state.scenario.begin());
            state.current_area_id = 1;
            const auto& target = targets.front();
            const auto boarded = exdeath_routes::route(19, {1,58,122}, state, target.goal, target.kind,
                                                       false, 1, {}, {}, baked_route::Cell{1,87,105});
            check(boarded.path.success, "the native Guardian Tree return can reach the ship's first boarding tile");
            if (!boarded.path.success)
                std::printf("  FFV Ship: %ls / %ls\n", boarded.path.reason.c_str(), boarded.path.error.c_str());
        }
    }
    later[139] = 1;
    if (const auto flight = ff5_stage(later).objective) {
        const auto targets = contacts(*flight, 19);
        if (!targets.empty()) {
            auto state = progress_at({}, 19);
            std::copy(later.begin(), later.end(), state.scenario.begin());
            const auto& target = targets.front();
            const auto alternate = exdeath_routes::route(19, {1,175,138}, state, target.goal, target.kind, false, 19);
            check(alternate.path.success, "the Rift accepts the native upgraded airship variant");
            const auto foot = exdeath_routes::route(19, {1,175,138}, state, target.goal, target.kind);
            check(!foot.path.success, "the Rift cannot be approached on foot without a craft");
            state.scenario[139] = 0;
            const auto early = exdeath_routes::route(19, {1,175,138}, state, target.goal, target.kind, false, 7);
            check(!early.path.success, "the Rift contact is closed before the airship recovery scene");
        }
    }
    return verify_campaign("FFV", ff5_stage, ObjectiveId::Ff5Scenario, 0, 161, 150, arrivals);
}

int verify_ff6()
{
    Ff6Receipts flags;
    flags.fill(0);
    const auto receipt = [](const Objective& objective) {
        return static_cast<int>(objective.id) - static_cast<int>(ObjectiveId::Ff6Scenario);
    };
    const auto select = [&](int map, int ending) {
        int count = 0;
        while (flags[ending] == 0 && count < 120) {
            const auto stage = ff6_stage(flags, map);
            if (!stage.objective) break;
            const auto& objective = *stage.objective;
            const auto targets = contacts(objective, map);
            check(!targets.empty(), "FFVI Storyline target has its exact baked identity");
            if (targets.empty())
                std::printf("  FFVI map%d/%d type%d %ls\n", objective.map, objective.contacts[0],
                    static_cast<int>(objective.object_type), objective.label.data());
            const int bit = receipt(objective);
            if (bit < 0 || bit >= static_cast<int>(flags.size())) return -1;
            check(flags[bit] == 0, "FFVI Storyline advances an unmet receipt");
            flags[bit] = 1;
            ++count;
        }
        check(flags[ending] == 1, "FFVI chapter reaches its own native receipt");
        return count;
    };
    check(!ff6_stage(flags, 3).objective, "FFVI opening is native-owned");
    flags[0] = 1;
    int count = select(3, 50);
    check(count >= 30, "FFVI opening and Returners' Hideout include intervening milestones");
    const auto locke = ff6_stage(flags, 584).objective;
    check(locke && receipt(*locke) == 51, "scenario selection recommends Locke first");
    flags[103] = 1;
    const auto sabin = ff6_stage(flags, 350).objective;
    check(sabin && receipt(*sabin) == 72, "a player-picked Sabin scenario overrides the recommended order");
    count += select(350, 135);
    flags[126] = flags[135] = 0;
    auto buy = ff6_stage(flags, 84, false);
    auto feed = ff6_stage(flags, 87, true);
    check(buy.objective && buy.objective->map == 87 && feed.objective && feed.objective->map == 1,
          "Gau guidance switches from the Mobliz item shop to Veldt battles after buying meat");
    flags[126] = flags[135] = 1;
    flags[65] = 1;
    const auto light = ff6_stage(flags, 320);
    check(light.objective && !light.local_guide, "the Narshe guards and entrance are still prerequisites");
    flags[66] = flags[67] = 1;
    const auto light_ready = ff6_stage(flags, 320);
    check(light_ready.objective && light_ready.local_guide, "the native light path uses existing local guidance");
    count += select(320, 69);
    const auto pending = ff6_stage(flags, 584).objective;
    check(pending && receipt(*pending) == 51, "scenario selection only recommends unfinished scenarios");
    flags[51] = 1;
    count += select(38, 64);
    flags[136] = 1;
    count += select(3, 297);
    count += select(2, 473);
    auto tower = ff6_stage(flags, 549);
    check(tower.objective && receipt(*tower.objective) == 591 && tower.local_guide,
          "Kefka's Tower uses the native three-party guide");
    auto retreat = ff6_stage(flags, 268);
    check(retreat.objective && receipt(*retreat.objective) == 473 && !retreat.local_guide,
          "a party that left the tower can route back to the Falcon's entrance");
    if (retreat.objective) {
        const auto targets = contacts(*retreat.objective, 2);
        if (!targets.empty()) {
            auto state = progress_at({}, 2);
            std::copy(flags.begin(), flags.end(), state.scenario.begin());
            state.scenario[473] = 0;
            state.current_area_id = 1;
            const auto& goal = targets.front();
            const auto flight = exdeath_routes::route(2, {1,128,195}, state, goal.goal, goal.kind, false, 5);
            check(flight.path.success && !flight.path.walk_phases.empty() &&
                  flight.path.walk_phases.back().manual_after &&
                  !flight.path.walk_phases.back().cells.empty() &&
                  flight.path.walk_phases.back().cells.back() == goal.goal.cell,
                  "the actual Falcon route reaches the Kefka's Tower airship contact");
            if (!flight.path.success) std::printf("  FFVI Tower: %ls / %ls\n", flight.path.reason.c_str(), flight.path.error.c_str());
        }
    }
    flags[501] = 1;
    check(!ff6_stage(flags, 571).complete, "the summit arrival does not certify Kefka's defeat");
    flags[591] = 1;
    check(ff6_stage(flags, 571).complete, "the final boss victory continuation completes FFVI");
    check(count >= 95, "all FFVI pre-final chapters terminate with their own progression");

    flags.fill(0);
    flags[0] = 1;
    const auto first = ff6_stage(flags, 3).objective;
    if (first && !contacts(*first, 3).empty()) {
        auto state = progress_at({}, 3);
        std::copy(flags.begin(), flags.end(), state.scenario.begin());
        const auto goal = contacts(*first, 3).front();
        const auto route = exdeath_routes::route(3, {1,38,39}, state, goal.goal, goal.kind);
        check(route.path.success, "the planner reaches the first Narshe patrol from its southern approach");
    }
    std::printf("FFVI Storyline: %d ordered objectives, %d total failures\n", count, exdeath_routes::failures);
    return exdeath_routes::failures || fixture::wrong_owner ? 3 : 0;
}
}

int main()
{
    const auto read = [](const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(file), {});
    };
    constexpr std::array campaigns{verify, verify_ff2, verify_ff3, verify_ff4, verify_ff5, verify_ff6};
    int result = 0;
    for (size_t index = 0; index < campaigns.size(); ++index) {
        const auto directory = "data/ff" + std::to_string(index + 1) + "/";
        const auto maps = read(directory + "maps.bin");
        if (!baked::load(maps.data(), maps.size()) ||
            !json::parse(read(directory + "mapstate.json"), fixture::records)) {
            std::printf("FF%zu Storyline fixtures could not load\n", index + 1);
            result = std::max(result, 2);
            continue;
        }
        fixture::game_id = static_cast<game::Id>(index + 1);
        baked_nav::g_loaded = baked_nav::g_prepared = true;
        exdeath_routes::failures = exdeath_routes::assertions = exdeath_routes::legs = 0;
        std::thread worker([&] { result = std::max(result, campaigns[index]()); });
        worker.join();
    }
    return result;
}
