#include "battle.h"
#include "battle_internal.h"
#include "battle_util.h"
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

std::wstring visible_enemy_art(void* enemy)
{
    if (!kBattleEnemyData.is(enemy)) return {};
    auto* entity = ref::rd<void*>(unit_info(enemy), fSpriteEntity);
    auto* body = ref::rd<void*>(entity, fBodyRenderer);
    auto* sprite = ref::rd<void*>(body, fSprite);
    if (!sprite || !displayed::renderer_visible(ref::rd<void*>(body, fRenderer))) return {};

    // The displayed sprite name encodes the group and palette. A monster's
    // master record can still name its opening art after a transformation.
    auto name = unity::name(sprite);
    if (name.size() > 48) return {};
    for (auto& ch : name) {
        if (ch >= L'A' && ch <= L'Z') ch += L'a' - L'A';
        if (!((ch >= L'a' && ch <= L'z') || (ch >= L'0' && ch <= L'9') || ch == L'_'))
            return {};
    }
    return name.starts_with(L"mn_ff") ? name : std::wstring{};
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
} // namespace ffpr::battle
