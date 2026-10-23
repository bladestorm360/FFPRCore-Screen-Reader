// Urgent defense observations come from the game's actual field-entity update
// producer. Only the two authored defense maps and their bounded enemy IDs are
// inspected. Native pointers remain in the sliced capture closure; route costs,
// closest-party advice and formatting use the independent owned worker lane.
#include "story_model.h"
#include "../../core/core.h"
#include "../../core/hooks.h"
#include "../../core/worker_job.h"
#include "../../core/field/field_internal.h"
#include "../../core/field/capture_budget.h"
#include "../../core/ui/preferences.h"
#include "../../core/say.h"
#include "../../core/log.h"
#include <algorithm>

namespace ffpr::ff6::defense_watch {
namespace {
// Native FF6 FF2550 tail-calls FieldCharaEntity.UpdateEntity (FE1510).
// Observe the completed NPC producer, avoiding every other field entity's base
// update and dependence on which intermediate overrides call that base.
ref::Method mUpdate{field::r::kFieldNonPlayer,"UpdateEntity",0};
using Update=void(*)(void*,const void*);Update o_update;
int map=0;bool pending=false,dirty=false;
std::optional<field::baked_route::Cell> protected_cell;
std::array<std::optional<field::baked_route::Cell>,64> cells;
std::array<bool,64> inside_range{},announced{};
worker_job::Owner work;
bool ready(){return queries::is("GameStatePatches.IsFieldNavigationReady")&&!queries::is("GameStatePatches.IsInputTransitionBlocked")&&!queries::is("BattleStateHelper.IsInBattle")&&!menu_state::any_active();}
bool enemy(int id){return map==318?(id>=18&&id<=23):map==19?(id==24||(id>=38&&id<=48)):false;}
void check();
void retire(){work.cancel();frame::cancel_label("FF6.Defense.Capture");pending=dirty=false;}
void reset(){retire();map=field::current_map_id();if(map!=318&&map!=19)map=0;protected_cell.reset();cells={};inside_range={};announced={};}
struct Input {story::Snapshot state;std::shared_ptr<field::baked_nav::ExitSnapshot> navigation;std::array<bool,64> announced;std::function<bool()> cancelled;double requested=0,native_ms=0,captured=0;};
struct Result {field::baked_nav::DefenseAdvice advice;double requested=0,native_ms=0,captured=0,queue_ms=0,compute_ms=0;};
void check() {
    if(!map||!ready()||!field::baked_nav::prepared())return;
    if(pending){dirty=true;return;}
    // Party/battle admission only interrupts epochs. Retire old callbacks here,
    // after the game has returned to the native field owner and is ready.
    work.cancel();frame::cancel_label("FF6.Defense.Capture");
    auto input=std::make_shared<Input>();input->requested=frame::now();input->state=story::capture_snapshot();
    input->navigation=field::baked_nav::begin_exit_capture(false,true);
    if(!input->navigation)return;
    input->native_ms=(frame::now()-input->requested)*1000;
    input->announced=announced;
    const auto epoch=work.epoch();const auto generation=epoch->load();
    input->cancelled=[epoch,generation]{return epoch->load()!=generation;};
    auto cursor=std::make_shared<field::EntityCursor>();pending=true;
    frame::every_frame([input,cursor]()mutable{
        if(input->cancelled()||input->state.map!=map||!ready()){pending=false;return false;}
        const auto slice=frame::now();field::CaptureBudget budget;bool more=true;
        while(budget.available()) {
            void* entity=nullptr;if(!(more=cursor->next(entity)))break;
            if(entity)field::baked_nav::capture_exit_obstacle(*input->navigation,entity);
        }
        input->native_ms+=(frame::now()-slice)*1000;
        if(more)return true;
        const auto finish=frame::now();field::baked_nav::finish_exit_capture(*input->navigation);
        input->native_ms+=(frame::now()-finish)*1000;
        input->captured=frame::now();
        const auto origin=input->state.player;const int captured_map=input->state.map;
        worker_job::start(work,std::move(*input),[](Input& input){
            const auto began=frame::now();Result result;
            result.requested=input.requested;result.native_ms=input.native_ms;result.captured=input.captured;
            result.queue_ms=(began-input.captured)*1000;
            auto plan=story::plan(input.state);
            if(!plan.defense)return result;
            auto& enemies=plan.defense->enemies;
            enemies.erase(std::remove_if(enemies.begin(),enemies.end(),[&](const auto& goal){return goal.entity_id<0||goal.entity_id>=64||input.announced[static_cast<size_t>(goal.entity_id)];}),enemies.end());
            result.advice=field::baked_nav::defense_advice(*input.navigation,*plan.defense,input.cancelled);
            result.compute_ms=(frame::now()-began)*1000;return result;
        },[origin,captured_map](Result result){
            pending=false;
            if(captured_map!=map||!ready())return;
            // A scene can complete without immediately changing maps. Current
            // scenario flags must still authorize an alert at publication.
            const auto flags=field::map_state::snapshot();
            if(flags.scenario[map==318?13:137]!=1||flags.scenario[map==318?14:138]!=0)return;
            auto& advice=result.advice;
            if(advice.intercept&&enemy(advice.intercept->entity_id)) {
                const auto id=static_cast<size_t>(advice.intercept->entity_id);
                field::baked_route::Cell at;
                const bool stale=!field::baked_nav::player_cell(field::map_handle(),field::player(),at)||!(at==origin)||
                    !cells[id]||!(*cells[id]==advice.intercept->cell);
                if(stale)dirty=true;
                else if(inside_range[id]&&!announced[id]&&!advice.warning.empty()&&ui::prefs::speech_enabled()) {
                    announced[id]=true;
                    log_::write("[Defense] map=%d native=%.3f capture=%.3f queue=%.3f compute=%.3f total=%.3f oldest_age=%.3f ms",map,result.native_ms,
                        (result.captured-result.requested)*1000,result.queue_ms,result.compute_ms,(frame::now()-result.requested)*1000,(frame::now()-result.requested)*1000);
                    ffpr::say(advice.warning,true,{speech::Feature::Directions,result.requested,result.requested});
                }
            }
            if(dirty){dirty=false;check();}
        },background::Lane::Navigation);
        return false;
    },"FF6.Defense.Capture",INT_MAX);
}
void proximity(int id) {
    if(!protected_cell||!cells[static_cast<size_t>(id)])return;
    const auto at=*cells[static_cast<size_t>(id)];
    const bool inside=std::max(std::abs(at.x-protected_cell->x),std::abs(at.y-protected_cell->y))<=3;
    const bool entered=inside&&!inside_range[static_cast<size_t>(id)];
    inside_range[static_cast<size_t>(id)]=inside;
    if(!inside)announced[static_cast<size_t>(id)]=false;
    if(entered)check();
}
void observed(void* self) {
    if(!map||!field::r::kFieldNonPlayer.is(self)||!field::entity_is_active(self))return;
    void* property=field::entity_property(self);if(!property)return;
    const int id=ref::rd<int32_t>(property,field::r::fPropEntityId);
    const bool protect=id==(map==318?32:64);
    if(!protect&&!enemy(id))return;
    if(!ready())return;
    field::baked_route::Cell at;
    if(!field::baked_nav::cell_at(field::map_handle(),unity::local_position(unity::transform(self)),at))return;
    if(protect) {
        if(protected_cell&&*protected_cell==at)return;
        protected_cell=at;for(int i=0;i<64;++i)if(enemy(i))proximity(i);return;
    }
    if(cells[static_cast<size_t>(id)]&&*cells[static_cast<size_t>(id)]==at)return;
    cells[static_cast<size_t>(id)]=at;proximity(id);
}
void d_update(void* self,const void* mi){o_update(self,mi);if(map)hooks::guarded("FF6.Defense.Move",[=]{observed(self);});}
int install() {
    events::on(events::Kind::MapChanging,[]{retire();map=0;});
    events::on(events::Kind::MapChanged,[]{reset();});
    events::on(events::Kind::TitleScreen,[]{retire();map=0;});
    events::on(events::Kind::BattleEntering,[]{work.interrupt();});
    events::on(events::Kind::PartyChanging,[]{work.interrupt();pending=dirty=false;});
    events::on(events::Kind::BattleEnd,[]{pending=dirty=false;protected_cell.reset();cells={};inside_range={};});
    return hooks::attach(mUpdate,&d_update,&o_update)?1:0;
}
core::Install s_install{"FF6.DefenseWatch",&install,516,game::kFF6};
}
}
