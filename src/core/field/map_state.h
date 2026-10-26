#pragma once
#include "baked_map.h"
#include "baked_router.h"
#include "../json.h"
#include <array>
#include <map>
#include <optional>
#include <set>

namespace field::map_state {
// Private copied state for navigation. Projection never writes native flags or runs scripts.
struct Flags {
    std::array<std::optional<int>, 1024> scenario, treasure;
    std::array<std::optional<int>, 16> global;
    std::array<std::optional<int>, 8> area, local;
    int current_map_id = 0, current_area_id = 0;
    // Read-only result of FFIII Current.CheckChild's all-active-member test.
    // It is navigation input, never a write into the native map-local bank.
    std::optional<int> party_mini;
};
// SetEntities enables a group additively; a floor can have several independent
// switch/chest/story groups at once. group retains the final activation only for
// the small entry-guard result; membership always uses the complete set.
struct EntityState {
    std::optional<bool> visible;
    std::optional<int> x,y;
    bool operator==(const EntityState& other) const { return visible==other.visible&&x==other.x&&y==other.y; }
};
// When branches disagree, groups/entities contain only their proven common
// facts. known stays false so unknown gates remain possible obstacles. Actors
// whose visibility/placement differs cannot become certified destinations.
struct Selection {
    bool known=true; std::wstring group; std::set<std::wstring> groups;
    std::map<int,EntityState> entities; std::set<int> uncertain_entities;
};
Selection project(const json::Value& record, const Flags& flags);
Flags snapshot();
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
    baked::Object effective(const baked::Submap& sm, const baked::Object& object);
    uint64_t signature() const; // copied persistent state only; worker-safe
    // Lazily projected once per map in this owned resolver. Shared immutable
    // masks avoid scanning transition objects on every A*/flood edge or leg.
    std::shared_ptr<const baked::TransitionGrid> transitions(const baked::Submap& sm);
private:
    const Selection& selection(const baked::Submap& sm);
    Flags flags_;
    const json::Value& records_;
    std::map<int, Selection> cache_;
    std::map<const baked::Object*,bool> consumed_;
    std::map<int,std::shared_ptr<const baked::TransitionGrid>> transitions_;
};
void add_obstacles(Resolver& resolver, const baked::Submap& sm, baked_route::Request& request, bool gates = true, bool geometry_only = false);
// Counter geometry follows active groups, separately from collision/hazards.
void add_relays(Resolver& resolver, const baked::Submap& sm, baked_route::Request& request);
}
