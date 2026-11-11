// check-sources:
#include <cstdio>
#include "../src/games/ff3/puzzle_model.cpp"
int main() {
    using namespace ff3::puzzles;
    int failures=0,cases=0;
    const auto expect=[&](bool ok,const char* name){++cases;if(!ok){++failures;std::printf("FAIL %s\n",name);}};
    Snapshot s;s.flags.fill(0);s.map=169;
    expect(select(Family::Nepto,s).action==Action::Loading,"unknown form never certifies Mini");
    for(int form:{0,2}){s.mini=form;expect(select(Family::Nepto,s).action==Action::CastMini,"normal/mixed Nepto party must become all Mini");}
    s.mini=1;expect(select(Family::Nepto,s).action==Action::MiniOpening,"fresh Mini party goes to actual opening");
    s.flags[31]=1;s.mini=0;expect(select(Family::Nepto,s).action==Action::CastMini,"persistent entrance31 never replaces current form");
    s.mini=1;s.map=173;expect(select(Family::Nepto,s).action==Action::NeptoGuardian,"inside Mini temple goes to real guardian");
    s.flags[32]=1;expect(select(Family::Nepto,s).action==Action::NeptoEyePickup,"defeated guardian selects native fallback Eye5, not retired guardian or cutscene Eye7");
    s.flags[33]=1;s.mini={};expect(select(Family::Nepto,s).action==Action::ReturnEye,"Eye receipt outranks unknown form and retired entrance");
    s.flags[34]=1;s.map=1;expect(select(Family::Nepto,s).action==Action::Finished,"native final receipt finishes across scene/world return");
    s.flags.fill(0);s.map=174;
    for(int old_entry:{0,1})for(int form:{0,1,2}) {
        s.flags[39]=old_entry;s.frog=form;
        expect(select(Family::Owen,s).action==(form==1?Action::FrogWaterway:Action::CastFrog),"Owen entry/reentry uses current all-Frog state");
    }
    s.map=175;
    for(int form:{1,2}){s.frog=form;expect(select(Family::Owen,s).action==Action::RemoveFrog,"all or mixed Frog must return fully normal");}
    s.frog=0;expect(select(Family::Owen,s).action==Action::VoiceFirst,"normal party receives first native clue");
    s.flags[40]=1;expect(select(Family::Owen,s).action==Action::VoiceFourth,"first clue receipt advances");
    s.flags[41]=1;expect(select(Family::Owen,s).action==Action::VoiceFifth,"fourth-floor clue receipt advances");
    s.flags[42]=1;expect(select(Family::Owen,s).action==Action::OwenSwitch,"all clues lead to exact hidden control");
    s.flags[225]=1;s.flags[40]={};expect(select(Family::Owen,s).action==Action::OwenGuardian,"opened wall outranks missing earlier clues");
    s.flags[42]=0;expect(select(Family::Owen,s).action==Action::VoiceFifth,"opened wall never skips mandatory native42 boss-spawn event");
    s.flags[42]={};expect(select(Family::Owen,s).action==Action::Loading,"unknown mandatory boss-spawn receipt cannot certify the guardian");s.flags[42]=1;
    s.flags[43]=1;s.frog={};expect(select(Family::Owen,s).action==Action::OwenEngine,"victory outranks missing earlier form observation");
    s.flags[44]=1;s.map=1;expect(select(Family::Owen,s).action==Action::Finished,"engine native receipt finishes on world return");
    s.flags[44]=0;expect(select(Family::Owen,s).action==Action::Outside,"leaving unfinished puzzle never claims completion");
    s.flags.fill(0);s.map=45;s.mini=2;
    expect(select(Family::Tozus,s).action==Action::CastMini,"Tozus mixed form preparation");
    s.mini=1;expect(select(Family::Tozus,s).action==Action::Patient,"Tozus targets Antidote recipient");
    s.flags[29]=1;s.mini={};expect(select(Family::Tozus,s).action==Action::Finished,"healed patient outranks missing form data");
    s.flags.fill(0);s.map=194;s.mini=0;
    expect(select(Family::Hein,s).action==Action::HeinSoldier,"prison starts with native soldier help");
    s.flags[55]=1;expect(select(Family::Hein,s).action==Action::CastMini,"prison clue precedes required transformation");
    s.mini=1;expect(select(Family::Hein,s).action==Action::HeinHole,"Mini party uses exact escape hole");
    s.map=195;s.flags[56]=1;s.mini=2;expect(select(Family::Hein,s).action==Action::RemoveMini,"mixed Mini must all return to normal after prison escape");
    s.mini=0;expect(select(Family::Hein,s).action==Action::Finished,"prison form puzzle ends only after actual escape and normal form");
    s.map=194;expect(select(Family::Hein,s).action==Action::CastMini,"prison return rechecks current Mini instead of relying on56");
    s.flags.fill(0);s.map=84;s.frog={};
    expect(select(Family::Lake,s).action==Action::LakeApproach,"lake approach does not demand Frog before reaching its entrance floor");
    s.map=90;s.frog=2;expect(select(Family::Lake,s).action==Action::CastFrog,"lake mixed party needs all frogs");
    s.frog=1;expect(select(Family::Lake,s).action==Action::LakeWaterway,"lake all-Frog waterway");
    s.flags[46]=1;expect(select(Family::Lake,s).receipt==-1,"historical lake entry never creates repeated flag refreshes");
    s.map=185;expect(select(Family::Lake,s).action==Action::RemoveFrog,"lake dungeon prompts normal form");
    s.frog=0;expect(select(Family::Lake,s).action==Action::LakeGuardian,"lake normal party heads to horn guardian");
    s.flags[48]=1;s.frog={};expect(select(Family::Lake,s).action==Action::DwarfTalk,"recovered horn outranks form and entry dependencies");
    s.flags[49]=1;expect(select(Family::Lake,s).action==Action::DwarfAltar,"dwarf clue opens actual horn placement step");
    s.flags[50]=1;expect(select(Family::Lake,s).action==Action::Finished,"native altar50 completes horn placement");
    s.flags.fill(0);s.map=131;s.mini={};
    expect(select(Family::MagicCircle,s).action==Action::DogaIntro,"Manor begins with actual Doga arrival");
    s.flags[77]=1;expect(select(Family::MagicCircle,s).action==Action::DogaSwitch,"Manor includes existing passage control");
    s.flags[198]=1;s.mini=0;expect(select(Family::MagicCircle,s).action==Action::CastMini,"magic cave entrance requires all Mini");
    s.mini=1;expect(select(Family::MagicCircle,s).action==Action::MagicHole,"Manor targets actual Upper hole");
    s.map=238;s.flags[195]=1;s.flags[198]=0;expect(select(Family::MagicCircle,s).action==Action::MagicUpgrade,"inside cave never backtracks for optional Manor switch");
    s.mini=2;expect(select(Family::MagicCircle,s).action==Action::CastMini,"cave form recovery is current, not historical195");
    s.flags[78]=1;s.mini={};expect(select(Family::MagicCircle,s).action==Action::Finished,"actual Nautilus upgrade outranks missing forms");
    std::printf("FF3 puzzle model: %d current-form/receipt/resume cases, %d failures\n",cases,failures);
    return failures?1:0;
}
