#pragma once
#include <string>
namespace ffpr::battle::enemy_parts {
// The native monster identity distinguishes parts sharing a localized name.
// Unknown IDs/message identities keep the game's name. No HP/AI inference.
const wchar_t* descriptor(int game,int monster,const std::wstring& message_key);
std::wstring qualify(void* enemy,const std::wstring& name,const std::wstring& message_key);
}
