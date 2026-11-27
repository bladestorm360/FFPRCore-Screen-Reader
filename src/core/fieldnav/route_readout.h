#pragma once
#include <cstdint>
#include <functional>

namespace field { struct PathInfo; namespace baked_route { struct Cell; } }
namespace unity { struct Vec3; }
namespace ffpr::fieldnav::route_readout {
// Explicit reports replace any earlier directions. A completed chunk requests
// the next one through its original selection owner.
std::uint64_t begin(std::function<void()> refresh);
bool current(std::uint64_t token);
int leg_limit();
void follow(::field::PathInfo& path, std::uint64_t generation, std::function<void()> next,
            std::function<void()> refresh = {});
void cancel();
void motion_input();
bool expects_field_action();
void field_action_started(int map, const ::field::baked_route::Cell& at);
bool battle_refresh_pending();
void field_returned(bool same_map_action);
std::uint64_t obstacle_token();
void obstacle_hit(int map, const ::field::baked_route::Cell& at);
void movement_settled(std::uint64_t token, const unity::Vec3& position);
}
