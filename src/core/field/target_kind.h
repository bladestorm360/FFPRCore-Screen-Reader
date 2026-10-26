#pragma once
// Preserve what the selected row means when a manual object and a map exit
// overlap. Unlocking the door and reaching its interaction approach are separate.
#include "field_internal.h"
#include "field_api.h"

namespace field {
inline TargetKind navigation_target_kind(Kind kind,Category category){
    if(category==Category::Objects)return TargetKind::Interact;
    switch(kind){
    case Kind::NPC:case Kind::TreasureChest:return TargetKind::Interact;
    case Kind::MapExit:return TargetKind::Transition;
    case Kind::Vehicle:return TargetKind::Spot;
    // Save points and automatic events retain their native inference, including
    // tutorial overlaps, step-on warps and vehicle activation pads.
    default:return TargetKind::Infer;
    }
}
}
