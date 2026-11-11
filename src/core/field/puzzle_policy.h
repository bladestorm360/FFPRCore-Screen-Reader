#pragma once
#include "../json.h"
#include <string_view>

namespace field::puzzle_policy {
enum class Kind { Unknown,Navigation,Other };
inline unsigned mask(Kind kind){return kind==Kind::Navigation?1u:kind==Kind::Other?2u:3u;}
struct Policy {
    bool navigation=true,other=true;
    bool any()const{return navigation||other;}
    bool allows_mask(unsigned bits)const{return (!(bits&1u)||navigation)&&(!(bits&2u)||other);}
    bool allows(Kind kind)const {
        // Unclassified old/custom data remains compatible when both are on;
        // disabling either type never reveals an unqualified solution.
        return kind==Kind::Navigation?navigation:kind==Kind::Other?other:navigation&&other;
    }
};
inline Kind decode(std::wstring_view value) {
    return value==L"navigation"?Kind::Navigation:value==L"other"?Kind::Other:Kind::Unknown;
}
inline const json::Value* definition(const json::Value& root,std::wstring_view id) {
    const auto* types=root.get(L"guidance_types");return types?types->get(std::wstring(id)):nullptr;
}
inline Kind kind(const json::Value& root,std::wstring_view id,std::wstring_view instruction={}) {
    const auto* row=definition(root,id);if(!row)return Kind::Unknown;
    for(const auto* tag:{L"navigation",L"other"})if(const auto* overrides=row->get(tag)) {
        if(overrides->arr.size()>64)return Kind::Unknown;
        for(const auto& key:overrides->arr)if(!instruction.empty()&&key.s==instruction)return decode(tag);
    }
    const auto* base=row->get(L"kind");return base?decode(base->s):Kind::Unknown;
}
inline bool any_enabled(const json::Value& root,std::wstring_view id,Policy policy) {
    const auto* row=definition(root,id);if(!row)return policy.allows(Kind::Unknown);
    if(policy.allows(kind(root,id)))return true;
    if(policy.navigation)if(const auto* values=row->get(L"navigation");values&&!values->arr.empty())return true;
    if(policy.other)if(const auto* values=row->get(L"other");values&&!values->arr.empty())return true;
    if(const auto* extra=row->get(L"additional");extra&&extra->arr.size()<=2)
        for(const auto& value:extra->arr)if(policy.allows(decode(value.s)))return true;
    return false;
}
inline bool map_has_enabled_kind(const json::Value& root,int map,Policy policy) {
    const auto* types=root.get(L"guidance_types");if(!types||types->obj.size()>128)return policy.allows(Kind::Unknown);
    for(const auto& [id,row]:types->obj)if(const auto* maps=row.get(L"maps");maps&&maps->arr.size()<=128)
        for(const auto& value:maps->arr)if(value.n==map&&any_enabled(root,id,policy))return true;
    return false;
}
inline const wchar_t* disabled_key(Kind kind) {
    return kind==Kind::Navigation?L"Navigation puzzle guidance is off":
        kind==Kind::Other?L"Non-navigation puzzle guidance is off":L"Puzzle guidance is off";
}
}
