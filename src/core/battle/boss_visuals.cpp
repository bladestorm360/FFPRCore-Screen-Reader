#include "boss_visuals.h"
#include "battle_util.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../frame.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../ui/preferences.h"
#include "../unity.h"
#include <array>
#include <cmath>

namespace ffpr::battle::boss_visuals {
namespace {
constexpr auto kSpriteGames = game::kFF4 | game::kFF5;
constexpr auto kVisualGames = kSpriteGames | game::kFF6;
ref::Class kScriptedSprite{"Last.Battle.Function", "BattleFunction24", kSpriteGames};
ref::Method mCreateSprite{kScriptedSprite, "Create", 0};
ref::Class kDeformation{"Last.Battle.Function", "DeformationEnemyFunction", game::kFF5};
ref::Method mCreateDeformation{kDeformation, "Create", 0};
ref::Field fDeformedMonster{kDeformation, "monster"};
ref::Class kReplacement{"Last.Battle.Function", "BattleFunction37", kSpriteGames};
ref::Method mStartEnd{kReplacement, "StartEnd", 0};
ref::Field fOldUnits{kReplacement, "beforeUnits"}, fNewUnits{kReplacement, "afterUnits"};
ref::Class kFlowerSummon{"Last.Battle.Function", "BattleFunction20FF5", game::kFF5};
ref::Method mFlowerPlayedEnd{kFlowerSummon, "IsPlayedEnd", 0};
ref::Field fSummonedMonster{kFlowerSummon, "monster"}, fSummonedSlot{kFlowerSummon, "uniqueId"};
ref::Class kFunction{"Last.Battle", "BattleBaseFunction", kSpriteGames};
ref::Field fAct{kFunction, "<battleActData>k__BackingField"};
ref::Field fValue1{kFunction, "<Value1>k__BackingField"}, fValue2{kFunction, "<Value2>k__BackingField"};
ref::Field fValue3{kFunction, "<Value3>k__BackingField"};
ref::Class kAct{"Last.Battle", "BattleActData", kSpriteGames};
ref::Field fTargets{kAct, "<TargetUnitDatas>k__BackingField"};
ref::Class kSummon{"Last.Battle", "SummonActFunction", game::kFF4};
ref::Method mStartSummonEffect{kSummon, "StartEffect", 0}, mFadeBackFromSummon{kSummon, "StartWhiteInEffect", 0};
ref::Class kActFunction{"Last.Battle", "ActFunctionBase", game::kFF4};
ref::Field fSummonAction{kActFunction, "<BattleActData>k__BackingField"};
ref::Field fSummonAbilities{kAct, "abilityList"};
ref::Class kSummonAbility{"Last.Data.Master", "Ability", game::kFF4};
ref::Method mSummonAbilityId{kSummonAbility, "get_Id", 0};
ref::Class kInfo{"Last.Battle", "BattleUnitDataInfo", kVisualGames};
ref::Field fEntity{kInfo, "<BattleSpriteEntity>k__BackingField"};
ref::Class kEnemy{"Last.Battle", "BattleEnemyData", kSpriteGames};
ref::Field fMaster{kEnemy, "<MonsterMaster>k__BackingField"};
ref::Field fEnemySlot{kEnemy, "<UniqueId>k__BackingField"};
ref::Class kMonster{"Last.Data.Master", "Monster", kSpriteGames};
ref::Method mId{kMonster, "get_Id", 0}, mAsset{kMonster, "get_MonsterAssetId", 0};
ref::Class kSprite{"Last.Battle", "BattleSpriteEntity", kVisualGames};
ref::Field fBody{kSprite, "characterSR"}, fAnimState{kSprite, "AnimStateController"};
ref::Class kMesh{"Last.Battle", "EntityMeshRenderer", kVisualGames};
ref::Field fArt{kMesh, "sprite"}, fRenderer{kMesh, "meshRenderer"};
ref::Class kAnimator{"Last.Battle", "BattleEnemyAnimatorBase", kSpriteGames};
ref::Field fAnimatorEntity{kAnimator, "battleEnemyEntity"};
ref::Method mStageSprite{kAnimator, "ChangeSprite", 4};
ref::Method mCommitSprite{kAnimator, "ExecuteChangeSprite", 0}, mExitSprite{kAnimator, "ExitChangeSprite", 0};
ref::Class kAnimState{"Last.Battle", "AnimStateController", kVisualGames};
ref::Field fAnimator{kAnimState, "myBattleAnimatorBase"};
ref::Class kHide{"Last.Battle", "NoSelectConditionFunction", game::kFF6};
ref::Method mHide{kHide, "Start", 0}, mShow{kHide, "Remove", 0};
ref::Class kEnemyEntity{"Last.Battle", "BattleEnemyEntity", game::kFF6};
ref::Field fInitialAsset{kEnemyEntity, "monsterAssetId"};
ref::Class kFF6AnimState{"Last.Battle", "AnimStateController", game::kFF6};
ref::Field fStanding{kFF6AnimState, "currentStandingState"};
ref::Class kPaintingScript{"Last.Interpreter.Instructions.SystemCall", "Current", game::kFF6};
ref::Method mPainting{kPaintingScript, "ChadarnookSwich", 1};
ref::Class kFf5BattleScript{"Last.Interpreter.Instructions.SystemCall", "Current", game::kFF5};
ref::Method mSandwormMove{kFf5BattleScript, "SandWormMoveEnemy", 1};
ref::Method mSandwormReveal{kFf5BattleScript, "SandWormEffectAfter", 1};
ref::Method mNeoMove{kFf5BattleScript, "NeoExDeathMove", 1};
ref::Class kPaintMesh{"Last.Battle", "EntityMeshRenderer", game::kFF6};
ref::Method mSetFloat{kPaintMesh, "SetFloat", 2};
ref::Class kPlugManager{"Last.Battle", "BattlePlugManager", game::kFF5 | game::kFF6};
ref::Method mPlugInstance{kPlugManager, "Instance", 0}, mEnemyUnits{kPlugManager, "GetEnemyUnits", 0};
ref::Class kSwapPlugManager{"Last.Battle", "BattlePlugManager", kSpriteGames};
ref::Field fSwapStatus{kSwapPlugManager, "<BattleStatusControl>k__BackingField"};
ref::Class kSwapStatus{"Last.Battle", "BattleStatusControl", kSpriteGames};
ref::Field fSwapStatusInfo{kSwapStatus, "<BattleStatusControlInfo>k__BackingField"};
ref::Class kSwapStatusInfo{"Last.Battle", "BattleStatusControlInfo", kSpriteGames};
ref::Field fSwapEnemies{kSwapStatusInfo, "battleEnemyDatas"};

// These are identity tokens for native-owned objects. The only dereferences
// happen inside a native callback using its current owner. No worker or frame
// task retains a Unity object.
struct Request { uintptr_t animator = 0; uintptr_t entity = 0; SpriteChange change; };
std::array<Request, 8> requests;
unsigned pending = 0;
bool ready = false;
int painting_asset = 0;
bool sandworm_moved = false;
bool neo_exdeath_described = false;
int replacement_read_task = 0;
std::array<uintptr_t, 5> presented_flowers;

void cancel_replacement_read()
{
    if (replacement_read_task) frame::cancel(replacement_read_task);
    replacement_read_task = 0;
}

void close()
{
    ready = false;
    pending = 0;
    cancel_replacement_read();
}

void open()
{
    requests = {};
    painting_asset = 0;
    sandworm_moved = false;
    neo_exdeath_described = false;
    presented_flowers = {};
    ready = true;
}

bool observe() { return frame::on_main_thread() && ready; }
uintptr_t token(void* object) { return reinterpret_cast<uintptr_t>(object); }
void* entity_of(void* unit) { return ref::rd<void*>(unit_info(unit), fEntity); }
void* art_of(void* entity) { return ref::rd<void*>(ref::rd<void*>(entity, fBody), fArt); }
int monster_id(void* unit)
{
    if (!kBattleEnemyData.is(unit)) return 0;
    return ref::call<int32_t>(mId, ref::rd<void*>(unit, fMaster));
}

bool visible_body(void* entity)
{
    auto* mesh = ref::rd<void*>(entity, fBody);
    return ref::rd<void*>(mesh, fArt) && displayed::renderer_visible(ref::rd<void*>(mesh, fRenderer));
}

void speak(const wchar_t* key)
{
    if (key && ui::prefs::visual_descriptions_enabled()) say(T(key), false);
}

Request* request_for(uintptr_t animator)
{
    for (auto& request : requests)
        if (request.animator == animator) return &request;
    return nullptr;
}

void forget(Request& request)
{
    request = {};
    --pending;
}

// Match only the native staging call made by this action. Some conditions make
// Create skip the sprite write, leaving the animator's previous request intact.
thread_local Request creating;
struct CreateScope {
    Request previous = creating;
    explicit CreateScope(Request request) { creating = request; }
    ~CreateScope() { creating = previous; }
};

using Callback = void(*)(void*, const void*);
Callback o_create_sprite, o_create_deformation, o_commit_sprite, o_exit_sprite, o_start_end, o_hide, o_show;
Callback o_summon_effect, o_summon_fade_back;
using StageSprite = void(*)(void*, void*, void*, unity::Vec2, unity::Vec2, const void*);
StageSprite o_stage_sprite;

void d_stage_sprite(void* animator, void* sprite, void* shadow, unity::Vec2 face, unity::Vec2 mouth, const void* mi)
{
    o_stage_sprite(animator, sprite, shadow, face, mouth, mi);
    if ((!pending && !creating.animator) || !observe()) return;
    const auto owner = token(animator);
    if (auto* old = request_for(owner)) forget(*old);
    if (creating.animator != owner || !creating.change.key) return;
    creating.change.expected = token(sprite);
    if (!creating.change.changes_art()) return;
    if (auto* slot = request_for(0)) { *slot = creating; ++pending; }
}

void create_sprite(void* self, const void* mi, Callback original, bool changes_monster)
{
    Request request;
    if (observe()) hooks::guarded("BossVisual.SpriteAction", [&] {
        auto* act = ref::rd<void*>(self, fAct);
        auto* targets = ref::rd<void*>(act, fTargets);
        if (unity::array_length(targets) != 1) return;
        auto* unit = unity::array_at(targets, 0);
        auto* entity = entity_of(unit);
        if (!entity) return;
        auto* state = ref::rd<void*>(entity, fAnimState);
        auto* animator = ref::rd<void*>(state, fAnimator);
        if (ref::rd<void*>(animator, fAnimatorEntity) != entity) return;
        if (auto* old = request_for(token(animator))) forget(*old);
        const auto owner = monster_id(unit);
        if (!owner) return;
        const auto title = game::current();
        const auto value1 = ref::rd<int32_t>(self, fValue1);
        const auto value2 = ref::rd<int32_t>(self, fValue2);
        const auto value3 = ref::rd<int32_t>(self, fValue3);
        const wchar_t* key;
        if (changes_monster) {
            auto* monster = ref::rd<void*>(self, fDeformedMonster);
            if (!monster) return;
            key = deformation(title, owner, value1, ref::call<int32_t>(mAsset, monster), value3, value2);
        } else {
            key = scripted_sprite(title, owner, value1, value2, value3);
        }
        if (key) request = {token(animator), token(entity), {token(art_of(entity)), 0, key}};
    });
    CreateScope scope(request);
    original(self, mi);
}

void d_create_sprite(void* self, const void* mi) { create_sprite(self, mi, o_create_sprite, false); }
void d_create_deformation(void* self, const void* mi) { create_sprite(self, mi, o_create_deformation, true); }

// The summon is a different native sequence from the boss's sprite swaps.
// Its attack phase starts after the whiteout, and the next fade brings the
// battle back. Both callbacks own the same action and run once per cast.
void describe_bahamut_summon(void* self, const wchar_t* key)
{
    if (!observe() || !ui::prefs::visual_descriptions_enabled()) return;
    hooks::guarded("BossVisual.BahamutSummon", [&] {
        auto* act = ref::rd<void*>(self, fSummonAction);
        auto* abilities = ref::rd<void*>(act, fSummonAbilities);
        if (unity::list_count(abilities) != 1) return;
        const auto id = ref::call<int32_t>(mSummonAbilityId, unity::list_at(abilities, 0));
        if (id == 75 || id == 459) speak(key);
    });
}

void d_summon_effect(void* self, const void* mi)
{
    o_summon_effect(self, mi);
    describe_bahamut_summon(self, L"BossVisual.BahamutSummon");
}

void d_summon_fade_back(void* self, const void* mi)
{
    o_summon_fade_back(self, mi);
    describe_bahamut_summon(self, L"BossVisual.BahamutSummonEnd");
}

// The animation first draws the previous art. Execute/Exit apply the new body
// texture and rebuild the mesh. Check presentation only after that commit.
void commit_sprite(void* animator, const void* mi, Callback original)
{
    original(animator, mi);
    if (!observe() || !pending) return;
    auto* request = request_for(token(animator));
    if (!request) return;
    hooks::guarded("BossVisual.SpritePresented", [&] {
        auto* entity = ref::rd<void*>(animator, fAnimatorEntity);
        const auto actual = token(art_of(entity));
        if (token(entity) != request->entity || actual != request->change.expected) {
            forget(*request);
            return;
        }
        if (!request->change.presented(actual, visible_body(entity))) return;
        const auto* key = request->change.key;
        forget(*request);
        speak(key);
    });
}

void d_commit_sprite(void* self, const void* mi) { commit_sprite(self, mi, o_commit_sprite); }
void d_exit_sprite(void* self, const void* mi) { commit_sprite(self, mi, o_exit_sprite); }

void* current_enemy_list()
{
    auto* manager = il2cpp::read_static_ptr(kSwapPlugManager.get(), "instance");
    auto* status = ref::rd<void*>(manager, fSwapStatus);
    auto* info = ref::rd<void*>(status, fSwapStatusInfo);
    return ref::rd<void*>(info, fSwapEnemies);
}

void observe_replacement_reveal(std::array<uintptr_t, 2> expected, const wchar_t* key)
{
    if (!ui::prefs::visual_descriptions_enabled()) return;
    cancel_replacement_read();
    // StartEnd queues destruction of the old body. Reacquire the replacement
    // from the current status list after the renderer has had a frame to draw.
    // The deferred check owns only an identity token, never a native pointer.
    constexpr int max_frames = 4;
    replacement_read_task = frame::every_frame([expected, key, remaining = max_frames]() mutable {
        const bool finished = hooks::guarded_r("BossVisual.ReplacementVisible", true, [&] {
            if (!observe() || !ui::prefs::visual_descriptions_enabled()) return true;
            auto* enemies = current_enemy_list();
            const int count = expected[1] ? 2 : 1;
            if (unity::list_count(enemies) != count) return true;
            bool drawn = true;
            for (int i = 0; i < count; ++i) {
                auto* unit = unity::list_at(enemies, i);
                if (token(unit) != expected[i]) return true;
                drawn &= visible_body(entity_of(unit));
            }
            if (!drawn) return false;
            speak(key);
            return true;
        });
        if (!finished && --remaining > 0) return true;
        replacement_read_task = 0;
        return false;
    }, "BossVisual.ReplacementVisible", max_frames);
}

void d_start_end(void* self, const void* mi)
{
    const wchar_t* key = nullptr;
    std::array<uintptr_t, 2> incoming{};
    if (observe()) hooks::guarded("BossVisual.PartyIdentity", [&] {
        const int party = ref::rd<int32_t>(self, fValue1);
        if (party != 445 && party != 519 && party != 601 && party != 654 && party != 655 && (party < 540 || party > 542)) return;
        auto* before = ref::rd<void*>(self, fOldUnits);
        auto* after = ref::rd<void*>(self, fNewUnits);
        if (party == 601 && game::is(game::Id::FF5) && unity::array_length(after) == 2) {
            // The surviving Gargoyle invokes this exact party only after
            // losing its partner. Both restored bodies must actually appear.
            auto* first = unity::array_at(after, 0);
            auto* second = unity::array_at(after, 1);
            if (first != second && monster_id(first) == 345 && monster_id(second) == 345) {
                key = L"BossVisual.GargoylesRise";
                incoming = {token(first), token(second)};
            }
            return;
        }
        if (unity::array_length(before) != 1 || unity::array_length(after) != 1) return;
        auto* old_unit = unity::array_at(before, 0);
        auto* new_unit = unity::array_at(after, 0);
        key = replacement(game::current(), party, monster_id(old_unit), monster_id(new_unit));
        if (key) incoming = {token(new_unit), 0};
    });
    o_start_end(self, mi);
    if (key && observe()) hooks::guarded("BossVisual.PartyShown", [&] {
        observe_replacement_reveal(incoming, key);
    });
}

using PlayedEnd = bool(*)(void*, const void*);
PlayedEnd o_flower_played_end;

bool d_flower_played_end(void* self, const void* mi)
{
    const bool finished = o_flower_played_end(self, mi);
    if (!finished || !observe() || !ui::prefs::visual_descriptions_enabled()) return finished;
    hooks::guarded("BossVisual.FlowerSummoned", [&] {
        // The native completion can also return true without creating an
        // enemy. Require its actual slot and a drawn Flower in the live list.
        const int slot = ref::rd<int32_t>(self, fSummonedSlot);
        if (slot <= 0) return;
        const int id = ref::call<int32_t>(mId, ref::rd<void*>(self, fSummonedMonster));
        if (id < 320 || id > 324) return;
        auto* enemies = current_enemy_list();
        const int count = unity::list_count(enemies);
        if (count < 1 || count > 9) return;
        for (int i = 0; i < count; ++i) {
            auto* unit = unity::list_at(enemies, i);
            if (ref::rd<int32_t>(unit, fEnemySlot) != slot || monster_id(unit) != id) continue;
            auto& presented = presented_flowers[id - 320];
            if (presented != token(unit) && visible_body(entity_of(unit))) {
                presented = token(unit);
                speak(L"BossVisual.DragonFlower");
            }
            return;
        }
    });
    return finished;
}

void hide_change(void* self, const void* mi, Callback original, bool withdrawing)
{
    void* state = nullptr;
    void* entity = nullptr;
    int before = -1;
    int asset = 0;
    if (observe()) hooks::guarded("BossVisual.ShellIdentity", [&] {
        entity = entity_of(function_unit(self));
        asset = ref::rd<int32_t>(entity, fInitialAsset);
        if (!no_select_actor(asset, withdrawing) || (withdrawing && !visible_body(entity))) return;
        state = ref::rd<void*>(entity, fAnimState);
        if (!state) return;
        before = ref::rd<int32_t>(state, fStanding);
    });
    original(self, mi);
    if (state && observe()) hooks::guarded("BossVisual.NoSelectShown", [&] {
        if (ref::rd<void*>(entity, fAnimState) != state) return;
        const auto* key = no_select_pose(asset, before, ref::rd<int32_t>(state, fStanding), withdrawing);
        if (key && (withdrawing || visible_body(entity))) speak(key);
    });
}

void d_hide(void* self, const void* mi) { hide_change(self, mi, o_hide, true); }
void d_show(void* self, const void* mi) { hide_change(self, mi, o_show, false); }

// The FFVI script applies alpha 1 to the newly exposed body, then the shadow.
// Only watch these writes while that exact script is on the native stack.
thread_local void** painting_reveal = nullptr;
struct PaintingScope {
    void** previous = painting_reveal;
    explicit PaintingScope(void*& body) { painting_reveal = &body; }
    ~PaintingScope() { painting_reveal = previous; }
};
using SetFloat = void(*)(void*, void*, float, const void*);
SetFloat o_set_float;
void d_set_float(void* mesh, void* property, float value, const void* mi)
{
    o_set_float(mesh, property, value, mi);
    if (!painting_reveal || *painting_reveal || value != 1.0f) return;
    hooks::guarded("BossVisual.PaintingAlpha", [&] {
        if (ref::str(property) == L"_Alpha") *painting_reveal = mesh;
    });
}

using Script = int32_t(*)(void*, const void*);
Script o_painting;
Script o_sandworm_move, o_sandworm_reveal, o_neo_move;
void* enemy_units()
{
    auto* manager = ref::call_static<void*>(mPlugInstance);
    return ref::call<void*>(mEnemyUnits, manager);
}

void* sandworm_body()
{
    auto* enemies = enemy_units();
    const int count = unity::array_length(enemies);
    if (count < 1 || count > 9) return nullptr;
    for (int index = 0; index < count; ++index) {
        auto* unit = unity::array_at(enemies, index);
        if (monster_id(unit) == 294) return entity_of(unit);
    }
    return nullptr;
}

int32_t d_sandworm_move(void* interpreter, const void* mi)
{
    if (!observe() || sandworm_moved) return o_sandworm_move(interpreter, mi);
    void* transform = nullptr;
    unity::Vec3 before{};
    hooks::guarded("BossVisual.SandwormBefore", [&] {
        transform = unity::transform(sandworm_body());
        if (transform) before = unity::local_position(transform);
    });
    const auto result = o_sandworm_move(interpreter, mi);
    if (transform && observe()) hooks::guarded("BossVisual.SandwormMoved", [&] {
        if (unity::transform(sandworm_body()) != transform) return;
        const auto after = unity::local_position(transform);
        sandworm_moved = std::abs(after.x-before.x) > 0.01f || std::abs(after.y-before.y) > 0.01f;
    });
    return result;
}

int32_t d_sandworm_reveal(void* interpreter, const void* mi)
{
    const auto result = o_sandworm_reveal(interpreter, mi);
    if (result == 0 && observe() && sandworm_moved) hooks::guarded("BossVisual.SandwormShown", [&] {
        sandworm_moved = false;
        if (visible_body(sandworm_body())) speak(L"BossVisual.SandwormMoved");
    });
    return result;
}

int32_t d_neo_move(void* interpreter, const void* mi)
{
    const auto result = o_neo_move(interpreter, mi);
    if (!observe() || neo_exdeath_described || !ui::prefs::visual_descriptions_enabled()) return result;
    // The later movement command runs after the native replacement has loaded
    // its assets. Wait for the one visible body in the new party. The other
    // targetable parts have transparent sprites.
    hooks::guarded("BossVisual.NeoExdeathShown", [&] {
        auto* enemies = enemy_units();
        const int count = unity::array_length(enemies);
        if (count < 1 || count > 9) return;
        for (int index = 0; index < count; ++index) {
            auto* unit = unity::array_at(enemies, index);
            if (monster_id(unit) != 388 || !visible_body(entity_of(unit))) continue;
            neo_exdeath_described = true;
            speak(L"BossVisual.NeoExdeath");
            break;
        }
    });
    return result;
}

int32_t d_painting(void* interpreter, const void* mi)
{
    if (!observe()) return o_painting(interpreter, mi);
    void* body = nullptr;
    const auto result = [&] {
        PaintingScope scope(body);
        return o_painting(interpreter, mi);
    }();
    if (!body || !observe()) return result;
    hooks::guarded("BossVisual.PaintingShown", [&] {
        auto* enemies = enemy_units();
        const int count = unity::array_length(enemies);
        if (count < 1 || count > 9) return;
        for (int index = 0; index < count; ++index) {
            auto* entity = entity_of(unity::array_at(enemies, index));
            if (ref::rd<void*>(entity, fBody) != body) continue;
            const int asset = ref::rd<int32_t>(entity, fInitialAsset);
            if ((asset == 300 || asset == 317) && visible_body(entity)) {
                const auto* key = painting(painting_asset, asset);
                painting_asset = asset;
                speak(key);
            }
            break;
        }
    });
    return result;
}

int install()
{
    if (!game::in(kVisualGames)) return 0;
    events::on(events::Kind::BattleEntering, &close);
    events::on(events::Kind::BattleStart, &open);
    events::on(events::Kind::BattleEnd, &close);
    events::on(events::Kind::TitleScreen, &close);
    if (game::is(game::Id::FF6)) {
        const int hide = hooks::attach(mHide, &d_hide, &o_hide);
        const int show = hooks::attach(mShow, &d_show, &o_show);
        const int painting = hooks::attach(mPainting, &d_painting, &o_painting);
        return hide + show + painting + hooks::attach(mSetFloat, &d_set_float, &o_set_float);
    }
    const int sprite = hooks::attach(mCreateSprite, &d_create_sprite, &o_create_sprite);
    const int stage = hooks::attach(mStageSprite, &d_stage_sprite, &o_stage_sprite);
    const int commit = hooks::attach(mCommitSprite, &d_commit_sprite, &o_commit_sprite);
    const int party = hooks::attach(mStartEnd, &d_start_end, &o_start_end);
    const int shared = sprite + stage + commit + party;
    // FFIV folds ExecuteChangeSprite and ExitChangeSprite to the same body.
    if (game::is(game::Id::FF4)) {
        const int summon = hooks::attach(mStartSummonEffect, &d_summon_effect, &o_summon_effect);
        return shared + summon + hooks::attach(mFadeBackFromSummon, &d_summon_fade_back, &o_summon_fade_back);
    }
    const int exit = hooks::attach(mExitSprite, &d_exit_sprite, &o_exit_sprite);
    const int deform = hooks::attach(mCreateDeformation, &d_create_deformation, &o_create_deformation);
    const int sandworm = hooks::attach(mSandwormMove, &d_sandworm_move, &o_sandworm_move);
    return shared + exit + deform + sandworm + hooks::attach(mSandwormReveal, &d_sandworm_reveal, &o_sandworm_reveal) +
           hooks::attach(mNeoMove, &d_neo_move, &o_neo_move) +
           hooks::attach(mFlowerPlayedEnd, &d_flower_played_end, &o_flower_played_end);
}
core::Install registration{"Battle.BossVisuals", &install, 117};
}

bool revealed_painting_art(int asset)
{
    return observe() && painting_asset != 0 && painting_asset == asset;
}

} // namespace ffpr::battle::boss_visuals
