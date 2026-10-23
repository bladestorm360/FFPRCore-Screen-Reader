#pragma once
#include "../../core/field/baked_nav.h"
#include "../../core/field/map_state.h"

// FF6 scenario rules are original navigation facts/instructions in puzzles.json.
// This worker-only model never invokes scripts, changes flags or owns a pointer.
namespace ffpr::ff6::story {
struct Snapshot {
    field::map_state::Flags flags;
    int map=0;
    field::baked_route::Cell player;
    std::wstring locale;
    struct Party { int map=0;std::optional<unity::Vec3> position;std::wstring serialized; };
    std::array<Party,3> parties; // copied native saves; serialized fallback is parsed on the worker
};
struct Plan {
    std::vector<field::baked_nav::GuideTarget> targets;
    std::wstring introduction, note, hold_note;
    std::optional<field::baked_nav::DefenseContext> defense;
    std::optional<field::map_state::Flags> anticipated_release;
    int map=0;
    bool supported=false, finished=false, unknown=false;
};
bool supported_map(int map);
Plan plan(const Snapshot& snapshot);
Snapshot capture_snapshot(); // native owner only, bounded three-party observation
}
