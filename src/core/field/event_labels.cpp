#include "event_labels.h"
#include "field_internal.h"
#include "../modtext.h"

namespace field {
bool event_route_only(const std::wstring& raw) {
    const auto* identity=data::game_json("eventlabels").get(raw);
    return identity && identity->is_string() && (identity->s==L"World map transition" || identity->s==L"@route-only");
}
namespace {
ref::Class kDefaultProperty{"Last.Map","PropertyDefaultEntity"};
ref::Field fAsset{kDefaultProperty,"<AssetId>k__BackingField"};
std::wstring label_text(const std::wstring& label)
{
    // This reviewed metadata is pruned by both navigation producers. Keep a
    // safe generic fallback for callers that format before the baked filter.
    if (label == L"@route-only") return T(L"Event trigger");
    // Exact reviewed actor names reuse the game-specific entity catalog. This
    // payload is a proper name, never a script description or future outcome.
    constexpr std::wstring_view prefix = L"@name:";
    if (label.starts_with(prefix)) {
        const auto key = label.substr(prefix.size());
        const auto* entry = data::game_json("translation").get(key);
        if (entry && entry->is_object()) {
            if (modtext::lang() == L"ja") return key;
            const auto* value = entry->get(modtext::lang());
            if (!value) value = entry->get(L"en");
            if (value && value->is_string() && !value->s.empty()) return value->s;
        }
        return T(L"Interactive object");
    }
    return T(label);
}
}
std::wstring safe_event_label(void* entity, const std::wstring& type)
{
    if (type == L"ToLayer") return T(L"Layer transition");
    if (type == L"Warp Tile") return T(L"Warp Tile");
    void* property = entity_property(entity);
    if(kDefaultProperty.is(property)) {
        const auto asset=ref::rd<int32_t>(property,fAsset);
        const auto* label=data::game_json("eventlabels").get(L"@sprite:"+std::to_wstring(asset));
        if(label && label->is_string() && !label->s.empty())return label_text(label->s);
    }
    const auto raw = property ? ref::rd_str(property, r::fPropName) : std::wstring();
    const int entity_id = property ? ref::rd<int32_t>(property, r::fPropEntityId) : -1;
    const int object_type = property ? ref::rd<int32_t>(property, r::fPropObjectType) : -1;
    return safe_event_label(raw, type, current_map_id_or_user_data(), entity_id, object_type);
}
std::wstring safe_event_label(const std::wstring& raw, const std::wstring& type)
{
    return safe_event_label(raw, type, 0, -1);
}
std::wstring safe_event_label(const std::wstring& raw, const std::wstring& type, int map_id, int entity_id, int object_type)
{
    if (type == L"ToLayer") return T(L"Layer transition");
    if (type == L"Warp Tile") return T(L"Warp Tile");
    const auto& labels = data::game_json("eventlabels");
    if (map_id > 0 && entity_id >= 0) {
        const auto key = L"@object:" + std::to_wstring(map_id) + L":" + std::to_wstring(entity_id) +
            L":" + std::to_wstring(object_type) + L":" + raw;
        if (const auto* label = labels.get(key); label && label->is_string() && !label->s.empty()) return label_text(label->s);
    }
    if (const auto* label = labels.get(raw); label && label->is_string() && !label->s.empty()) return label_text(label->s);
    // Unknown future identifiers never fall back to raw names or unrevealed dialogue.
    return type == L"Event" ? T(L"Event trigger") : T(L"Interactive object");
}
}
