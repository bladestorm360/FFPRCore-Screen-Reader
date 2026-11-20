#include "field_api.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../fieldnav/map_transition.h"
#include "../frame.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../refs.h"
#include "../say.h"
#include "../system/system.h"
#include "../unity.h"
#include <algorithm>
#include <array>
#include <string_view>

namespace ffpr::dialogue {
namespace {
ref::Class kPuppet{"Last.Interpreter.Instructions", "Puppet"};
ref::Method mPlayAnim{kPuppet, "PlayAnim", 1};
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

struct Cue {
    game::Id title;
    int map, actor, instruction;
    std::wstring_view script, text;
};

// Selected native PlayAnim occurrences from the frozen field corpus. A clip
// can share an actor with a setup, repeat or alternate script on the same map.
constexpr std::array cues{
    Cue{game::Id::FF1, 1, 204, 70, L"sc_e_0015_1", L"Canal excavation cue"},
    Cue{game::Id::FF1, 1, 91, 91, L"sc_e_0025_2", L"Levistone sand cue"},
    Cue{game::Id::FF1, 60, 119, 56, L"sc_e_0028", L"Fairy release lights cue"},
    Cue{game::Id::FF1, 123, 1, 18, L"sc_e_0001_5", L"Opening crystals glow cue"},
    Cue{game::Id::FF2, 1, 333, 31, L"sc_e_0074", L"Mysidian seal light cue"},
    Cue{game::Id::FF2, 1, 335, 81, L"sc_e_0019_1", L"Dreadnought steam cue"},
    Cue{game::Id::FF2, 1, 351, 100, L"sc_e_0039_8", L"Dreadnought escape flames cue"},
    Cue{game::Id::FF2, 1, 347, 74, L"sc_e_0081", L"Cyclone funnel cue"},
    Cue{game::Id::FF3, 306, 11, 22, L"sc_e_0321", L"Crystal Tower mirror light cue"},
    Cue{game::Id::FF3, 52, 24, 14, L"sc_e_0264", L"Saronia cannon smoke cue"},
    Cue{game::Id::FF4, 1, 1102, 36, L"sc_e_0045_1", L"Agart eruption cue"},
    Cue{game::Id::FF4, 1, 164, 82, L"sc_e_0015", L"Damcyan flames cue"},
    Cue{game::Id::FF4, 1, 1088, 48, L"sc_e_0053_10", L"Underground passage smoke cue"},
    Cue{game::Id::FF4, 1, 1107, 126, L"sc_e_0070_14", L"Giant of Babil explosions cue"},
    Cue{game::Id::FF5, 1, 431, 35, L"sc_e_0067_2", L"Ronka rising cue"},
    Cue{game::Id::FF5, 1, 431, 37, L"sc_e_0514_2", L"Ronka rising cue"},
    Cue{game::Id::FF5, 1, 723, 38, L"sc_e_0476_1", L"Ghido shrine spray cue"},
    Cue{game::Id::FF5, 1, 437, 87, L"sc_e_0075_8", L"Ronka ruins explosions cue"},
    Cue{game::Id::FF5, 19, 809, 29, L"sc_e_0137_2", L"Merged world water cue"},
    Cue{game::Id::FF5, 19, 800, 38, L"sc_e_0138_1", L"Void sphere cue"},
    Cue{game::Id::FF6, 1, 247, 44, L"sc_e_0036_2", L"Figaro first descent cue"},
    Cue{game::Id::FF6, 2, 193, 44, L"sc_e_0673_1", L"Figaro return cue"},
    Cue{game::Id::FF6, 2, 197, 35, L"sc_e_1002_2", L"Light of Judgment sweep cue"}
};

using PlayAnim = int (*)(void*, const void*);
PlayAnim native_play = nullptr;
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
    // The merged-world water effect is the largest installed bundle here.
    if (count < 1 || count > 56) return false;
    for (int tried = 0; tried < std::min(count, 8); ++tried) {
        const int index = next % count;
        next = index + 1;
        auto* particle = unity::array_at(particles, index);
        if (unity::alive(particle) && ref::call<int>(mParticleCount, particle) > 0 &&
            displayed::renderer_visible(unity::get_component(particle, kParticleRenderer))) return true;
    }
    return false;
}

void schedule_effect_cue(size_t index, int core_index, uint64_t generation)
{
    queued[index] = true;
    // Retain only an authored row, the fixed core index and the native event
    // generation. Each deferred observation resolves its current renderer.
    frame::every_frame([index, core_index, generation, next = 0]() mutable {
        if (system::field_event_generation() != generation) return false;
        const auto& cue = cues[index];
        auto* interpreter = running_scene();
        if (!interpreter || ::field::current_map_id() != cue.map ||
            ref::rd_str_bounded(interpreter, fScript, 48) != cue.script) return false;
        auto* cores = ref::rd<void*>(interpreter, fCores);
        if (core_index >= unity::array_length(cores)) return false;
        auto* core = unity::array_at(cores, core_index);
        if (!kPuppetCore.is(core) || ref::rd<int>(core, fActorId) != cue.actor) return false;
        if (fieldnav::map_transition::is_screen_fading() || !shown_particles(ref::rd<void*>(core, fActor), next)) return true;
        say(T(cue.text), false);
        return false;
    }, "SceneEffects.Draw", 16);
}

void observe_first_play(void* core)
{
    const auto generation = system::field_event_generation();
    if (!generation) return;
    const int actor = ref::rd<int>(core, fActorId);
    const auto title = game::current();
    const auto found = std::find_if(cues.begin(), cues.end(), [title, actor](const Cue& cue) {
        return cue.title == title && cue.actor == actor;
    });
    if (found == cues.end() || !ref::call<bool>(mFirstClock, core) ||
        !kEffect.is(ref::rd<void*>(core, fActor))) return;
    const int instruction = ref::call<int>(mProgramCounter, core);
    const int core_index = ref::rd<int>(core, fCoreId);
    if (core_index < 1 || core_index > 32) return;
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
        if (!queued[index] && cue.title == title && cue.map == map && cue.actor == actor &&
            cue.instruction == instruction && cue.script == script) {
            schedule_effect_cue(index, core_index, generation);
            return;
        }
    }
}

int play_anim(void* core, const void* method)
{
    const int result = native_play(core, method);
    if (frame::on_main_thread()) hooks::guarded("SceneEffects.PlayAnim", [&] { observe_first_play(core); });
    return result;
}

int install_effect_cues() { return hooks::attach(mPlayAnim, &play_anim, &native_play) ? 1 : 0; }
core::Install install{"SceneEffects", &install_effect_cues, 130};
}
} // namespace ffpr::dialogue
