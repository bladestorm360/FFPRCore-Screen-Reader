#pragma once
#include <string>
namespace ffpr::battle::action_effects {
// Native action lifetime only; the result's own function supplies its ability.
void observe_action(void* act,const std::wstring& actor,const std::wstring& description);
void finish_action();
// Native commandless NotDelete Boost follow-ups reuse the parent heading (or
// have no heading). Self-Destruct remains a separate displayed action.
bool redundant_boost_use(void* act);
// The anonymous Invincible support action and its later native conclusion share
// one displayed cue. Identity only; neither function observes renderer changes.
bool invincible_artillery_action();
bool invincible_artillery_result(void* ability);
// Existing native CreateDamageView phase, including zero-value condition hits.
// Never called by persistent aura or sprite-animation lifecycle events.
void observe_result(void* function,void* target,int value,int hit_type);
}
