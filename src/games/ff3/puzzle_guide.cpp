// Native-owner adapter for FFIII's form-dependent puzzles. Only an explicit
// request reads party conditions. Native events observe cheap operation bits or
// schedule one readiness continuation. Owned targets use the shared route worker.
#include "puzzle_guide.h"
#include "puzzle_model.h"
#include <climits>
#include "../../core/core.h"
#include "../../core/frame.h"
#include "../../core/hooks.h"
#include "../../core/modtext.h"
#include "../../core/say.h"
#include "../../core/text.h"
#include "../../core/field/baked_nav.h"
#include "../../core/field/field_api.h"
#include "../../core/field/map_state.h"
#include "../../core/field/scene_events.h"
#include "../../core/ui/preferences.h"

namespace ff3::puzzles {
namespace {
struct Entry {std::wstring id,text,arg;int step=0;bool manual=false;field::baked_nav::MapGoal goal;};
struct Definition {Family family=Family::None;int count=0;std::vector<Entry> actions;};
std::vector<Definition> definitions;
bool loaded=false,pending=false;
Family current=Family::None;
Action last=Action::Loading;
int receipt=-1;
uint64_t epoch=0;
std::optional<int> mini,frog;
std::optional<field::baked_nav::MapGoal> preview;
std::wstring owned_line;
int owned_step=0,owned_count=0;
std::wstring presented_line() {
    // Cache the instruction rather than its optional caption, so changing the
    // setting also affects a repeat inside a native menu without a game read.
    return ui::prefs::puzzle_guide_verbose()&&owned_step>0&&owned_count>0?
        TF(L"Step {0} of {1}: {2}",{text::num(owned_step),text::num(owned_count),owned_line}):owned_line;
}
bool ready() {
    return !menu_state::any_active()&&!queries::is("GameStatePatches.IsInputTransitionBlocked")&&
        queries::is("GameStatePatches.IsFieldNavigationReady")&&!queries::is("BattleStateHelper.IsInBattle");
}
void clear_preview() {if(preview)field::baked_nav::clear_preview_destination(*preview);preview.reset();}
bool load() {
    if(loaded)return !definitions.empty();
    if(!data::game_data_ready())return false;
    loaded=true;
    const auto* rows=data::game_json("puzzles").get(L"form_guides");
    if(!rows||rows->type!=json::Value::Type::Array||rows->arr.size()!=6)return false;
    for(const auto& row:rows->arr) {
        const auto* id=row.get(L"id");const auto* actions=row.get(L"actions");
        if(!id||!actions||actions->type!=json::Value::Type::Array||actions->arr.empty()||actions->arr.size()>16){definitions.clear();return false;}
        Definition definition;definition.family=id->s==L"nepto_forms"?Family::Nepto:id->s==L"owen_forms"?Family::Owen:
            id->s==L"tozus_forms"?Family::Tozus:id->s==L"hein_forms"?Family::Hein:id->s==L"lake_forms"?Family::Lake:
            id->s==L"magic_circle_forms"?Family::MagicCircle:Family::None;
        if(definition.family==Family::None){definitions.clear();return false;}
        definition.count=static_cast<int>(actions->arr.size());
        for(const auto& action:actions->arr) {
            Entry entry;
            if(const auto* v=action.get(L"id"))entry.id=v->s;
            if(const auto* v=action.get(L"text"))entry.text=v->s;
            if(const auto* v=action.get(L"arg"))entry.arg=v->s;
            if(const auto* v=action.get(L"step"))entry.step=static_cast<int>(v->n);
            if(const auto* v=action.get(L"manual_only"))entry.manual=v->b;
            if(!entry.manual) {
                const auto* map=action.get(L"map");const auto* entity=action.get(L"entity");const auto* raw=action.get(L"raw");
                const auto* layer=action.get(L"layer");const auto* x=action.get(L"x");const auto* y=action.get(L"y");const auto* examine=action.get(L"examine");
                if(!map||!entity||!raw||!layer||!x||!y||!examine){definitions.clear();return false;}
                entry.goal.map_id=static_cast<int>(map->n);entry.goal.entity_id=static_cast<int>(entity->n);entry.goal.raw_name=raw->s;
                entry.goal.cell={static_cast<int>(layer->n),static_cast<int>(x->n),static_cast<int>(y->n)};entry.goal.examine=examine->b;
                if(family_at(entry.goal.map_id)!=definition.family||entry.goal.entity_id<0||entry.goal.raw_name.empty()||entry.goal.cell.layer<0||entry.goal.cell.layer>2){definitions.clear();return false;}
            }
            if(entry.id.empty()||entry.text.empty()||entry.step<1||entry.step>definition.count){definitions.clear();return false;}
            for(const auto& old:definition.actions)if(old.id==entry.id){definitions.clear();return false;}
            definition.actions.push_back(std::move(entry));
        }
        for(const auto& old:definitions)if(old.family==definition.family){definitions.clear();return false;}
        definitions.push_back(std::move(definition));
    }
    return definitions.size()==6;
}
Snapshot capture(bool explicit_request) {
    Snapshot snapshot;snapshot.map=field::current_map_id();
    // Only this finite family's native bits; no flag-bank/actor/world scan.
    const auto copy=[&](std::initializer_list<int> bits){for(const int bit:bits)snapshot.flags[bit]=field::map_state::scenario_flag(bit);};
    if(current==Family::Nepto)copy({31,32,33,34});
    else if(current==Family::Owen)copy({39,40,41,42,43,44,225});
    else if(current==Family::Tozus)copy({29});
    else if(current==Family::Hein)copy({55,56});
    else if(current==Family::Lake)copy({46,48,49,50});
    else if(current==Family::MagicCircle)copy({77,78,195,198});
    if(explicit_request) {
        if(current==Family::Nepto&&snapshot.flags[34]!=1&&snapshot.flags[33]!=1)mini=field::map_state::party_condition_state(401);
        if(current==Family::Owen&&snapshot.flags[44]!=1&&(snapshot.map==174||snapshot.flags[43]!=1))frog=field::map_state::party_condition_state(402);
        if(current==Family::Tozus&&snapshot.flags[29]!=1)mini=field::map_state::party_condition_state(401);
        if(current==Family::Hein)mini=field::map_state::party_condition_state(401);
        if(current==Family::Lake&&snapshot.flags[50]!=1&&snapshot.flags[48]!=1)frog=field::map_state::party_condition_state(402);
        if(current==Family::MagicCircle&&snapshot.flags[78]!=1)mini=field::map_state::party_condition_state(401);
    }
    snapshot.mini=mini;snapshot.frog=frog;return snapshot;
}
void refresh(bool explicit_request) {
    if(!ui::prefs::navigation_puzzle_guidance()){stop(false);return;}
    if(current==Family::None||!ready())return;
    const auto snapshot=capture(explicit_request);const auto decision=select(current,snapshot);
    receipt=decision.receipt;
    if(decision.action==Action::Outside){stop(false);return;}
    if(decision.action==Action::Finished){stop(false);ffpr::say(T(L"Puzzle guide finished"),explicit_request);return;}
    if(!explicit_request&&decision.action==last)return; // native operation identity, never text deduplication
    ++epoch;clear_preview();last=decision.action;
    if(decision.action==Action::Loading) {
        owned_step=owned_count=0;
        owned_line=T(explicit_request?L"Navigation data is loading":L"Repeat the guide to check the party's current form.");
        ffpr::say(owned_line,explicit_request);return;
    }
    const Definition* definition=nullptr;const Entry* entry=nullptr;
    for(const auto& row:definitions)if(row.family==current){definition=&row;for(const auto& candidate:row.actions)if(candidate.id==action_id(decision.action)){entry=&candidate;break;}break;}
    if(!definition||!entry){stop(false);ffpr::say(T(L"Navigation data is loading"),explicit_request);return;}
    const auto argument=entry->arg.rfind(L"modtext:",0)==0?T(entry->arg.substr(8)):entry->arg;
    auto instruction=entry->arg.empty()?T(entry->text):TF(entry->text,{argument});
    if((decision.action==Action::CastMini&&snapshot.mini==2)||(decision.action==Action::CastFrog&&snapshot.frog==2))
        instruction=TF(L"Cast {0} only on members who do not yet have its effect, then repeat the guide.",{argument});
    if((decision.action==Action::RemoveMini&&snapshot.mini==2)||(decision.action==Action::RemoveFrog&&snapshot.frog==2))
        instruction=TF(L"Cast {0} only on members who still have its effect, then repeat the guide.",{argument});
    owned_line=std::move(instruction);owned_step=entry->step;owned_count=definition->count;
    if(entry->manual){ffpr::say(presented_line(),explicit_request);return;}
    preview=entry->goal;const auto context=epoch;const auto family=current;auto line=presented_line();
    field::baked_nav::request_route(entry->goal,entry->goal.examine?field::TargetKind::Interact:field::TargetKind::Spot,L"",
        [context,family,line=std::move(line),explicit_request](field::PathInfo path) mutable {
            if(current!=family||epoch!=context||!ready()||!ui::prefs::navigation_puzzle_guidance())return;
            if(path.success) {if(!path.description.empty()&&path.description!=T(L"No movement needed"))line+=L". "+path.description;}
            else line+=L". "+(!path.reason.empty()?path.reason:T(L"No path to the next step"));
            if(ui::prefs::mod_warnings_enabled()&&!path.warning.empty())ffpr::say_sequence({path.warning,std::move(line)},explicit_request);
            else ffpr::say(line,explicit_request);
        },false);
}
void schedule() {
    if(current==Family::None||pending)return;
    pending=true;
    frame::every_frame([] {
        if(!pending||current==Family::None)return false;
        if(!ready())return true;
        pending=false;refresh(false);return false;
    },"FF3.PuzzleGuide.Ready",INT_MAX);
}
void invalidate_forms() {mini.reset();frog.reset();++epoch;}
int install() {
    events::on(events::Kind::MapChanging,[]{++epoch;pending=false;frame::cancel_label("FF3.PuzzleGuide.Ready");});
    events::on(events::Kind::MapChanged,[]{schedule();});
    events::on(events::Kind::BattleEntering,[]{invalidate_forms();clear_preview();});
    events::on(events::Kind::BattleEnd,[]{schedule();});
    events::on(events::Kind::MenuClosed,[]{invalidate_forms();schedule();});
    events::on(events::Kind::DialogueClosed,[]{schedule();});
    events::on(events::Kind::TitleScreen,[]{stop(false);});
    events::on(events::Kind::SceneLoaded,[]{if(field::scene_name()==L"Title")stop(false);});
    return 0;
}
core::Install installed{"FF3.PuzzleGuide",&install,514,game::kFF3};
}
bool supported(int map){
    // Preserve the independent optional Manor switch guide on completed visits
    // when the cave was entered without using that physical control.
    if(map==133&&field::map_state::scenario_flag(78)==1&&field::map_state::scenario_flag(198)!=1)return false;
    return family_at(map)!=Family::None;
}
bool active(){return current!=Family::None;}
bool repeatable(){return active()&&!queries::is("BattleStateHelper.IsInBattle")&&
    (menu_state::any_active()||queries::is("MessageChoice.IsOpen")||queries::is("MessageWindowPatches.IsInDialogue"));}
void next() {
    if(!ui::prefs::navigation_puzzle_guidance()){stop(false);ffpr::say(T(L"Navigation puzzle guidance is off"),true);return;}
    if(repeatable()&&!ready()){if(!owned_line.empty())ffpr::say(presented_line(),true);return;}
    if(!ready())return;
    if(!load()){ffpr::say(T(L"Navigation data is loading"),true);return;}
    if(!active()) {current=family_at(field::current_map_id());last=Action::Loading;receipt=-1;mini.reset();frog.reset();++epoch;}
    refresh(true);
}
void move() {
    if(!active()||!ready()||receipt<0)return;
    if(field::map_state::scenario_flag(receipt)==1)schedule();
}
void stop(bool announce) {
    const bool was_active=active();++epoch;pending=false;frame::cancel_label("FF3.PuzzleGuide.Ready");clear_preview();
    current=Family::None;receipt=-1;last=Action::Loading;mini.reset();frog.reset();owned_line.clear();owned_step=owned_count=0;
    if(announce&&was_active)ffpr::say(T(L"Puzzle guide stopped"),true);
}
}
