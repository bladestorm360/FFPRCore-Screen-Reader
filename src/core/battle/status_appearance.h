#pragma once
#include <string>

namespace ffpr::battle {
// Manual appearance input on the ready battle owner. Reads the selected
// body's attached presentation and returns owned localized prose.
std::wstring visible_status_appearance(void* entity);
std::wstring visible_party_status_icons(void* player);
}
