#pragma once
#include "airship_transitions.h"
#include "field_internal.h"
#include "../game.h"

// FFIII world1 has 1,524 fixed default-group continent-edge actors. The
// extracted map and all its puppet programs prove that these actors do not
// move or acquire script-owned visibility. Native transportation hiding keeps
// them present through cache4; their power/selectors still require fresh reads.
// Identify only this exact subset, without Unity names/transforms/parent walks.
// Every other actor follows ordinary live observation. No native pointer is
// retained by this helper or supplied to a worker.
namespace field::floating_border {
inline bool native_name_is(void* value,const wchar_t* wanted) {
    if(!value)return false;
    const auto size=std::char_traits<wchar_t>::length(wanted);
    if(size>64||il2cpp::read_field<int32_t>(value,il2cpp::kStringLengthOff)!=static_cast<int>(size))return false;
    for(size_t i=0;i<size;++i)
        if(il2cpp::read_field<wchar_t>(value,static_cast<int32_t>(il2cpp::kStringCharsOff+i*sizeof(wchar_t)))!=wanted[i])return false;
    return true;
}
inline const baked::Object* identify(void* entity,int map_id) {
    if(map_id!=1||!game::is(game::Id::FF3)||!entity||!live_entity(entity))return nullptr;
    const auto* map=baked::submap_by_map_id(1);
    if(!map||!map->world)return nullptr;
    const auto& grid=baked::grids(*map);
    if(!grid.entity_index_prepared)return nullptr;
    if(r::fPropObjectType.get()<0||r::fPropEntityId.get()<0||r::fEventScriptId.get()<0||
       r::fPropName.get()<0||r::fCacheActiveEnable.get()<0)return nullptr;
    void* property=entity_property(entity);
    if(!property||ref::rd<int32_t>(property,r::fPropObjectType)!=2)return nullptr;
    const int id=ref::rd<int32_t>(property,r::fPropEntityId);
    const auto at=grid.objects_by_entity.find(id);
    if(at==grid.objects_by_entity.end())return nullptr;
    const baked::Object* found=nullptr;
    for(const size_t index:at->second) {
        const auto& object=grid.objects[index];
        const auto* rule=air_transitions::match(3,1,object);
        if(!rule)continue;
        if(found)return nullptr;
        if(!r::kEventTriggerEntity.is(entity)||!r::kPropertyEvent.is(property)||
           ref::rd<int32_t>(property,r::fEventScriptId)!=rule->script||
           !native_name_is(ref::rd<void*>(property,r::fPropName),rule->raw))return nullptr;
        // A cache owner other than the native transportation switch is outside
        // the verified fixed-default contract, so retain the full live path.
        if((ref::rd<int32_t>(entity,r::fCacheActiveEnable)&~(1<<4))!=0)return nullptr;
        found=&object;
    }
    return found;
}
}
