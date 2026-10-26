#pragma once
#include <string>

// FF6 parks the other corps in FieldController.partyEntityList, outside the
// ordinary map entity lists. These helpers are native-owner observations only.
namespace field::party_entities {
bool next_native(void* controller,int& index,void*& entity);
bool is_native(void* entity);
int native_slot(void* entity); // zero-based slot, -1 when not currently owned
// Native followers exchange places with the player. They remain talk targets,
// independently of FFVI's separate, physically blocking parked corps.
bool is_companion(void* entity);
std::wstring companion_name(void* entity);
}
