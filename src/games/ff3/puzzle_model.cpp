#include "puzzle_model.h"
#include <utility>
namespace ff3::puzzles {
Family family_at(int map) {
    if(map>=169&&map<=173)return Family::Nepto;
    if(map>=174&&map<=184)return Family::Owen;
    if(map>=45&&map<=50)return Family::Tozus;
    if(map>=194&&map<=205)return Family::Hein;
    if((map>=84&&map<=90)||(map>=185&&map<=188))return Family::Lake;
    if((map>=131&&map<=133)||(map>=238&&map<=241))return Family::MagicCircle;
    return Family::None;
}
Decision select(Family family,const Snapshot& s) {
    if(family==Family::None)return {Action::Outside};
    const int finish=family==Family::Nepto?34:family==Family::Owen?44:family==Family::Tozus?29:
        family==Family::Lake?50:family==Family::MagicCircle?78:-1;
    if(finish>=0) {
        if(!s.flags[finish])return {};
        if(s.flags[finish]==1)return {Action::Finished,finish};
    }
    if(family_at(s.map)!=family)return {Action::Outside};
    if(family==Family::Tozus) {
        if(!s.mini)return {};
        return s.mini==1?Decision{Action::Patient,29}:Decision{Action::CastMini};
    }
    if(family==Family::Hein) {
        if(!s.flags[56]||!s.mini)return {};
        if(s.map!=194&&s.flags[56]==1)
            return s.mini==0?Decision{Action::Finished,56}:Decision{Action::RemoveMini};
        if(s.mini==1)return {Action::HeinHole,s.flags[56]==1?-1:56};
        if(!s.flags[55])return {};
        return s.flags[55]==1?Decision{Action::CastMini}:Decision{Action::HeinSoldier,55};
    }
    if(family==Family::Lake) {
        if(!s.flags[48])return {};
        if(s.flags[48]==1) {
            if(!s.flags[49])return {};
            return s.flags[49]==1?Decision{Action::DwarfAltar,50}:Decision{Action::DwarfTalk,49};
        }
        if(s.map<185||s.map>188) {
            if(s.map!=90)return {Action::LakeApproach};
            if(!s.frog)return {};
            return s.frog==1?Decision{Action::LakeWaterway,s.flags[46]==1?-1:46}:Decision{Action::CastFrog};
        }
        if(!s.frog)return {};
        return s.frog==0?Decision{Action::LakeGuardian,48}:Decision{Action::RemoveFrog};
    }
    if(family==Family::MagicCircle) {
        if(!s.flags[77]||!s.flags[195])return {};
        if(s.flags[77]!=1)return {Action::DogaIntro,77};
        // Include the Manor's existing physical control while approaching the
        // hole; an already-entered cave never sends the party back for it.
        if(s.map<=133&&s.flags[195]!=1) {
            if(!s.flags[198])return {};
            if(s.flags[198]!=1)return {Action::DogaSwitch,198};
        }
        if(!s.mini)return {};
        if(s.mini!=1)return {Action::CastMini};
        if(s.map<=133)return {Action::MagicHole,s.flags[195]==1?-1:195};
        return {Action::MagicUpgrade,78};
    }
    if(family==Family::Nepto) {
        if(!s.flags[33])return {};
        // The Eye receipt outranks Mini and the retired small-opening variant.
        // Recovered-Eye resumes must return to the statue, in either form.
        if(s.flags[33]==1)return {Action::ReturnEye,34};
        if(!s.mini)return {};
        if(s.mini!=1)return {Action::CastMini};
        if(s.map==169)return {Action::MiniOpening,s.flags[31]==1?-1:31};
        // The initializer replaces the defeated guardian with interactive Eye5
        // while32 is set and33 is not. Eye7 is only a cutscene prop.
        if(!s.flags[32])return {};
        if(s.flags[32]==1)return {Action::NeptoEyePickup,33};
        return {Action::NeptoGuardian,33};
    }
    if(s.map==174) {
        //39 proves an earlier entry, not today's party form. Every return to
        // the basement must satisfy the native all-Frog condition again.
        if(!s.frog)return {};
        return s.frog==1?Decision{Action::FrogWaterway,s.flags[39]==1?-1:39}:Decision{Action::CastFrog};
    }
    if(!s.flags[43])return {};
    if(s.flags[43]==1)return {Action::OwenEngine,44};
    if(!s.frog)return {};
    // Mixed includes even one remaining frog; it cannot certify all normal.
    if(s.frog!=0)return {Action::RemoveFrog};
    if(!s.flags[225])return {};
    if(s.flags[225]==1) {
        // Opening the physical wall does not set the fifth-floor event42.
        // The native top-floor initializer requires42 before spawning Medusa.
        // Earlier optional clues may be skipped; this story gate may not.
        if(!s.flags[42])return {};
        return s.flags[42]==1?Decision{Action::OwenGuardian,43}:Decision{Action::VoiceFifth,42};
    }
    for(const auto entry:std::array<std::pair<int,Action>,3>{{{40,Action::VoiceFirst},{41,Action::VoiceFourth},{42,Action::VoiceFifth}}}) {
        if(!s.flags[entry.first])return {};
        if(s.flags[entry.first]!=1)return {entry.second,entry.first};
    }
    return {Action::OwenSwitch,225};
}
const wchar_t* action_id(Action a) {
    switch(a) {
    case Action::CastMini:return L"cast_mini";
    case Action::MiniOpening:return L"mini_opening";
    case Action::NeptoGuardian:return L"nepto_guardian";
    case Action::NeptoEyePickup:return L"nepto_eye_pickup";
    case Action::ReturnEye:return L"return_eye";
    case Action::CastFrog:return L"cast_frog";
    case Action::FrogWaterway:return L"frog_waterway";
    case Action::RemoveFrog:return L"remove_frog";
    case Action::VoiceFirst:return L"voice_first";
    case Action::VoiceFourth:return L"voice_fourth";
    case Action::VoiceFifth:return L"voice_fifth";
    case Action::OwenSwitch:return L"owen_switch";
    case Action::OwenGuardian:return L"owen_guardian";
    case Action::OwenEngine:return L"owen_engine";
    case Action::Patient:return L"patient";
    case Action::HeinSoldier:return L"hein_soldier";
    case Action::HeinHole:return L"hein_hole";
    case Action::RemoveMini:return L"remove_mini";
    case Action::LakeApproach:return L"lake_approach";
    case Action::LakeWaterway:return L"lake_waterway";
    case Action::LakeGuardian:return L"lake_guardian";
    case Action::DwarfTalk:return L"dwarf_talk";
    case Action::DwarfAltar:return L"dwarf_altar";
    case Action::DogaIntro:return L"doga_intro";
    case Action::DogaSwitch:return L"doga_switch";
    case Action::MagicHole:return L"magic_hole";
    case Action::MagicUpgrade:return L"magic_upgrade";
    default:return L"";
    }
}
}
