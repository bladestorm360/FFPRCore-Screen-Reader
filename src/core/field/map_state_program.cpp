// Read-only projection of the navigation subset of a native map initializer.
#include "map_state.h"
#include <vector>
#include <climits>

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
                    // An unknown local may only control animation. Accept the selection
                    // only when every possible branch ends with the same entity group.
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
            default: return {false, {}};
            }
        }
        if (agreed && (agreed->groups != state.groups || agreed->entities != state.entities)) return {false, {}};
        agreed = Selection{true, state.group, std::move(state.groups),std::move(state.entities)};
    }
    return agreed.value_or(Selection{});
}

const Selection& Resolver::selection(const baked::Submap& sm)
{
    const int id = baked::submap_index(&sm);
    if (auto it = cache_.find(id); it != cache_.end()) return it->second;
    const auto* record = records_.get(baked::name_of(sm));
    auto selected = record ? project(*record, flags_) : Selection{};
    return cache_.emplace(id,std::move(selected)).first->second;
}
bool Resolver::active(const baked::Submap& sm, const baked::Object& object, bool obstacle)
{
    const auto& selected = selection(sm);
    // PointIn/ToLayer and other graph metadata have their own ID domains. A
    // puppet changes a physical actor (and its derived Warp), never those IDs.
    const bool puppet = object.type==baked::ObjType::Entity || object.type==baked::ObjType::AnimEntity ||
        object.type==baked::ObjType::NPC || object.type==baked::ObjType::ShopNPC || object.type==baked::ObjType::Warp ||
        object.type==baked::ObjType::Event || object.type==baked::ObjType::RandomEvent || object.type==baked::ObjType::SwitchEvent;
    if(selected.known && puppet) {
        const auto entity=selected.entities.find(object.entity_id());
        if(entity!=selected.entities.end()&&entity->second.visible==std::optional<bool>(false))return false;
    }
    const auto group = baked::str(object.group);
    if (group.empty() || group == L"entity_default" || group == L"default") return true;
    return selected.known ? selected.groups.count(group) != 0 : obstacle;
}
baked::Object Resolver::effective(const baked::Submap& sm,const baked::Object& object) {
    auto result=object;
    if(object.type!=baked::ObjType::Entity && object.type!=baked::ObjType::AnimEntity &&
       object.type!=baked::ObjType::NPC && object.type!=baked::ObjType::ShopNPC && object.type!=baked::ObjType::Warp &&
       object.type!=baked::ObjType::Event && object.type!=baked::ObjType::RandomEvent && object.type!=baked::ObjType::SwitchEvent)return result;
    const auto& selected=selection(sm);
    if(!selected.known)return result;
    const auto at=selected.entities.find(object.entity_id());
    if(at==selected.entities.end())return result;
    // The bake's supported dimensions fit signed tile fields. Keep malformed
    // projection coordinates from wrapping into an unrelated route cell.
    if(at->second.x&&*at->second.x>=SHRT_MIN&&*at->second.x<=SHRT_MAX)result.tx=static_cast<int16_t>(*at->second.x);
    if(at->second.y&&*at->second.y>=SHRT_MIN&&*at->second.y<=SHRT_MAX)result.ty=static_cast<int16_t>(*at->second.y);
    return result;
}
bool Resolver::known(const baked::Submap& sm) { return selection(sm).known; }
bool Resolver::consumed(const baked::Submap& sm, const baked::Object& object) {
    if (const auto it=consumed_.find(&object);it!=consumed_.end()) return it->second;
    const auto* record=records_.get(baked::name_of(sm));
    const auto* guards=record?record->get(L"events"):nullptr;
    bool found=false,all=true;
    if (guards) for (const auto& guard:guards->arr) {
        const auto* id=guard.get(L"id"),*group=guard.get(L"group"),*type=guard.get(L"type"),*cell=guard.get(L"cell"),*raw=guard.get(L"raw"),*code=guard.get(L"code");
        if (!id||!group||!type||!cell||cell->arr.size()!=3||!raw||!code||code->arr.empty()) continue;
        const int native_type=number(*type);
        const bool same_type=static_cast<int>(object.type)==native_type ||
            (object.type==baked::ObjType::Warp && (object.d==1 ? native_type==2||native_type==28 : native_type==5));
        auto name=baked::object_identifier(object);
        if (!same_type||number(*id)!=object.entity_id()||group->s!=baked::str(object.group)||raw->s!=name||
            cell->arr[0].n!=object.layer||cell->arr[1].n!=object.tx||cell->arr[2].n!=object.ty) continue;
        found=true;
        const auto availability=project(guard,flags_);
        all &= availability.known&&availability.group.empty();
    }
    return consumed_[&object]=found&&all;
}
uint64_t Resolver::signature() const {
    uint64_t hash = 1469598103934665603ull;
    const auto bank = [&](const auto& values) {
        for (const auto& value : values) {
            hash ^= value ? (static_cast<uint64_t>(static_cast<uint32_t>(*value)) << 1) | 1 : 0;
            hash *= 1099511628211ull;
        }
    };
    bank(flags_.scenario); bank(flags_.treasure); bank(flags_.global); bank(flags_.area); bank(flags_.local);
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
void add_obstacles(Resolver& resolver, const baked::Submap& sm, baked_route::Request& request, bool gates, bool geometry_only)
{
    request.live_hazards = true;
    request.live_traps = true;
    for (const auto& original : baked::grids(sm).objects) {
        if(geometry_only && original.type!=baked::ObjType::Entity && original.type!=baked::ObjType::AnimEntity &&
           original.type!=baked::ObjType::CollisionEntity)continue;
        if (!resolver.active(sm, original, true)) continue;
        const auto o=resolver.effective(sm,original);
        const bool npc = o.type == baked::ObjType::NPC || o.type == baked::ObjType::ShopNPC;
        const bool solid = o.type == baked::ObjType::TreasureBox ||
            (npc && (o.d & 1)) ||
            (gates && ((o.type == baked::ObjType::Entity || o.type == baked::ObjType::AnimEntity) && (o.d & 1)));
        const bool collider = gates && o.type == baked::ObjType::CollisionEntity;
        const bool used=resolver.consumed(sm,original);
        const bool trigger = baked_route::step_event(o)&&!used;
        if (!solid && !collider && !trigger) continue;
        for (int y = o.ty; y < o.ty + o.th; ++y) for (int x = o.tx; x < o.tx + o.tw; ++x) {
            baked_route::Cell cell{sm.world || (npc && (o.d & 2)) ? -1 : o.layer, x, y};
            if (solid) request.blocked.push_back(cell);
            if (collider) {
                if (o.a == 6 || o.a == 7) request.directional.push_back({cell, o.a == 6 ? 1 : 2});
                else request.blocked.push_back(cell);
            }
            if (trigger) request.hazards.push_back(cell);
            if (!used&&baked::is_trap(o)) request.traps.push_back(cell);
        }
    }
}
}
