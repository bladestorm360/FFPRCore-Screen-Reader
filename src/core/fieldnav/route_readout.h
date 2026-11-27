#pragma once
#include <cstdint>
#include <functional>

namespace field { struct PathInfo; }
namespace ffpr::fieldnav::route_readout {
// Explicit reports replace any earlier directions. A completed chunk requests
// the next one through its original selection owner.
std::uint64_t begin();
bool current(std::uint64_t token);
int leg_limit();
void follow(::field::PathInfo& path, std::uint64_t generation, std::function<void()> next);
void cancel();
void motion_input();
}
