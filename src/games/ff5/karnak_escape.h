#pragma once

namespace ff5::karnak {
enum class Route { EquipmentThenElixirs, EquipmentOnly, AllChests };
inline constexpr char preference[] = "KarnakEscapeRoute";
inline constexpr int exit_seconds = 180;
inline constexpr bool is_castle(int map) { return map >= 95 && map <= 111; }
}
