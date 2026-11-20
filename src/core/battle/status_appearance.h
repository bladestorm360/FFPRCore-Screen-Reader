#pragma once
#include <string>
#include <string_view>

namespace ffpr::battle {
// Manual appearance input on the ready battle owner. Reads the selected
// body's attached presentation and returns owned localized prose.
std::wstring visible_status_appearance(void* entity, void* unit_info);
std::wstring visible_party_status_icons(void* player);
std::wstring transformed_body_art(void* sprite, std::wstring_view base_art, std::wstring_view pose);
std::wstring rendered_shield_art(void* sprite, std::wstring_view pose);
}
