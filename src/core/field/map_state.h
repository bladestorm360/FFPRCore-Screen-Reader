#pragma once
#include "baked_map.h"
#include "baked_router.h"
#include "../json.h"
#include <array>
#include <map>
#include <optional>
#include <set>
#include <span>

namespace field::map_state {
inline constexpr std::array ff4_door_key_contents{68, 74, 75};

// Private copied state for navigation. Projection never writes native flags or runs scripts.
struct Flags {
    std::array<std::optional<int>, 1024> scenario, treasure;
    std::array<std::optional<int>, 16> global;
    std::array<std::optional<int>, 8> area, local;
    int current_map_id = 0, current_area_id = 0;
    // Read-only result of FFIII Current.CheckChild's all-active-member test.
    // It is navigation input, never a write into the native map-local bank.
    std::optional<int> party_mini;
    // FFIII consumable locks: owner-copied inventory and enabled active corps.
    // Missing/unready data stays unknown. Native script wrappers are not run.
    std::optional<int> magic_keys;
    std::optional<bool> party_thief;
    // Read-only important inventory, in ff4_door_key_contents order.
    std::optional<unsigned> ff4_door_keys;
    // FFVI's six Narshe locks require canonical character2 in the active
    // corps, rather than merely in the owned roster. Owned scalar only.
    std::optional<bool> party_locke;
    // Ebot's Rock's first coral feeding requires canonical Strago8 in the
    // active corps. Copied in the same bounded pass as Locke, with no extra
    // native calls or worker-side pointers.
    std::optional<bool> party_strago;
    // Duncan's later training region requires canonical Sabin6 in the active
    // corps. Reuse the same bounded party-role pass; reserve ownership is not
    // enough and a missing/unfinished member remains unknown.
    std::optional<bool> party_sabin;
    // Crescent's helmet-search initializer requires active canonical Gau12.
    // Reuse the existing role pass; reserve ownership cannot reveal its events.
    std::optional<bool> party_gau;
    // Observation Post's first visit tests active Terra1 and nonempty corps
    // count, not reserve ownership. Same existing role pass, owned values only.
    std::optional<bool> party_terra;
    // Doma's dream entrance requires active canonical Cyan3 and four members.
    // Reuse the bounded role/count pass; never substitute reserve ownership.
    std::optional<bool> party_cyan;
    // Umaro recruitment requires canonical Mog11 in the active corps. This
    // is guidance input; ordinary conversation remains a valid destination.
    std::optional<bool> party_mog;
    std::optional<int> active_party_members;
};
// SetEntities enables a group additively; a floor can have several independent
// switch/chest/story groups at once. group retains the final activation only for
// the small entry-guard result; membership always uses the complete set.
struct EntityState {
    std::optional<bool> visible;
    std::optional<int> x,y,layer;
    bool operator==(const EntityState& other) const { return visible==other.visible&&x==other.x&&y==other.y&&layer==other.layer; }
};
// When branches disagree, groups/entities contain only their proven common
// facts. known stays false so unknown gates remain possible obstacles. Actors
// whose visibility/placement differs cannot become certified destinations.
struct Selection {
    bool known=true; std::wstring group; std::set<std::wstring> groups;
    std::map<int,EntityState> entities; std::set<int> uncertain_entities;
    // Only assignments common to all completed initializer branches. Patrol
    // uncertainty does not hide an otherwise known visible actor or change gates.
    std::map<int,int> routes;
};
Selection project(const json::Value& record, const Flags& flags);
Flags snapshot();
// Read one native scenario bit on the owning thread; unavailable state is unknown.
std::optional<int> scenario_flag(int absolute_index);
// Explicit Storyline request on the ready field owner. Read only the requested
// prefix of the scenario bank, leaving its output unknown if storage is absent.
void read_scenario_flags(std::span<std::optional<int>> flags);
// FFII explicit guide requests only. Read the bounded native learned-word list
// on its owner; absence/malformed storage is unknown, never a learning receipt.
std::optional<bool> keyword_learned(int content_id);
// Copy a bounded group of FFII word receipts by content ID in one pass.
void read_keyword_flags(int first_content_id, std::span<std::optional<bool>> flags);
// FFIII explicit guide requests only:401 Mini/402 Frog;0 none,1 all,2 mixed.
// Uses the same bounded active-member ownership as the navigation Mini predicate.
std::optional<int> party_condition_state(int condition_type);
// Single current-floor value. Never borrows locals from a different map.
std::optional<int> local_value(int map_id, int index);
// Explicit guide requests only: the reviewed FFV flagless control remains in
// the native actor list after Hide. Missing/destroyed/over-budget is unknown.
std::optional<bool> entity_hidden(int map_id, int entity_id, const std::wstring& raw_name);
void observe_current(int map_id, std::wstring group);
void reset_observed(); // map reload, including a reload of the same map ID

class Resolver {
public:
    Resolver();
    explicit Resolver(Flags flags, const json::Value& records) : flags_(std::move(flags)), records_(records) {}
    // Unknown variants are excluded from destinations and included as possible obstacles.
    bool active(const baked::Submap& sm, const baked::Object& object, bool obstacle = false);
    std::optional<bool> treasure_opened(const baked::Submap& sm, const baked::Object& object) const;
    bool known(const baked::Submap& sm);
    bool consumed(const baked::Submap& sm, const baked::Object& object);
    uint64_t transportation_mask(const baked::Submap& sm,const baked::Object& object);
    baked::Object effective(const baked::Submap& sm, const baked::Object& object);
    // Reviewed closed-door collision planes can differ from its visible label
    // plane. Exact native opening counterparts qualify each extra plane.
    unsigned collision_layers(const baked::Submap& sm,const baked::Object& object) const;
    // Exact FFIII consumable-door identity and source-qualified nested cost.
    // This only permits a virtual route. The loaded-floor planner must stop
    // before the closed actor for manual native unlocking, never walk through it.
    bool can_unlock(const baked::Submap& sm,const baked::Object& object) const;
    // Exact story NPC or party-sensitive pushback event. Unlike an ordinary
    // patrol/event it blocks onward traversal even when NPC avoidance is off.
    bool story_barrier(const baked::Submap& sm,const baked::Object& object) const;
    struct WalkOpenContact {
        const baked::Object* contact;
        std::vector<const baked::Object*> blockers;
    };
    const std::vector<WalkOpenContact>& walk_open_contacts(const baked::Submap& sm);
    // Reviewed native patrol poses for a future floor. These are first-leg
    // evidence only; the loaded floor still requires its live actor snapshot.
    std::vector<baked_route::Cell> route_positions(const baked::Submap& sm,const baked::Object& object);
    // Read only the already copied scenario bank; no native access on workers.
    std::optional<int> scenario_value(size_t index) const { return index < flags_.scenario.size() ? flags_.scenario[index] : std::nullopt; }
    std::optional<int> treasure_value(size_t index) const { return index < flags_.treasure.size() ? flags_.treasure[index] : std::nullopt; }
    uint64_t signature() const; // copied persistent state only; worker-safe
    // Lazily projected once per map/kind in this owned resolver. Shared immutable
    // masks avoid scanning transition objects on every A*/flood edge or leg.
    // doors_only is for loaded FFIII world1: native capture supplies all warp
    // bits, so only the unchanged native-door bits need initializer projection.
    std::shared_ptr<const baked::TransitionGrid> transitions(const baked::Submap& sm,bool doors_only=false);
private:
    const Selection& selection(const baked::Submap& sm);
    Flags flags_;
    const json::Value& records_;
    // Positive native observations are additive, never a complete inventory.
    // Merge lazily with initializer facts on the navigation worker; capture
    // only copies the bounded native-owned group set.
    int observed_index_ = -1;
    std::set<std::wstring> observed_groups_;
    std::map<int, Selection> cache_;
    std::map<const baked::Object*,bool> consumed_;
    std::map<const baked::Object*,uint64_t> transportation_;
    std::map<int,std::shared_ptr<const baked::TransitionGrid>> transitions_;
    std::map<int,std::vector<WalkOpenContact>> walk_open_contacts_;
};
// NPC omission is only a cross-floor continuation proof. Current-floor movement
// still uses authoritative live blockers and stops at an actual interaction.
void add_obstacles(Resolver& resolver, const baked::Submap& sm, baked_route::Request& request, bool gates = true, bool geometry_only = false, bool with_npcs = true);
// Counter geometry follows active groups, separately from collision/hazards.
void add_relays(Resolver& resolver, const baked::Submap& sm, baked_route::Request& request);
}
