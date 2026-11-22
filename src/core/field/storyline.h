#pragma once

#include "baked_nav.h"
#include "baked_map.h"
#include <array>
#include <optional>
#include <string_view>

namespace field::storyline {
// Persistent native receipts for the opening FF1 quests. The route origin is
// unnecessary until planning movement to the selected objective's contacts.
struct CorneliaProgress {
    std::optional<int> opening, audience, garland, returned, lute, bridge, crossed, ship, crown, eye;
    std::optional<int> met_matoya, tonic, key, powder, canal;
};

enum class ObjectiveId {
    Audience, Garland, Lute, LeaveCastle, CrossBridge, GetShip, FindCrown, BringCrown,
    MeetMatoya, ReturnEye, WakePrince, GetPowder, OpenCanal,
    BridgeAdvice, ShipAdvice, CrownAdvice, EyeAdvice, PrinceAdvice, PowderAdvice,
    Vampire, StarRuby, GiveRuby, EarthRod, EarthSlab, Lich, Canoe, Marilith, Levistone,
    Airship, BuyFairy, ReleaseFairy, Oxyale, MeetMermaid, BoardSubmarine, RosettaStone,
    Kraken, LearnLufenian, Chime, WarpCube, Tiamat, EnterPast, ReturnToPast, LuteSlab,
    PastLich, PastMarilith, PastKraken, PastTiamat, Chaos,
    EarthAdvice, GiantAdvice, FireAdvice, IceAdvice, AirshipAdvice,
    CaravanAdvice, FairyAdvice, RosettaAdvice, WarpAdvice, PastAdvice,
    Ff2Scenario = 100, Ff2Keyword = 300, Ff2Advice = 400,
    Ff3Scenario = 500, Ff3EyeRecovery = 900,
    Ff3KazusAdvice, Ff3TozusAdvice, Ff3GutscoAdvice, Ff3GoldorAdvice, Ff3DalgAdvice,
    Ff4Scenario = 1000, Ff5Scenario = 1400, Ff6Scenario = 1900
};
struct Objective {
    ObjectiveId id;
    std::wstring_view label;
    int map;
    std::array<int, 3> contacts;
    TargetKind kind;
    baked::ObjType object_type;
    std::optional<baked_route::Cell> authored_spawn{};
    std::wstring_view instruction{};
    int transportation_id = 0;
};

std::optional<Objective> cornelia_objective(const CorneliaProgress& progress);
std::optional<Objective> cornelia_advice(ObjectiveId objective, const CorneliaProgress& progress);

using Ff1Receipts = std::array<std::optional<int>, 43>;
struct Stage {
    std::optional<Objective> objective, advice;
    bool complete = false;
    bool local_guide = false;
};
Stage ff1_stage(const Ff1Receipts& scenario, int origin_map);
using Ff2Receipts = std::array<std::optional<int>, 101>;
inline constexpr int kFirstFf2Keyword = 917;
using Ff2Keywords = std::array<std::optional<bool>, 15>;
Stage ff2_stage(const Ff2Receipts& scenario, const Ff2Keywords& learned);
// The Crystal Tower mirror uses native ScenarioFlag2 slot 95.
using Ff3Receipts = std::array<std::optional<int>, 352>;
Stage ff3_stage(const Ff3Receipts& scenario, int origin_map = 0);
// Tower of Babel's Lugae lock records slot 17 in ScenarioFlag2.
using Ff4Receipts = std::array<std::optional<int>, 274>;
Stage ff4_stage(const Ff4Receipts& scenario);
using Ff5Receipts = std::array<std::optional<int>, 429>;
Stage ff5_stage(const Ff5Receipts& scenario);
using Ff6Receipts = std::array<std::optional<int>, 592>;
Stage ff6_stage(const Ff6Receipts& scenario, int origin_map, std::optional<bool> has_dried_meat = {});

// The saved receipts select the quest. The player's browse selection can skip
// advice without inventing a completion flag for an ordinary conversation.
struct Selection {
    std::optional<ObjectiveId> stage, advice;
    bool prefer_advice = true;
    Objective resolve(const Objective& objective, const std::optional<Objective>& hint, bool cycle);
};

// The field caller has already applied Selection to the stage's objective.
std::wstring_view selection_label_key(const Stage& selected, bool spoilers);

// Copy exact baked contacts from their owning map. The caller must supply a
// ready immutable map catalog and let the router check current state/movement.
std::vector<baked_nav::GuideTarget> contacts(const Objective& objective, int origin_map);
}
