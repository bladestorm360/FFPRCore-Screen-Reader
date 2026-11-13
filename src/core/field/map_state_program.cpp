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
    // Exact compiled Duncan/Sabin predicate, using an owned active-corps bit.
    if (category == 15 && index == 6 && flags.party_sabin) return *flags.party_sabin?1:0;
    if (category == 15 && index == 12 && flags.party_gau) return *flags.party_gau?1:0;
    if (category == 15 && index == 1 && flags.party_terra) return *flags.party_terra?1:0;
    if (category == 15 && index == 3 && flags.party_cyan) return *flags.party_cyan?1:0;
    if (category == 15 && index == 0) return flags.active_party_members;
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
    // A reviewed current-room program can preserve actions taken after the
    // native initializer. Remote routes still project the original reload.
    const auto* current_code = same_map ? record.get(L"current_code") : nullptr;
    const auto* code = current_code ? current_code : record.get(L"code");
    if (!code || code->arr.empty()) return {};
    const auto* entry = record.get(L"entry");
    struct Machine {
        int pc; std::wstring group;
        std::vector<int> stack;
        std::map<Key, std::optional<int>> shadow;
        std::set<std::wstring> groups;
        struct Puppet {int actor;EntityState effect;bool visibility_unknown=false;};
        std::map<int,Puppet> puppets;
        std::map<int,EntityState> entities;
        std::map<int,int> routes;
        std::set<int> uncertain_visibility;
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
                if((row.size()!=6&&row.size()!=7)||i(1)<0||i(1)>=32)return {false,{}};
                EntityState effect;
                if(i(3)==0||i(3)==1)effect.visible=i(3)!=0;
                if(i(4)!=INT_MIN)effect.x=i(4);
                if(i(5)!=INT_MIN)effect.y=i(5);
                // Existing six-column facts retain their source layer. The
                // exact Mt. Kolts initializer additionally proves its relocated
                // Vargas layer;malformed layer values never grant placement.
                if(row.size()==7){if(i(6)<0||i(6)>2)return {false,{}};effect.layer=i(6);}
                state.puppets[i(1)]={i(2),effect,i(3)==-2};
                break;
            }
            case 11:
                for(const auto& [slot,entry]:state.puppets) {
                    if(entry.actor<=0)continue;
                    const auto& effect=entry.effect;
                    if(!entry.visibility_unknown&&!effect.visible.has_value()&&!effect.x&&!effect.y&&!effect.layer)continue;
                    if(row.size()>1&&i(1)!=0)return {false,{}}; // asynchronous final state is not ready evidence
                    auto& into=state.entities[entry.actor];
                    // Native Ready has already shown this actor. An opaque
                    // later body cannot retain a previous Hide as a receipt.
                    if(entry.visibility_unknown){into.visible.reset();state.uncertain_visibility.insert(entry.actor);}
                    else if(effect.visible.has_value()){into.visible=effect.visible;state.uncertain_visibility.erase(entry.actor);}
                    if(effect.x)into.x=effect.x;
                    if(effect.y)into.y=effect.y;
                    if(effect.layer)into.layer=effect.layer;
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
            agreed->uncertain_entities=std::move(state.uncertain_visibility);
        } else {
            agreed->uncertain_entities.insert(state.uncertain_visibility.begin(),state.uncertain_visibility.end());
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
        object.type==baked::ObjType::Event || object.type==baked::ObjType::RandomEvent || object.type==baked::ObjType::SwitchEvent ||
        object.type==baked::ObjType::CollisionEntity;
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
       object.type!=baked::ObjType::Event && object.type!=baked::ObjType::RandomEvent && object.type!=baked::ObjType::SwitchEvent &&
       object.type!=baked::ObjType::CollisionEntity)return result;
    const auto& selected=selection(sm);
    const auto at=selected.entities.find(object.entity_id());
    if(at==selected.entities.end())return result;
    // The bake's supported dimensions fit signed tile fields. Keep malformed
    // projection coordinates from wrapping into an unrelated route cell.
    if(at->second.x&&*at->second.x>=SHRT_MIN&&*at->second.x<=SHRT_MAX)result.tx=static_cast<int16_t>(*at->second.x);
    if(at->second.y&&*at->second.y>=SHRT_MIN&&*at->second.y<=SHRT_MAX)result.ty=static_cast<int16_t>(*at->second.y);
    if(at->second.layer&&*at->second.layer>=0&&*at->second.layer<=2)result.layer=static_cast<uint8_t>(*at->second.layer);
    return result;
}
unsigned Resolver::collision_layers(const baked::Submap& sm,const baked::Object& object) const {
    const unsigned ordinary=object.layer<=2?1u<<object.layer:0;
    if(object.type!=baked::ObjType::Entity||!(object.d&1))return ordinary;
    const auto* record=records_.get(baked::name_of(sm));const auto* rows=record?record->get(L"collision_layers"):nullptr;
    if(!rows||rows->arr.size()>32)return ordinary;
    for(const auto& row:rows->arr){
        const auto *id=row.get(L"id"),*group=row.get(L"group"),*raw=row.get(L"raw"),*cell=row.get(L"cell"),*layers=row.get(L"layers");
        if(!id||!group||!raw||!cell||cell->arr.size()!=3||!layers||layers->arr.empty()||layers->arr.size()>3)continue;
        if(number(*id)!=object.entity_id()||group->s!=baked::str(object.group)||raw->s!=baked::object_identifier(object)||
           number(cell->arr[0])!=object.layer||number(cell->arr[1])!=object.tx||number(cell->arr[2])!=object.ty)continue;
        unsigned mask=ordinary;for(const auto& layer:layers->arr){const int n=number(layer);if(n<0||n>2)return ordinary;mask|=1u<<n;}return mask;
    }
    return ordinary;
}
bool Resolver::can_unlock(const baked::Submap& sm,const baked::Object& object) const {
    if(object.type!=baked::ObjType::Entity||!(object.d&1)||object.tw!=1||object.th!=1)return false;
    if(baked::game_number()==4) {
        if(!flags_.ff4_door_keys||!*flags_.ff4_door_keys)return false;
        const auto* record=records_.get(baked::name_of(sm));const auto* rows=record?record->get(L"item_gates"):nullptr;
        if(!rows||rows->arr.size()>2)return false;
        for(const auto& row:rows->arr) {
            const auto *id=row.get(L"id"),*group=row.get(L"group"),*raw=row.get(L"raw"),*cell=row.get(L"cell"),
                *content=row.get(L"content"),*receipt=row.get(L"receipt");
            if(!id||!group||!raw||!cell||cell->arr.size()!=3||!content||!receipt)continue;
            if(number(*id)!=object.entity_id()||group->s!=baked::str(object.group)||raw->s!=baked::object_identifier(object)||
               number(cell->arr[0])!=object.layer||number(cell->arr[1])!=object.tx||number(cell->arr[2])!=object.ty)continue;
            const int flag=number(*receipt);
            if(flag!=58&&(flag<271||flag>273))return false;
            const auto key=std::find(ff4_door_key_contents.begin(),ff4_door_key_contents.end(),number(*content));
            return key!=ff4_door_key_contents.end() &&
                (*flags_.ff4_door_keys & (1u << (key-ff4_door_key_contents.begin()))) != 0;
        }
        return false;
    }
    if(baked::game_number()==6) {
        // Rust Rid is a persistent native purchase receipt, not a consumable
        // inventory guess. Only this exact Zozo actor tests361 and opens362.
        // A valid onward journey still stops at the closed door for interaction.
        if(sm.first_map_id==383) {
            if(flags_.scenario[361]!=std::optional<int>(1))return false;
            const auto* record=records_.get(baked::name_of(sm));const auto* rows=record?record->get(L"item_gates"):nullptr;
            if(!rows||rows->arr.size()!=1)return false;
            const auto& row=rows->arr.front();
            const auto *id=row.get(L"id"),*group=row.get(L"group"),*raw=row.get(L"raw"),*cell=row.get(L"cell"),
                *ability=row.get(L"ability_scenario"),*receipt=row.get(L"receipt");
            return id&&group&&raw&&cell&&cell->arr.size()==3&&ability&&receipt&&
                number(*id)==104&&object.entity_id()==104&&group->s==baked::str(object.group)&&raw->s==baked::object_identifier(object)&&
                number(cell->arr[0])==object.layer&&number(cell->arr[1])==object.tx&&number(cell->arr[2])==object.ty&&
                number(*ability)==361&&number(*receipt)==362;
        }
        // Only these source-qualified Narshe locks use Locke's native active-
        // corps test. They cost no item; the current-floor planner must still
        // stop at the real closed door for manual interaction.
        if(sm.first_map_id!=3||flags_.party_locke!=std::optional<bool>(true))return false;
        const auto* record=records_.get(baked::name_of(sm));const auto* rows=record?record->get(L"party_gates"):nullptr;
        if(!rows||rows->arr.size()!=6)return false;
        for(const auto& row:rows->arr) {
            const auto *id=row.get(L"id"),*group=row.get(L"group"),*raw=row.get(L"raw"),*cell=row.get(L"cell"),
                *receipt=row.get(L"receipt"),*character=row.get(L"character_status");
            if(!id||!group||!raw||!cell||cell->arr.size()!=3||!receipt||!character)continue;
            if(number(*id)!=object.entity_id()||group->s!=baked::str(object.group)||raw->s!=baked::object_identifier(object)||
               number(cell->arr[0])!=object.layer||number(cell->arr[1])!=object.tx||number(cell->arr[2])!=object.ty)continue;
            return number(*character)==2&&number(*receipt)>=411&&number(*receipt)<=416;
        }
        return false;
    }
    if(flags_.party_thief!=std::optional<bool>(true)&&(!flags_.magic_keys||*flags_.magic_keys<1))return false;
    const auto* record=records_.get(baked::name_of(sm));const auto* rows=record?record->get(L"key_gates"):nullptr;
    if(!rows||rows->arr.size()>13)return false;
    for(const auto& row:rows->arr) {
        const auto *id=row.get(L"id"),*group=row.get(L"group"),*raw=row.get(L"raw"),*cell=row.get(L"cell"),
            *receipt=row.get(L"receipt"),*parents=row.get(L"prerequisites");
        if(!id||!group||!raw||!cell||cell->arr.size()!=3||!receipt||!parents||parents->arr.size()>1)continue;
        if(number(*id)!=object.entity_id()||group->s!=baked::str(object.group)||raw->s!=baked::object_identifier(object)||
           number(cell->arr[0])!=object.layer||number(cell->arr[1])!=object.tx||number(cell->arr[2])!=object.ty)continue;
        const int bit=number(*receipt);if(bit<205||bit>217)return false;
        int required=1;
        for(const auto& parent:parents->arr) {
            const int prior=number(parent);if(prior<205||prior>217||prior==bit)return false;
            // Unknown never proves that an ancestor has already been paid.
            if(flags_.scenario[static_cast<size_t>(prior)]!=std::optional<int>(1))++required;
        }
        return flags_.party_thief==std::optional<bool>(true)||
            (flags_.magic_keys&&*flags_.magic_keys>=required);
    }
    return false;
}
bool Resolver::story_barrier(const baked::Submap& sm,const baked::Object& object) const {
    const bool dwarven=baked::game_number()==4&&(sm.first_map_id==119||sm.first_map_id==121);
    const bool desert=baked::game_number()==5&&sm.first_map_id==286;
    const bool fork=baked::game_number()==5&&sm.first_map_id==443;
    if(desert&&(flags_.scenario[58]==std::optional<int>(1)||flags_.scenario[59]==std::optional<int>(1)))return false;
    if(fork&&flags_.scenario[142]==std::optional<int>(1))return false;
    if(((baked::game_number()==6&&(sm.first_map_id==162||sm.first_map_id==125||sm.first_map_id==143))||dwarven||desert||fork)&&
       (object.type==baked::ObjType::Event||object.type==baked::ObjType::NPC)) {
        const auto* record=records_.get(baked::name_of(sm));const auto* rows=record?record->get(L"story_actor_gates"):nullptr;
        if(!rows||rows->arr.empty()||rows->arr.size()>32)return false;
        for(const auto& row:rows->arr) {
            const auto *id=row.get(L"id"),*type=row.get(L"type"),*group=row.get(L"group"),*raw=row.get(L"raw"),*cell=row.get(L"cell"),*size=row.get(L"size");
            if(!id||!type||!group||!raw||!cell||cell->arr.size()!=3||!size||size->arr.size()!=2)continue;
            if(number(*type)!=int(object.type)||number(*id)!=object.entity_id()||group->s!=baked::str(object.group)||raw->s!=baked::object_identifier(object)||
               number(cell->arr[0])!=object.layer||number(cell->arr[1])!=object.tx||number(cell->arr[2])!=object.ty||
               number(size->arr[0])!=object.tw||number(size->arr[1])!=object.th)continue;
            // The exact initializer selects0350 before207,0440 without
            // Terra,0441 with solo Terra, or the allowed0442 scene. Only the
            // source-qualified active pushback actors block onward travel;
            // Albrook0426 similarly pushes the player out of its port
            // approach until native257/284 retires that group. add_obstacles
            // applies normal native group selection first. Vector0375's
            // detection regions also boot the player out after victory;
            // crossing them cannot substitute for the actual crate bypass.
            // The Dwarven Castle initializer selects three stationary guards
            // before scenario 46. A later-floor NPC retry must respect them.
            // Fork Tower's five pre-tablet contacts similarly return the
            // controlled player south;only native142 opens onward travel.
            return true;
        }
        return false;
    }
    if(baked::game_number()!=1||sm.first_map_id!=81||object.type!=baked::ObjType::NPC||!(object.d&1)||object.tw!=1||object.th!=1)return false;
    const auto* record=records_.get(baked::name_of(sm));const auto* rows=record?record->get(L"story_actor_gates"):nullptr;
    if(!rows||rows->arr.size()!=1)return false;
    const auto& row=rows->arr.front();const auto *id=row.get(L"id"),*type=row.get(L"type"),*group=row.get(L"group"),*raw=row.get(L"raw"),*cell=row.get(L"cell");
    if(!id||!type||!group||!raw||!cell||cell->arr.size()!=3||number(*type)!=int(object.type)||number(*id)!=object.entity_id()||
       group->s!=baked::str(object.group)||raw->s!=baked::object_identifier(object)||
       number(cell->arr[0])!=object.layer||number(cell->arr[1])!=object.tx||number(cell->arr[2])!=object.ty)return false;
    // Native initialization loads Giant11 before opening18; its own script
    // branches on Ruby17. Unknown copied receipts stay conservative. An owned
    // Ruby validates the prerequisite without pretending the NPC has moved.
    return flags_.scenario[17]!=std::optional<int>(1)&&flags_.scenario[18]!=std::optional<int>(1);
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
    bool found=false,all=true,destination_guard=false,mask_guard=false;
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
        // The same native pad has a deterministic first-entry phase and fresh
        // random repeats. Exact derived-arc flags distinguish those receipts;
        // the physical Event itself never inherits this graph-only filter.
        if(const auto* mask=guard.get(L"warp_mask")) {
            if(object.type!=baked::ObjType::Warp||mask->arr.size()!=2||
               (object.d&number(mask->arr[0]))!=number(mask->arr[1]))continue;
            if(!mask_guard){found=false;all=true;mask_guard=true;}
        } else if(mask_guard)continue;
        if(const auto* target=guard.get(L"destination")) {
            // One native Event can transfer to different real floors. This
            // bounded prefix grants only the matching derived Warp; it never
            // hides the physical event or overwrites the initializer state.
            if(object.type!=baked::ObjType::Warp||target->arr.size()!=2||number(target->arr[0])!=object.a||number(target->arr[1])!=object.b)continue;
            if(!destination_guard){found=false;all=true;destination_guard=true;}
        } else if(destination_guard)continue;
        found=true;
        const auto availability=project(guard,flags_);
        const auto* strict=guard.get(L"require_known");
        all &= (!availability.known&&strict&&strict->b)||(availability.known&&availability.group.empty());
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
    hash *= 1099511628211ull;
    hash ^= flags_.magic_keys ? (static_cast<uint64_t>(*flags_.magic_keys)<<1)|1 : 0;
    hash *= 1099511628211ull;
    hash ^= flags_.party_thief ? (static_cast<uint64_t>(*flags_.party_thief)<<1)|1 : 0;
    if(flags_.ff4_door_keys) {
        hash *= 1099511628211ull;hash^=0x4646344b4559ull+*flags_.ff4_door_keys;
    }
    if(flags_.party_locke) {
        // Preserve older games' exact signature path when this FFVI-only
        // observation is absent, and invalidate both known membership states.
        hash *= 1099511628211ull;hash^=0x4c4f434b45ull+static_cast<uint64_t>(*flags_.party_locke);
    }
    if(flags_.party_strago) {
        hash *= 1099511628211ull;hash^=0x53545241474full+static_cast<uint64_t>(*flags_.party_strago);
    }
    if(flags_.party_sabin) {
        hash *= 1099511628211ull;hash^=0x534142494eull+static_cast<uint64_t>(*flags_.party_sabin);
    }
    if(flags_.party_gau) {
        hash *= 1099511628211ull;hash^=0x474155ull+static_cast<uint64_t>(*flags_.party_gau);
    }
    if(flags_.party_terra) {
        hash *= 1099511628211ull;hash^=0x5445525241ull+static_cast<uint64_t>(*flags_.party_terra);
    }
    if(flags_.active_party_members) {
        hash *= 1099511628211ull;hash^=0x4d454d42455253ull+static_cast<uint64_t>(*flags_.active_party_members);
    }
    if(flags_.party_cyan) {
        hash *= 1099511628211ull;hash^=0x4359414eull+static_cast<uint64_t>(*flags_.party_cyan);
    }
    if(flags_.party_mog) {
        hash *= 1099511628211ull;hash^=0x4d4f47ull+static_cast<uint64_t>(*flags_.party_mog);
    }
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
std::shared_ptr<const baked::TransitionGrid> Resolver::transitions(const baked::Submap& sm,bool doors_only)
{
    const int key=baked::submap_index(&sm)*2+(doors_only?1:0);
    if(const auto found=transitions_.find(key);found!=transitions_.end())return found->second;
    const auto& grid=baked::grids(sm);auto mask=std::make_shared<baked::TransitionGrid>();
    for(const auto& original:grid.objects) {
        const int bits=baked::transition_bits(original);
        // Current world1 capture replaces every script-warp bit with current
        // native power. Do not project 1,524 discarded warp variants first.
        // Remote floors retain the complete conservative transition projection.
        if(doors_only&&bits!=1)continue;
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
const std::vector<Resolver::WalkOpenContact>& Resolver::walk_open_contacts(const baked::Submap& sm)
{
    const int index = baked::submap_index(&sm);
    auto [cached, fresh] = walk_open_contacts_.try_emplace(index);
    auto& contacts = cached->second;
    if (!fresh) return contacts;
    const auto* record = records_.get(baked::name_of(sm));
    const auto* rows = record ? record->get(L"walk_open") : nullptr;
    if (!rows || rows->arr.size() > 16) return contacts;
    const auto& grid = baked::grids(sm);
    const auto lookup = [&](const json::Value& key) -> const baked::Object* {
        if (key.arr.size() != 6) return nullptr;
        const auto& a = key.arr;
        for (const auto& object : grid.objects) {
            if (object.entity_id() == number(a[1]) && int(object.type) == number(a[0]) &&
                object.layer == number(a[3]) && object.tx == number(a[4]) && object.ty == number(a[5]) &&
                baked::str(object.group) == a[2].s) return &object;
        }
        return nullptr;
    };
    for (const auto& row : rows->arr) {
        const auto* source = row.get(L"contact");
        const auto* script = row.get(L"script");
        const auto* blockers = row.get(L"blockers");
        if (!source || !script || !blockers || blockers->arr.size() > 16) continue;
        const auto* contact = lookup(*source);
        if (!contact || !baked_route::step_event(*contact) || contact->a != number(*script) ||
            contact->b != 2 || contact->tw != 1 || contact->th != 1) continue;
        WalkOpenContact entry{contact, {}};
        for (const auto& key : blockers->arr) {
            const auto* object = lookup(key);
            if (object && (object->d & 1) && object->tw == 1 && object->th == 1)
                entry.blockers.push_back(object);
        }
        if (!entry.blockers.empty()) contacts.push_back(std::move(entry));
    }
    return contacts;
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
    std::vector<const baked::Object*> ordinary_contacts;
    std::vector<const baked::Object*> auto_blockers;
    for (const auto& entry : resolver.walk_open_contacts(sm)) {
        const auto& contact = *entry.contact;
        if (!resolver.active(sm, contact) || resolver.consumed(sm, contact)) continue;
        const auto source = resolver.effective(sm, contact);
        if (source.tx != contact.tx || source.ty != contact.ty || source.layer != contact.layer) continue;
        ordinary_contacts.push_back(&contact);
        for (const auto* blocker : entry.blockers) {
            if (!resolver.active(sm, *blocker, true)) continue;
            const auto door = resolver.effective(sm, *blocker);
            if (door.tx != blocker->tx || door.ty != blocker->ty || door.layer != blocker->layer) continue;
            auto_blockers.push_back(blocker);
            request.walk_open_doors.push_back({{door.layer,door.tx,door.ty},{source.layer,source.tx,source.ty}});
        }
    }
    for (const auto& original : baked::grids(sm).objects) {
        const bool story_actor=resolver.story_barrier(sm,original);
        if(geometry_only && original.type!=baked::ObjType::Entity && original.type!=baked::ObjType::AnimEntity && original.type!=baked::ObjType::EffectEntity &&
           original.type!=baked::ObjType::CollisionEntity&&!story_actor)continue;
        if (!resolver.active(sm, original, true)) continue;
        const auto o=resolver.effective(sm,original);
        const uint64_t ids=resolver.transportation_mask(sm,original);
        const bool restricted=ids!=~uint64_t(0);
        const bool npc = o.type == baked::ObjType::NPC || o.type == baked::ObjType::ShopNPC;
        if(npc&&!with_npcs&&!(gates&&story_actor))continue;
        if (std::find(auto_blockers.begin(),auto_blockers.end(),&original)!=auto_blockers.end()) continue;
        const bool solid = o.type == baked::ObjType::TreasureBox ||
            (gates&&story_actor&&o.type==baked::ObjType::Event)||
            (npc && (o.d & 1) && (story_actor?gates:with_npcs)) ||
            (gates && !resolver.can_unlock(sm,original) && ((o.type == baked::ObjType::Entity || o.type == baked::ObjType::AnimEntity || o.type == baked::ObjType::EffectEntity) && (o.d & 1)));
        const bool collider = gates && o.type == baked::ObjType::CollisionEntity;
        const bool used=resolver.consumed(sm,original);
        const bool terminal=baked::world_exit(o)&&!used;
        const auto* source_exit=baked::world_exit_at(baked::grids(sm),o.entity_id(),baked::object_identifier(o),o.tx,o.ty);
        const bool exit_source=source_exit&&(!(source_exit->d&(baked::kAutomaticCurrent|baked::kConditionalSceneExit))||!resolver.consumed(sm,*source_exit));
        const bool trigger = baked_route::step_event(o)&&!used&&!exit_source&&
            std::find(ordinary_contacts.begin(),ordinary_contacts.end(),&original)==ordinary_contacts.end();
        const int transition_bits=used?0:baked::transition_bits(o);
        if (!solid && !collider && !trigger&&!terminal&&!(restricted&&transition_bits)) continue;
        for (int y = o.ty; y < o.ty + o.th; ++y) for (int x = o.tx; x < o.tx + o.tw; ++x) {
            baked_route::Cell cell{sm.world || (npc && (o.d & 2)) ? -1 : o.layer, x, y};
            if(restricted&&transition_bits)mark((transition_bits&2)?Rules::Terminal:Rules::GroundTerminal,cell,ids);
            if (solid) {
                if(restricted)mark((!story_actor&&(npc||o.type==baked::ObjType::TreasureBox))?Rules::Solid:Rules::Gate,cell,ids);
                else request.blocked.push_back(cell);
                if(gates&&o.type==baked::ObjType::Entity) {
                    const unsigned layers=resolver.collision_layers(sm,original);
                    for(int layer=0;layer<3;++layer)if(layer!=cell.layer&&(layers&(1u<<layer))){
                        const baked_route::Cell extra{layer,x,y};
                        if(restricted)mark(Rules::Gate,extra,ids);else request.blocked.push_back(extra);
                    }
                }
            }
            if (collider) {
                if(restricted) {
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
