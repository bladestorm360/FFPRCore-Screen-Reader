#pragma once

#include <cstdint>
#include "../game.h"

namespace ffpr::battle::boss_visuals {

// Manual appearance reads can reuse the native reveal. Before a confirmed
// alpha write or outside the current battle there is no visible painting.
bool revealed_painting_art(int asset);

// Identity of an audited visible form, derived from the encounter and its
// authored transition. A null key leaves the normal game speech in charge.
constexpr const wchar_t* scripted_sprite(game::Id title, int owner, int mode, int asset, int effect)
{
    if (title == game::Id::FF5 && (owner == 281 || owner == 282) && mode == 2 && asset == 246 && effect == 0)
        return L"BossVisual.WingsClosed";
    if (title != game::Id::FF4) return nullptr;
    switch (owner) {
    case 178:
        if (asset == 191 && mode == 1) return L"BossVisual.WindUp";
        if (asset == 190 && (mode == 2 || mode == 0)) return L"BossVisual.WindDown";
        break;
    case 165:
        if (asset == 176) return L"BossVisual.BombSwells";
        break;
    case 163:
        // Form 173 has its own native weakening message.
        if (effect == 167 && ((asset >= 168 && asset <= 172) || asset == 174))
            return L"BossVisual.Tentacle";
        break;
    case 171:
        // The lightning break has the same destination art in mode 0.
        if (asset == 182 && mode == 3) return L"BossVisual.WaterDown";
        break;
    case 188:
        if (effect == 167 && asset == 201) return L"BossVisual.CloakOpen";
        // The mode resets also request the closed art. The presentation guard
        // speaks only if they actually move Rubicante from open to closed.
        if (asset == 202 && (effect == 167 || (effect == 0 && (mode == 0 || mode == 1))))
            return L"BossVisual.CloakClosed";
        break;
    case 189:
        if (asset == 204) return L"BossVisual.SwordUp";
        if (asset == 203) return L"BossVisual.SwordDown";
        break;
    case 193:
        // Each face has a distinct battle sprite. The intervening AI mode
        // resets reapply the same face and do not announce a new one.
        if (asset == 212 && mode == 1) return L"BossVisual.AsuraCalm";
        if (asset == 213 && mode == 0) return L"BossVisual.AsuraAngry";
        if (asset == 214 && mode == 2) return L"BossVisual.AsuraGrinning";
        break;
    }
    return nullptr;
}

constexpr const wchar_t* deformation(game::Id title, int owner, int monster, int asset, int mode, int effect)
{
    if (title != game::Id::FF5 || (owner != 281 && owner != 282) || effect != 101) return nullptr;
    if (monster == 281 && asset == 245 && (mode == 0 || mode == 3)) return L"BossVisual.WingsOpen";
    if (monster == 282 && asset == 246 && (mode == 1 || mode == 2)) return L"BossVisual.WingsClosed";
    return nullptr;
}

constexpr const wchar_t* replacement(game::Id title, int party, int before, int after)
{
    if (title == game::Id::FF5 && before >= 290 && before <= 292 && before != after) {
        if (party == 445 && after == 290) return L"BossVisual.FlamePerson";
        if (party == 654 && after == 291) return L"BossVisual.FlameHand";
        if (party == 655 && after == 292) return L"BossVisual.FlameVortex";
    }
    if (title == game::Id::FF4) {
        if (party == 540 && before == 228 && after == 227) return L"BossVisual.LordFire";
        if (party == 541 && before == 194 && after == 195) return L"BossVisual.LordWater";
        if (party == 542 && before == 195 && after == 228) return L"BossVisual.LordWind";
    }
    return nullptr;
}

constexpr const wchar_t* shell(int asset, bool withdrawing)
{
    if (asset == 271) return withdrawing ? L"BossVisual.YmirIn" : L"BossVisual.YmirOut";
    if (asset == 272) return withdrawing ? L"BossVisual.WhelkIn" : L"BossVisual.WhelkOut";
    return nullptr;
}

constexpr bool no_select_actor(int asset, bool withdrawing)
{
    return asset == 271 || asset == 272 || asset == 311 ||
           (!withdrawing && (asset == 278 || asset == 279));
}

constexpr const wchar_t* no_select_pose(int asset, int before, int after, bool withdrawing)
{
    if (before == after) return nullptr;
    // Opera Ultros uses dedicated hide and show effects. Other Ultros
    // encounters share the art but do not enter these two poses.
    if (asset == 311) {
        if (withdrawing && after == 18) return L"BossVisual.UltrosSlipsAway";
        if (!withdrawing && after == 19) return L"BossVisual.UltrosReappears";
        return nullptr;
    }
    if (!withdrawing && after == 7) {
        if (asset == 278) return L"BossVisual.ShivaTakesOver";
        if (asset == 279) return L"BossVisual.IfritTakesOver";
    }
    // The two shell encounters can use ordinary or dedicated dissolve art.
    const bool shell_pose = withdrawing ? after == 4 || after == 12 : after == 7 || after == 11;
    return shell_pose ? shell(asset, withdrawing) : nullptr;
}

constexpr const wchar_t* painting(int previous, int asset)
{
    if (asset == previous) return nullptr;
    if (asset == 300) return L"BossVisual.Goddess";
    // The opening demon has native dialogue. Later switches are silent.
    if (asset == 317 && previous != 0) return L"BossVisual.Demon";
    return nullptr;
}

struct SpriteChange {
    uintptr_t before = 0;
    uintptr_t expected = 0;
    const wchar_t* key = nullptr;
    constexpr bool changes_art() const { return key && before && expected && before != expected; }
    constexpr bool presented(uintptr_t after, bool visible) const
    {
        return changes_art() && expected == after && visible;
    }
};

} // namespace ffpr::battle::boss_visuals
