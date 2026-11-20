#include "battle.h"
#include "battle_internal.h"
#include "battle_util.h"
#include "boss_visuals.h"
#include "../displayed_ui.h"
#include "../frame.h"
#include "../modtext.h"
#include "../say.h"
#include "../unity.h"

namespace ffpr::battle {
namespace {
ref::Class kUnitInfo{"Last.Battle", "BattleUnitDataInfo"};
ref::Field fSpriteEntity{kUnitInfo, "<BattleSpriteEntity>k__BackingField"};
ref::Class kSpriteEntity{"Last.Battle", "BattleSpriteEntity"};
ref::Field fBodyRenderer{kSpriteEntity, "characterSR"};
ref::Class kMesh{"Last.Battle", "EntityMeshRenderer"};
ref::Field fRenderer{kMesh, "meshRenderer"};
ref::Field fSprite{kMesh, "sprite"};
ref::Class kPlayerEntity{"Last.Battle", "BattlePlayerEntity"};
ref::Field fCharacterStatus{kPlayerEntity, "characterStatusId"};
ref::Field fJob{kPlayerEntity, "jobId"};
ref::Class kWeaponPlayer{"Last.Battle", "BattlePlayerEntity", game::kFF1};
ref::Field fRightWeapon{kWeaponPlayer, "battleWeaponRight"};
ref::Class kWeapon{"Last.Battle", "BattleWeapon", game::kFF1};
ref::Field fWeaponRenderer{kWeapon, "BattleWeaponSR"};
ref::Field fWeaponAsset{kWeapon, "loadWeaponAsset"};
ref::Class kWeaponAsset{"Last.Battle", "LoadWeaponAsset", game::kFF1};
ref::Field fWeaponGroup{kWeaponAsset, "<groupName>k__BackingField"};
ref::Class kWeaponSpriteRenderer{"UnityEngine", "SpriteRenderer", game::kFF1};
ref::Method mWeaponSprite{kWeaponSpriteRenderer, "get_sprite", 0};
ref::Class kMasterManager{"Last.Data.Master", "MasterManager"};
ref::Class kAssets{"Last.Data", "CharacterAssetNameProvider"};
ref::Method mGetAssetName{kAssets, "GetAssetName", 4};

void* visible_body_sprite(void* entity)
{
    auto* body = ref::rd<void*>(entity, fBodyRenderer);
    auto* sprite = ref::rd<void*>(body, fSprite);
    return sprite && displayed::renderer_visible(ref::rd<void*>(body, fRenderer)) ? sprite : nullptr;
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

std::wstring visible_enemy_art(void* enemy)
{
    if (!kBattleEnemyData.is(enemy)) return {};
    auto* entity = ref::rd<void*>(unit_info(enemy), fSpriteEntity);
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
    return art_key(ref::str(ref::call_static<void*>(mGetAssetName, master, status, job, false)), L"bc_ff");
}

std::wstring visible_weapon_art(void* entity)
{
    if (!game::is(game::Id::FF1)) return {};
    auto* weapon = ref::rd<void*>(entity, fRightWeapon);
    auto* renderer = ref::rd<void*>(weapon, fWeaponRenderer);
    if (!displayed::renderer_visible(renderer) || !unity::alive(ref::call<void*>(mWeaponSprite, renderer))) return {};
    // Weapon pose names are shared across different blades. The group's full
    // art key, including its palette, identifies the assets for this renderer.
    auto* asset = ref::rd<void*>(weapon, fWeaponAsset);
    return art_key(ref::rd_str_bounded(asset, fWeaponGroup, 48), L"wp_ff");
}
} // namespace

bool read_focused_enemy_appearance()
{
    if (!frame::on_main_thread() || !battle_presentation_ready()) return false;
    const auto art = visible_enemy_art(focused_enemy_unit());
    if (art.empty()) return false;
    const auto key = L"Enemy appearance " + art;
    if (!modtext::has(key)) return false;
    say(T(key), true);
    return true;
}

void announce_acting_character_appearance()
{
    if (!frame::on_main_thread() || !battle_presentation_ready()) return;
    auto* unit = acting_party_unit();
    if (!kBattlePlayerData.is(unit)) return;
    auto* entity = ref::rd<void*>(unit_info(unit), fSpriteEntity);
    if (!kPlayerEntity.is(entity) || !visible_body_sprite(entity)) return;
    const auto character = L"Character appearance " + visible_player_art(entity);
    auto description = modtext::has(character) ? T(character) : std::wstring{};
    const auto weapon = L"Weapon appearance " + visible_weapon_art(entity);
    if (modtext::has(weapon)) {
        if (!description.empty()) description += L" ";
        description += T(weapon);
    }
    if (!description.empty()) say(description, true);
}
} // namespace ffpr::battle
