#include "custom_cues.h"
#include "cue_runtime.h"
#include "volume_settings.h"
#include "../settings.h"
#include <algorithm>

namespace sound::custom {
namespace {
#define CUSTOM_KEYS(file) {"CustomSound." file ".Enabled","CustomSound." file ".Volume","CustomSound." file ".Interval","CustomSound." file ".Loop"}
constexpr std::array<Keys,cues::count> names{{
 CUSTOM_KEYS("footstep"),CUSTOM_KEYS("wall_bump"),CUSTOM_KEYS("wall_north"),CUSTOM_KEYS("wall_south"),
 CUSTOM_KEYS("wall_east"),CUSTOM_KEYS("wall_west"),CUSTOM_KEYS("beacon"),CUSTOM_KEYS("beacon_blocked"),
 CUSTOM_KEYS("exp_counter"),CUSTOM_KEYS("landing"),CUSTOM_KEYS("npc"),CUSTOM_KEYS("chest"),
 CUSTOM_KEYS("exit"),CUSTOM_KEYS("door"),CUSTOM_KEYS("event"),CUSTOM_KEYS("trap"),CUSTOM_KEYS("vehicle"),CUSTOM_KEYS("save_point")
}};
#undef CUSTOM_KEYS
}
const Keys& keys(cues::Id cue) { return names.at(size_t(cue)); }
Options capture() {
    Options result;result.enabled=settings::get_bool(enabled_key,true);
    if(!result.enabled)return result;
    for(size_t i=0;i<cues::count;++i) {
        const auto id=cues::Id(i);const auto& key=names[i];auto& out=result.cues[i];
        out.enabled=settings::get_bool(key.enabled,true);
        out.gain=levels::get(key.volume,50)/50.f;
        if(periodic(id))out.repeat_ms=clamp_interval(settings::get_int(key.repeat,0));
        if(wall(id))out.loop=settings::get_bool(key.loop,true);
    }
    return result;
}
int override_ms(cues::Id cue) {
    const auto i=size_t(cue);
    if(i>=cues::count||!periodic(cue)||!(runtime::custom_available_mask()&(uint32_t(1)<<i))||
       !settings::get_bool(enabled_key,true)||!settings::get_bool(names[i].enabled,true))return 0;
    return clamp_interval(settings::get_int(names[i].repeat,0));
}
}
