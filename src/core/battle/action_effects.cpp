// FFIII zero-value condition results have a visual effect but no damage digits.
// Observe the actual per-target result phase shared with native Miss rendering.
// An aura/renderer reinitialization is never an action or a result producer.
#include "action_effects.h"
#include "battle.h"
#include "battle_util.h"
#include "battle_message_line.h"
#include "../core.h"
#include "../hooks.h"
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

ref::Class kFf5Provider{"Serial.FF5.Calc", "UniqueUpdateParameterProvider", game::kFF5};
ref::Field fFf5Act{kFf5Provider, "battleActData"};
ref::Class kFf5Act{"Last.Battle", "BattleActData", game::kFF5};
ref::Field fFf5Abilities{kFf5Act, "abilityList"}, fFf5Targets{kFf5Act, "<TargetUnitDatas>k__BackingField"};
ref::Class kFf5Ability{"Last.Data.Master", "Ability", game::kFF5};
ref::Method mFf5AbilityId{kFf5Ability, "get_Id", 0};
ref::Class kFf5Player{"", "BattlePlayerData", game::kFF5};
ref::Field fPlayerCorps{kFf5Player, "<corps>k__BackingField"};
ref::Class kCorps{"Last.Data.User", "Corps", game::kFF5};
ref::Method mCorpsId{kCorps, "get_Id", 0};

using ParameterUpdate = void (*)(void*, void*, void*, void*, il2cpp::String*, const void*);
ParameterUpdate original_ff5_update;
constexpr int kFrontRow = 1, kBackRow = 2, kMagnet = 483;
uintptr_t magnet_reported_action = 0;

void retire_magnet() { magnet_reported_action = 0; }

int party_row(void* player)
{
    auto* corps = ref::rd<void*>(player, fPlayerCorps);
    return corps ? ref::call<int32_t>(mCorpsId, corps) : 0;
}

void ff5_parameter_update(void* self, void* attacker_unit, void* target, void* results_dict,
                          il2cpp::String* key, const void* mi)
{
    void* player = nullptr;
    uintptr_t action = 0;
    int previous = 0;
    if (key && frame::on_main_thread() && battle_presentation_ready())
        hooks::guarded("Battle.MagnetBefore", [&] {
            wchar_t name[24]{};
            if (!il2cpp::read_string(key, name, std::size(name)) || std::wstring_view(name) != L"POSITION_CHANGE") return;
            auto* act = ref::rd<void*>(self, fFf5Act);
            action = reinterpret_cast<uintptr_t>(act);
            if (!action || action == magnet_reported_action) return;
            auto* abilities = ref::rd<void*>(act, fFf5Abilities);
            if (unity::list_count(abilities) < 1 ||
                ref::call<int32_t>(mFf5AbilityId, unity::list_at(abilities, 0)) != kMagnet) return;
            // Magnet uses the first native selected target. The separate Row
            // command uses the callback's target argument and has its own path.
            auto* targets = ref::rd<void*>(act, fFf5Targets);
            if (unity::array_length(targets) < 1) return;
            auto* selected = unity::array_at(targets, 0);
            if (!kFf5Player.is(selected)) return;
            previous = party_row(selected);
            if (previous == kBackRow || previous == kFrontRow) player = selected;
        });
    original_ff5_update(self, attacker_unit, target, results_dict, key, mi);
    if (player && battle_presentation_ready()) hooks::guarded("Battle.MagnetResult", [&] {
        if (reinterpret_cast<uintptr_t>(ref::rd<void*>(self, fFf5Act)) != action ||
            magnet_reported_action == action || party_row(player) != kFrontRow) return;
        const auto name = unit_name(player);
        if (name.empty()) return;
        magnet_reported_action = action;
        ffpr::say(TF(previous == kBackRow ? L"Magnet moved {0} to front row" :
                                           L"Magnet target {0} already in front row", {name}), false);
    });
}

int install_ff5_formation_results()
{
    auto* method = find_method_param_type(kFf5Provider.get(), "UpdateParameter", 4, 0, "BattleUnitData");
    events::on(events::Kind::BattleEntering, &retire_magnet);
    events::on(events::Kind::BattleEnd, &retire_magnet);
    events::on(events::Kind::SceneLoaded, &retire_magnet);
    return hooks::attach_addr(il2cpp::method_pointer(method), "FFV.UpdateParameter.BattleUnitData",
        reinterpret_cast<void*>(&ff5_parameter_update), reinterpret_cast<void**>(&original_ff5_update));
}
core::Install ff5_formation_results{"Battle.FormationResult", &install_ff5_formation_results, 109, game::kFF5};
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
void finish_action() {
    if (!frame::on_main_thread()) return;
    if (game::is(game::Id::FF5)) retire_magnet();
    else finish();
}
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
