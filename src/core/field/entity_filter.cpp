#include "entity_filter.h"
#include "target_kind.h"
#include "navigation_labels.h"
#include "save_points.h"
#include <algorithm>
#include <cmath>
namespace field::filter_worker {
namespace {
struct Cancelled {};
void check_cancelled(const FilterInput& input) {
    if (input.cancelled && input.cancelled()) throw Cancelled{};
}
float point_distance(Vec3 a, Vec3 b) {
    const auto x=a.x-b.x, y=a.y-b.y, z=a.z-b.z;
    return static_cast<float>(std::sqrt(static_cast<double>(x*x+y*y+z*z)));
}
// Exact copied identity only: the native ID zero is a valid identity too.
bool continued_row(const FilterRow& row, const FilterInput& input) {
    return input.continued_goal && (row.remote_map ? row.remote_map : input.current_map) == input.continued_map &&
        (row.remote_map || row.cell_valid) && row.cell_x == input.continued_x && row.cell_y == input.continued_y &&
        (input.continued_entity_id < 0 || row.entity_id == input.continued_entity_id) &&
        (input.continued_raw.empty() || row.raw_name == input.continued_raw);
}
// Shared with grouped-exit retention so the worker never prunes a row that
// ordinary restoration could identify. Names translated for speech are not identity.
bool saved_selection_row(const FilterRow& row, const FilterInput& input) {
    if (!input.selected_has) return false;
    if (input.selected_transportation_id > 0) {
        const int map = row.remote_map ? row.remote_map : input.current_map;
        return row.kind == Kind::Vehicle && row.transportation_id == input.selected_transportation_id && map == input.selected_map;
    }
    if (input.selected_entity_id >= 0 && row.entity_id != input.selected_entity_id) return false;
    if (!input.selected_raw.empty() && row.raw_name != input.selected_raw) return false;
    if (input.selected_map > 0) {
        const int map = row.remote_map ? row.remote_map : input.current_map;
        if (map != input.selected_map || row.category != input.selected_category) return false;
        const bool moving_identity = input.selected_entity_id >= 0 && (row.kind == Kind::NPC || row.kind == Kind::Vehicle);
        const bool at_anchor = row.remote_map || row.cell_valid
            ? row.cell_x == input.selected_x && row.cell_y == input.selected_y
            : point_distance(row.position, input.selected_position) < 0.5f;
        return moving_identity || at_anchor;
    }
    return input.selected_remote > 0
        ? row.remote_map == input.selected_remote && row.cell_x == input.selected_x && row.cell_y == input.selected_y
        : !row.remote_map && row.category == input.selected_category && point_distance(row.position, input.selected_position) < 0.5f;
}
void restore_selection(Filtered& result, const FilterInput& input) {
    if (input.continued_goal) for (size_t i = 0; i < result.order.size(); ++i) {
        if (continued_row(input.rows[result.order[i]], input)) {
            result.continued = static_cast<int>(i); break;
        }
    }
    for (size_t i = 0; i < result.order.size(); ++i)
        if (input.rows[result.order[i]].selected) { result.restored = static_cast<int>(i); return; }
    for (size_t i = 0; i < result.order.size(); ++i)
        if (saved_selection_row(input.rows[result.order[i]], input)) { result.restored = static_cast<int>(i); return; }
}

}
static Filtered build_impl(FilterInput& input) {
    check_cancelled(input);
    Filtered result;
    std::vector<npc_names::Entry> names,objects;
    for (auto& row : input.rows) {
        check_cancelled(input);
        if (row.available && row.kind == Kind::NPC)
            names.push_back({&row.name, row.label, row.remote_map, 0,
                row.remote_map ? static_cast<float>(row.cell_x) : row.numbering_position.x,
                row.remote_map ? static_cast<float>(row.cell_y) : -row.numbering_position.y});
        // Number exact translated object labels independently of NPCs, before
        // category/distance/reachability pruning. Captured originals prevent a
        // later scan from treating Book1 as a new base or adding another suffix.
        else if(row.available&&row.category==Category::Objects)
            objects.push_back({&row.name,row.label,row.remote_map,row.cell_layer,
                row.remote_map?static_cast<float>(row.cell_x):row.numbering_position.x,
                row.remote_map?static_cast<float>(row.cell_y):-row.numbering_position.y});
    }
    npc_names::renumber(names); // before category/distance/reachability inclusion
    npc_names::renumber(objects);
    check_cancelled(input);
    for (const auto& row : input.rows) {
        check_cancelled(input);
        if (row.kind == Kind::MapExit && !row.remote_map && (!input.reachable_only || !input.world || row.route_distance>=0)) result.exits.push_back(row.numbering_position);
        result.names.push_back(row.name);
        if (row.available && (!input.reachable_only || !row.story_gated) && (input.category == Category::All || input.category == row.category)) result.order.push_back(row.index);
    }
    if (input.sort) std::stable_sort(result.order.begin(), result.order.end(), [&](size_t a, size_t b) {
        check_cancelled(input);
        const auto distance = [&](size_t i) {
            const auto& row = input.rows[i];
            return row.remote_map ? 1.0e9f + static_cast<float>(i) : point_distance(row.position, input.player);
        };
        return distance(a) < distance(b);
    });
    std::unordered_set<std::wstring> pads;
    std::unordered_map<int,size_t> exits;
    std::unordered_map<std::wstring,size_t> events;
    if(input.events)for(size_t index:result.order) {
        check_cancelled(input);
        const auto& row=input.rows[index];if(row.event_group.empty())continue;
        const auto [at,inserted]=events.emplace(row.event_group,index);if(inserted)continue;
        const auto& old=input.rows[at->second];
        const bool better=row.route_distance>=0 && (old.route_distance<0 || row.route_distance<old.route_distance);
        const bool fallback=row.route_distance<0 && old.route_distance<0 && input.sort && point_distance(row.position,input.player)<point_distance(old.position,input.player);
        if(better||fallback)at->second=index;
    }
    if (input.exits) for (size_t index : result.order) {
        check_cancelled(input);
        const auto& row = input.rows[index];
        if (row.kind != Kind::MapExit || row.aerial_transition || row.destination <= 0) continue;
        // World-unreachable rows are pruned below and cannot represent a group.
        if (input.reachable_only && input.world && row.route_distance < 0) continue;
        const auto found = exits.find(row.destination);
        if (found == exits.end()) { exits.emplace(row.destination,index); continue; }
        const auto& old = input.rows[found->second];
        // Pending continuation wins; otherwise preserve the player's physical
        // selection before considering distance. Both candidates already passed
        // availability, story and world-reachability guards.
        const auto priority = [&](const FilterRow& candidate) {
            if (continued_row(candidate, input)) return 3;
            if (candidate.selected) return 2;
            // Recreated static exits require the copied map and cell; the
            // legacy position-only fallback remains for ordinary restoration.
            if (input.selected_map > 0 && (candidate.remote_map || candidate.cell_valid) &&
                saved_selection_row(candidate, input)) return 1;
            return 0;
        };
        const int row_priority = priority(row), old_priority = priority(old);
        if (row_priority != old_priority) {
            if (row_priority > old_priority) found->second = index;
            continue;
        }
        if (row_priority > 0) continue;
        // Grouping is independent from list sorting. Prefer the shortest usable
        // route; if every indoor exit is blocked, retain one for its explanation.
        const bool better = row.route_distance >= 0 &&
            (old.route_distance < 0 || row.route_distance < old.route_distance);
        const bool fallback = row.route_distance < 0 && old.route_distance < 0 && input.sort &&
            point_distance(row.position,input.player) < point_distance(old.position,input.player);
        if (better || fallback) found->second = index;
    }
    result.order.erase(std::remove_if(result.order.begin(), result.order.end(), [&](size_t index) {
        check_cancelled(input);
        const auto& row = input.rows[index];
        if (input.reachable_only && input.world && row.kind == Kind::MapExit && row.route_distance < 0) return true;
        // Group only explicitly enabled native identities, never spoken text.
        if (input.pads && !row.pad.empty() && !pads.insert(row.pad).second) return true;
        if (input.exits && row.kind == Kind::MapExit && !row.aerial_transition && row.destination > 0 && exits[row.destination] != index) return true;
        if (input.events && !row.event_group.empty() && events[row.event_group] != index) {
            // A remembered explicit objective is never rebound to the group's
            // newly nearest sibling during movement or a floor continuation.
            if(!continued_row(row, input))return true;
        }
        return input.layer && row.kind == Kind::Event && row.event_type == L"ToLayer";
    }), result.order.end());
    restore_selection(result, input);
    return result;
}
Filtered build(FilterInput& input) {
    try { return build_impl(input); }
    catch (const Cancelled&) { return {}; }
}
static Categories categories_impl(FilterInput& input) {
    check_cancelled(input);
    for(auto& row:input.rows)row.story_gated=false;
    save_points::consolidate(input);
    if (input.state) for (auto& row:input.rows) {
        check_cancelled(input);
        if (row.remote_map || row.entity_id<0 || (row.kind!=Kind::NPC&&row.kind!=Kind::Event)) continue;
        const auto* sm=baked::submap_by_map_id(input.current_map);if(!sm)continue;
        if (!std::isfinite(row.position.x)||!std::isfinite(row.position.y)||std::abs(row.position.x)>1.e6||std::abs(row.position.y)>1.e6)continue;
        const int x=static_cast<int>(std::floor(sm->width*.5+row.position.x/16)),
                  y=static_cast<int>(std::floor(sm->height*.5-row.position.y/16));
        bool matched=false,all=true;
        for(const auto& object:baked::grids(*sm).objects) {
            if (object.entity_id()!=row.entity_id || baked::object_identifier(object)!=row.raw_name ||
                (row.kind!=Kind::NPC&&!object.contains(x,y))) continue;
            matched=true;all &= input.state->consumed(*sm,object);
        }
        if(matched&&all)row.available=false;
    }
    if(!prepare_navigation_labels(input))throw Cancelled{};
    if(input.reachable_only && input.story_gates && input.navigation) {
        std::vector<baked_nav::StoryTarget> targets;
        std::vector<size_t> indices;
        for(size_t i=0;i<input.rows.size();++i) {
            check_cancelled(input);
            const auto& row=input.rows[i];if(!row.available)continue;
            const int map=row.remote_map?row.remote_map:input.current_map;
            const auto* sm=baked::submap_by_map_id(map);if(!sm)continue;
            baked_route::Cell cell{sm->world?1:row.cell_layer,row.cell_x,row.cell_y};
            if(!row.remote_map) {
                if(!std::isfinite(row.position.x)||!std::isfinite(row.position.y)||std::abs(row.position.x)>1.e6||std::abs(row.position.y)>1.e6)continue;
                cell.x=static_cast<int>(std::floor(sm->width*.5+row.position.x/16));
                cell.y=static_cast<int>(std::floor(sm->height*.5-row.position.y/16));
            }
            const TargetKind kind=navigation_target_kind(row.kind,row.category);
            baked_nav::MapGoal goal;goal.map_id=map;goal.cell=cell;
            goal.entity_id=row.entity_id;goal.raw_name=row.raw_name;
            targets.push_back({std::move(goal),kind});indices.push_back(i);
        }
        const auto hidden=baked_nav::story_gate_mask(*input.navigation,targets,input.cancelled);
        check_cancelled(input);
        for(size_t i=0;i<hidden.size();++i)input.rows[indices[i]].story_gated=hidden[i];
    }
    if (input.navigation) {
        std::vector<baked_nav::ExitTarget> targets;
        std::vector<size_t> indices;
        for (size_t i=0; i<input.rows.size(); ++i) {
            check_cancelled(input);
            const auto& row = input.rows[i];
            if (row.kind != Kind::MapExit && (!input.events || row.event_group.empty())) continue;
            targets.push_back({row.position,row.remote_map,row.destination,{row.cell_layer,row.cell_x,row.cell_y},
                row.kind==Kind::MapExit?TargetKind::Transition:TargetKind::Infer});
            indices.push_back(i);
        }
        const auto distances = baked_nav::exit_distances(*input.navigation,targets,input.cancelled);
        check_cancelled(input);
        for (size_t i=0; i<distances.size(); ++i) input.rows[indices[i]].route_distance = distances[i];
    }
    Categories result;
    if(input.navigation) {
        std::optional<baked_nav::MapGoal> continued,selected;
        if(input.continued_goal&&input.continued_aerial) {
            baked_nav::MapGoal goal;goal.map_id=input.continued_map;goal.cell={1,input.continued_x,input.continued_y};
            goal.entity_id=input.continued_entity_id;goal.raw_name=input.continued_raw;continued=std::move(goal);
        }
        if(input.selected_has&&input.selected_map>0&&(input.selected_category==Category::MapExits||input.selected_category==Category::Events)) {
            baked_nav::MapGoal goal;goal.map_id=input.selected_map;goal.cell={1,input.selected_x,input.selected_y};
            goal.entity_id=input.selected_entity_id;goal.raw_name=input.selected_raw;selected=std::move(goal);
        }
        result.surface_target=baked_nav::nautilus_surface_target(*input.navigation,input.cancelled,input.reachable_only);
        result.air_target=baked_nav::air_transition_target(*input.navigation,continued,selected,input.cancelled,false,input.reachable_only);
        // Independently publish the active Saronia story event in Events. The
        // ordinary castle transition keeps its own native ground entrance.
        result.story_target=baked_nav::air_transition_target(*input.navigation,continued,selected,input.cancelled,true,input.reachable_only);
        // Its reviewed destination name is localized by the helper. Discovery
        // must not depend on an ordinary Castle exit surviving another filter.
        check_cancelled(input);
        if(result.surface_target) {
            const auto& target=*result.surface_target;FilterRow row;row.index=input.rows.size();row.kind=Kind::MapExit;
            row.category=Category::MapExits;row.position=row.numbering_position=target.position;
            row.name=target.name;row.entity_id=target.goal.entity_id;row.raw_name=target.goal.raw_name;
            row.cell_valid=true;row.cell_x=target.goal.cell.x;row.cell_y=target.goal.cell.y;row.cell_layer=target.goal.cell.layer;
            row.route_distance=target.distance;row.destination=target.destination_map;input.rows.push_back(std::move(row));
        }
        if(result.air_target) {
            const auto& target=*result.air_target;FilterRow row;row.index=input.rows.size();row.kind=Kind::MapExit;
            row.category=Category::MapExits;row.aerial_transition=true;row.position=row.numbering_position=target.position;
            row.name=target.name;row.entity_id=target.goal.entity_id;row.raw_name=target.goal.raw_name;
            row.cell_valid=true;row.cell_x=target.goal.cell.x;row.cell_y=target.goal.cell.y;row.cell_layer=target.goal.cell.layer;
            row.route_distance=target.distance;row.destination=target.destination_map;input.rows.push_back(std::move(row));
        }
        if(result.story_target) {
            const auto& target=*result.story_target;FilterRow row;row.index=input.rows.size();row.kind=Kind::Event;
            row.category=Category::Events;row.aerial_transition=true;row.position=row.numbering_position=target.position;
            row.name=target.name;row.entity_id=target.goal.entity_id;row.raw_name=target.goal.raw_name;row.event_type=L"Event";
            row.cell_valid=true;row.cell_x=target.goal.cell.x;row.cell_y=target.goal.cell.y;row.cell_layer=target.goal.cell.layer;
            row.route_distance=target.distance;row.destination=target.destination_map;input.rows.push_back(std::move(row));
        }
    }
    input.category = Category::All;
    auto& all = result.views[0];
    all = build_impl(input);
    for (size_t i = 0; i < all.order.size(); ++i) {
        check_cancelled(input);
        const auto index = all.order[i];
        auto& view = result.views[static_cast<size_t>(input.rows[index].category)];
        if (&view == &all) continue;
        view.order.push_back(index);
    }
    for (size_t category = 1; category < result.views.size(); ++category)
        restore_selection(result.views[category], input);
    const bool exits=input.exits,events=input.events,pads=input.pads;
    input.exits=input.events=input.pads=false;
    result.audio_order=build_impl(input).order;
    input.exits=exits;input.events=events;input.pads=pads;
    return result;
}
Categories build_categories(FilterInput& input) {
    try { return categories_impl(input); }
    catch (const Cancelled&) { return {}; }
}
}
