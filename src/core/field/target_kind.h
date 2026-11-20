#pragma once
// Preserve what the selected row means when a manual object and a map exit
// overlap. Unlocking the door and reaching its interaction approach are separate.
#include "field_internal.h"
#include "field_api.h"

namespace field {
inline TargetKind navigation_target_kind(Kind kind,Category category,bool aerial=false){
    if(aerial)return TargetKind::AirshipTransition;
    if(category==Category::Objects)return TargetKind::Interact;
    // Reviewed boss contacts browse as NPCs while the original map object
    // still requires a manual interaction from an adjacent tile.
    if(kind==Kind::Event&&category==Category::NPCs)return TargetKind::Interact;
    switch(kind){
    case Kind::NPC:case Kind::TreasureChest:return TargetKind::Interact;
    case Kind::MapExit:return TargetKind::Transition;
    case Kind::Vehicle:case Kind::SavePoint:return TargetKind::Spot;
    // Automatic events retain native inference for step-on warps and pads.
    default:return TargetKind::Infer;
    }
}
}
