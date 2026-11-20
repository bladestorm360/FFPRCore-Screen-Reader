#include "storyline.h"
#include "baked_map.h"
#include <algorithm>

namespace field::storyline {
namespace {
constexpr Objective kMatoyaIntro{ObjectiveId::MeetMatoya, L"Meet Matoya", 12, {17, -1, -1},
                                TargetKind::Infer, baked::ObjType::Event};
}

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
    if (progress.ship != 1) return {};
    if (progress.crown == 0)
        return Objective{ObjectiveId::FindCrown, L"Find the crown in Marsh Cave", 75, {40, -1, -1}, TargetKind::Interact, baked::ObjType::TreasureBox};
    if (progress.crown != 1) return {};
    if (progress.eye == 0)
        return Objective{ObjectiveId::BringCrown, L"Bring the crown to the Western Keep", 33, {105, -1, -1},
                         TargetKind::Interact, baked::ObjType::NPC};
    if (progress.eye != 1) return {};
    // Matoya's entrance scene must run before the eye handover group takes over.
    if (progress.tonic == 0 && progress.met_matoya == 0) return kMatoyaIntro;
    if (progress.met_matoya != 1) return {};
    if (progress.tonic == 0)
        return Objective{ObjectiveId::ReturnEye, L"Return Matoya's crystal eye", 12, {15, -1, -1},
                         TargetKind::Interact, baked::ObjType::NPC};
    if (progress.tonic != 1) return {};
    if (progress.key == 0)
        return Objective{ObjectiveId::WakePrince, L"Take the jolt tonic to the elven prince", 32, {116, -1, -1},
                         TargetKind::Interact, baked::ObjType::NPC};
    if (progress.key != 1) return {};
    if (progress.powder == 0)
        return Objective{ObjectiveId::GetPowder, L"Find the nitro powder in Cornelia Castle", 2, {10, -1, -1},
                         TargetKind::Interact, baked::ObjType::TreasureBox};
    if (progress.powder == 1 && progress.canal == 0)
        return Objective{ObjectiveId::OpenCanal, L"Bring the nitro powder to Nerrick", 21, {45, -1, -1},
                         TargetKind::Interact, baked::ObjType::NPC};
    return {};
}

std::optional<Objective> cornelia_advice(ObjectiveId objective, const CorneliaProgress& progress)
{
    // The dancer changes advice with the native scenario flags. Use her only
    // during the stages where her own script delivers the advertised clue.
    const auto dancer = [](ObjectiveId id) {
        return Objective{id, L"Speak to the dancer in Cornelia", 4, {83, -1, -1},
                         TargetKind::Interact, baked::ObjType::NPC};
    };
    switch (objective) {
    case ObjectiveId::CrossBridge: return dancer(ObjectiveId::BridgeAdvice);
    case ObjectiveId::GetShip: return dancer(ObjectiveId::ShipAdvice);
    case ObjectiveId::FindCrown:
    case ObjectiveId::BringCrown:
        if (progress.met_matoya == 0) return kMatoyaIntro;
        if (progress.met_matoya == 1) return dancer(ObjectiveId::CrownAdvice);
        return {};
    case ObjectiveId::ReturnEye: return dancer(ObjectiveId::EyeAdvice);
    case ObjectiveId::WakePrince: return dancer(ObjectiveId::PrinceAdvice);
    case ObjectiveId::GetPowder:
        return Objective{ObjectiveId::PowderAdvice, L"Ask the dwarf about nitro powder", 21, {1, -1, -1},
                         TargetKind::Interact, baked::ObjType::NPC};
    default: return {};
    }
}

Objective Selection::resolve(const Objective& objective, const std::optional<Objective>& hint, bool cycle)
{
    const auto hint_id = hint ? std::optional{hint->id} : std::nullopt;
    if (stage != objective.id || advice != hint_id) {
        stage = objective.id;
        advice = hint_id;
        prefer_advice = true;
    }
    if (cycle && hint) prefer_advice = !prefer_advice;
    return prefer_advice && hint ? *hint : objective;
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
