#pragma once
#include <vector>
#include "cue_bank.h"

// The mod's sound effects, through the shared embedded/custom WAV cue bank and
// worker-owned SDL3 mixer with optional OpenAL Soft loopback HRTF. This contract is what the
// field, battle and audio-loop code call; every function is safe to call when audio is
// unavailable (it does nothing).
namespace sound {

enum class Direction { North, South, East, West };

/// Request the SDL owner/device worker without waiting. Idempotent; native owner.
void initialize();
void shutdown();
bool available();

void play_wall_bump();
void play_footstep();

/// Continuous wall tones for the given directions; replaces the previous set.
void play_wall_tones_looped(const std::vector<Direction>& directions);
void stop_wall_tone();
bool wall_tone_playing();

/// One beacon ping at the full map-space displacement from the player.
/// The audio worker chooses actual HRTF bearing or legacy stereo/rear pitch.
void play_beacon(float east, float north, float volume_scale, bool blocked = false);

/// Battle-result EXP counter tick (a rapid ticking tone while the bar animates).
void play_exp_counter();
void top_up_exp_counter();
void stop_exp_counter();

/// Vehicle landing ping (FF1 ToneGenerator.GenerateLandingPing), panned. `frequency` 0 = the
/// default tone; FF5 gave each direction its own (sound::constants::LandingPingFrequencies).
void play_landing_ping(float pan, float volume_scale, int frequency = 0);

/// Settings preview through the same cue bank and spatial renderer.
void preview(cues::Id cue);

} // namespace sound
