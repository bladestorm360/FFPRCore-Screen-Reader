// Read-only projection of the navigation subset of a native map initializer.
#include "map_state.h"
#include <vector>
#include <climits>
#include <algorithm>

namespace field::map_state {
namespace {
int number(const json::Value& v) { return static_cast<int>(v.n); }
using Key = std::pair<int, int>;
std::optional<int> value(int category, int index, const Flags& flags, const std::map<Key, std::optional<int>>& shadow)
{
    if (category == -1) return index;
    if (auto it = shadow.find({category, index}); it != shadow.end()) return it->second;
    if (index < 0) return {};
    if (category >= 0 && category < 8 && index < 256) {
        const size_t bit = static_cast<size_t>((category % 4) * 256 + index);
        return category < 4 ? flags.scenario[bit] : flags.treasure[bit];
    }
    if (category == 14 && index == 401) return flags.party_mini;
    if (category == 8 && index < 16) return flags.global[static_cast<size_t>(index)];
    if (category == 9 && index < 8) return flags.area[static_cast<size_t>(index)];
    if (category == 10 && index < 8) return flags.local[static_cast<size_t>(index)];
    // Locals from another floor are unknown unless that initializer assigns them.
    return {};
}
std::optional<bool> compare(int op, int a, int b)
{
    switch (op) {
    case 0: return a == b; case 1: return a != b; case 2: return a < b;
    case 3: return a > b; case 4: return a <= b; case 5: return a >= b;
    default: return {};
    }
}
}

Selection project(const json::Value& record, const Flags& input)
{
    // Selector-only metadata must not turn a missing initializer into a known
    // empty group set. Preserve the prior conservative unknown projection.
    if(const auto* unknown=record.get(L"initializer_unknown");unknown&&unknown->b)return {false,{}};
    // Area locals survive floor switches within their native area. Map locals
    // belong only to that exact loaded map. Never borrow another floor's state.
    Flags flags=input;
    const auto* area=record.get(L"area_id");
    if (!area || input.current_area_id<=0 || number(*area)!=input.current_area_id) flags.area.fill({});
    bool same_map=false;
    if (const auto* maps=record.get(L"map_ids"))
        for (const auto& map:maps->arr) same_map |= number(map)==input.current_map_id && input.current_map_id>0;
    if (!same_map) flags.local.fill({});
    const auto* code = record.get(L"code");
    if (!code || code->arr.empty()) return {};
    const auto* entry = record.get(L"entry");
    struct Machine {
        int pc; std::wstring group;
        std::vector<int> stack;
        std::map<Key, std::optional<int>> shadow;
        std::set<std::wstring> groups;
        std::map<int,std::pair<int,EntityState>> puppets;
        std::map<int,EntityState> entities;
        std::map<int,int> routes;
    };
    std::vector<Machine> branches{{entry ? number(*entry) : 0, {}, {}, {}}};
    std::optional<Selection> agreed;
    int budget = 8192;
    size_t explored = 0;
    while (!branches.empty()) {
        if (++explored > 64) return {false, {}};
        Machine state = std::move(branches.back()); branches.pop_back();
        bool finished = false;
        while (!finished) {
            if (--budget <= 0 || state.pc < 0 || static_cast<size_t>(state.pc) >= code->arr.size()) return {false, {}};
            const auto& row = code->arr[static_cast<size_t>(state.pc++)].arr;
            if (row.empty()) return {false, {}};
            auto i = [&](size_t n) { return n < row.size() ? number(row[n]) : -2; };
            switch (i(0)) {
            case 0: break;
            case 1:
                if (row.size() < 2 || !row[1].is_string()) return {false, {}};
                state.group = row[1].s;
                if (!state.group.empty()) state.groups.insert(state.group);
                break;
            case 2: {
                if (i(3) < 0 || i(3) > 5) return {false, {}};
                const auto lhs = value(i(1), i(2), flags, state.shadow), rhs = value(i(4), i(5), flags, state.shadow);
                if (!lhs || !rhs) {
                    // Explore both paths. Completed branches retain their
                    // intersection, never a guessed story-group selection.
                    branches.push_back(state);
                    state.pc = i(6);
                } else {
                    const auto test = compare(i(3), *lhs, *rhs);
                    if (!test) return {false, {}};
                    // Native reaction jumps to iValues[2] when comparison result == 1.
                    if (*test) state.pc = i(6);
                }
                break;
            }
            case 3: state.pc = i(1); break;
            case 4:
                if (state.stack.size() >= 16) return {false, {}};
                state.stack.push_back(state.pc); state.pc = i(1); break;
            case 5:
                if (state.stack.empty()) finished = true;
                else { state.pc = state.stack.back(); state.stack.pop_back(); }
                break;
            case 6: finished = true; break;
            case 7: state.shadow[{i(1), i(2)}] = i(3); break;
            case 8: state.shadow[{i(1), i(2)}] = value(i(3), i(4), flags, state.shadow); break;
            case 10: {
                if(row.size()!=6||i(1)<0||i(1)>=32)return {false,{}};
                EntityState effect;
                if(i(3)==0||i(3)==1)effect.visible=i(3)!=0;
                if(i(4)!=INT_MIN)effect.x=i(4);
                if(i(5)!=INT_MIN)effect.y=i(5);
                state.puppets[i(1)]={i(2),effect};
                break;
            }
            case 11:
                for(const auto& [slot,entry]:state.puppets) {
                    if(entry.first<=0)continue;
                    const auto& effect=entry.second;
                    if(!effect.visible.has_value()&&!effect.x&&!effect.y)continue;
                    if(row.size()>1&&i(1)!=0)return {false,{}}; // asynchronous final state is not ready evidence
                    auto& into=state.entities[entry.first];
                    if(effect.visible.has_value())into.visible=effect.visible;
                    if(effect.x)into.x=effect.x;
                    if(effect.y)into.y=effect.y;
                }
                if(state.entities.size()>4096)return {false,{}};
                state.puppets.clear(); // completed native cores sleep until another SetPuppet
                break;
            case 12:
                if(row.size()!=3||i(1)<=0||state.routes.size()>=64)return {false,{}};
                if(i(2)>0)state.routes[i(1)]=i(2);else state.routes.erase(i(1));
                break;
            default: return {false, {}};
            }
        }
        if(!agreed) {
            agreed=Selection{true,state.group,std::move(state.groups),std::move(state.entities)};
            agreed->routes=std::move(state.routes);
        } else {
            agreed->known &= agreed->groups==state.groups && agreed->entities==state.entities;
            if(agreed->group!=state.group)agreed->group.clear();
            for(auto it=agreed->groups.begin();it!=agreed->groups.end();)
                if(!state.groups.count(*it))it=agreed->groups.erase(it);else ++it;
            for(const auto& [id,effect]:state.entities) {
                const auto at=agreed->entities.find(id);
                if(at==agreed->entities.end() || !(at->second==effect))agreed->uncertain_entities.insert(id);
            }
            for(auto it=agreed->entities.begin();it!=agreed->entities.end();) {
                const auto at=state.entities.find(it->first);
                if(at==state.entities.end() || !(at->second==it->second)) {
                    agreed->uncertain_entities.insert(it->first);it=agreed->entities.erase(it);
                } else ++it;
            }
            for(auto it=agreed->routes.begin();it!=agreed->routes.end();) {
                const auto at=state.routes.find(it->first);
                if(at==state.routes.end()||at->second!=it->second)it=agreed->routes.erase(it);else ++it;
            }
        }
    }
    return agreed.value_or(Selection{});
}

const Selection& Resolver::selection(const baked::Submap& sm)
{
    const int id = baked::submap_index(&sm);
    if (auto it = cache_.find(id); it != cache_.end()) return it->second;
    const auto* record = records_.get(baked::name_of(sm));
    auto selected = record ? project(*record, flags_) : Selection{};
    // SetEntities enables groups additively. A later callback proves presence
    // of those groups, not absence of earlier ones, nor actor placement/show
    // changes. Preserve the initializer's certainty and effective-actor facts.
    if(id==observed_index_)selected.groups.insert(observed_groups_.begin(),observed_groups_.end());
    return cache_.emplace(id,std::move(selected)).first->second;
}
bool Resolver::active(const baked::Submap& sm, const baked::Object& object, bool obstacle)
{
    const auto& selected = selection(sm);
    // PointIn/ToLayer and other graph metadata have their own ID domains. A
    // puppet changes a physical actor (and its derived Warp), never those IDs.
    const bool puppet = object.type==baked::ObjType::Entity || object.type==baked::ObjType::AnimEntity || object.type==baked::ObjType::EffectEntity ||
        object.type==baked::ObjType::NPC || object.type==baked::ObjType::ShopNPC || object.type==baked::ObjType::Warp ||
        object.type==baked::ObjType::Event || object.type==baked::ObjType::RandomEvent || object.type==baked::ObjType::SwitchEvent;
    if(puppet) {
        if(!obstacle && selected.uncertain_entities.count(object.entity_id()))return false;
        const auto entity=selected.entities.find(object.entity_id());
        if(entity!=selected.entities.end()&&entity->second.visible==std::optional<bool>(false))return false;
    }
    const auto group = baked::str(object.group);
    if (group.empty() || group == L"entity_default" || group == L"default") return true;
    return selected.groups.count(group)!=0 || (!selected.known && obstacle);
}
baked::Object Resolver::effective(const baked::Submap& sm,const baked::Object& object) {
    auto result=object;
    if(object.type!=baked::ObjType::Entity && object.type!=baked::ObjType::AnimEntity && object.type!=baked::ObjType::EffectEntity &&
       object.type!=baked::ObjType::NPC && object.type!=baked::ObjType::ShopNPC && object.type!=baked::ObjType::Warp &&
       object.type!=baked::ObjType::Event && object.type!=baked::ObjType::RandomEvent && object.type!=baked::ObjType::SwitchEvent)return result;
    const auto& selected=selection(sm);
    const auto at=selected.entities.find(object.entity_id());
    if(at==selected.entities.end())return result;
    // The bake's supported dimensions fit signed tile fields. Keep malformed
    // projection coordinates from wrapping into an unrelated route cell.
    if(at->second.x&&*at->second.x>=SHRT_MIN&&*at->second.x<=SHRT_MAX)result.tx=static_cast<int16_t>(*at->second.x);
    if(at->second.y&&*at->second.y>=SHRT_MIN&&*at->second.y<=SHRT_MAX)result.ty=static_cast<int16_t>(*at->second.y);
    return result;
}
bool Resolver::known(const baked::Submap& sm) { return selection(sm).known; }
std::vector<baked_route::Cell> Resolver::route_positions(const baked::Submap& sm,const baked::Object& object) {
    std::vector<baked_route::Cell> result;
    if(object.type!=baked::ObjType::NPC&&object.type!=baked::ObjType::ShopNPC)return result;
    const auto& selected=selection(sm);
    const auto assignment=selected.routes.find(object.entity_id());
    // Route0 is a separately source-pinned native Area2/Route3 default, not a
    // guessed assignment. Unknown initializers or stationary actors cannot use
    // it; a reviewed assigned controller always owns its positive route ID.
    const int route_id=assignment==selected.routes.end()?0:assignment->second;
    if((!route_id&&(!selected.known||!(object.d&16)))||!active(sm,object))return result;
    const auto* record=records_.get(baked::name_of(sm));
    const auto* patrols=record?record->get(L"patrols"):nullptr;
    if(!patrols||patrols->arr.size()>64)return result;
    for(const auto& patrol:patrols->arr) {
        const auto *id=patrol.get(L"id"),*route=patrol.get(L"route"),*group=patrol.get(L"group"),
            *raw=patrol.get(L"raw"),*home=patrol.get(L"home"),*cells=patrol.get(L"cells");
        if(!id||!route||!group||!raw||!home||!cells||home->arr.size()!=3||cells->arr.empty()||cells->arr.size()>32)continue;
        if(number(*id)!=object.entity_id()||number(*route)!=route_id||group->s!=baked::str(object.group)||
            raw->s!=baked::object_identifier(object)||number(home->arr[0])!=object.layer||
            number(home->arr[1])!=object.tx||number(home->arr[2])!=object.ty)continue;
        if(!result.empty())return {}; // ambiguous source fingerprint grants no path
        for(const auto& row:cells->arr) {
            if(row.arr.size()!=3)return {};
            const baked_route::Cell cell{number(row.arr[0]),number(row.arr[1]),number(row.arr[2])};
            if(cell.layer<0||cell.layer>2||cell.x<0||cell.x>=sm.width||cell.y<0||cell.y>=sm.height)return {};
            result.push_back(cell);
        }
    }
    return result;
}
bool Resolver::consumed(const baked::Submap& sm, const baked::Object& object) {
    if (const auto it=consumed_.find(&object);it!=consumed_.end()) return it->second;
    const auto* record=records_.get(baked::name_of(sm));
    const auto* guards=record?record->get(L"events"):nullptr;
    bool found=false,all=true,destination_guard=false;
    if (guards) for (const auto& guard:guards->arr) {
        // Most world-map guards belong to another actor. Do not decode all
        // guard fields or allocate its name before this exact identity check.
        const auto* id=guard.get(L"id");
        if(!id||number(*id)!=object.entity_id())continue;
        const auto *group=guard.get(L"group"),*type=guard.get(L"type"),*cell=guard.get(L"cell"),*raw=guard.get(L"raw"),*code=guard.get(L"code");
        if (!id||!group||!type||!cell||cell->arr.size()!=3||!raw||!code||code->arr.empty()) continue;
        const int native_type=number(*type);
        const bool same_type=static_cast<int>(object.type)==native_type ||
            (object.type==baked::ObjType::Warp && ((object.d&255)==1 ? native_type==2||native_type==28 : native_type==5));
        auto name=baked::object_identifier(object);
        if (!same_type||number(*id)!=object.entity_id()||group->s!=baked::str(object.group)||raw->s!=name||
            cell->arr[0].n!=object.layer||cell->arr[1].n!=object.tx||cell->arr[2].n!=object.ty) continue;
        if(const auto* target=guard.get(L"destination")) {
            // One native Event can transfer to different real floors. This
            // bounded prefix grants only the matching derived Warp; it never
            // hides the physical event or overwrites the initializer state.
            if(object.type!=baked::ObjType::Warp||target->arr.size()!=2||number(target->arr[0])!=object.a||number(target->arr[1])!=object.b)continue;
            if(!destination_guard){found=false;all=true;destination_guard=true;}
        } else if(destination_guard)continue;
        found=true;
        const auto availability=project(guard,flags_);
        all &= availability.known&&availability.group.empty();
    }
    return consumed_[&object]=found&&all;
}
uint64_t Resolver::transportation_mask(const baked::Submap& sm,const baked::Object& object) {
    if(const auto cached=transportation_.find(&object);cached!=transportation_.end())return cached->second;
    const baked::Object* source=&object;
    if(object.type==baked::ObjType::Warp) {
        // Derived arcs retain their real native actor's exact type. Ambiguous
        // coincident sources cannot grant a vehicle-specific opening.
        source=nullptr;
        bool ambiguous=false;
        const auto consider=[&](const baked::Object& native) {
            if(native.type!=baked::ObjType::Event&&native.type!=baked::ObjType::RandomEvent&&native.type!=baked::ObjType::Entity)return;
            if(native.entity_id()!=object.entity_id()||native.layer!=object.layer||native.tx!=object.tx||native.ty!=object.ty||
                baked::object_identifier(native)!=baked::object_identifier(object)||baked::str(native.group)!=baked::str(object.group))return;
            if(source){ambiguous=true;return;}
            source=&native;
        };
        const auto& grid=baked::grids(sm);
        if(grid.entity_index_prepared) {
            const auto found=grid.objects_by_entity.find(object.entity_id());
            if(found!=grid.objects_by_entity.end())for(size_t index:found->second)consider(grid.objects[index]);
        } else for(const auto& native:grid.objects)consider(native);
        if(!source||ambiguous)return transportation_[&object]=~uint64_t(0);
        return transportation_[&object]=transportation_mask(sm,*source);
    }
    const auto* row=records_.get(baked::name_of(sm));row=row?row->get(L"transport"):nullptr;
    const auto key=std::to_wstring(static_cast<int>(source->type))+L":"+std::to_wstring(source->entity_id())+L":"+
        std::to_wstring(source->layer)+L":"+std::to_wstring(source->tx)+L":"+std::to_wstring(source->ty);
    row=row?row->get(key):nullptr;row=row?row->get(baked::str(source->group)):nullptr;
    row=row?row->get(baked::object_identifier(*source)):nullptr;
    uint64_t result=~uint64_t(0);
    if(row) {
        result=0;
        if(row->type!=json::Value::Type::Array||row->arr.size()>64)result=~uint64_t(0);
        else for(const auto& id:row->arr) {
            const int value=number(id);if(value<=0||value>=64){result=~uint64_t(0);break;}
            result|=uint64_t(1)<<value;
        }
    }
    return transportation_[&object]=result;
}

uint64_t Resolver::signature() const {
    uint64_t hash = 1469598103934665603ull;
    const auto bank = [&](const auto& values) {
        for (const auto& value : values) {
            hash ^= value ? (static_cast<uint64_t>(static_cast<uint32_t>(*value)) << 1) | 1 : 0;
            hash *= 1099511628211ull;
        }
    };
    bank(flags_.scenario); bank(flags_.treasure); bank(flags_.global);
    hash ^= flags_.party_mini ? (static_cast<uint64_t>(*flags_.party_mini)<<1)|1 : 0;
    hash *= 1099511628211ull; bank(flags_.area); bank(flags_.local);
    hash ^= static_cast<uint64_t>(flags_.current_map_id); hash *= 1099511628211ull;
    hash ^= static_cast<uint64_t>(flags_.current_area_id); hash *= 1099511628211ull;
    return hash;
}
std::optional<bool> Resolver::treasure_opened(const baked::Submap& sm, const baked::Object& object) const
{
    if (object.type != baked::ObjType::TreasureBox) return {};
    const auto* record = records_.get(baked::name_of(sm));
    const auto* chests = record ? record->get(L"treasures") : nullptr;
    if (!chests) return {};
    for (const auto& v : chests->arr) {
        const auto& r = v.arr;
        if (r.size() < 3 || number(r[0]) != object.entity_id() || r[1].s != baked::str(object.group)) continue;
        // Native MapUIManager.GetFlag uses flag_id/256 and flag_id%256 directly.
        const int bit = number(r[2]);
        if (bit < 0 || bit >= 1024) return {};
        const auto value = flags_.treasure[static_cast<size_t>(bit)];
        return value ? std::optional<bool>(*value != 0) : std::nullopt;
    }
    return {};
}
void add_relays(Resolver& resolver, const baked::Submap& sm, baked_route::Request& request)
{
    request.live_relays = true;
    request.relays.clear();
    if (sm.world) return;
    for (const auto& object : baked::grids(sm).objects)
        if (object.type == baked::ObjType::RelayInteraction && resolver.active(sm, object))
            request.relays.push_back(object);
}
std::shared_ptr<const baked::TransitionGrid> Resolver::transitions(const baked::Submap& sm)
{
    const int key=baked::submap_index(&sm);
    if(const auto found=transitions_.find(key);found!=transitions_.end())return found->second;
    const auto& grid=baked::grids(sm);auto mask=std::make_shared<baked::TransitionGrid>();
    for(const auto& original:grid.objects) {
        const int bits=baked::transition_bits(original);
        if(!bits||!active(sm,original,true)||consumed(sm,original)||transportation_mask(sm,original)!=~uint64_t(0))continue;
        const auto object=effective(sm,original);
        auto& layer=(*mask)[sm.world?0:object.layer];
        if(layer.empty())layer.assign(static_cast<size_t>(grid.width)*grid.height,0);
        for(int y=std::max(0,int(object.ty));y<std::min(grid.height,int(object.ty)+object.th);++y)
            for(int x=std::max(0,int(object.tx));x<std::min(grid.width,int(object.tx)+object.tw);++x)
                layer[static_cast<size_t>(y)*grid.width+x]|=static_cast<uint8_t>(bits);
    }
    transitions_[key]=mask;return mask;
}
void add_obstacles(Resolver& resolver, const baked::Submap& sm, baked_route::Request& request, bool gates, bool geometry_only, bool with_npcs)
{
    if(request.transport_rules&&request.transport_rules->submap!=&sm)request.transport_rules.reset();
    request.transport_gates=gates;
    request.transitions=resolver.transitions(sm);
    if (!geometry_only) add_relays(resolver, sm, request);
    request.live_hazards = true;
    request.live_traps = true;
    using Rules=baked_route::TransportRules;
    std::shared_ptr<Rules> prepared;
    const auto mark=[&](Rules::Kind kind,const baked_route::Cell& cell,uint64_t ids) {
        if(!ids)return;
        if(!prepared)prepared=request.transport_rules?std::make_shared<Rules>(*request.transport_rules):std::make_shared<Rules>();
        baked_route::add_transport_cell(*prepared,kind,sm,cell,ids);
    };
    for (const auto& original : baked::grids(sm).objects) {
        if(geometry_only && original.type!=baked::ObjType::Entity && original.type!=baked::ObjType::AnimEntity && original.type!=baked::ObjType::EffectEntity &&
           original.type!=baked::ObjType::CollisionEntity)continue;
        if (!resolver.active(sm, original, true)) continue;
        const auto o=resolver.effective(sm,original);
        const uint64_t ids=resolver.transportation_mask(sm,original);
        const bool restricted=ids!=~uint64_t(0);
        const bool npc = o.type == baked::ObjType::NPC || o.type == baked::ObjType::ShopNPC;
        if(npc&&!with_npcs)continue;
        const bool solid = o.type == baked::ObjType::TreasureBox ||
            (npc && (o.d & 1)) ||
            (gates && ((o.type == baked::ObjType::Entity || o.type == baked::ObjType::AnimEntity || o.type == baked::ObjType::EffectEntity) && (o.d & 1)));
        const bool collider = gates && o.type == baked::ObjType::CollisionEntity;
        const bool used=resolver.consumed(sm,original);
        const bool terminal=baked::world_exit(o)&&!used;
        const bool exit_source=baked::world_exit_at(baked::grids(sm),o.entity_id(),baked::object_identifier(o),o.tx,o.ty)!=nullptr;
        const bool trigger = baked_route::step_event(o)&&!used&&!exit_source;
        const int transition_bits=used?0:baked::transition_bits(o);
        if (!solid && !collider && !trigger&&!terminal&&!(restricted&&transition_bits)) continue;
        for (int y = o.ty; y < o.ty + o.th; ++y) for (int x = o.tx; x < o.tx + o.tw; ++x) {
            baked_route::Cell cell{sm.world || (npc && (o.d & 2)) ? -1 : o.layer, x, y};
            if(restricted&&transition_bits)mark((transition_bits&2)?Rules::Terminal:Rules::GroundTerminal,cell,ids);
            if (solid) {
                if(restricted)mark((npc||o.type==baked::ObjType::TreasureBox)?Rules::Solid:Rules::Gate,cell,ids);
                else request.blocked.push_back(cell);
            }
            if (collider) {
                if(restricted&&o.a!=0) {
                    if(!prepared)prepared=request.transport_rules?std::make_shared<Rules>(*request.transport_rules):std::make_shared<Rules>();
                    baked_route::add_transport_collision(*prepared,sm,cell,o.a,ids);
                } else if(!restricted)baked_route::add_collision(request.blocked,request.directional,cell,o.a);
            }
            if (trigger) {if(restricted)mark(Rules::Hazard,cell,ids);else request.hazards.push_back(cell);}
            if (terminal||(!used&&baked::is_trap(o))) {if(restricted)mark(Rules::Trap,cell,ids);else request.traps.push_back(cell);}
        }
    }
    if(prepared)request.transport_rules=std::move(prepared);
}
}
