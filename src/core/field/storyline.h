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
    std::optional<int> opening, audience, garland, returned, lute, bridge, crossed, ship, crown;
};

enum class ObjectiveId { Audience, Garland, Lute, LeaveCastle, CrossBridge, GetShip, FindCrown };
struct Objective {
    ObjectiveId id;
    std::wstring_view label;
    int map;
    std::array<int, 3> contacts;
    TargetKind kind;
    baked::ObjType object_type;
};

std::optional<Objective> cornelia_objective(const CorneliaProgress& progress);
// Copy exact baked contacts from their owning map. The caller must supply a
// ready immutable map catalog and let the router check current state/movement.
std::vector<baked_nav::GuideTarget> contacts(const Objective& objective, int origin_map);
}
