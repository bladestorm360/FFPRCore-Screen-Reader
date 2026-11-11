#pragma once
#include <array>
#include <optional>
#include <string_view>

namespace ffpr::ff6::palace {
// Owned identifiers from one native Select first-clock occurrence. No native
// pointers or rendered text cross this boundary, and no instruction runs here.
struct Question {
    bool valid=false;
    std::array<std::array<wchar_t,32>,6> options{};
    std::array<wchar_t,32> prompt{};
};
enum class Kind { None,Toast,War,Doma,Kefka,Ask,More,LastWords,Recall,Break,
                  Resume,Help,ConfirmHelp,Battle,FinishTour };
struct Selection {Kind kind=Kind::None;int option=0;};
inline std::wstring_view option(const Question& q,size_t i){return q.options[i].data();}
inline std::wstring_view prompt(const Question& q){return q.prompt.data();}
inline bool group(const Question& q,const std::array<std::wstring_view,3>& ids,int count) {
    if(!q.valid)return false;
    int actual=0;
    for(size_t i=0;i<q.options.size();++i) {
        const auto key=option(q,i);if(key.empty())continue;
        if(i!=static_cast<size_t>(actual)||++actual>count)return false;
        bool known=false;for(int j=0;j<count;++j)known|=key==ids[static_cast<size_t>(j)];
        if(!known)return false;
        for(size_t j=0;j<i;++j)if(key==option(q,j))return false;
    }
    return actual==count;
}
inline int index(const Question& q,std::wstring_view id) {
    for(size_t i=0;i<q.options.size();++i)if(option(q,i)==id)return static_cast<int>(i)+1;
    return 0;
}
inline Kind identify(const Question& q) {
    if(!q.valid)return Kind::None;
    constexpr std::array<std::wstring_view,3> menus[]={
        {L"E0502_00_177_c_01",L"E0502_00_177_c_02",L"E0502_00_177_c_03"},
        {L"E0502_01_177_c_01",L"E0502_01_177_c_02",L"E0502_01_177_c_03"},
        {L"E0502_02_177_c_01",L"E0502_02_177_c_02",L"E0502_02_177_c_03"},
        {L"E0502_03_177_c_01",L"E0502_03_177_c_02",L"E0502_03_177_c_03"},
        {L"E0502_04_177_c_01",L"E0502_04_177_c_02",L"E0502_04_177_c_03"},
        {L"E0502_06_177_c_01",L"E0502_06_177_c_02",{}},
        {L"E0502_07_177_c_01",L"E0502_07_177_c_02",{}},
        {L"E0502_08_177_c_01",L"E0502_08_177_c_02",{}},
        {L"E0504_00_177_c_01",L"E0504_00_177_c_02",L"E0504_00_177_c_03"}};
    constexpr Kind kinds[]{Kind::Toast,Kind::War,Kind::Doma,Kind::Kefka,Kind::Ask,
        Kind::More,Kind::LastWords,Kind::Break,Kind::LastWords};
    for(size_t i=0;i<std::size(menus);++i)if(group(q,menus[i],i>=5&&i<=7?2:3)) {
        if(i==4) {
            if(prompt(q)==L"E0502_04_177_a_01")return Kind::Ask;
            if(prompt(q)==L"E0502_00_177_a_19")return Kind::Recall;
            return Kind::None; // identical options, different real question
        }
        return kinds[i];
    }
    // Native generic Yes/No has empty string operands. Its actual preceding
    // Msg identity distinguishes resuming from the optional break battle.
    for(size_t i=0;i<q.options.size();++i)if(!option(q,i).empty())return Kind::None;
    if(prompt(q)==L"E0504_03_177_a_01")return Kind::Resume;
    if(prompt(q)==L"E0504_02_177_a_01")return Kind::Help;
    if(prompt(q)==L"E0504_01_177_a_01")return Kind::ConfirmHelp;
    if(prompt(q)==L"E0505_00_177_a_01")return Kind::Battle;
    if(prompt(q)==L"E0474_00_999_a_25")return Kind::FinishTour;
    return Kind::None;
}
struct State {
    std::array<std::optional<int>,8> local;
    std::optional<bool> break_done,tour_done;
};
inline Selection answer(const Question& q,const State& state) {
    const auto kind=identify(q);std::wstring_view wanted;
    switch(kind) {
    case Kind::Toast:wanted=L"E0502_00_177_c_03";break;
    case Kind::War:wanted=L"E0502_01_177_c_01";break;
    case Kind::Doma:wanted=L"E0502_02_177_c_02";break;
    case Kind::Kefka:wanted=L"E0502_03_177_c_02";break;
    case Kind::LastWords:
        wanted=index(q,L"E0504_00_177_c_02")?L"E0504_00_177_c_02":L"E0502_07_177_c_01";break;
    case Kind::Break:wanted=L"E0502_08_177_c_01";break;
    case Kind::Ask:case Kind::More: {
        int remaining=0;
        for(int i=1;i<=3;++i) {
            const auto value=state.local[static_cast<size_t>(i)];
            if(!value||(*value!=0&&*value!=1))return {kind,0};
            if(!remaining&&*value==0)remaining=i;
        }
        if(kind==Kind::More)wanted=remaining?L"E0502_06_177_c_01":L"E0502_06_177_c_02";
        else {
            // A player may have reopened the menu after all three questions.
            // One native repeat is then unavoidable; the following More menu
            // exits. The guide's own normal sequence never enters that loop.
            constexpr std::wstring_view ids[]{L"E0502_04_177_c_01",L"E0502_04_177_c_02",L"E0502_04_177_c_03"};
            wanted=ids[static_cast<size_t>(remaining?remaining-1:0)];
        }
        break;
    }
    case Kind::Recall: {
        const auto first=state.local[4];if(!first||*first<1||*first>3)return {kind,0};
        constexpr std::wstring_view ids[]{L"E0502_04_177_c_01",L"E0502_04_177_c_02",L"E0502_04_177_c_03"};
        wanted=ids[static_cast<size_t>(*first-1)];break;
    }
    case Kind::Resume:return {kind,state.break_done?(*state.break_done?1:2):0};
    case Kind::FinishTour:return {kind,state.tour_done?(*state.tour_done?1:2):0};
    case Kind::Help:case Kind::ConfirmHelp:case Kind::Battle:return {kind,1};
    default:return {};
    }
    return {kind,index(q,wanted)};
}
}
