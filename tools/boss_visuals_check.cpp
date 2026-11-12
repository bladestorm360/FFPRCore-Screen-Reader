// check-sources: json.cpp text.cpp
// Exercise the shipped form policy and the localized cues without a game.
#include "../src/core/battle/boss_visuals.h"
#include "../src/core/json.h"
#include <array>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string_view>

namespace {
using namespace ffpr::battle::boss_visuals;
using game::Id;
int failures = 0;

void expect(bool passed, const char* why)
{
    if (passed) return;
    ++failures;
    std::printf("FAIL %s\n",why);
}

json::Value catalog(int title)
{
    std::ifstream input("data/ff"+std::to_string(title)+"/mod_text.json",std::ios::binary);
    const std::string bytes(std::istreambuf_iterator<char>(input),{});
    json::Value result;
    expect(json::parse(bytes,result),"Game mod text parses");
    return result;
}

void check_key(const json::Value& catalog, const wchar_t* key)
{
    const auto* entry = key ? catalog.get(key) : nullptr;
    expect(entry != nullptr,"Audited visual has a localized cue");
    if (!entry) return;
    constexpr std::array locales{L"en",L"ja",L"fr",L"it",L"de",L"es",L"ko",L"zht",L"zhc",L"ru",L"th",L"pt"};
    for (auto* language : locales) {
        const auto* text=entry->get(language);
        expect(text&&text->is_string()&&!text->s.empty(),"Cue is present in each game language");
    }
}
}

int main()
{
    const auto iv=catalog(4), v=catalog(5), vi=catalog(6);
    for (const auto& cue : {
         deformation(Id::FF4,178,191,1,167),deformation(Id::FF4,178,190,2,167),
         deformation(Id::FF4,165,176,1,167),deformation(Id::FF4,163,168,4,167),
         deformation(Id::FF4,171,182,3,167),deformation(Id::FF4,188,201,0,167),
         deformation(Id::FF4,188,202,0,167),deformation(Id::FF4,189,204,0,0),
         deformation(Id::FF4,189,203,0,0),deformation(Id::FF4,193,214,2,0),
         replacement(Id::FF4,540,228,227),replacement(Id::FF4,541,194,195),
         replacement(Id::FF4,542,195,228)}) check_key(iv,cue);
    for (const auto& cue : {
         deformation(Id::FF5,281,246,282,1),deformation(Id::FF5,282,245,281,0),
         replacement(Id::FF5,445,291,290),replacement(Id::FF5,654,290,291),
         replacement(Id::FF5,655,291,292),L"BossVisual.SandwormMoved"}) check_key(v,cue);
    for (const auto& cue : {
         no_select_pose(271,0,4,true),no_select_pose(271,0,7,false),
         no_select_pose(272,0,12,true),no_select_pose(272,0,11,false),
         no_select_pose(278,0,7,false),no_select_pose(279,0,7,false),
         no_select_pose(311,0,18,true),no_select_pose(311,0,19,false),
         painting(317,300),painting(300,317)}) check_key(vi,cue);

    expect(!deformation(Id::FF4,162,166,1,167)&&!deformation(Id::FF4,162,165,2,167),
           "Mist Dragon uses native text in both directions");
    expect(!deformation(Id::FF4,171,182,0,167)&&!deformation(Id::FF4,171,184,2,167),
           "Cagnazzo retains the native lightning and buildup messages");
    expect(!deformation(Id::FF4,163,173,14,167)&&!deformation(Id::FF4,226,201,0,167),
           "The scripted Octomammoth and Rubicante text takes priority");
    expect(!replacement(Id::FF5,602,301,302)&&!replacement(Id::FF5,654,291,291),
           "Identical art and unrelated replacements are silent");
    expect(!deformation(Id::FF6,178,191,1,167)&&!shell(269,true),
           "The shell itself and the wrong title cannot produce a form cue");
    expect(!deformation(Id::FF4,178,190,0,0)&&!deformation(Id::FF4,163,168,5,0)&&
           !deformation(Id::FF4,188,202,0,0)&&!deformation(Id::FF4,193,213,1,0)&&
           !deformation(Id::FF5,281,246,2,0),
           "Mode updates carrying the existing image do not qualify");
    expect(no_select_pose(271,0,12,true)&&no_select_pose(271,0,4,true)&&
           no_select_pose(272,0,11,false)&&no_select_pose(272,0,7,false),
           "Shell cues accept both verified hide and emergence animations");
    expect(!no_select_pose(271,12,12,true)&&!no_select_pose(272,12,6,false)&&
           !no_select_pose(311,0,7,false)&&!no_select_pose(280,0,7,false),
           "Repeated and unrelated standing states are silent");
    expect(!painting(0,317)&&!painting(317,317)&&painting(317,300)&&painting(300,317),
           "Opening dialogue and recurring painting forms are distinguished");
    const SpriteChange wind{1,2,deformation(Id::FF4,178,191,1,167)};
    expect(wind.presented(2,true)&&!wind.presented(1,true)&&!wind.presented(2,false),
           "The expected sprite must actually become visible");
    expect(!SpriteChange{2,2,wind.key}.presented(2,true)&&!SpriteChange{}.presented(2,true),
           "A repeated sprite or missing request cannot produce another cue");
    std::printf("Boss visual form, native text and locale checks: %d failures\n",failures);
    return failures?1:0;
}
