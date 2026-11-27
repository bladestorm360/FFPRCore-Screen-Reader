#include "field_api.h"
#include "../core.h"
#include "../dialogue/dialogue.h"
#include "../displayed_ui.h"
#include "../fieldnav/map_transition.h"
#include "../frame.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../refs.h"
#include "../say.h"
#include "../system/system.h"
#include "../ui/preferences.h"
#include "../unity.h"
#include <algorithm>
#include <array>
#include <string_view>

namespace ffpr::dialogue {
namespace {
ref::Class kPuppet{"Last.Interpreter.Instructions", "Puppet"};
ref::Method mPlayAnim{kPuppet, "PlayAnim", 1}, mShow{kPuppet, "Show", 1};
ref::Class kFlightPuppet{"Last.Interpreter.Instructions", "Puppet", game::kFF2 | game::kFF4};
ref::Method mMoveBirdView{kFlightPuppet, "MoveBirdView", 1};
ref::Class kFader{"Last.Interpreter.Instructions", "External.Fader", game::kFF4};
ref::Method mColorFade{kFader, "ColorFade", 1};
ref::Class kCore{"Last.Interpreter", "Core"}, kPuppetCore{"Last.Interpreter", "PuppetCore"};
ref::Method mFirstClock{kCore, "IsFirstClock", 0}, mProgramCounter{kCore, "get_pc", 0};
ref::Field fCoreId{kCore, "<coreID>k__BackingField"};
ref::Field fActorId{kPuppetCore, "<entityID>k__BackingField"}, fActor{kPuppetCore, "<entity>k__BackingField"};
ref::Class kIntegrator{"Last.Interpreter", "Integrator"};
ref::Field fCores{kIntegrator, "cores"}, fScript{kIntegrator, "<scriptName>k__BackingField"};
ref::Field fField{kIntegrator, "isFieldScript"}, fWorking{kIntegrator, "working"}, fPause{kIntegrator, "isPauseScript"};
ref::Class kEffect{"Last.Entity.Field", "FieldEffectParticleEntity"};
ref::Field fPlaying{kEffect, "isPlaying"}, fParticles{kEffect, "particleArray"};
ref::Class kParticle{"UnityEngine", "ParticleSystem"}, kParticleRenderer{"UnityEngine", "ParticleSystemRenderer"};
ref::Method mParticleCount{kParticle, "get_particleCount", 0};

enum class CueTrigger { PlayAnim, Show, MoveBirdView, ColorFade };
struct Cue {
    game::Id title;
    int map, actor, instruction;
    std::wstring_view script, text;
    CueTrigger trigger = CueTrigger::PlayAnim;
    std::wstring_view art{}, pose{};
    double settle_seconds = 0;
};
// The planet fracture is the largest inspected prefab in this registry.
// Only eight emitters are checked on any one owner frame.
constexpr int scene_emitter_limit = 74;

// Selected native animation and display commands from the frozen field corpus. A clip
// can share an actor with a setup, repeat or alternate script on the same map.
constexpr std::array cues{
    Cue{game::Id::FF1, 1, 204, 70, L"sc_e_0015_1", L"Canal excavation cue"},
    Cue{game::Id::FF1, 1, 102, 86, L"sc_e_0015_1", L"Canal water plume cue", CueTrigger::PlayAnim, L"mo_ff1_e009_c00"},
    Cue{game::Id::FF1, 80, 28, 120, L"sc_e_0021_2", L"Crystal altar restoration cue", CueTrigger::PlayAnim, L"mo_ff1_b022_c03"},
    Cue{game::Id::FF1, 87, 39, 115, L"sc_e_0023_2", L"Crystal altar restoration cue", CueTrigger::PlayAnim, L"mo_ff1_b022_c02"},
    Cue{game::Id::FF1, 1, 91, 91, L"sc_e_0025_2", L"Levistone sand cue"},
    Cue{game::Id::FF1, 1, 203, 178, L"sc_e_0025_2", L"Levistone airship rise cue", CueTrigger::PlayAnim, L"mo_ff1_v001_c00"},
    Cue{game::Id::FF1, 60, 119, 56, L"sc_e_0028", L"Fairy release lights cue"},
    Cue{game::Id::FF1, 52, 67, 46, L"sc_e_0030", L"Onrac woman vanishes cue", CueTrigger::PlayAnim, L"mo_ff1_n029_c00"},
    Cue{game::Id::FF1, 98, 19, 59, L"sc_e_0031_1", L"Onrac barrel surfaces cue", CueTrigger::PlayAnim, L"mo_ff1_v005_c00"},
    Cue{game::Id::FF1, 96, 26, 111, L"sc_e_0036_2", L"Crystal altar restoration cue", CueTrigger::PlayAnim, L"mo_ff1_b022_c00"},
    Cue{game::Id::FF1, 112, 9, 109, L"sc_e_0037_2", L"Crystal altar restoration cue", CueTrigger::PlayAnim, L"mo_ff1_b022_c01"},
    Cue{game::Id::FF1, 72, 123, 153, L"sc_e_0038", L"Black Crystal fusion cue"},
    Cue{game::Id::FF1, 123, 1, 18, L"sc_e_0001_5", L"Opening crystals glow cue"},
    Cue{game::Id::FF1, 127, 3, 67, L"sc_e_0044_3", L"Ending title card cue", CueTrigger::Show, L"mo_ff1_e113_c00"},
    Cue{game::Id::FF2, 1, 333, 31, L"sc_e_0074", L"Mysidian seal light cue"},
    Cue{game::Id::FF2, 1, 335, 81, L"sc_e_0019_1", L"Dreadnought steam cue"},
    Cue{game::Id::FF2, 1, 233, 91, L"sc_e_0035", L"Dreadnought captures airship cue", CueTrigger::MoveBirdView, L"mo_ff2_v006_c00"},
    Cue{game::Id::FF2, 1, 351, 100, L"sc_e_0039_8", L"Dreadnought escape flames cue"},
    Cue{game::Id::FF2, 1, 347, 74, L"sc_e_0081", L"Cyclone funnel cue"},
    Cue{game::Id::FF2, 1, 384, 53, L"sc_e_0086_1", L"Palamecia eruption cue"},
    Cue{game::Id::FF2, 1, 382, 34, L"sc_e_0087", L"Jade entrance light cue"},
    Cue{game::Id::FF2, 7, 18, 113, L"sc_e_0001_3", L"Altair bed stars cue"},
    Cue{game::Id::FF2, 13, 6, 204, L"sc_e_0016", L"Tobul hammer cue", CueTrigger::PlayAnim, L"mo_ff2_n022_c00", L"attackl_01"},
    Cue{game::Id::FF2, 14, 106, 182, L"sc_e_0001", L"Fynn fire ring cue"},
    Cue{game::Id::FF2, 27, 12, 225, L"sc_e_0006", L"Scott falls still cue", CueTrigger::PlayAnim, L"mo_ff2_p010_c00"},
    Cue{game::Id::FF2, 139, 21, 285, L"sc_e_0029_1", L"Snow Cavern boulder cue", CueTrigger::PlayAnim, L"mo_ff2_b002_c00"},
    Cue{game::Id::FF2, 149, 300, 96, L"sc_e_0034", L"Kashuan Sunfire cue"},
    Cue{game::Id::FF2, 175, 17, 190, L"sc_e_0039", L"Dreadnought engine ignites cue"},
    Cue{game::Id::FF2, 180, 6, 159, L"sc_e_0050", L"Deist wyvern falls cue", CueTrigger::PlayAnim, L"mo_ff2_n027_c00"},
    Cue{game::Id::FF2, 240, 44, 202, L"sc_e_0078", L"Minwu seal beam cue"},
    Cue{game::Id::FF2, 316, 48, 77, L"sc_e_0089_1", L"Emperor dissolves cue"},
    Cue{game::Id::FF2, 292, 43, 13, L"sc_e_0088", L"Jade exit lightning cue"},
    Cue{game::Id::FF2, 355, 3, 61, L"sc_e_0089_6", L"Ending title card cue", CueTrigger::Show, L"mo_ff2_e101_c00"},
    Cue{game::Id::FF2, 41, 8, 55, L"sc_e_0080_2", L"Deist wyvern lands cue", CueTrigger::PlayAnim, L"mo_ff2_n026_c00"},
    Cue{game::Id::FF3, 306, 11, 22, L"sc_e_0321", L"Crystal Tower mirror light cue"},
    Cue{game::Id::FF3, 52, 24, 14, L"sc_e_0264", L"Saronia cannon smoke cue"},
    Cue{game::Id::FF3, 2, 2339, 36, L"sc_e_0067_1_1", L"Amur Enterprise bubbles cue"},
    Cue{game::Id::FF3, 1, 73, 32, L"sc_e_0060_3", L"Hein castle flyover cue", CueTrigger::PlayAnim, L"mo_ff3_b053_c00"},
    Cue{game::Id::FF3, 4, 932, 92, L"sc_e_0074", L"Saronia Enterprise hit cue"},
    Cue{game::Id::FF3, 314, 24, 146, L"sc_e_0092_1", L"Cloud of Darkness arrival cue", CueTrigger::Show, L"mo_ff3_n033_c00"},
    Cue{game::Id::FF3, 142, 6, 85, L"sc_e_0081", L"Unei wakes cue", CueTrigger::PlayAnim, L"mo_ff3_n007_c00"},
    Cue{game::Id::FF3, 314, 22, 344, L"sc_e_0092_2", L"Doga and Unei restore the party cue"},
    Cue{game::Id::FF3, 341, 3, 83, L"sc_e_0101_19", L"Ending title card cue", CueTrigger::Show, L"mo_ff3_e102_c00"},
    Cue{game::Id::FF3, 1, 72, 37, L"sc_e_0026", L"Nepto approach Bahamut cue", CueTrigger::PlayAnim, L"mo_ff3_n038_c00"},
    Cue{game::Id::FF4, 1, 1102, 36, L"sc_e_0045_1", L"Agart eruption cue"},
    Cue{game::Id::FF4, 1, 174, 27, L"sc_e_0001", L"Red Wings flight formation cue", CueTrigger::MoveBirdView, L"mo_ff4_v003_c01", L"", 2.5},
    Cue{game::Id::FF4, 21, 0, 9, L"sc_e_0108", L"Baron chamber flashes cue", CueTrigger::ColorFade},
    Cue{game::Id::FF4, 21, 6, 69, L"sc_e_0108", L"Baron king apparition cue", CueTrigger::PlayAnim, L"mo_ff4_n006_c02"},
    Cue{game::Id::FF4, 343, 16, 228, L"sc_e_0001_2", L"Mysidia soldiers strike cue"},
    Cue{game::Id::FF4, 343, 20, 406, L"sc_e_0001_2", L"Mysidia crystal vanishes cue"},
    Cue{game::Id::FF4, 67, 63, 126, L"sc_e_0008", L"Mist Bomb ring cue"},
    Cue{game::Id::FF4, 67, 55, 138, L"sc_e_0008", L"Mist houses burn cue"},
    Cue{game::Id::FF4, 1, 164, 82, L"sc_e_0015", L"Damcyan flames cue"},
    Cue{game::Id::FF4, 1, 1019, 151, L"sc_e_0015", L"Damcyan ruins reveal cue", CueTrigger::Show, L"mo_ff4_b049_c00"},
    Cue{game::Id::FF4, 1, 193, 56, L"sc_e_0023_9", L"Fabul ship lurches cue", CueTrigger::MoveBirdView, L"mo_ff4_v001_c00"},
    Cue{game::Id::FF4, 350, 21, 87, L"sc_e_0029_1", L"Paladin trial sword light cue"},
    Cue{game::Id::FF4, 100, 18, 146, L"sc_e_0081_1", L"Edward plays harp cue", CueTrigger::PlayAnim, L"mo_ff4_p007_c00"},
    Cue{game::Id::FF4, 208, 13, 290, L"sc_e_0042_1", L"Tower of Zot lightning cue"},
    Cue{game::Id::FF4, 220, 17, 97, L"sc_e_0052_2", L"Super cannon bursts cue"},
    Cue{game::Id::FF4, 1, 1088, 48, L"sc_e_0053_10", L"Underground passage smoke cue"},
    Cue{game::Id::FF4, 239, 33, 129, L"sc_e_0056_1", L"Edge anger flare cue"},
    Cue{game::Id::FF4, 1, 100, 50, L"sc_e_0068_5", L"Lunar Whale rises cue", CueTrigger::MoveBirdView, L"mo_ff4_v006_c00"},
    Cue{game::Id::FF4, 1, 0, 16, L"sc_e_0070_5", L"Giant awakening red light cue", CueTrigger::ColorFade},
    Cue{game::Id::FF4, 1, 1087, 97, L"sc_e_0070_5", L"Giant awakening smoke cue"},
    Cue{game::Id::FF4, 1, 85, 103, L"sc_e_0070_5", L"Giant awakening reveal cue", CueTrigger::Show, L"mo_ff4_n039_c00"},
    Cue{game::Id::FF4, 1, 95, 154, L"sc_e_0070_5", L"Giant awakening attack cue"},
    Cue{game::Id::FF4, 1, 1107, 126, L"sc_e_0070_14", L"Giant of Babil explosions cue"},
    // The CPU aftermath cuts to a separate, dialogue-free exterior shot.
    Cue{game::Id::FF4, 1, 1107, 180, L"sc_e_0072_2", L"Giant exterior destruction cue"},
    Cue{game::Id::FF4, 358, 1, 19, L"sc_e_0072_5", L"Lunar Whale second moon transit cue"},
    Cue{game::Id::FF4, 161, 0, 1, L"sc_ship_0004", L"Lunar Whale crystal flashes cue", CueTrigger::ColorFade},
    Cue{game::Id::FF4, 357, 0, 1, L"sc_ship_0003", L"Lunar Whale crystal flashes cue", CueTrigger::ColorFade},
    Cue{game::Id::FF4, 355, 1, 19, L"sc_ship_0004_2", L"Lunar Whale spaceflight cue"},
    Cue{game::Id::FF4, 358, 1, 19, L"sc_ship_0003_2", L"Lunar Whale spaceflight cue"},
    Cue{game::Id::FF4, 248, 11, 67, L"sc_e_0071_1", L"Scarmiglione defeat cue"},
    Cue{game::Id::FF4, 248, 12, 80, L"sc_e_0071_1", L"Cagnazzo defeat cue"},
    Cue{game::Id::FF4, 248, 13, 93, L"sc_e_0071_1", L"Barbariccia defeat cue"},
    Cue{game::Id::FF4, 248, 14, 105, L"sc_e_0071_1", L"Rubicante defeat cue"},
    Cue{game::Id::FF4, 154, 20, 79, L"sc_e_0073", L"Lunar Subterrane transport cue"},
    Cue{game::Id::FF4, 296, 0, 9, L"sc_e_0073_1", L"Lunar Subterrane arrival cue", CueTrigger::ColorFade},
    Cue{game::Id::FF4, 298, 20, 14, L"sc_e_0116", L"Lunar Subterrane sealed sword cue"},
    Cue{game::Id::FF4, 302, 0, 4, L"sc_e_0117", L"Lunar Subterrane blue chamber cue", CueTrigger::ColorFade},
    Cue{game::Id::FF4, 312, 0, 7, L"sc_e_0118", L"Lunar Subterrane green chamber cue", CueTrigger::ColorFade},
    Cue{game::Id::FF4, 317, 7, 226, L"sc_e_0074_1", L"Zeromus figure forms cue", CueTrigger::PlayAnim, L"mo_ff4_n038_c00"},
    Cue{game::Id::FF4, 317, 13, 203, L"sc_e_0074_5", L"Zeromus final burst cue"},
    Cue{game::Id::FF4, 329, 27, 135, L"sc_e_0074_20", L"Baron wedding Edge leap cue", CueTrigger::PlayAnim, L"mo_ff4_p011_c00"},
    Cue{game::Id::FF4, 329, 3, 146, L"sc_e_0074_20", L"Baron wedding raised hands cue", CueTrigger::PlayAnim, L"mo_ff4_p003_c00"},
    Cue{game::Id::FF4, 351, 3, 16, L"sc_e_0074_16", L"Mount Ordeals Kain epilogue cue", CueTrigger::PlayAnim, L"mo_ff4_p002_c00"},
    Cue{game::Id::FF4, 354, 3, 70, L"sc_e_0074_21", L"Ending title card cue", CueTrigger::Show, L"mo_ff4_e101_c00"},
    Cue{game::Id::FF5, 1, 431, 35, L"sc_e_0067_2", L"Ronka rising cue"},
    Cue{game::Id::FF5, 1, 431, 37, L"sc_e_0514_2", L"Ronka rising cue"},
    Cue{game::Id::FF5, 1, 723, 38, L"sc_e_0476_1", L"Ghido shrine spray cue"},
    Cue{game::Id::FF5, 1, 437, 87, L"sc_e_0075_8", L"Ronka ruins explosions cue"},
    Cue{game::Id::FF5, 71, 48, 169, L"sc_e_0001", L"Tycoon wind drake cue", CueTrigger::PlayAnim, L"mo_ff5_n042_c00"},
    Cue{game::Id::FF5, 209, 78, 190, L"sc_e_0017_9", L"Wind Shrine shards glow cue"},
    Cue{game::Id::FF5, 211, 19, 210, L"sc_e_0030", L"Siren mirage appears cue", CueTrigger::PlayAnim, L"mo_ff5_n103_c02"},
    Cue{game::Id::FF5, 271, 42, 172, L"sc_e_0046_1", L"Fire-Powered Ship engine blasts cue"},
    Cue{game::Id::FF5, 1, 320, 43, L"sc_e_0048_2", L"Karnak Castle blasts cue"},
    Cue{game::Id::FF5, 1, 162, 33, L"sc_e_0066_1", L"Catapult hatch opens cue", CueTrigger::PlayAnim, L"mo_ff5_b052_c00"},
    Cue{game::Id::FF5, 1, 500, 51, L"sc_e_0066_1", L"Catapult airship launch cue", CueTrigger::PlayAnim, L"mo_ff5_v004_c00"},
    Cue{game::Id::FF5, 584, 16, 187, L"sc_e_0084_1", L"Meteor journey light cue"},
    Cue{game::Id::FF5, 136, 31, 43, L"sc_e_0094", L"Moogle village scatters cue", CueTrigger::PlayAnim, L"mo_ff5_n119_c00"},
    Cue{game::Id::FF5, 19, 809, 29, L"sc_e_0137_2", L"Merged world water cue"},
    Cue{game::Id::FF5, 19, 400, 55, L"sc_e_0137_4", L"Merged world Bahamut cue", CueTrigger::Show, L"mo_ff5_n045_c00"},
    Cue{game::Id::FF5, 19, 800, 38, L"sc_e_0138_1", L"Void sphere cue"},
    Cue{game::Id::FF5, 569, 33, 112, L"sc_e_0161_9", L"Rift final dark sphere cue"},
    Cue{game::Id::FF5, 209, 75, 114, L"sc_e_0001_4", L"Wind Shrine shatter cue"},
    Cue{game::Id::FF5, 596, 2, 21, L"sc_e_0001_6", L"Opening meteor cue", CueTrigger::PlayAnim, L"", L"", 0.7},
    Cue{game::Id::FF5, 629, 5, 46, L"sc_e_0162_47", L"Ending white birds cue"},
    Cue{game::Id::FF5, 644, 12, 62, L"sc_e_0162_48", L"Ending mounted drake cue", CueTrigger::PlayAnim, L"mo_ff5_n050_c01"},
    Cue{game::Id::FF5, 644, 4, 134, L"sc_e_0162_48", L"Ending mounted chocobo cue", CueTrigger::Show, L"mo_ff5_n047_c02"},
    Cue{game::Id::FF5, 406, 12, 156, L"sc_e_0125_1", L"Exdeath castle shards cue"},
    Cue{game::Id::FF6, 1, 247, 44, L"sc_e_0036_2", L"Figaro first descent cue"},
    Cue{game::Id::FF6, 580, 7, 117, L"sc_e_0012_b", L"Frozen esper shines cue", CueTrigger::PlayAnim, L"mo_ff6_n085_c00"},
    Cue{game::Id::FF6, 50, 11, 136, L"sc_e_0079", L"Celes in the cell cue", CueTrigger::PlayAnim, L"mo_ff6_p007"},
    Cue{game::Id::FF6, 75, 72, 149, L"sc_e_0118_2", L"Doma guards fall cue", CueTrigger::PlayAnim, L"mo_ff6_n078_c01"},
    Cue{game::Id::FF6, 581, 8, 189, L"sc_e_0259_b", L"Terra esper form appears cue", CueTrigger::PlayAnim, L"mo_ff6_p015"},
    Cue{game::Id::FF6, 607, 9, 145, L"sc_e_0558_4", L"Opera soldiers onstage cue", CueTrigger::PlayAnim, L"mo_ff6_n003_c05"},
    Cue{game::Id::FF6, 1, 255, 49, L"sc_e_0558_9", L"Blackjack lift off cue", CueTrigger::PlayAnim, L"mo_ff6_v004_c00"},
    Cue{game::Id::FF6, 180, 219, 234, L"sc_e_0555_5", L"Thamasa magicite forms cue", CueTrigger::PlayAnim, L"mo_ff6_b065_c00"},
    Cue{game::Id::FF6, 2, 193, 44, L"sc_e_0673_1", L"Figaro return cue"},
    Cue{game::Id::FF6, 2, 197, 35, L"sc_e_1002_2", L"Light of Judgment sweep cue"},
    Cue{game::Id::FF6, 431, 61, 70, L"sc_e_0607_2", L"Warring Triad flashes cue", CueTrigger::PlayAnim, L"mo_ff6_b009_c00"},
    Cue{game::Id::FF6, 605, 6, 72, L"sc_e_0607_3", L"World of Ruin smoke cue"},
    Cue{game::Id::FF6, 1, 253, 34, L"sc_e_0607_7", L"World of Ruin shockwave cue"},
    Cue{game::Id::FF6, 584, 229, 25, L"sc_e_0607_9", L"World of Ruin planet fracture cue", CueTrigger::PlayAnim, L"", L"", 1.4},
    Cue{game::Id::FF6, 571, 28, 82, L"sc_e_1004", L"Kefka barrier cue"},
    Cue{game::Id::FF6, 592, 11, 41, L"sc_e_1032_14", L"Falcon finale flock cue", CueTrigger::PlayAnim, L"mo_ff6_e016_c00"},
    Cue{game::Id::FF6, 584, 230, 42, L"sc_e_1032_18", L"Ending title card cue", CueTrigger::Show, L"mo_ff6_e101_c00"}
};

using SceneCommand = int (*)(void*, const void*);
SceneCommand native_play = nullptr, native_show = nullptr, native_move_bird_view = nullptr, native_color_fade = nullptr;
uint64_t last_generation = 0;
std::array<bool, cues.size()> queued{};

void* running_scene()
{
    auto* interpreter = ref::instance(kIntegrator);
    return ref::rd<bool>(interpreter, fField) && ref::rd<bool>(interpreter, fWorking) &&
        !ref::rd<bool>(interpreter, fPause) ? interpreter : nullptr;
}

bool shown_particles(void* actor, int& next)
{
    if (!kEffect.is(actor) || !unity::alive(actor) || !ref::rd<bool>(actor, fPlaying)) return false;
    auto* particles = ref::rd<void*>(actor, fParticles);
    const int count = unity::array_length(particles);
    // Larger effects use consecutive slices on the same short observation.
    // The Palamecia eruption and the planet fracture use multiple slices.
    if (count < 1 || count > scene_emitter_limit) return false;
    for (int tried = 0; tried < std::min(count, 8); ++tried) {
        const int index = next % count;
        next = index + 1;
        auto* particle = unity::array_at(particles, index);
        if (unity::alive(particle) && ref::call<int>(mParticleCount, particle) > 0 &&
            displayed::renderer_visible(unity::get_component(particle, kParticleRenderer))) return true;
    }
    return false;
}

void schedule_scene_cue(size_t index, int core_index, uint64_t generation)
{
    queued[index] = true;
    // Retain only an authored row, the fixed core index and the native event
    // generation. Each deferred observation resolves its current renderer.
    auto observe = [index, core_index, generation, next = 0]() mutable {
        if (system::field_event_generation() != generation || !ui::prefs::visual_descriptions_enabled()) return false;
        const auto& cue = cues[index];
        auto* interpreter = running_scene();
        if (!interpreter || ::field::current_map_id() != cue.map ||
            ref::rd_str_bounded(interpreter, fScript, 48) != cue.script) return false;
        auto* cores = ref::rd<void*>(interpreter, fCores);
        if (core_index >= unity::array_length(cores)) return false;
        auto* core = unity::array_at(cores, core_index);
        if (!kPuppetCore.is(core) || ref::rd<int>(core, fActorId) != cue.actor) return false;
        if (fieldnav::map_transition::is_screen_fading()) return true;
        auto* actor = ref::rd<void*>(core, fActor);
        const bool drawn = cue.art.empty()
            ? shown_particles(actor, next)
            : drawn_field_actor_art(actor, cue.pose) == cue.art;
        if (!drawn) return true;
        say(T(cue.text), false);
        return false;
    };
    const double delay = cues[index].settle_seconds;
    if (delay > 0) {
        // This effect begins before an authored initial fade. Start the bounded
        // read near the end of that fade without repeatedly consulting the screen.
        frame::after_seconds(delay, [observe, generation] {
            if (system::field_event_generation() == generation && ui::prefs::visual_descriptions_enabled())
                frame::every_frame(observe, "SceneEffects.Draw", 16);
        }, "SceneEffects.Reveal");
    } else {
        frame::every_frame(observe, "SceneEffects.Draw", 16);
    }
}

void observe_first_play(void* core, CueTrigger trigger)
{
    const bool color_fade = trigger == CueTrigger::ColorFade;
    const int core_index = ref::rd<int>(core, fCoreId);
    if (color_fade ? core_index != 0 : core_index < 1 || core_index > 32) return;
    const auto generation = system::field_event_generation();
    if (!generation) return;
    const int actor = color_fade ? 0 : ref::rd<int>(core, fActorId);
    const auto title = game::current();
    const auto found = std::find_if(cues.begin(), cues.end(), [title, actor, trigger](const Cue& cue) {
        return cue.title == title && cue.actor == actor && cue.trigger == trigger;
    });
    if (found == cues.end() || !ref::call<bool>(mFirstClock, core)) return;
    const int instruction = ref::call<int>(mProgramCounter, core);
    auto* interpreter = running_scene();
    if (!interpreter) return;
    const auto script = ref::rd_str_bounded(interpreter, fScript, 48);
    const int map = ::field::current_map_id();
    if (generation != last_generation) {
        last_generation = generation;
        queued.fill(false);
    }
    for (size_t index = 0; index < cues.size(); ++index) {
        const auto& cue = cues[index];
        if (!queued[index] && cue.title == title && cue.map == map && cue.actor == actor && cue.trigger == trigger &&
            cue.instruction == instruction && cue.script == script) {
            if (!ui::prefs::visual_descriptions_enabled()) return;
            if (color_fade) {
                // This exact first-clock command has started the native color
                // filter. A loading fade can still obscure it.
                if (fieldnav::map_transition::is_screen_fading()) return;
                queued[index] = true;
                say(T(cue.text), false);
                return;
            }
            if (cue.art.empty() && !kEffect.is(ref::rd<void*>(core, fActor))) return;
            schedule_scene_cue(index, core_index, generation);
            return;
        }
    }
}

int play_anim(void* core, const void* method)
{
    const int result = native_play(core, method);
    if (frame::on_main_thread())
        hooks::guarded("SceneEffects.PlayAnim", [&] { observe_first_play(core, CueTrigger::PlayAnim); });
    return result;
}

int show(void* core, const void* method)
{
    const int result = native_show(core, method);
    if (frame::on_main_thread()) hooks::guarded("SceneEffects.Show", [&] { observe_first_play(core, CueTrigger::Show); });
    return result;
}

int move_bird_view(void* core, const void* method)
{
    const int result = native_move_bird_view(core, method);
    if (frame::on_main_thread())
        hooks::guarded("SceneEffects.MoveBirdView", [&] { observe_first_play(core, CueTrigger::MoveBirdView); });
    return result;
}

int color_fade(void* core, const void* method)
{
    const int result = native_color_fade(core, method);
    if (frame::on_main_thread())
        hooks::guarded("SceneEffects.ColorFade", [&] { observe_first_play(core, CueTrigger::ColorFade); });
    return result;
}

int install_effect_cues()
{
    int count = hooks::attach(mPlayAnim, &play_anim, &native_play) ? 1 : 0;
    if (hooks::attach(mShow, &show, &native_show)) ++count;
    if (hooks::attach(mMoveBirdView, &move_bird_view, &native_move_bird_view)) ++count;
    if (hooks::attach(mColorFade, &color_fade, &native_color_fade)) ++count;
    return count;
}
core::Install install{"SceneEffects", &install_effect_cues, 130};
}
} // namespace ffpr::dialogue
