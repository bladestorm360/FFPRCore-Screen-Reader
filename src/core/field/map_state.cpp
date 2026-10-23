// Native entity-group changes invalidate route plans, even without a map transition.
#include "baked_nav.h"
#include "field_internal.h"
#include "../core.h"
#include "../hooks.h"
#include "../modtext.h"
#include "map_state.h"

namespace field {
namespace {
ref::Method mGroup{r::kFieldController, "SetEventEntityGroup", 1};
ref::Class kManager{"Last.Map", "MapManager"}, kModel{"Last.Map", "MapModel"};
ref::Field fModel{kManager, "currentMapModel"}, fMapId{kModel, "mapId"};
using Group = void (*)(void*, void*, const void*);
Group o_group;
void d_group(void* self, void* group, const void* mi)
{
    o_group(self, group, mi);
    hooks::guarded("Field.EntityGroup", [=] {
        void* model = ref::rd<void*>(ref::rd<void*>(self, r::fMapManager), fModel);
        if (model) map_state::observe_current(ref::rd<int32_t>(model, fMapId), ref::str(group));
        baked_nav::invalidate();
        scanner::invalidate_map();
    });
}
int install() {
    events::on(events::Kind::MapChanging, [] { map_state::reset_observed(); });
    events::on(events::Kind::TitleScreen, [] { map_state::reset_observed(); });
    return hooks::attach(mGroup, &d_group, &o_group) ? 1 : 0;
}
core::Install s_install{"Field.MapState", &install, 110};
}

namespace map_state {
namespace {
ref::Class kStorage{"Last.Interpreter", "DataStorage"}, kFlags{"Last.Interpreter", "DataStorage.Flags"};
ref::Field fScenario{kStorage, "scenario"}, fTreasure{kStorage, "treasure"}, fGlobal{kStorage, "global"};
ref::Field fArea{kStorage, "area"}, fLocal{kStorage, "map"}, fStorageMap{kStorage, "<currentMapId>k__BackingField"};
ref::Field fUserArea{r::kUserDataManager, "<CurrentAreaId>k__BackingField"};
ref::Field fBits{kFlags, "bits"}, fLength{kFlags, "length"};
ref::Method mInstance{kStorage, "get_Instance", 0};
int g_observed_map = 0;
std::wstring g_observed_group;
std::set<std::wstring> g_observed_groups;
void copy_flags(void* source, std::array<std::optional<int>, 1024>& target)
{
    void* bits = ref::rd<void*>(source, fBits);
    const int length = ref::rd<int32_t>(source, fLength);
    // All six native Flags constructors allocate ceil(bitLength / 8) uints,
    // although Get indexes 32 bits per uint. A 1024-flag bank therefore has
    // 128 backing words; only its first 32 words contain logical flags.
    const int words = unity::array_length(bits);
    if (length != 1024 || words < 32 || words > 128) return;
    for (int word = 0; word < 32; ++word) {
        const uint32_t v = unity::array_value<uint32_t>(bits, word);
        for (int bit = 0; bit < 32; ++bit) target[static_cast<size_t>(word * 32 + bit)] = (v >> bit) & 1;
    }
}
}
Flags snapshot()
{
    Flags result;
    void* storage = ref::call_static<void*>(mInstance);
    if (!storage) return result;
    copy_flags(ref::rd<void*>(storage, fScenario), result.scenario);
    copy_flags(ref::rd<void*>(storage, fTreasure), result.treasure);
    void* global = ref::rd<void*>(storage, fGlobal);
    if (unity::array_length(global) == 16)
        for (int i = 0; i < 16; ++i) result.global[static_cast<size_t>(i)] = unity::array_value<int32_t>(global, i);
    const auto copy_local = [&](ref::Field& field, auto& into) {
        void* values = ref::rd<void*>(storage, field);
        if (unity::array_length(values) == static_cast<int>(into.size()))
            for (size_t i=0; i<into.size(); ++i) into[i]=unity::array_value<int32_t>(values,static_cast<int>(i));
    };
    copy_local(fArea,result.area); copy_local(fLocal,result.local);
    result.current_map_id=ref::rd<int32_t>(storage,fStorageMap);
    if (void* user=ref::call_static<void*>(r::mUserDataInstance)) result.current_area_id=ref::rd<int32_t>(user,fUserArea);
    return result;
}
void reset_observed() {
    g_observed_map=0; g_observed_group.clear(); g_observed_groups.clear();
}
void observe_current(int map_id, std::wstring group)
{
    if (map_id==g_observed_map && group!=g_observed_group) baked_nav::reset_transition_history();
    if (map_id != g_observed_map) g_observed_groups.clear();
    if (!group.empty()) g_observed_groups.insert(group);
    g_observed_map = map_id; g_observed_group = std::move(group);
}
Resolver::Resolver() : Resolver(snapshot(), data::game_json("mapstate"))
{
    if (const auto* sm = baked::submap_by_map_id(g_observed_map))
        cache_[baked::submap_index(sm)] = {true, g_observed_group, g_observed_groups};
}
}
}
