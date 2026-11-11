#pragma once
#include "story_model.h"

namespace ffpr::ff6::story {
// The only roof clock is the game's current five-second0850 timer. Neither
// worker planning nor the mod's optional waiting cue advances or freezes it.
inline bool roof_fresh(const Snapshot& s) {
    return s.roof_seconds&&*s.roof_seconds>0&&*s.roof_seconds<=5.001f&&
        (s.roof_frozen||*s.roof_seconds>=4.75f);
}
Plan roof_plan(const Snapshot&,const json::Value& guide); // owned worker data
void capture_roof_timer(Snapshot&); // native owner only; no retained pointers
bool install_roof_timer(void(*reset)(),bool(*wanted)()); // one exact reset producer
}
