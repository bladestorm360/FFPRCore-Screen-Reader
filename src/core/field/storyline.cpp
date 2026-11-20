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
        return Objective{ObjectiveId::Audience, L"Meet the King of Cornelia", 3, {14, -1, -1}, TargetKind::Interact};
    if (progress.garland == 0)
        return Objective{ObjectiveId::Garland, L"Rescue Princess Sarah", 72, {32, -1, -1}, TargetKind::Interact};
    if (progress.returned != 1) return {};
    if (progress.lute == 0)
        return Objective{ObjectiveId::Lute, L"Receive the Lute from Princess Sarah", 3, {7, -1, -1}, TargetKind::Interact};
    if (progress.bridge == 0)
        return Objective{ObjectiveId::LeaveCastle, L"Leave Cornelia Castle", 2, {45, 44, 46}, TargetKind::Transition};
    if (progress.crossed == 0)
        return Objective{ObjectiveId::CrossBridge, L"Cross the new bridge", 1, {89, -1, -1}, TargetKind::Infer};
    return {};
}

std::vector<baked_nav::GuideTarget> contacts(const Objective& objective)
{
    std::vector<baked_nav::GuideTarget> targets;
    const auto* map = baked::submap_by_map_id(objective.map);
    if (!map) return targets;
    const auto& objects = baked::grids(*map).objects;
    for (const int entity : objective.contacts) {
        if (entity < 0) continue;
        const auto type = objective.kind == TargetKind::Interact ? baked::ObjType::NPC : baked::ObjType::Event;
        const auto match = std::find_if(objects.begin(), objects.end(), [entity, type](const auto& object) {
            return object.entity_id() == entity && object.type == type;
        });
        if (match == objects.end()) continue;
        baked_nav::MapGoal goal;
        goal.map_id = objective.map;
        goal.cell = {map->world ? 1 : match->layer, match->tx, match->ty};
        goal.entity_id = entity;
        goal.raw_name = baked::object_identifier(*match);
        goal.allow_world_transit = true;
        targets.push_back({std::move(goal), objective.kind, {}});
    }
    return targets;
}
}
