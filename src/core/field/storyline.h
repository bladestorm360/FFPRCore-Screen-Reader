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
    Ff2Scenario = 100, Ff2Keyword = 300, Ff2Advice = 400
};
struct Objective {
    ObjectiveId id;
    std::wstring_view label;
    int map;
    std::array<int, 3> contacts;
    TargetKind kind;
    baked::ObjType object_type;
};

std::optional<Objective> cornelia_objective(const CorneliaProgress& progress);
std::optional<Objective> cornelia_advice(ObjectiveId objective, const CorneliaProgress& progress);

using Ff1Receipts = std::array<std::optional<int>, 43>;
struct Stage {
    std::optional<Objective> objective, advice;
    bool complete = false;
};
Stage ff1_stage(const Ff1Receipts& scenario, int origin_map);
using Ff2Receipts = std::array<std::optional<int>, 101>;
inline constexpr int kFirstFf2Keyword = 917;
using Ff2Keywords = std::array<std::optional<bool>, 15>;
Stage ff2_stage(const Ff2Receipts& scenario, const Ff2Keywords& learned);

// The saved receipts select the quest. The player's browse selection can skip
// advice without inventing a completion flag for an ordinary conversation.
struct Selection {
    std::optional<ObjectiveId> stage, advice;
    bool prefer_advice = true;
    Objective resolve(const Objective& objective, const std::optional<Objective>& hint, bool cycle);
};

// Copy exact baked contacts from their owning map. The caller must supply a
// ready immutable map catalog and let the router check current state/movement.
std::vector<baked_nav::GuideTarget> contacts(const Objective& objective, int origin_map);
}
