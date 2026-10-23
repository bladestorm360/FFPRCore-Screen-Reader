#include "party_entities.h"
#include "field_internal.h"
#include "../game.h"

namespace field::party_entities {
namespace {
ref::Class kController{"Last.Map","FieldController",game::kFF6};
ref::Class kParty{"Last.Entity.Field","FieldPartyEntity",game::kFF6};
ref::Field fParties{kController,"partyEntityList"};
void* collection(void* controller) {
    if(!game::is(game::Id::FF6)||!controller)return nullptr;
    void* array=ref::rd<void*>(controller,fParties);
    const int count=unity::array_length(array);
    return count>0&&count<=3?array:nullptr;
}
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
