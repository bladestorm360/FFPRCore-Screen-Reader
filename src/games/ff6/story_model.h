#pragma once
#include "../../core/field/baked_nav.h"
#include "../../core/field/map_state.h"
#include "../../core/field/puzzle_policy.h"

// FF6 scenario rules are original navigation facts/instructions in puzzles.json.
// This worker-only model never invokes scripts, changes flags or owns a pointer.
namespace ffpr::ff6::story {
struct Snapshot {
    field::map_state::Flags flags;
    int map=0;
    field::baked_route::Cell player;
    std::wstring locale;
    field::puzzle_policy::Policy guidance; // owned settings, never read by a worker
    struct Party { int map=0;std::optional<unity::Vec3> position;std::wstring serialized; };
    std::array<Party,3> parties; // copied native saves; serialized fallback is parsed on the worker
    std::function<bool()> cancelled; // owned epoch only; no native pointers
    std::optional<float> roof_seconds; // exact current0850 timer, copied on native owner
    bool roof_frozen=false; // existing explicit user timer-freeze setting
};
struct Plan {
    struct BridgeCrossing {
        std::vector<field::baked_route::Cell> cells; // owned, cardinal, at most six steps
        int mode=0,phase=-1; // exact0443/0444 callback phase, never a guessed clock
        bool immediate=false; // already travelling: never wait on a temporary tile
    };
    std::optional<BridgeCrossing> bridge;
    std::vector<field::baked_nav::GuideTarget> targets;
    std::wstring introduction, note, hold_note;
    std::optional<field::baked_nav::DefenseContext> defense;
    std::optional<field::map_state::Flags> anticipated_release;
    int map=0;
    int finish_on_exit=0; // armed only by an actual completed native puzzle stage
    std::vector<int> finish_through; // explicitly reviewed normal-field floors before that exit
    int finish_on_scenario=-1; // explicit persistent completion through native battles/scenes
    bool supported=false, finished=false, unknown=false;
    unsigned puzzle_types=0; // required output types, checked again on owner publication
    bool roof_timed=false; // preview first; native next cycle releases a short Go cue
    field::baked_route::Cell roof_start;
    bool owzer_door_wait=false; // exact safe staging cell; native cycle supplies the cue
};
bool supported_map(int map);
Plan plan(const Snapshot& snapshot);
Snapshot capture_snapshot(); // native owner only, bounded three-party observation
}
