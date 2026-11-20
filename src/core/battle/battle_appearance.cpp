#include "battle.h"
#include "battle_internal.h"
#include "battle_util.h"
#include "boss_visuals.h"
#include "status_appearance.h"
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
ref::Field fRightWeapon{kPlayerEntity, "battleWeaponRight"}, fLeftWeapon{kPlayerEntity, "battleWeaponLeft"};
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

void append(std::wstring& description, const std::wstring& detail)
{
    if (detail.empty()) return;
    if (!description.empty()) description += L" ";
    description += detail;
}
} // namespace

bool read_focused_enemy_appearance()
{
    if (!frame::on_main_thread() || !battle_presentation_ready()) return false;
    auto* enemy = focused_enemy_unit();
    if (!kBattleEnemyData.is(enemy)) return false;
    auto* entity = ref::rd<void*>(unit_info(enemy), fSpriteEntity);
    const auto art = visible_enemy_art(entity);
    if (art.empty()) return false;
    const auto key = L"Enemy appearance " + art;
    auto description = modtext::has(key) ? T(key) : std::wstring{};
    append(description, visible_status_appearance(entity));
    if (description.empty()) return false;
    say(description, true);
    return true;
}

void announce_acting_character_appearance()
{
    if (!frame::on_main_thread() || !battle_presentation_ready()) return;
    auto* unit = acting_party_unit();
    if (!kBattlePlayerData.is(unit)) return;
    auto* entity = ref::rd<void*>(unit_info(unit), fSpriteEntity);
    if (!kPlayerEntity.is(entity)) return;
    auto* sprite = visible_body_sprite(entity);
    if (!sprite) return;
    const auto character = L"Character appearance " + visible_player_art(entity);
    auto description = modtext::has(character) ? T(character) : std::wstring{};
    const auto pose = art_key(unity::name(sprite), L"");
    if (pose == L"down_00") append(description, T(L"Status appearance pose down"));
    if (pose == L"dying_00" || pose == L"dyingr_00") append(description, T(L"Status appearance pose dying"));
    for (auto* hand : {&fRightWeapon, &fLeftWeapon}) {
        const auto weapon = L"Weapon appearance " + visible_weapon_art(ref::rd<void*>(entity, *hand));
        if (modtext::has(weapon)) append(description, T(weapon));
    }
    append(description, visible_status_appearance(entity));
    append(description, visible_party_status_icons(unit));
    if (!description.empty()) say(description, true);
}
} // namespace ffpr::battle
