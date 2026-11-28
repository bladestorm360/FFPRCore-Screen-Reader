#include "battle.h"
#include "battle_internal.h"
#include "battle_util.h"
#include "boss_visuals.h"
#include "status_appearance.h"
#include "../displayed_ui.h"
#include "../frame.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../ui/preferences.h"
#include "../unity.h"
#include <array>

namespace ffpr::battle {
namespace {
ref::Class kUnitInfo{"Last.Battle", "BattleUnitDataInfo"};
ref::Field fSpriteEntity{kUnitInfo, "<BattleSpriteEntity>k__BackingField"};
ref::Class kSpriteEntity{"Last.Battle", "BattleSpriteEntity"};
ref::Field fBodyRenderer{kSpriteEntity, "characterSR"};
ref::Class kMountedEntity{"Last.Battle", "BattleSpriteEntity", game::kFF6};
ref::Field fRidingArmor{kMountedEntity, "rideOnMagicArmor"}, fArmor{kMountedEntity, "battleMagicArmor"};
ref::Class kMagicArmor{"Last.Battle", "BattleMagicArmor", game::kFF6};
ref::Field fArmorBody{kMagicArmor, "entityMeshRenderer"};
ref::Class kMesh{"Last.Battle", "EntityMeshRenderer"};
ref::Field fRenderer{kMesh, "meshRenderer"};
ref::Field fSprite{kMesh, "sprite"};
ref::Class kPlayerEntity{"Last.Battle", "BattlePlayerEntity"};
ref::Field fCharacterStatus{kPlayerEntity, "characterStatusId"};
ref::Field fJob{kPlayerEntity, "jobId"};
ref::Field fRightWeapon{kPlayerEntity, "battleWeaponRight"}, fLeftWeapon{kPlayerEntity, "battleWeaponLeft"};
ref::Field fShield{kPlayerEntity, "battleShield"};
ref::Class kShield{"Last.Battle", "BattleShield"};
ref::Field fShieldRenderer{kShield, "shieldSR"};
ref::Class kWeapon{"Last.Battle", "BattleWeapon"};
ref::Field fWeaponRenderer{kWeapon, "BattleWeaponSR"};
ref::Field fWeaponAsset{kWeapon, "loadWeaponAsset"};
ref::Class kWeaponAsset{"Last.Battle", "LoadWeaponAsset"};
ref::Field fWeaponGroup{kWeaponAsset, "<groupName>k__BackingField"};
ref::Class kWeaponSpriteRenderer{"UnityEngine", "SpriteRenderer"};
ref::Method mWeaponSprite{kWeaponSpriteRenderer, "get_sprite", 0};
ref::Class kMasterManager{"Last.Data.Master", "MasterManager"};
ref::Class kAssets{"Last.Data", "CharacterAssetNameProvider"};
ref::Method mGetAssetName{kAssets, "GetAssetName", 4};
ref::Class kBattlePlug{"Last.Battle", "BattlePlugManager", game::kFF5};
ref::Field fStatusControl{kBattlePlug, "<BattleStatusControl>k__BackingField"};
ref::Class kStatusControl{"Last.Battle", "BattleStatusControl", game::kFF5};
ref::Field fStatusControlInfo{kStatusControl, "<BattleStatusControlInfo>k__BackingField"};
ref::Class kStatusControlInfo{"Last.Battle", "BattleStatusControlInfo", game::kFF5};
ref::Field fLiveParty{kStatusControlInfo, "battlePlayerDatas"};

struct OldAppearance {
    uintptr_t identity = 0;
    int pending = 0;
};
std::array<OldAppearance, 4> old_appearances;
constexpr char kOldAppearanceTask[] = "Battle.OldAppearance";

void* live_old_target(uintptr_t identity)
{
    auto* plug = il2cpp::read_static_ptr(kBattlePlug.get(), "instance");
    auto* status = ref::rd<void*>(plug, fStatusControl);
    auto* info = ref::rd<void*>(status, fStatusControlInfo);
    auto* party = ref::rd<void*>(info, fLiveParty);
    const int count = unity::list_count(party);
    if (count < 1 || count > static_cast<int>(old_appearances.size())) return nullptr;
    for (int i = 0; i < count; ++i) {
        auto* unit = unity::list_at(party, i);
        if (reinterpret_cast<uintptr_t>(unit) == identity) return unit;
    }
    return nullptr;
}

void* visible_mesh_sprite(void* body)
{
    auto* sprite = ref::rd<void*>(body, fSprite);
    return sprite && displayed::renderer_visible(ref::rd<void*>(body, fRenderer)) ? sprite : nullptr;
}

void* visible_body_sprite(void* entity) { return visible_mesh_sprite(ref::rd<void*>(entity, fBodyRenderer)); }

std::wstring magic_armor_appearance(void* entity)
{
    if (!game::is(game::Id::FF6) || !ref::rd<bool>(entity, fRidingArmor)) return {};
    // Native battle initialization loads bc_ff6_p022 into this separate
    // walker renderer. The rider keeps their own visible costume.
    auto* armor = ref::rd<void*>(entity, fArmor);
    if (!visible_mesh_sprite(ref::rd<void*>(armor, fArmorBody))) return {};
    const auto* key = L"Status appearance body bc_ff6_p022";
    return modtext::has(key) ? T(key) : std::wstring{};
}

std::wstring art_key(std::wstring name, std::wstring_view prefix)
{
    if (name.size() > 48) return {};
    for (auto& ch : name) {
        if (ch >= L'A' && ch <= L'Z') ch += L'a' - L'A';
        if (!((ch >= L'a' && ch <= L'z') || (ch >= L'0' && ch <= L'9') || ch == L'_'))
            return {};
    }
    return name.starts_with(prefix) ? name : std::wstring{};
}

std::wstring visible_enemy_art(void* entity)
{
    // The displayed sprite encodes the group and palette. The monster master
    // can still name the opening art after a transformation.
    auto art = art_key(unity::name(visible_body_sprite(entity)), L"mn_ff");
    if (game::is(game::Id::FF6)) {
        const int painting = art == L"mn_ff6_127_c00" ? 300 : art == L"mn_ff6_144_c00" ? 317 : 0;
        if (painting && !boss_visuals::revealed_painting_art(painting)) return {};
    }
    return art;
}

std::wstring visible_player_art(void* entity)
{
    // Battle character sprite names are pose names shared across outfits.
    // This asset provider uses the status and job from the live renderer owner
    // and resolves the same initialized body table used by the native loader.
    const auto status = ref::rd<int32_t>(entity, fCharacterStatus);
    const auto job = ref::rd<int32_t>(entity, fJob);
    if (status <= 0 || job < 0) return {};
    auto* master = ref::instance(kMasterManager);
    if (!master) return {};
    auto art = art_key(ref::str(ref::call_static<void*>(mGetAssetName, master, status, job, false)), L"");
    // FFVI also assigns a field costume to a battle body. The same key has
    // separate field and battle wording selected by the current context.
    return art.starts_with(L"bc_ff") || art.starts_with(L"mo_ff") ? art : std::wstring{};
}

std::wstring visible_weapon_art(void* weapon)
{
    auto* renderer = ref::rd<void*>(weapon, fWeaponRenderer);
    if (!displayed::renderer_visible(renderer) || !unity::alive(ref::call<void*>(mWeaponSprite, renderer))) return {};
    // Weapon pose names are shared across different blades. The group's full
    // art key, including its palette, identifies the assets for this renderer.
    auto* asset = ref::rd<void*>(weapon, fWeaponAsset);
    return art_key(ref::rd_str_bounded(asset, fWeaponGroup, 48), L"wp_ff");
}

std::wstring visible_shield_art(void* entity)
{
    auto* shield = ref::rd<void*>(entity, fShield);
    auto* renderer = ref::rd<void*>(shield, fShieldRenderer);
    if (!displayed::renderer_visible(renderer)) return {};
    auto* sprite = ref::call<void*>(mWeaponSprite, renderer);
    return unity::alive(sprite) ? rendered_shield_art(sprite, unity::name(sprite)) : std::wstring{};
}

void append(std::wstring& description, const std::wstring& detail)
{
    if (detail.empty()) return;
    if (!description.empty()) description += L" ";
    description += detail;
}
} // namespace

bool ff5_old_appearance_visible(void* player)
{
    if (!game::is(game::Id::FF5) || !frame::on_main_thread() ||
        !battle_presentation_ready() || !kBattlePlayerData.is(player)) return false;
    auto* entity = ref::rd<void*>(unit_info(player), fSpriteEntity);
    if (!kPlayerEntity.is(entity)) return false;
    auto* sprite = visible_body_sprite(entity);
    if (!sprite) return false;
    const auto sprite_name = unity::name(sprite);
    return rendered_ff5_old_sprite(sprite, visible_player_art(entity), sprite_name, art_key(sprite_name, L""));
}

void observe_ff5_old_appearance(void* player)
{
    if (!frame::on_main_thread() || !battle_presentation_ready() ||
        !kBattlePlayerData.is(player)) return;
    const auto identity = reinterpret_cast<uintptr_t>(player);
    OldAppearance* available = nullptr;
    for (auto& row : old_appearances) {
        if (row.identity == identity) return; // One cue per visible application.
        if (!row.identity && !available) available = &row;
    }
    if (!available) return;
    auto& row = *available;
    row.identity = identity;
    // Native condition Add can precede the first drawn aged pose. Reacquire
    // the live player each time. One short task per affected slot, no scene scan.
    constexpr int max_frames = 90;
    row.pending = frame::every_frame([&row, identity, ticks = 0]() mutable {
        if (!battle_presentation_ready() || row.identity != identity) {
            row = {};
            return false;
        }
        if (ticks++ % 3 == 0 && hooks::guarded_r("Battle.OldAppearance", false, [&] {
            auto* unit = live_old_target(identity);
            if (!ff5_old_appearance_visible(unit)) return false;
            say(unit_name(unit) + L": " + T(L"Old"), false);
            return true;
        })) {
            row.pending = 0;
            return false;
        }
        if (ticks < max_frames) return true;
        row = {};
        return false;
    }, kOldAppearanceTask, max_frames);
}

void retire_ff5_old_appearance(void* player)
{
    const auto identity = reinterpret_cast<uintptr_t>(player);
    for (auto& row : old_appearances) {
        if (row.identity != identity) continue;
        if (row.pending) frame::cancel(row.pending);
        row = {};
        return;
    }
}

void reset_ff5_old_appearance()
{
    for (auto& row : old_appearances) {
        if (row.pending) frame::cancel(row.pending);
        row = {};
    }
}

bool read_focused_enemy_appearance()
{
    if (!frame::on_main_thread() || !battle_presentation_ready() || !ui::prefs::visual_descriptions_enabled()) return false;
    auto* enemy = focused_enemy_unit();
    if (!kBattleEnemyData.is(enemy)) return false;
    auto* info = unit_info(enemy);
    auto* entity = ref::rd<void*>(info, fSpriteEntity);
    const auto art = visible_enemy_art(entity);
    const auto key = L"Enemy appearance " + art;
    auto description = !art.empty() && modtext::has(key) ? T(key) : std::wstring{};
    append(description, visible_status_appearance(entity, info));
    if (description.empty()) return false;
    say(description, true);
    return true;
}

void announce_acting_character_appearance()
{
    if (!frame::on_main_thread() || !battle_presentation_ready() || !ui::prefs::visual_descriptions_enabled()) return;
    auto* unit = acting_party_unit();
    if (!kBattlePlayerData.is(unit)) return;
    auto* info = unit_info(unit);
    auto* entity = ref::rd<void*>(info, fSpriteEntity);
    if (!kPlayerEntity.is(entity)) return;
    auto* sprite = visible_body_sprite(entity);
    const auto armor = magic_armor_appearance(entity);
    if (!sprite) {
        auto description = armor;
        append(description, visible_status_appearance(entity, info));
        if (!description.empty()) say(description, true);
        return;
    }
    const auto base_art = visible_player_art(entity);
    const auto sprite_name = unity::name(sprite);
    const auto pose = art_key(sprite_name, L"");
    const auto alternate = transformed_body_art(sprite, base_art, sprite_name);
    const auto body_key = L"Status appearance body " + alternate;
    const auto character = L"Character appearance " + base_art;
    auto description = modtext::has(character) ? T(character) : std::wstring{};
    if (!alternate.empty()) {
        // A pose can keep the normal pixels even within a transformed set.
        // The data supplies pose-specific wording where the change is drawn.
        if (modtext::has(body_key)) description = T(body_key);
        else if (alternate != base_art && modtext::has(L"Character appearance " + alternate))
            description = T(L"Character appearance " + alternate);
        else if (game::is(game::Id::FF5) && modtext::has(L"Status appearance changed " + alternate + L" " + pose))
            description = T(alternate.ends_with(L"_o") ? L"Status appearance body old" : L"Status appearance body zombie");
    }
    if (pose == L"down_00") append(description, T(L"Status appearance pose down"));
    if (pose == L"dying_00" || pose == L"dyingr_00") append(description, T(L"Status appearance pose dying"));
    for (auto* hand : {&fRightWeapon, &fLeftWeapon}) {
        const auto weapon = L"Weapon appearance " + visible_weapon_art(ref::rd<void*>(entity, *hand));
        if (modtext::has(weapon)) append(description, T(weapon));
    }
    const auto shield = L"Weapon appearance " + visible_shield_art(entity);
    if (modtext::has(shield)) append(description, T(shield));
    append(description, armor);
    append(description, visible_status_appearance(entity, info));
    append(description, visible_party_status_icons(unit));
    if (!description.empty()) say(description, true);
}
} // namespace ffpr::battle
