#pragma once
#include "baked_router.h"

// Authored navigation marker, not a native Event. Exact paired terrain and
// native surfacing position evidence: docs/re/DOGA_SURFACE_MARKER_20261029.md.
namespace field::nautilus_surface {
inline constexpr baked_route::Cell village_cell{1,150,203}; // native surface GotoMap171 -> Village151
inline constexpr int map = 3, destination = 4, entity = -300014;
inline constexpr baked_route::Cell cell{1,150,204};
inline constexpr const wchar_t* identity = L"ffpr:surface:doga_village";
inline bool matches(int game, int current_map, const baked_route::Cell& at,
                    int id, const std::wstring& raw) {
    return game==3 && current_map==map && at==cell && id==entity && raw==identity;
}
}
