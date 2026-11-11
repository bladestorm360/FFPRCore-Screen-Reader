#pragma once
#include <array>
#include <optional>

// Owned FFIII puzzle frontier. Native observation and route work have separate
// owners; this model only selects the next verified operation from copied facts.
namespace ff3::puzzles {
enum class Family { None,Nepto,Owen,Tozus,Hein,Lake,MagicCircle };
enum class Action { Loading,Outside,Finished,CastMini,MiniOpening,NeptoGuardian,NeptoEyePickup,ReturnEye,
    CastFrog,FrogWaterway,RemoveFrog,VoiceFirst,VoiceFourth,VoiceFifth,OwenSwitch,OwenGuardian,OwenEngine,
    Patient,HeinSoldier,HeinHole,RemoveMini,LakeApproach,LakeWaterway,LakeGuardian,DwarfTalk,DwarfAltar,
    DogaIntro,DogaSwitch,MagicHole,MagicUpgrade };
struct Snapshot {
    int map=0;
    std::array<std::optional<int>,256> flags;
    std::optional<int> mini,frog; //0 none,1 all,2 mixed; absent unknown
};
struct Decision {Action action=Action::Loading;int receipt=-1;};
Family family_at(int map);
Decision select(Family family,const Snapshot& snapshot);
const wchar_t* action_id(Action action);
}
