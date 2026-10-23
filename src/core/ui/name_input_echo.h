#pragma once
#include <algorithm>
#include <string>

namespace ui::name_input {
// Unity KeyCode values; derive speech from what the native edit actually did.
// Repeated equal letters remain separate key occurrences, never text-deduped.
inline std::wstring echo(const std::wstring& before,const std::wstring& after,int old_caret,int caret,int key) {
    if(before.size()>128||after.size()>128)return {};
    if(key==8 && after.size()<before.size()) {
        size_t prefix=0,suffix=0;
        while(prefix<after.size()&&before[prefix]==after[prefix])++prefix;
        while(suffix<after.size()-prefix&&before[before.size()-1-suffix]==after[after.size()-1-suffix])++suffix;
        return before.substr(prefix,before.size()-prefix-suffix);
    }
    if(before!=after||caret==old_caret||caret<0||caret>static_cast<int>(after.size()))return {};
    if(key<273||key>276)return {};
    int at=caret<old_caret?caret:old_caret;
    at=std::clamp(at,0,static_cast<int>(after.size()));
    if(at>=static_cast<int>(after.size()))return {};
    size_t first=static_cast<size_t>(at),last=first+1;
    // Never speak half a UTF-16 surrogate pair on Windows.
    if(after[first]>=0xDC00&&after[first]<=0xDFFF&&first&&after[first-1]>=0xD800&&after[first-1]<=0xDBFF)--first;
    if(after[last-1]>=0xD800&&after[last-1]<=0xDBFF&&last<after.size()&&after[last]>=0xDC00&&after[last]<=0xDFFF)++last;
    return after.substr(first,last-first);
}
}
