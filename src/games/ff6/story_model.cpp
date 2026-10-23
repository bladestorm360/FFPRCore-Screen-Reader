// State-driven story objectives, kept separate from native observation/speech.
// Each predicate is a documented game flag, local, party, or occupied switch.
// The shared route worker still decides actual reachability with current gates.
#include "story_model.h"
#include <algorithm>
#include <cmath>
#include "../../core/core.h"
#include "../../core/modtext.h"
#include "../../core/field/event_labels.h"
#include "../../core/field/entity_translator.h"
#include "../../core/field/npc_names.h"
#include "../../core/text.h"

namespace ffpr::ff6::story {
namespace {
int integer(const json::Value& value,const wchar_t* key,int fallback=0) {
    const auto* v=value.get(key);return v?static_cast<int>(v->n):fallback;
}
std::wstring string(const json::Value& value,const wchar_t* key) {
    const auto* v=value.get(key);return v?v->s:std::wstring();
}
std::optional<int> state_value(const Snapshot& state,const std::wstring& bank,int index) {
    if(index<0)return {};
    if(bank==L"scenario"&&index<1024)return state.flags.scenario[static_cast<size_t>(index)];
    if(bank==L"global"&&index<16)return state.flags.global[static_cast<size_t>(index)];
    if(bank==L"area"&&index<8)return state.flags.area[static_cast<size_t>(index)];
    if(bank==L"local"&&index<8)return state.flags.local[static_cast<size_t>(index)];
    if(bank==L"map")return state.map;
    if(bank==L"party")return state.flags.global[4]; // native Current.SwitchCorps writes zero-based slot
    return {};
}
// Unknown is distinct from false: missing live state must not certify an action.
std::optional<bool> matches(const json::Value* conditions,const Snapshot& state) {
    if(!conditions)return true;
    if(conditions->arr.size()>32)return {};
    bool unknown=false;
    for(const auto& row:conditions->arr) {
        if(row.arr.size()!=4||!row.arr[0].is_string()||!row.arr[2].is_string())return {};
        const auto actual=state_value(state,row.arr[0].s,static_cast<int>(row.arr[1].n));
        if(!actual){unknown=true;continue;}
        const int wanted=static_cast<int>(row.arr[3].n);
        const auto& op=row.arr[2].s;
        const bool pass=op==L"eq"?*actual==wanted:op==L"ne"?*actual!=wanted:
            op==L"lt"?*actual<wanted:op==L"ge"?*actual>=wanted:false;
        if(!pass)return false;
    }
    return unknown?std::optional<bool>{}:std::optional<bool>{true};
}
bool map_in(const json::Value& guide,int map) {
    const auto* maps=guide.get(L"maps");if(!maps||maps->arr.size()>128)return false;
    for(const auto& id:maps->arr)if(id.n==map)return true;
    return false;
}
struct Target {
    field::baked_nav::GuideTarget route;
    std::wstring label;
    field::npc_names::Label npc;
    bool is_npc=false;
};
std::optional<Target> target(const json::Value& row) {
    Target out;
    auto& goal=out.route.goal;
    goal.map_id=integer(row,L"map");goal.entity_id=integer(row,L"entity",-1);
    const auto* map=field::baked::submap_by_map_id(goal.map_id);if(!map)return {};
    const auto raw=string(row,L"raw"),group=string(row,L"group");
    const field::baked::Object* found=nullptr;
    if(goal.entity_id>=0) {
        for(const auto& object:field::baked::grids(*map).objects) {
            if(object.type==field::baked::ObjType::Warp||object.type==field::baked::ObjType::PointIn)continue;
            if(object.entity_id()!=goal.entity_id||(!raw.empty()&&field::baked::object_identifier(object)!=raw)||
                (!group.empty()&&field::baked::str(object.group)!=group))continue;
            if(found)return {}; // a recipe must identify one physical native target
            found=&object;
        }
        if(!found)return {};
        goal.cell={found->layer,found->tx,found->ty};
        goal.raw_name=field::baked::object_identifier(*found);
        out.is_npc=found->type==field::baked::ObjType::NPC||found->type==field::baked::ObjType::ShopNPC;
        if(out.is_npc) {
            out.label=field::translate_entity_name_snapshot(goal.raw_name);
            out.npc=field::npc_display_label_snapshot(out.label);
        } else out.label=field::safe_event_label(goal.raw_name,L"Interactive Object",goal.map_id,goal.entity_id,static_cast<int>(found->type));
    } else {
        goal.cell={integer(row,L"layer",1),integer(row,L"x"),integer(row,L"y")};
        out.label=T(string(row,L"label"));
    }
    const auto kind=string(row,L"kind");
    out.route.kind=kind==L"interact"?field::TargetKind::Interact:kind==L"spot"?field::TargetKind::Spot:
        kind==L"transition"?field::TargetKind::Transition:field::TargetKind::Infer;
    out.route.instruction=string(row,L"text"); // key, localized after stable NPC numbering
    return out;
}
}
bool supported_map(int map) {
    return map==19 || map==318 || map==576 || map==624 || (map>=355&&map<=372) ||
        (map>=412&&map<=420) || (map>=112&&map<=118) || (map>=497&&map<=499) ||
        (map>=542&&map<=571) || map==590 || map==595 || map==596;
}
Plan plan(const Snapshot& state) {
    modtext::LocaleScope locale(state.locale);
    Plan out;out.map=state.map;
    const auto* guides=data::game_json("puzzles").get(L"story_guides");
    if(!guides||guides->arr.size()>16)return out;
    for(const auto& guide:guides->arr) {
        if(!map_in(guide,state.map))continue;
        out.supported=true;
        const auto eligible=matches(guide.get(L"when"),state);
        if(!eligible){out.unknown=true;break;}
        if(!*eligible){out.finished=true;break;}
        out.introduction=T(string(guide,L"introduction"));
        const auto* rules=guide.get(L"rules");
        if(!rules||rules->arr.size()>128){out.unknown=true;break;}
        std::vector<Target> targets;
        for(const auto& rule:rules->arr) {
            const auto enabled=matches(rule.get(L"when"),state);
            if(!enabled){out.unknown=true;continue;}
            if(!*enabled)continue;
            if(const auto* at=rule.get(L"at")) {
                const auto location=target(*at);
                if(!location){out.unknown=true;continue;}
                if(location->route.goal.map_id!=state.map||!(location->route.goal.cell==state.player))continue;
            }
            const auto note=string(rule,L"note");
            if(const auto* release=rule.get(L"release")) {
                // A held plate may be left only along a route which remains
                // valid after its native release script closes/moves geometry.
                if(release->arr.size()!=3||release->arr[0].s!=L"area"){out.unknown=true;continue;}
                const int index=static_cast<int>(release->arr[1].n);
                if(index<0||index>=8){out.unknown=true;continue;}
                out.anticipated_release=state.flags;
                out.anticipated_release->area[static_cast<size_t>(index)]=static_cast<int>(release->arr[2].n);
                out.hold_note=T(note);continue;
            }
            if(!note.empty()){out.note=T(note);return out;}
            const auto* list=rule.get(L"targets");if(!list)continue;
            if(list->arr.size()>32){out.unknown=true;continue;}
            for(const auto& row:list->arr) {
                if(targets.size()>=32){out.unknown=true;break;}
                auto ready=target(row);
                if(ready)targets.push_back(std::move(*ready));else out.unknown=true;
            }
        }
        std::vector<field::npc_names::Entry> names;
        for(auto& item:targets)if(item.is_npc)names.push_back({&item.label,item.npc,item.route.goal.map_id,item.route.goal.cell.layer,
            static_cast<float>(item.route.goal.cell.x),static_cast<float>(item.route.goal.cell.y)});
        field::npc_names::renumber(names);
        for(auto& item:targets) {
            item.route.instruction=TF(item.route.instruction,{item.label});
            out.targets.push_back(std::move(item.route));
        }
        if(const auto* defended=guide.get(L"defend")) {
            if(auto actor=target(*defended)) {
                field::baked_nav::DefenseContext context;
                context.protected_actor=actor->route.goal;
                context.warning=T(string(guide,L"danger"));
                context.current_party=std::clamp(state.flags.global[4].value_or(0),0,2);
                for(size_t i=1;i<out.targets.size();++i)context.enemies.push_back(out.targets[i].goal);
                for(size_t i=0;i<state.parties.size();++i) {
                    const auto& party=state.parties[i];if(party.map!=state.map)continue;
                    auto position=party.position;
                    if(!position&&!party.serialized.empty()&&party.serialized.size()<=16384) {
                        json::Value saved;
                        if(json::parse(text::to_utf8(party.serialized),saved))if(const auto* p=saved.get(L"position")) {
                            const auto* x=p->get(L"x"),*y=p->get(L"y");
                            if(x&&y&&x->type==json::Value::Type::Number&&y->type==json::Value::Type::Number)position=unity::Vec3{static_cast<float>(x->n),static_cast<float>(y->n),0};
                        }
                    }
                    if(position) {
                        const auto* sm=field::baked::submap_by_map_id(state.map);
                        const double x=sm->width*.5+position->x/16,y=sm->height*.5-position->y/16;
                        if(std::isfinite(x)&&std::isfinite(y)&&x>=0&&y>=0&&x<sm->width&&y<sm->height)
                            context.parties[i]=field::baked_route::Cell{1,static_cast<int>(x),static_cast<int>(y)};
                    }
                }
                context.parties[static_cast<size_t>(context.current_party)]=state.player;
                out.defense=std::move(context);
            }
        }
        if(out.targets.empty()&&!out.unknown)out.note=out.hold_note.empty()?T(string(guide,L"complete")):out.hold_note;
        return out;
    }
    if(out.unknown&&out.targets.empty())out.note=T(L"Story guidance is waiting for current game state.");
    else if(out.finished)out.note=T(L"This story sequence is complete.");
    return out;
}
}
