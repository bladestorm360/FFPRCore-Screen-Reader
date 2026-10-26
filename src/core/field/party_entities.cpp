#include "party_entities.h"
#include "field_internal.h"
#include "../game.h"
#include "entity_translator.h"

namespace field::party_entities {
namespace {
ref::Class kController{"Last.Map","FieldController",game::kFF6};
ref::Class kParty{"Last.Entity.Field","FieldPartyEntity",game::kFF6};
ref::Class kCompanion{"Last.Entity.Field","FieldCompanionEntity"};
ref::Class kCompanionController{"Last.Map","FieldController",game::kFF3};
ref::Field fCompanionEntity{kCompanionController,"companionEntity"};
ref::Class kSaveMapManager{"Last.Map","SaveMapManager",game::kFF3};
ref::Method mSaveMapInstance{kSaveMapManager,"get_Instance",0};
ref::Field fSavedCompanion{kSaveMapManager,"<CompanionEntity>k__BackingField"};
ref::Class kCompanionData{"Last.Map","SaveCompanionEntityData",game::kFF3};
ref::Field fCompanionAsset{kCompanionData,"fieldCharacterAssetId"};
ref::Field fParties{kController,"partyEntityList"};
void* collection(void* controller) {
    if(!game::is(game::Id::FF6)||!controller)return nullptr;
    void* array=ref::rd<void*>(controller,fParties);
    const int count=unity::array_length(array);
    return count>0&&count<=3?array:nullptr;
}
}
bool is_companion(void* entity) {return entity && kCompanion.is(entity);}
std::wstring companion_name(void* entity) {
    if(!game::is(game::Id::FF3)||!is_companion(entity))return {};
    void* map=field_map();
    void* controller=map?ref::rd<void*>(map,r::fFieldController):nullptr;
    if(!controller||ref::rd<void*>(controller,fCompanionEntity)!=entity)return {};
    void* saved=ref::rd<void*>(ref::call_static<void*>(mSaveMapInstance),fSavedCompanion);
    if(!kCompanionData.is(saved))return {};
    // SetupCompanionEntity creates a synthetic PropertyNpc with AssetId zero.
    // ChangeCompanionEntityAsset uses this saved base identity to select the
    // actual sprite (possibly a condition variant). Read that same source only
    // for its current owner; no cached join event, proximity or party-slot guess.
    const wchar_t* raw=nullptr;
    switch(ref::rd<int32_t>(saved,fCompanionAsset)) {
    case 33:raw=L"サラ";break;case 34:raw=L"シド";break;
    case 35:raw=L"デッシュ";break;case 36:raw=L"エリア";break;
    case 37:raw=L"アルス";break;case 38:raw=L"ドーガ";break;
    case 39:raw=L"ウネ";break;case 41:raw=L"グツコー";break;
    }
    return raw?translate_entity_name(raw):std::wstring();
}
bool next_native(void* controller,int& index,void*& entity) {
    entity=nullptr;void* array=collection(controller);
    if(!array||index<0||index>=unity::array_length(array))return false;
    entity=unity::array_at(array,index++);return true;
}
bool is_native(void* entity) {
    return game::is(game::Id::FF6)&&entity&&kParty.is(entity);
}
int native_slot(void* entity) {
    if(!is_native(entity))return -1;
    void* map=field_map();
    void* controller=map?ref::rd<void*>(map,r::fFieldController):nullptr;
    void* array=collection(controller);if(!array)return -1;
    for(int i=0;i<unity::array_length(array);++i)if(unity::array_at(array,i)==entity)return i;
    return -1;
}
}
