#include "enemy_parts.h"
#include "../game.h"
#include "../refs.h"
#include "../modtext.h"

namespace ffpr::battle::enemy_parts {
namespace {
ref::Class kEnemy{"Last.Battle","BattleEnemyData",game::kFF5|game::kFF6};
ref::Class kMonster{"Last.Data.Master","Monster",game::kFF5|game::kFF6};
ref::Field fMonster{kEnemy,"<MonsterMaster>k__BackingField"};
ref::Method mId{kMonster,"get_Id",0};
struct Part { int game,id;const wchar_t* message;const wchar_t* label; };
// Reviewed actual sprite assets, not target-list order. Shells and heads share
// message keys in the game. Chadarnook changes between two distinct images.
// FF5's four Neo Exdeath scripts explicitly identify their visible body areas;
// the invisible master511 is intentionally excluded, as are indistinguishable
// doppelgangers. IDs plus the current native message prevent stale form labels.
constexpr Part parts[]={
    {5,387,L"MSG_MON_NAME_182",L"Boss part: Center"},
    {5,388,L"MSG_MON_NAME_182",L"Boss part: Lower right"},
    {5,389,L"MSG_MON_NAME_182",L"Boss part: Upper right"},
    {5,390,L"MSG_MON_NAME_182",L"Boss part: Lower left"},
    {6,257,L"MSG_MON_NAME_254",L"Boss part: Shell"},
    {6,309,L"MSG_MON_NAME_254",L"Boss part: Head"},
    {6,258,L"MSG_MON_NAME_255",L"Boss part: Shell"},
    {6,310,L"MSG_MON_NAME_255",L"Boss part: Head"},
    {6,287,L"MSG_MON_NAME_279",L"Boss part: Goddess"},
    {6,329,L"MSG_MON_NAME_279",L"Boss part: Demon"},
};
}
const wchar_t* descriptor(int game,int monster,const std::wstring& message_key) {
    for(const auto& part:parts)
        if(part.game==game&&part.id==monster&&message_key==part.message)return part.label;
    return nullptr;
}
std::wstring qualify(void* enemy,const std::wstring& name,const std::wstring& message_key) {
    if(!enemy||name.empty()||(!game::is(game::Id::FF5)&&!game::is(game::Id::FF6)))return name;
    void* master=ref::rd<void*>(enemy,fMonster);if(!master)return name;
    const auto* part=descriptor(static_cast<int>(game::current()),ref::call<int32_t>(mId,master),message_key);
    return part?name+L" ("+T(part)+L")":name;
}
}
