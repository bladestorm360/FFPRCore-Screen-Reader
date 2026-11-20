#include "storyline.h"
#include "baked_map.h"
#include <algorithm>
#include <span>

namespace field::storyline {
namespace {
using Id = ObjectiveId;
using Type = baked::ObjType;
using Kind = TargetKind;

constexpr Objective kMatoyaIntro{ObjectiveId::MeetMatoya, L"Meet Matoya", 12, {17, -1, -1},
                                TargetKind::Infer, baked::ObjType::Event};

constexpr Objective interact(Id id, std::wstring_view label, int map, int entity, Type type = Type::NPC)
{
    return {id, label, map, {entity, -1, -1}, Kind::Interact, type};
}

// Receipt order follows prerequisites and the recommended story order. Native
// victory/pickup continuations own each receipt; later optional progression
// (including out-of-order elemental bosses) is never inferred from its index.
struct Ff1Step {
    int receipt;
    Objective objective;
    std::optional<Objective> advice{};
};
constexpr std::array kFf1Journey{
    Ff1Step{16, interact(Id::Vampire, L"Defeat the vampire", 78, 54),
                interact(Id::EarthAdvice, L"Ask about the Cavern of Earth", 34, 67)},
    Ff1Step{17, interact(Id::StarRuby, L"Take the star ruby", 78, 22, Type::TreasureBox)},
    Ff1Step{18, interact(Id::GiveRuby, L"Give the star ruby to the giant", 81, 11),
                interact(Id::GiantAdvice, L"Ask what the giant likes to eat", 34, 66)},
    Ff1Step{19, interact(Id::EarthRod, L"Ask Sadda about the Earth Crystal", 40, 9)},
    Ff1Step{20, interact(Id::EarthSlab, L"Use the earth rod on the stone slab", 78, 52, Type::Entity)},
    Ff1Step{21, interact(Id::Lich, L"Defeat Lich", 80, 33, Type::AnimEntity)},
    Ff1Step{22, interact(Id::Canoe, L"Get the canoe from the Crescent Lake sages", 41, 57)},
    Ff1Step{23, interact(Id::Marilith, L"Defeat Marilith", 87, 33, Type::AnimEntity),
                interact(Id::FireAdvice, L"Ask the sage about the Fire Crystal", 41, 42)},
    Ff1Step{24, interact(Id::Levistone, L"Find the levistone in the Cavern of Ice", 91, 8, Type::AnimEntity),
                interact(Id::IceAdvice, L"Ask about the levistone", 41, 32)},
    Ff1Step{25, Objective{Id::Airship, L"Raise the airship from the desert", 1, {82, -1, -1}, Kind::Infer, Type::Event},
                interact(Id::AirshipAdvice, L"Ask the elf how to raise the airship", 22, 47)},
    Ff1Step{27, interact(Id::BuyFairy, L"Buy the bottled faerie from the caravan", 59, 8),
                interact(Id::CaravanAdvice, L"Ask where the caravan is camped", 52, 40)},
    Ff1Step{28, Objective{Id::ReleaseFairy, L"Bring the faerie back to Gaia", 60, {97, 96, 98}, Kind::Infer, Type::Event}},
    Ff1Step{29, interact(Id::Oxyale, L"Speak to the faerie at the spring", 60, 90)},
    Ff1Step{30, interact(Id::MeetMermaid, L"Speak to the woman by the submarine", 52, 67)},
    Ff1Step{31, interact(Id::RosettaStone, L"Find the Rosetta Stone in the Sunken Shrine", 103, 42, Type::TreasureBox)},
    Ff1Step{34, interact(Id::Kraken, L"Defeat Kraken", 96, 38, Type::AnimEntity)},
    Ff1Step{32, interact(Id::LearnLufenian, L"Bring the Rosetta Stone to Dr. Unne", 34, 126),
                interact(Id::RosettaAdvice, L"Ask the scholar about the Rosetta Stone", 52, 42)},
    Ff1Step{33, interact(Id::Chime, L"Get the chime in Lufenia", 70, 131)},
    Ff1Step{26, interact(Id::WarpCube, L"Get the warp cube from the robot", 104, 23),
                interact(Id::WarpAdvice, L"Ask the robot about the warp cube", 106, 20)},
    Ff1Step{35, interact(Id::Tiamat, L"Defeat Tiamat", 112, 3, Type::AnimEntity)},
    Ff1Step{36, interact(Id::EnterPast, L"Enter the Black Crystal time portal", 72, 39, Type::Warp),
                interact(Id::PastAdvice, L"Ask the bat about the Black Crystal", 72, 12)},
    Ff1Step{37, interact(Id::LuteSlab, L"Play the lute at the stone slab", 115, 9, Type::Entity)},
    Ff1Step{38, interact(Id::PastLich, L"Defeat Lich in the past", 116, 20, Type::AnimEntity)},
    Ff1Step{39, interact(Id::PastMarilith, L"Defeat Marilith in the past", 117, 52, Type::AnimEntity)},
    Ff1Step{40, interact(Id::PastKraken, L"Defeat Kraken in the past", 118, 69, Type::AnimEntity)},
    Ff1Step{41, interact(Id::PastTiamat, L"Defeat Tiamat in the past", 119, 14, Type::AnimEntity)},
    Ff1Step{42, interact(Id::Chaos, L"Face Chaos", 120, 6)},
};
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

Stage ff1_stage(const Ff1Receipts& scenario, int origin_map)
{
    if (scenario[42] == 1) return {{}, {}, true};
    if (scenario[15] != 1) {
        const CorneliaProgress early{scenario[0], scenario[1], scenario[2], scenario[3],
            scenario[5], scenario[6], scenario[7], scenario[9], scenario[10], scenario[11],
            scenario[8], scenario[12], scenario[13], scenario[14], scenario[15]};
        const auto objective = cornelia_objective(early);
        return {objective, objective ? cornelia_advice(objective->id, early) : std::nullopt};
    }
    for (const auto& step : kFf1Journey) {
        if (!scenario[step.receipt]) return {};
        if (scenario[step.receipt] == 1) continue;
        auto objective = step.objective;
        auto advice = step.advice;
        // The mermaid disappears after teaching the route. Subsequent trips
        // use the real barrel, including a return after leaving the shrine.
        if ((objective.id == Id::RosettaStone || objective.id == Id::Kraken) &&
            (origin_map < 96 || origin_map > 103))
            objective = interact(Id::BoardSubmarine, L"Board the submarine to the Sunken Shrine", 52, 64, Type::Warp);
        if (objective.id == Id::ReleaseFairy)
            advice = interact(Id::FairyAdvice, L"Ask who took the faerie", 60, 78);
        // Teleporting out of the past leaves receipt 36 set. The repeatable
        // portal has its own step trigger and no first-entry conversation.
        if (step.receipt >= 37 && (origin_map < 113 || origin_map > 120)) {
            objective = {Id::ReturnToPast, L"Return to the past through the time portal", 72, {64, -1, -1},
                         Kind::Transition, Type::Warp};
            advice = {};
        }
        return {objective, advice};
    }
    return {};
}

#include "storyline_ff2.inl"
#include "storyline_ff3.inl"
#include "storyline_ff4.inl"
#include "storyline_ff5.inl"
#include "storyline_ff6.inl"

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
    if (next.kind == Kind::Spot && next.contacts[0] < 0 && next.authored_spawn) {
        baked_nav::MapGoal goal;
        goal.map_id = next.map;
        goal.cell = *next.authored_spawn;
        goal.allow_world_transit = true;
        targets.push_back({std::move(goal), Kind::Spot, std::wstring(next.instruction)});
        return targets;
    }
    for (const int entity : next.contacts) {
        if (entity < 0) continue;
        const auto match = std::find_if(objects.begin(), objects.end(), [entity, &next](const auto& object) {
            return object.entity_id() == entity && object.type == next.object_type;
        });
        if (match == objects.end()) continue;
        baked_nav::MapGoal goal;
        goal.map_id = next.map;
        goal.cell = next.authored_spawn.value_or(baked_route::Cell{map->world ? 1 : match->layer, match->tx, match->ty});
        goal.entity_id = entity;
        goal.raw_name = baked::object_identifier(*match);
        goal.examine = next.kind == Kind::Interact && next.id >= Id::Vampire;
        goal.allow_world_transit = true;
        targets.push_back({std::move(goal), next.kind, std::wstring(next.instruction)});
    }
    return targets;
}
}
