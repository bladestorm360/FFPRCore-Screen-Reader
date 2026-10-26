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
void restore_selection(Filtered& result, const FilterInput& input) {
    if (input.continued_goal) for (size_t i = 0; i < result.order.size(); ++i) {
        const auto& row = input.rows[result.order[i]];
        if(!input.continued_raw.empty()&&row.raw_name!=input.continued_raw)continue;
        const bool map_matches = row.remote_map ? row.remote_map == input.continued_map
            : input.current_map == input.continued_map;
        if (map_matches && (row.remote_map || row.cell_valid)
            && (input.continued_entity_id < 0 || row.entity_id == input.continued_entity_id)
            && row.cell_x == input.continued_x && row.cell_y == input.continued_y) {
            result.continued = static_cast<int>(i); break;
        }
    }
    for (size_t i = 0; i < result.order.size(); ++i)
        if (input.rows[result.order[i]].selected) { result.restored = static_cast<int>(i); return; }
    if (input.selected_has) {
        for (size_t i = 0; i < result.order.size(); ++i) {
            const auto& row = input.rows[result.order[i]];
            // Vehicle FieldEntities can share a default entity ID/name. The
            // Transportation model ID owns identity across native recreation,
            // distance re-sorts and battle. Never substitute the nearest ship.
            if (input.selected_transportation_id > 0) {
                if (row.kind != Kind::Vehicle || row.transportation_id != input.selected_transportation_id) continue;
                const int map = row.remote_map ? row.remote_map : input.current_map;
                if (map == input.selected_map) { result.restored = static_cast<int>(i); return; }
                continue;
            }
            if (input.selected_entity_id >= 0 && row.entity_id != input.selected_entity_id) continue;
            if (!input.selected_raw.empty()&&row.raw_name!=input.selected_raw)continue;
            if (input.selected_map > 0) {
                const int map = row.remote_map ? row.remote_map : input.current_map;
                if (map != input.selected_map || row.category != input.selected_category) continue;
                const bool moving_identity = input.selected_entity_id >= 0 && (row.kind == Kind::NPC || row.kind == Kind::Vehicle);
                const bool at_anchor = row.remote_map || row.cell_valid
                    ? row.cell_x == input.selected_x && row.cell_y == input.selected_y
                    : point_distance(row.position, input.selected_position) < 0.5f;
                if (moving_identity || at_anchor) {
                    result.restored = static_cast<int>(i); return;
                }
                continue;
            }
            const bool match = input.selected_remote > 0
                ? row.remote_map == input.selected_remote && row.cell_x == input.selected_x && row.cell_y == input.selected_y
                : !row.remote_map && row.category == input.selected_category && point_distance(row.position, input.selected_position) < 0.5f;
            if (match) { result.restored = static_cast<int>(i); return; }
        }
        // A translated name is not identity (especially type-only chests).
        // With no native ID, require the exact map/category/anchor above.
    }
}
}
static Filtered build_impl(FilterInput& input) {
    check_cancelled(input);
    Filtered result;
    std::vector<npc_names::Entry> names;
    for (auto& row : input.rows) {
        check_cancelled(input);
        if (row.available && row.kind == Kind::NPC)
            names.push_back({&row.name, row.label, row.remote_map, 0,
                row.remote_map ? static_cast<float>(row.cell_x) : row.numbering_position.x,
                row.remote_map ? static_cast<float>(row.cell_y) : -row.numbering_position.y});
    }
    npc_names::renumber(names); // before category/distance/reachability inclusion
    check_cancelled(input);
    for (const auto& row : input.rows) {
        check_cancelled(input);
        if (row.kind == Kind::MapExit && !row.remote_map && (!input.world || row.route_distance>=0)) result.exits.push_back(row.numbering_position);
        result.names.push_back(row.name);
        if (row.available && !row.story_gated && (input.category == Category::All || input.category == row.category)) result.order.push_back(row.index);
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
        if (row.kind != Kind::MapExit || row.destination <= 0) continue;
        const auto found = exits.find(row.destination);
        if (found == exits.end()) { exits.emplace(row.destination,index); continue; }
        const auto& old = input.rows[found->second];
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
        if (input.world && row.kind == Kind::MapExit && row.route_distance < 0) return true;
        // Group only explicitly enabled native identities, never spoken text.
        if (input.pads && !row.pad.empty() && !pads.insert(row.pad).second) return true;
        if (input.exits && row.kind == Kind::MapExit && row.destination > 0 && exits[row.destination] != index) return true;
        if (input.events && !row.event_group.empty() && events[row.event_group] != index) {
            // A remembered explicit objective is never rebound to the group's
            // newly nearest sibling during movement or a floor continuation.
            const bool continued=input.continued_goal && (row.remote_map?row.remote_map:input.current_map)==input.continued_map &&
                (row.remote_map || row.cell_valid) && row.cell_x==input.continued_x && row.cell_y==input.continued_y &&
                (input.continued_entity_id<0 || row.entity_id==input.continued_entity_id) &&
                (input.continued_raw.empty() || row.raw_name==input.continued_raw);
            if(!continued)return true;
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
    if(input.story_gates && input.navigation) {
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
    input.category = Category::All;
    Categories result;
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
