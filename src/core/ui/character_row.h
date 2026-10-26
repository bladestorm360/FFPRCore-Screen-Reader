#pragma once
#include <string>

namespace ui::character_row {
/// Called only by a native row-focus producer. No polling, scene lookup or input
/// interception. Cursor reparenting/visibility owns I-key detail validity.
void announce(void* content, void* cursor, std::wstring line, void* overview_window = nullptr);
std::wstring details();
void clear();
}
