#pragma once
#include <string>
namespace field {
// Reviewed routine borders and passive bookkeeping remain native data, never
// scene destinations. Real conditional travel retains its named destination.
// This queries immutable identity metadata, independent of grouping/language.
bool event_route_only(const std::wstring& raw);
// Exact native actor contacts that present and initiate a boss encounter.
std::wstring reviewed_boss_name(const std::wstring& raw, int map_id, int entity_id, int object_type);
std::wstring safe_event_label(void* entity, const std::wstring& type);
// Copied raw identifier only: immutable label table, safe on an owned worker.
std::wstring safe_event_label(const std::wstring& raw, const std::wstring& type);
// Copied native map/entity IDs resolve otherwise shared/empty developer names.
std::wstring safe_event_label(const std::wstring& raw, const std::wstring& type, int map_id, int entity_id, int object_type = -1);
}
