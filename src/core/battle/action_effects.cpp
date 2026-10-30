// FFIII zero-value condition results have a visual effect but no damage digits.
// Observe the actual per-target result phase shared with native Miss rendering.
// An aura/renderer reinitialization is never an action or a result producer.
#include "action_effects.h"
#include "battle.h"
#include "battle_util.h"
#include "battle_message_line.h"
#include "../core.h"
#include "../modtext.h"
#include "../unity.h"
#include <array>
#include <atomic>

namespace ffpr::battle::action_effects {
namespace {
enum class Kind {None,Boost,Attract,Guard};
ref::Class kAct{"Last.Battle","BattleActData",game::kFF3};
ref::Field fAbilities{kAct,"abilityList"},fCommand{kAct,"<Command>k__BackingField"};
ref::Field fAttacker{kAct,"<AttackUnitData>k__BackingField"};
ref::Field fActType{kAct,"<CurrentActType>k__BackingField"};
ref::Class kCommand{"Last.Data.Master","Command",game::kFF3};
ref::Method mCommandAbility{kCommand,"get_AbilityId",0};
ref::Class kAbility{"Last.Data.Master","Ability",game::kFF3};
ref::Method mAbilityId{kAbility,"get_Id",0};
ref::Class kFunction{"Last.Battle","BattleBaseFunction",game::kFF3};
ref::Field fFunctionAct{kFunction,"<battleActData>k__BackingField"};
constexpr size_t capacity=32;
struct Result {void* function=nullptr;void* act=nullptr;void* target=nullptr;};
std::array<Result,capacity> results;
size_t count=0;
void* attacker=nullptr; // compared only during its native-owned action lifetime
void* root_action=nullptr;
Kind current=Kind::None;
std::wstring actor,cue;
std::atomic<uint64_t> epoch{1};
bool enabled=false;
bool child_identity_ready=false;
bool artillery=false;

int ability(void* act) {
    if(!act)return 0;
    void* list=ref::rd<void*>(act,fAbilities);const int n=unity::list_count(list);
    if(n==1) {
        void* item=unity::list_at(list,0);
        return item?ref::call<int32_t>(mAbilityId,item):0;
    }
    if(n!=0)return 0;
    void* command=ref::rd<void*>(act,fCommand);
    return command?ref::call<int32_t>(mCommandAbility,command):0;
}
void finish() {current=Kind::None;attacker=root_action=nullptr;count=0;artillery=false;}
void retire() {finish();epoch.fetch_add(1,std::memory_order_acq_rel);}
struct Payload {std::wstring line;uint64_t generation;};
void submit(Payload payload) {
    if(epoch.load(std::memory_order_acquire)!=payload.generation)return;
    // Same owner/UI-frame speech producer as rendered Miss. Translation was
    // copied once at action start; no data-worker backlog can reorder targets.
    ffpr::say(payload.line,false);
}
int install() {
    if(!game::is(game::Id::FF3))return 0;
    enabled=fFunctionAct.get()>=0&&fAbilities.get()>=0&&fCommand.get()>=0&&fAttacker.get()>=0&&
        mAbilityId.ptr()&&mCommandAbility.ptr();
    child_identity_ready=fActType.get()>=0&&fAbilities.get()>=0&&fCommand.get()>=0&&mAbilityId.ptr();
    if(enabled) {
        events::on(events::Kind::BattleEntering,&retire);
        events::on(events::Kind::BattleEnd,&retire);
        events::on(events::Kind::SceneLoaded,&retire);
    }
    return 0; // metadata only: the existing CreateDamageView hook owns this phase
}
core::Install registration{"Battle.ActionEffects",&install,109,game::kFF3};
}
bool redundant_boost_use(void* act) {
    if(!game::is(game::Id::FF3)||!frame::on_main_thread()||!child_identity_ready||!act)return false;
    // Native BattleFunction46.CreateAbility creates Command=null/NotDelete5.
    // Content137/139 reuse parent94's name;138 has no name.140 instead displays
    // Self-Destruct and has an independent explosion result: preserve both.
    if(ref::rd<int32_t>(act,fActType)!=5||ref::rd<void*>(act,fCommand))return false;
    const int id=ability(act);return id>=137&&id<=139;
}
void observe_action(void* act,const std::wstring& name,const std::wstring& description) {
    if(!frame::on_main_thread()||!enabled)return;
    finish();const int id=ability(act);
    // Content399 / Ability141 / MessageConclusion508 all name MSG_SYSTEM_023.
    // Reuse this existing bounded identity read; artillery is not an aura result.
    artillery=id==141;
    if(id==94||id==137||id==139)current=Kind::Boost;
    else if(id==96)current=Kind::Attract;
    else if(id==87)current=Kind::Guard;
    if(current==Kind::None)return;
    attacker=ref::rd<void*>(act,fAttacker);
    root_action=act;
    actor=name.substr(0,256);
    const auto label=description.substr(0,256);
    cue=current==Kind::Boost?TF(L"A glowing aura appears around {0}",{actor}):
        current==Kind::Guard?TF(L"A glowing barrier appears around {0}",{actor}):
        label.empty()?T(L"Affected"):TF(L"Affected by {0}",{label});
    if(cue.size()>512)cue.resize(512);
}
bool invincible_artillery_action() {
    return game::is(game::Id::FF3)&&frame::on_main_thread()&&artillery;
}
bool invincible_artillery_result(void* value) {
    return game::is(game::Id::FF3)&&frame::on_main_thread()&&enabled&&value&&
        ref::call<int32_t>(mAbilityId,value)==141;
}
void finish_action() {if(frame::on_main_thread())finish();}
void observe_result(void* function,void* target,int value,int hit_type) {
    // Native Hit(0) with zero value suppresses digits; Miss(2) keeps its own
    // actual visible Miss producer. Non/Zero/recovery are not successful cues.
    if(!frame::on_main_thread()||!enabled||current==Kind::None||value!=0||hit_type!=0||!function||!target||!in_battle())return;
    void* act=ref::rd<void*>(function,fFunctionAct);
    if(!act||!attacker||ref::rd<void*>(act,fAttacker)!=attacker)return;
    const int id=ability(act);
    // Boost's random function owns a new child BattleActData. Only137/139
    // request the verified Focus visual;94/138/140 cannot imply that glow.
    if(current==Kind::Boost ? ((id!=137&&id!=139)||target!=attacker) :
       current==Kind::Guard ? (id!=87||act!=root_action||target!=attacker) :
       (id!=96||act!=root_action||!kBattleEnemyData.is(target)))return;
    for(size_t i=0;i<count;++i)
        if(results[i].function==function&&results[i].act==act&&results[i].target==target)return;
    if(count==capacity)return;
    // Guard a native function/target result event, never its text. The native
    // attack/result dictionaries can address the same target in one phase.
    results[count++]={function,act,target};
    const bool self_effect=current==Kind::Boost||current==Kind::Guard;
    auto name=self_effect?actor:unit_name(target).substr(0,256);
    if(name.empty())return;
    // Self effects use one localized sentence with the actor in its natural position.
    // Draw retains its per-enemy result heading, shared with native Miss.
    Payload payload{self_effect?cue:name+L": "+cue,epoch.load(std::memory_order_relaxed)};
    // The Miss observer also starts on the next UI frame. Preserve action-name
    // pairing when pending, without a new renderer poll, timeout or native wait.
    const int frames=message_line::pending_action.set||message_line::pending_message.set?message_line::kPairFrames:1;
    frame::after_frames(frames,[payload=std::move(payload)]()mutable{submit(std::move(payload));},"Battle.TargetEffectResult");
}
}
