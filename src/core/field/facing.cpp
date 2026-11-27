#include "field_api.h"
#include "field_internal.h"
#include "baked_nav.h"
#include "../refs.h"
#include <cmath>

namespace field {
namespace {
ref::Class kFacingPlayer{"Last.Entity.Field", "FieldPlayer"};
ref::Class kFacingEntity{"Last.Entity.Field", "FieldEntity"};
ref::Class kFacingSprite{"Last.Entity.Field", "FieldSpriteEntity"};
ref::Method mFacingLookAt{kFacingSprite, "LookAt", 2};
ref::Method mFacingIsMoving{kFacingEntity, "get_IsMoving", 0};
ref::Field fFacingVector{kFacingEntity, "directionVector"};
ref::Class kTargetObserver{"Last.Map", "InteractiveTargetObserver"};
ref::Field fTargetObserver{r::kFieldController, "interactiveTargetObserver"};
ref::Field fObserverPlayer{kTargetObserver, "fieldPlayerEntity"};
ref::Field fNativeTarget{kTargetObserver, "interactiveTargetEntity"};
ref::Field fObserving{kTargetObserver, "isObserving"};
ref::Method mCanAct{r::kIInteractiveEntity, "get_CanAction", 0};
ref::Method mTakeAction{kFacingPlayer, "TakeAction", 1};
constexpr unity::Vec2 headings[] = {{0,1}, {1,0}, {0,-1}, {-1,0}};

bool faces(void* player, int direction)
{
    if (direction < 0 || direction > 3 || fFacingVector.get() < 0) return false;
    const auto actual = ref::rd<unity::Vec2>(player, fFacingVector);
    return std::fabs(actual.x - headings[direction].x) < .01f &&
           std::fabs(actual.y - headings[direction].y) < .01f;
}
}

bool face_player(void* player, int direction)
{
    if (direction < 0 || direction > 3 || !unity::alive(player) || !kFacingPlayer.is(player)
        || field::player() != player || !mFacingLookAt.ptr() || !mFacingIsMoving.ptr()
        || fFacingVector.get() < 0) return false;
    try {
        using Moving = bool (*)(void*, const void*);
        if (reinterpret_cast<Moving>(mFacingIsMoving.ptr())(player, mFacingIsMoving.get())) return false;
        // FieldSpriteEntity.LookAt is the player's inherited override. It updates the
        // direction vector and native idle sprite; it never asks the player to move.
        using Look = void (*)(void*, unity::Vec2, bool, const void*);
        reinterpret_cast<Look>(mFacingLookAt.ptr())(player, headings[direction], true, mFacingLookAt.get());
        return faces(player, direction);
    } catch (...) { ref::call_failed_note("Field.FacePlayer"); return false; }
}

void* autowalk_interaction_candidate(void* map, void* player, const void* observed_by, int direction,
    const route_progress::InteractionTarget& expected, const void* identity)
{
    if (!player || !observed_by || !r::kFieldController.is(map) || !faces(player, direction) || expected.entity_id < 0 ||
        r::fPropEntityId.get() < 0 || !mTakeAction.ptr()) return nullptr;
    auto* observer = ref::rd<void*>(map, fTargetObserver);
    if (observer != observed_by || ref::rd<void*>(observer, fObserverPlayer) != player ||
        !ref::rd<bool>(observer, fObserving)) return nullptr;
    auto* candidate = ref::rd<void*>(observer, fNativeTarget);
    if (!candidate || (identity && candidate != identity) || !entity_is_active(candidate) ||
        !implements(candidate, r::kIInteractiveEntity)) return nullptr;
    auto* property = entity_property(candidate);
    if (!property || ref::rd<int>(property, r::fPropEntityId) != expected.entity_id ||
        (expected.object_type >= 0 && object_type_of(property) != expected.object_type)) return nullptr;
    auto* transform = unity::transform(candidate);
    baked_route::Cell cell;
    if (!transform || !baked_nav::cell_at(map, unity::local_position(transform), cell) ||
        cell.x != expected.cell.x || cell.y != expected.cell.y ||
        game_object_layer(candidate) - 9 != expected.cell.layer || !call_virtual_bool(mCanAct, candidate))
        return nullptr;
    return candidate;
}

bool confirm_autowalk_interaction(void* player, void* candidate)
{
    if (!player || !candidate || !mTakeAction.ptr()) return false;
    ref::call<void>(mTakeAction, player, candidate);
    return true;
}
}
