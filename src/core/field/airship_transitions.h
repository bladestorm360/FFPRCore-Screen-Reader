#pragma once
#include "baked_map.h"
namespace field::air_transitions {
// Reviewed native Event identities and exact transportation masks; no game pointers.
struct Rule { int map,script,destination; const wchar_t* raw; unsigned mask; };
const Rule* match(int game_id,int map,int script,const std::wstring& raw);
const Rule* match(int game_id,int map,const baked::Object& object);
bool possible_raw(int game_id,int map,const std::wstring& raw);
bool possible_aircraft(int game_id,int map,int transport_id);
bool accepts(const Rule& rule,int transport_id);
// Saronia's native story entry is separate from continent-edge consolidation.
// It ends at an active airborne scene trigger, not a town's foot entrance.
const Rule* saronia_entry(int game_id,int map,int script,const std::wstring& raw);
const Rule* saronia_entry(int game_id,int map,const baked::Object& object);
}
