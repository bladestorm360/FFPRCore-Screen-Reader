#pragma once
#include "cue_bank.h"
#include <array>
#include <algorithm>

namespace sound::custom {
inline constexpr const char* enabled_key="CustomSounds";
// Existing WAV-file installation is an opt-in. Defaults preserve that playback;
// an unset interval preserves the original cadence rather than changing it.
struct Cue {
    bool enabled=true,loop=true;
    int repeat_ms=0;
    float gain=1;
    bool operator==(const Cue&) const=default;
};
struct Options {
    bool enabled=true;
    std::array<Cue,::sound::cues::count> cues{};
    bool operator==(const Options&) const=default;
};
struct Keys { const char* enabled;const char* volume;const char* repeat;const char* loop; };
const Keys& keys(cues::Id cue);
inline bool wall(cues::Id cue) { return cue>=cues::Id::WallNorth&&cue<=cues::Id::WallWest; }
inline bool periodic(cues::Id cue) { return cue!=cues::Id::Footstep&&cue!=cues::Id::WallBump&&cue<cues::Id::Count; }
inline int clamp_interval(int ms) { return ms==0?0:std::clamp(ms/100*100,200,5000); }
inline bool selected(const Options& options,const cues::Bank& bank,cues::Id cue) {
    const auto index=size_t(cue);
    return index<cues::count&&options.enabled&&options.cues[index].enabled&&bank[index].custom;
}
// Native settings owner only. Fixed18 records, no files/SDK/worker waits.
Options capture();
// Existing native periodic producers only. Zero means keep their original
// timing; missing/disabled custom PCM never changes generated-sound timing.
int override_ms(cues::Id cue);
}
