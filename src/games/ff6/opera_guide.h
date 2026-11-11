#pragma once
#include <string_view>

// Owned identifiers only, never lyrics or native pointers. The native event
// compares the failure IDs c02/c01/c02; its choice coroutine returns the ID at
// the selected array index. Derive advice from that actual order, not a cursor
// assumption. Unknown, duplicate and mixed-question arrays have no answer.
namespace ffpr::ff6::opera_guide {
struct Choice {int question=0,correct=0;};
inline Choice choice(std::wstring_view a,std::wstring_view b) {
    constexpr std::wstring_view ids[3][2]={
        {L"E0318_00_118_c_01",L"E0318_00_118_c_02"},
        {L"E0318_01_118_c_01",L"E0318_01_118_c_02"},
        {L"E0318_02_118_c_01",L"E0318_02_118_c_02"}};
    for(int q=0;q<3;++q) {
        const int good=q==1?1:0;
        if(a==ids[q][0]&&b==ids[q][1])return {q+1,good+1};
        if(b==ids[q][0]&&a==ids[q][1])return {q+1,2-good};
    }
    return {};
}
}
