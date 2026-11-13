#pragma once
#include "baked_router.h"

namespace field::walk_open_doors {
using baked_route::Cell;
// Exact Castle Baron door actors and the step-on contacts whose scripts remove
// them. Contacts on both lanes open the same pair of actors. The rear door's
// automatic contact is only on its south side.
struct Door {
    int first_blocker, y, first_south_contact, south_y, south_script;
    int first_north_contact, north_y, north_script;
};
inline constexpr Door baron[] = {
    {43,30,142,31,1033,49,28,1001},
    {46,19,144,20,1034,0,0,0},
};

inline const Door* blocker(const baked::Submap& map,const baked::Object& object)
{
    if(map.first_map_id!=4||baked::game_number()!=4||object.type!=baked::ObjType::Entity||
       !(object.d&1)||object.layer!=0||object.tw!=1||object.th!=1||
       object.tx<19||object.tx>20||baked::str(object.group)!=L"entity_default")return nullptr;
    for(const auto& door:baron)
        if(object.ty==door.y&&object.entity_id()==door.first_blocker+object.tx-19)return &door;
    return nullptr;
}

inline bool contact(const Door& door,const baked::Object& object,bool south)
{
    const int first=south?door.first_south_contact:door.first_north_contact;
    return first&&object.type==baked::ObjType::Event&&object.b==2&&
        object.a==(south?door.south_script:door.north_script)&&
        object.ty==(south?door.south_y:door.north_y)&&object.tx>=19&&object.tx<=20&&
        object.entity_id()==first+object.tx-19&&object.layer==0&&object.tw==1&&object.th==1&&
        baked::str(object.group)==L"entity_default";
}

inline bool is_contact(const baked::Submap& map,const baked::Object& object)
{
    if(map.first_map_id!=4||baked::game_number()!=4)return false;
    for(const auto& door:baron)if(contact(door,object,true)||contact(door,object,false))return true;
    return false;
}
}
