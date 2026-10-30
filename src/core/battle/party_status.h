#pragma once
#include <string>
#include <vector>

namespace ffpr::battle::party_status {
// Read the visible row's rooted ally conditions afresh, plus its native red-HP
// danger equivalent. Native pointers stay on the input/game owner; only returned
// strings enter speech. The rotating UI status column is not current-state proof.
std::vector<std::wstring> current_lines(void* player, void* view);
}
