#pragma once
#include <array>
#include <cstdint>
#include <string>

namespace ffpr::battle::stat_feedback {
// Native-owner stack snapshots of the game's modifier dictionary. Values are
// never spoken: only the directions of actual applied changes leave this layer.
constexpr size_t slots=29;
struct Snapshot {
    void* parameter=nullptr;
    void* temporary=nullptr;
    std::array<int32_t,slots> values{};
    bool valid=false;
};
Snapshot capture(void* unit);
void updated(void* unit,const Snapshot& before);
// Current native action arguments also bind direct-only targets, even if they
// have never had a condition refresh. No global unit search is required.
void observe_action(void* actor,const std::wstring& actor_name,void* target_array);
void finish_action(void* actor,void* target_array);
void cleanup();

// Preserve the native Add phase: accumulate its nested stat refreshes, then let
// condition_added submit one combined condition/effect result. No text guard.
struct AddScope {
    void* unit;
    AddScope* previous;
    std::array<int64_t,slots> changes{};
    bool consumed=false;
    explicit AddScope(void* unit);
    ~AddScope();
};
bool announce_condition(void* unit,const std::wstring& name,int condition_type);
}
