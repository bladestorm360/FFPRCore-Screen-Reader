// Current ally conditions for H, the number row and ally targeting. The user's
// October29 follow-up explicitly requests live state, independent of rotating
// UI labels and transient effect animations. No status cache or hook is needed.
// Native source, bounds and verification: docs/re/BATTLE_LIVE_STATUS_20261029.md.
#include "party_status.h"
#include "battle_util.h"
#include "../displayed_ui.h"
#include "../il2cpp.h"
#include "../modtext.h"
#include "../refs.h"
#include "../unity.h"
#include <array>
#include <cmath>
#include <cstdint>

namespace ffpr::battle::party_status {
namespace {
ref::Class kColors{"Last.UI", "TextColors"};
ref::Class kGraphic{"UnityEngine.UI", "Graphic"};
ref::Method mColor{kGraphic, "get_color", 0};
constexpr int kSourceLimit = 128, kTotalLimit = kSourceLimit * 2;
struct Color { float r, g, b, a; };

bool critical_visible(void* view)
{
    void* hp = displayed::field(view, "currentHpText");
    const auto value = displayed::text(hp);
    if (value.empty() || value == L"0") return false;
    int packed = 0;
    // Critical functions can exist without a current danger presentation. Keep
    // the native red-HP equivalent rather than inventing our own HP threshold.
    if (!il2cpp::read_static_int(kColors.get(), "Red", packed)) return false;
    const auto bits = static_cast<uint32_t>(packed);
    if ((bits >> 24) == 0) return false;
    const auto color = ref::call<Color>(mColor, hp);
    const auto matches = [](float actual, unsigned byte) {
        return std::isfinite(actual) && std::fabs(actual - byte / 255.0f) < 0.5f / 255.0f;
    };
    return matches(color.r, bits & 255) && matches(color.g, (bits >> 8) & 255) &&
           matches(color.b, (bits >> 16) & 255) && color.a > 0;
}
}

std::vector<std::wstring> current_lines(void* player, void* view)
{
    std::vector<std::wstring> lines;
    if (critical_visible(view)) lines.push_back(T(L"Critical"));
    if (!player || !kBattlePlayerData.is(player)) return lines;
    void* info = unit_info(player);
    if (!info) return lines;
    void* current = current_condition_list(info_parameter(info));
    void* functions = info_functions(info);
    const int nc = unity::list_count(current), nf = unity::list_count(functions);
    // Match the existing native removal producer's two ready sources. Reject
    // malformed/oversized storage before any traversal; never retain pointers,
    // invoke an allocating ConfirmedConditionList union, or enumerate renderers.
    if (nc < 0 || nc > kSourceLimit || nf < 0 || nf > kSourceLimit) return lines;
    std::array<int, kTotalLimit> represented{};
    int count = 0;
    auto add = [&](void* condition) {
        if (!condition || condition_type(condition) == 4) return; // HP color owns Critical.
        const int id = condition_id(condition);
        if (id <= 0) return;
        for (int i = 0; i < count; ++i) if (represented[i] == id) return;
        represented[count++] = id;
        // Existing localized native names/fallbacks exclude unnamed bookkeeping
        // conditions. Identity, never localized text, coalesces stacked entries.
        auto name = condition_name(condition);
        if (!name.empty()) lines.push_back(std::move(name));
    };
    for (int i = 0; i < nc; ++i) add(unity::list_at(current, i));
    for (int i = 0; i < nf; ++i) add(function_condition(unity::list_at(functions, i)));
    return lines;
}
}
