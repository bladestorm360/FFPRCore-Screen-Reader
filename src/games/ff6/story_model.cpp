// State-driven story objectives, kept separate from native observation/speech.
// Each predicate is a documented game flag, local, party, or occupied switch.
// The shared route worker still decides actual reachability with current gates.
#include "story_model.h"
#include "dream_train_maze.h"
#include "zone_eater_roof.h"
#include "sealed_gate_model.h"
#include <algorithm>
#include <cmath>
#include <climits>
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
    if(bank==L"treasure"&&index<1024)return state.flags.treasure[static_cast<size_t>(index)];
    if(bank==L"global"&&index<16)return state.flags.global[static_cast<size_t>(index)];
    if(bank==L"area"&&index<8)return state.flags.area[static_cast<size_t>(index)];
    if(bank==L"local"&&index<8)return state.flags.local[static_cast<size_t>(index)];
    if(bank==L"map")return state.map;
    if(bank==L"party")return state.flags.global[4]; // native Current.SwitchCorps writes zero-based slot
    // Owned player-cell predicates select an explicit informational waypoint
    // from its approach side. They never infer native operation completion.
    if(bank==L"player_y"&&index==0)return state.player.y;
    if(bank==L"player_x"&&index==0)return state.player.x;
    if(bank==L"player_layer"&&index==0)return state.player.layer;
    if(bank==L"strago"&&index==0&&state.flags.party_strago)return *state.flags.party_strago?1:0;
    if(bank==L"gau"&&index==0&&state.flags.party_gau)return *state.flags.party_gau?1:0;
    if(bank==L"terra"&&index==0&&state.flags.party_terra)return *state.flags.party_terra?1:0;
    if(bank==L"cyan"&&index==0&&state.flags.party_cyan)return *state.flags.party_cyan?1:0;
    if(bank==L"mog"&&index==0&&state.flags.party_mog)return *state.flags.party_mog?1:0;
    if(bank==L"members"&&index==0)return state.flags.active_party_members;
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
        } else if(found->type==field::baked::ObjType::TreasureBox)out.label=T(L"Treasure Chest");
        else out.label=field::safe_event_label(goal.raw_name,L"Interactive Object",goal.map_id,goal.entity_id,static_cast<int>(found->type));
    } else {
        goal.cell={integer(row,L"layer",1),integer(row,L"x"),integer(row,L"y")};
        out.label=T(string(row,L"label"));
    }
    const auto kind=string(row,L"kind");
    if(const auto* strict=row.get(L"avoid_other_events"))goal.avoid_other_events=strict->b;
    if(const auto* crossings=row.get(L"allow_scripted_crossings"))goal.allow_scripted_crossings=crossings->b;
    // Reviewed fetch/escape guides can cross their real overworld connection.
    // Other recipes keep the existing indoor-only policy by default.
    if(const auto* world=row.get(L"allow_world_transit"))goal.allow_world_transit=world->b;
    out.route.kind=kind==L"interact"?field::TargetKind::Interact:kind==L"spot"?field::TargetKind::Spot:
        kind==L"transition"?field::TargetKind::Transition:field::TargetKind::Infer;
    out.route.instruction=string(row,L"text"); // key, localized after stable NPC numbering
    return out;
}
}
bool supported_map(int map) {
    if(map==345||map==346||map==348)return true;
    if(map>=334&&map<=343)return true;
    if(map>=290&&map<=295)return true;
    if(map==311||(map>=488&&map<=492))return true;
    if(map==274||map==278||(map>=383&&map<=395)||(map>=469&&map<=475))return true;
    if(map==201||map==433||map==434)return true;
    if(map==20||(map>=493&&map<=496))return true;
    return (map>=296&&map<=304) || (map>=520&&map<=538) || (map>=190&&map<=193) || map==597 || map==143 || map==162 || (map>=421&&map<=430) || (map>=377&&map<=382) || map==96 || (map>=84&&map<=91) || (map>=207&&map<=213) || (map>=165&&map<=179) || map==584 || map==19 || map==318 || map==320 || map==576 || map==624 || (map>=355&&map<=372) ||
        (map>=412&&map<=420) || (map>=112&&map<=118) || (map>=497&&map<=499) || map==2 || (map>=214&&map<=251) || (map>=435&&map<=441) || (map>=447&&map<=454) ||
        (map>=542&&map<=571) || map==590 || map==595 || map==596 || (map>=500&&map<=507) ||
        (map>=456&&map<=468) || map==577 || (map>=476&&map<=487) || (map>=442&&map<=446) || map==386 || map==387 || (map>=405&&map<=411) || map==431 || map==631 || map==432 || map==3 || map==16 || map==18 || map==20 || (map>=323&&map<=327) || (map>=539&&map<=541) || map==1 || (map>=38&&map<=65) || map==328 || map==333 || (map>=626&&map<=630);
}
Plan plan(const Snapshot& state) {
    modtext::LocaleScope locale(state.locale);
    Plan out;out.map=state.map;
    const auto& root=data::game_json("puzzles");
    const auto* guides=root.get(L"story_guides");
    if(!state.guidance.any()){out.note=T(L"Puzzle guidance is off");return out;}
    // The reviewed guides use one finite capacity. All older
    // models retain their selection/order; malformed oversized input is refused.
    // The complete later Opera episode brought the reviewed catalog past40 guides.
    // Retain a finite malformed-input limit; rules/candidates keep their own
    // unchanged bounds and all model work stays on owned data.
    if(!guides||guides->arr.size()>48)return out;
    for(const auto& guide:guides->arr) {
        if(!map_in(guide,state.map))continue;
        // The later Narshe chase traverses the earlier defense floor. Only an
        // explicitly qualified recipe may opt out before owning that map;
        // unknown selection state must not fall through to another episode.
        if(const auto* selection=guide.get(L"select_when")) {
            const auto selected=matches(selection,state);
            if(!selected){out.supported=true;out.unknown=true;return out;}
            if(!*selected)continue;
        }
        const auto id=string(guide,L"id");
        const auto base=field::puzzle_policy::kind(root,id);
        const auto blocked=[&](field::puzzle_policy::Kind kind) {
            Plan disabled;disabled.map=state.map;disabled.supported=true;
            disabled.note=T(field::puzzle_policy::disabled_key(kind));return disabled;
        };
        if(!field::puzzle_policy::any_enabled(root,id,state.guidance))return blocked(base);
        const auto qualified=[&](Plan result) {
            result.puzzle_types=field::puzzle_policy::mask(base);return result;
        };
        if(id==L"zone_eater_roof")return state.guidance.allows(base)?qualified(roof_plan(state,guide)):blocked(base);
        out.supported=true;
        // A complete map-puzzle journey can have independent story and
        // treasure receipts. Reuse the bounded owned-snapshot predicates;
        // an unknown receipt must never certify completion. No native read
        // or new observer is added, and prior single-flag guides are unchanged.
        if(const auto* end=guide.get(L"complete_when")) {
            const auto completed=matches(end,state);
            if(!completed){out.unknown=true;break;}
            if(*completed){out.finished=true;out.note=T(L"This story sequence is complete.");break;}
        }
        const int completion=integer(guide,L"complete_scenario",-1);
        if(completion>=0) {
            if(completion>=1024||!state.flags.scenario[static_cast<size_t>(completion)]){out.unknown=true;break;}
            if(*state.flags.scenario[static_cast<size_t>(completion)]==1) {
                out.finished=true;out.note=T(L"This story sequence is complete.");break;
            }
        }
        const auto eligible=matches(guide.get(L"when"),state);
        if(!eligible){out.unknown=true;break;}
        if(!*eligible){out.finished=true;out.note=T(string(guide,L"inactive"));break;}
        if(const auto* finish=guide.get(L"finish_on_completion");finish&&finish->b)
            out.finish_on_scenario=completion;
        if(state.guidance.allows(base)) {
            out.introduction=T(string(guide,L"introduction"));out.hold_note=T(string(guide,L"blocked"));
        }
        if(id==L"sealed_gate"&&state.map==407)return state.guidance.allows(base)?qualified(sealed_gate_bridge_plan(state,guide)):blocked(base);
        if(string(guide,L"id")==L"dream_train"&&state.map==525) {
            if(!state.guidance.allows(base))return blocked(base);
            out.puzzle_types=field::puzzle_policy::mask(base);
            const auto advice=dream_train_maze(state,data::game_json("mapstate"),state.cancelled);
            const auto waiting=[&] {out.unknown=true;out.note=T(L"Story guidance is waiting for current game state.");};
            if(advice.unknown||advice.cancelled||advice.entity<0){waiting();return out;}
            const json::Value* selected=nullptr;
            if(advice.actions==0&&!advice.exit) {
                const auto* pattern=guide.get(L"pattern_targets");
                if(!pattern||pattern->arr.size()!=6){waiting();return out;}
                constexpr int wanted[]{0,0,1,1,1,0};
                for(size_t i=0;i<6;++i) {
                    const auto actual=state.flags.local[i+1];
                    if(!actual){waiting();return out;}
                    if(*actual!=wanted[i]){selected=&pattern->arr[i];break;}
                }
            }
            if(!selected) {
                const auto* controls=guide.get(L"maze_targets");
                if(!controls||controls->arr.size()>8){waiting();return out;}
                for(const auto& row:controls->arr)if(integer(row,L"entity",-1)==advice.entity) {
                    if(selected){waiting();return out;}selected=&row;
                }
            }
            if(!selected){waiting();return out;}
            auto next=target(*selected);if(!next){waiting();return out;}
            next->route.instruction=TF(next->route.instruction,{next->label});
            out.targets.push_back(std::move(next->route));return out;
        }
        const auto* rules=guide.get(L"rules");
        if(!rules||rules->arr.size()>128){out.unknown=true;break;}
        const auto* first_match=guide.get(L"first_match");
        const bool ordered=first_match&&first_match->b;
        std::vector<Target> targets;
        auto withheld=field::puzzle_policy::Kind::Unknown;bool has_withheld=false;
        for(const auto& rule:rules->arr) {
            const auto enabled=matches(rule.get(L"when"),state);
            if(!enabled){
                out.unknown=true;
                // Ordered fetch/escape steps cannot fall through to a later
                // operation while their current native prerequisite is unknown.
                if(ordered)break;
                // An occupied pressure plate cannot safely inherit a later
                // release rule while its handoff condition is unknown.
                if(const auto* at=rule.get(L"at"))if(auto occupied=target(*at))
                    if(occupied->route.goal.map_id==state.map&&occupied->route.goal.cell==state.player) {
                        out.note=T(L"Story guidance is waiting for current game state.");return out;
                    }
                continue;
            }
            if(!*enabled)continue;
            std::optional<Target> occupied;
            if(const auto* at=rule.get(L"at")) {
                occupied=target(*at);
                if(!occupied){out.unknown=true;continue;}
                if(occupied->route.goal.map_id!=state.map||!(occupied->route.goal.cell==state.player))continue;
            }
            const auto note=string(rule,L"note");
            if(!note.empty()) {
                const auto type=field::puzzle_policy::kind(root,id,note);
                if(!state.guidance.allows(type))return blocked(type);
                out.puzzle_types|=field::puzzle_policy::mask(type);
            }
            if(const auto* leave=rule.get(L"leave_switch");leave&&leave->b&&occupied) {
                // A latched Tower door no longer requires this party's plate,
                // but the weight cannot fall onto it. Move to the reviewed
                // side: north is a real dead pocket once the weight lands.
                // Native geometry/party/event checks still validate that step.
                const auto* direction=rule.get(L"leave_direction");
                const auto unavailable=[&]{out.unknown=true;out.note=T(L"Story guidance is waiting for current game state.");};
                if(!direction||direction->arr.size()!=2){unavailable();return out;}
                const auto& x=direction->arr[0];const auto& y=direction->arr[1];
                if(x.type!=json::Value::Type::Number||y.type!=json::Value::Type::Number||
                    (x.n!=-1&&x.n!=0&&x.n!=1)||(y.n!=-1&&y.n!=0&&y.n!=1)||
                    std::abs(x.n)+std::abs(y.n)!=1){unavailable();return out;}
                const int dx=static_cast<int>(direction->arr[0].n),dy=static_cast<int>(direction->arr[1].n);
                {
                    auto next=occupied->route;
                    next.goal.entity_id=-1;next.goal.raw_name.clear();next.goal.cells.clear();
                    next.goal.cell.x+=dx;next.goal.cell.y+=dy;
                    next.goal.avoid_other_events=true;next.goal.allow_scripted_crossings=false;
                    next.kind=field::TargetKind::Spot;next.instruction=T(note);
                    out.targets.push_back(std::move(next));
                }
                return out;
            }
            if(const int exit_map=integer(rule,L"finish_on_exit");exit_map>0)out.finish_on_exit=exit_map;
            // Independent late map-puzzle receipts can outlive a story flag.
            // Arm that flag only for the actual final native interaction.
            const int finish=integer(rule,L"finish_on_scenario",-1);
            if(finish>=0&&finish<1024)out.finish_on_scenario=finish;
            if(const auto* through=rule.get(L"finish_through")) {
                if(out.finish_on_exit<=0||through->arr.size()>16){out.unknown=true;return out;}
                for(const auto& row:through->arr) {
                    if(row.type!=json::Value::Type::Number||!std::isfinite(row.n)||row.n<1||row.n>INT_MAX){out.unknown=true;return out;}
                    const int map=static_cast<int>(row.n);
                    if(!field::baked::submap_by_map_id(map)){out.unknown=true;return out;}
                    out.finish_through.push_back(map);
                }
            }
            if(const auto* release=rule.get(L"release")) {
                // A held plate may be left only along a route which remains
                // valid after its native release script closes/moves geometry.
                if(release->arr.size()!=3){out.unknown=true;continue;}
                const int index=static_cast<int>(release->arr[1].n);
                const auto& bank=release->arr[0].s;
                if(index<0||(bank==L"area"?index>=8:bank==L"scenario"?index>=1024:true)){out.unknown=true;continue;}
                out.anticipated_release=state.flags;
                const int value=static_cast<int>(release->arr[2].n);
                if(bank==L"area")out.anticipated_release->area[static_cast<size_t>(index)]=value;
                else out.anticipated_release->scenario[static_cast<size_t>(index)]=value;
                out.hold_note=T(note);continue;
            }
            if(!note.empty()){
                out.note=T(note);
                if(const auto* door=rule.get(L"owzer_door");door&&door->b)
                    out.owzer_door_wait=string(guide,L"id")==L"owzers_mansion"&&state.map==445&&
                        state.player==field::baked_route::Cell{1,23,23}&&state.flags.local[2]==1;
                return out;
            }
            const auto* list=rule.get(L"targets");if(!list)continue;
            if(list->arr.size()>32){out.unknown=true;continue;}
            for(const auto& row:list->arr) {
                if(targets.size()>=32){out.unknown=true;break;}
                // A timed tour offers only its still-unvisited native actors.
                // This opt-in per-target predicate keeps one ordered stage and
                // its bounded alternatives; older targets have no predicate.
                const auto available=matches(row.get(L"when"),state);
                if(!available){out.unknown=true;continue;}
                if(!*available)continue;
                const auto type=field::puzzle_policy::kind(root,id,string(row,L"text"));
                if(!state.guidance.allows(type)){withheld=type;has_withheld=true;continue;}
                auto ready=target(row);
                if(ready){out.puzzle_types|=field::puzzle_policy::mask(type);targets.push_back(std::move(*ready));}else out.unknown=true;
            }
            // Explicit opt-in: alternatives inside this one step still share
            // the planner, but later quest stages are never fallback targets.
            // Existing multi-control guides retain their additive rules.
            if(ordered)break;
        }
        if(targets.empty()&&has_withheld)return blocked(withheld);
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
        if(out.targets.empty())out.note=out.unknown?T(L"Story guidance is waiting for current game state."):
            (out.hold_note.empty()?T(string(guide,L"complete")):out.hold_note);
        return out;
    }
    if(out.unknown&&out.targets.empty())out.note=T(L"Story guidance is waiting for current game state.");
    else if(out.finished&&out.note.empty())out.note=T(L"This story sequence is complete.");
    // These optional episodes share ordinary town/world/cave maps. Retain a
    // useful response when its selection flags say it does not own the floor.
    else if(!out.supported&&((state.map>=190&&state.map<=193)||state.map==597||state.map==143||(state.map>=421&&state.map<=430)||(state.map>=84&&state.map<=91)||(state.map>=165&&state.map<=179)||(state.map>=207&&state.map<=213)||state.map==584||state.map==1||(state.map>=38&&state.map<=65)||state.map==328||state.map==333||(state.map>=626&&state.map<=630)||state.map==2||(state.map>=214&&state.map<=251)||(state.map>=435&&state.map<=441)||(state.map>=447&&state.map<=454)))
        out.note=T(L"No puzzle guide for this map");
    return out;
}
}
