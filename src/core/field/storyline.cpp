#include "storyline.h"
#include "baked_map.h"
#include <algorithm>

namespace field::storyline {
std::optional<Objective> cornelia_objective(const CorneliaProgress& progress)
{
    if (progress.opening != 1 || !progress.audience || !progress.garland ||
        !progress.returned || !progress.lute || !progress.bridge || !progress.crossed) return {};
    const std::array receipts{*progress.audience, *progress.garland, *progress.returned,
                              *progress.lute, *progress.bridge, *progress.crossed};
    // Receipts are cumulative in this sequence. A gap is an unknown save
    // state, including a rescue that has yet to finish its return scene.
    if (!std::is_sorted(receipts.rbegin(), receipts.rend()) || receipts.front() > 1 || receipts.back() < 0)
        return {};
    if (progress.audience == 0)
        return Objective{ObjectiveId::Audience, L"Meet the King of Cornelia", 3, {14, -1, -1}, TargetKind::Interact, baked::ObjType::NPC};
    if (progress.garland == 0)
        return Objective{ObjectiveId::Garland, L"Rescue Princess Sarah", 72, {32, -1, -1}, TargetKind::Interact, baked::ObjType::NPC};
    if (progress.returned != 1) return {};
    if (progress.lute == 0)
        return Objective{ObjectiveId::Lute, L"Receive the Lute from Princess Sarah", 3, {7, -1, -1}, TargetKind::Interact, baked::ObjType::NPC};
    if (progress.bridge == 0)
        return Objective{ObjectiveId::LeaveCastle, L"Leave Cornelia Castle", 2, {45, 44, 46}, TargetKind::Transition, baked::ObjType::Event};
    if (progress.crossed == 0)
        return Objective{ObjectiveId::CrossBridge, L"Cross the new bridge", 1, {89, -1, -1}, TargetKind::Infer, baked::ObjType::Event};
    // Bikke's victory continuation awards the ship before recording receipt 9.
    if (progress.ship == 0)
        return Objective{ObjectiveId::GetShip, L"Get a ship from Bikke", 13, {38, -1, -1}, TargetKind::Interact, baked::ObjType::NPC};
    if (progress.ship == 1 && progress.crown == 0)
        return Objective{ObjectiveId::FindCrown, L"Find the crown in Marsh Cave", 75, {40, -1, -1}, TargetKind::Interact, baked::ObjType::TreasureBox};
    return {};
}

std::vector<baked_nav::GuideTarget> contacts(const Objective& objective, int origin_map)
{
    // The acquired ship lives on the Overworld. Plan the first leg to the
    // actual town exit so the next request can use the live docked vessel.
    auto next = objective;
    if (objective.id == ObjectiveId::FindCrown && origin_map == 13) {
        next.map = 13;
        next.contacts = {5, -1, -1};
        next.kind = TargetKind::Transition;
        next.object_type = baked::ObjType::GotoMap;
    }
    std::vector<baked_nav::GuideTarget> targets;
    const auto* map = baked::submap_by_map_id(next.map);
    if (!map) return targets;
    const auto& objects = baked::grids(*map).objects;
    for (const int entity : next.contacts) {
        if (entity < 0) continue;
        const auto match = std::find_if(objects.begin(), objects.end(), [entity, &next](const auto& object) {
            return object.entity_id() == entity && object.type == next.object_type;
        });
        if (match == objects.end()) continue;
        baked_nav::MapGoal goal;
        goal.map_id = next.map;
        goal.cell = {map->world ? 1 : match->layer, match->tx, match->ty};
        goal.entity_id = entity;
        goal.raw_name = baked::object_identifier(*match);
        goal.allow_world_transit = true;
        targets.push_back({std::move(goal), next.kind, {}});
    }
    return targets;
}
}
