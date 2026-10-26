#pragma once
#include <string>
#include <vector>

namespace ffpr::battle::party_status {
// Read a visible battle row's danger color and, when its condition column is
// hidden, the same ally's currently rendered body/effects. Native pointers stay
// on the input/game owner; only the returned strings enter speech.
std::vector<std::wstring> extra_lines(void* player, void* view, bool body_conditions, void* condition_controller);
}
