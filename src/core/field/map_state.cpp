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
ref::Class kMiniUser{"Last.Management","UserDataManager",game::kFF3};
ref::Field fMiniCorps{kMiniUser,"<corpsList>k__BackingField"}, fMiniOwned{kMiniUser,"<OwnedCharacterList>k__BackingField"};
ref::Class kMiniCorps{"Last.Data.User","Corps",game::kFF3};
ref::Method mMiniMember{kMiniCorps,"get_CharacterId",0};
ref::Class kMiniCharacter{"Last.Data.User","OwnedCharacterData",game::kFF3};
ref::Method mMiniId{kMiniCharacter,"get_Id",0};
ref::Field fMiniParameter{kMiniCharacter,"parameter"};
ref::Class kMiniParameter{"Last.Data","CharacterParameterBase",game::kFF3};
ref::Field fMiniConditions{kMiniParameter,"<CurrentConditionList>k__BackingField"};
ref::Class kMiniCondition{"Last.Data.Master","Condition",game::kFF3};
ref::Method mMiniType{kMiniCondition,"get_ConditionType",0};
std::optional<int> party_mini(void* user) {
    if(!game::is(game::Id::FF3)||!user)return {};
    if(!mMiniMember||!mMiniId||!mMiniType||fMiniCorps.get()<0||fMiniOwned.get()<0||
        fMiniParameter.get()<0||fMiniConditions.get()<0)return {};
    void* corps=ref::rd<void*>(user,fMiniCorps);void* owned=ref::rd<void*>(user,fMiniOwned);
    const int nc=unity::list_count(corps),no=unity::list_count(owned);
    if(nc<1||nc>9||no<1||no>16)return {};
    int members=0,work=512;bool all=true;
    for(int i=0;i<nc;++i){
        void* slot=unity::list_at(corps,i);if(!slot)return {};
        const int id=ref::call<int32_t>(mMiniMember,slot);if(id<=0)continue;
        void* member=nullptr;
        for(int j=0;j<no;++j)if(void* candidate=unity::list_at(owned,j))
            if(ref::call<int32_t>(mMiniId,candidate)==id){member=candidate;break;}
        if(!member)return {};
        void* parameter=ref::rd<void*>(member,fMiniParameter);if(!parameter)return {};
        void* conditions=ref::rd<void*>(parameter,fMiniConditions);
        if(!conditions)return {}; // uninitialized data cannot establish a failed gate
        const int count=unity::list_count(conditions);if(count<0||count>128||count>work)return {};work-=count;
        bool mini=false;
        for(int j=0;j<count&&!mini;++j){
            void* condition=unity::list_at(conditions,j);if(!condition)return {};
            mini=ref::call<int32_t>(mMiniType,condition)==401;
        }
        all&=mini;++members;
    }
    return members?std::optional<int>(all?1:0):std::nullopt;
}
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
std::optional<int> scenario_flag(int absolute_index)
{
    if (absolute_index < 0 || absolute_index >= 1024 || !mInstance ||
        fScenario.get() < 0 || fBits.get() < 0 || fLength.get() < 0) return {};
    void* storage = ref::call_static<void*>(mInstance);
    if (!storage) return {};
    void* bank = ref::rd<void*>(storage, fScenario);
    if (!bank) return {};
    void* bits = ref::rd<void*>(bank, fBits);
    if (!bits || ref::rd<int32_t>(bank, fLength) != 1024) return {};
    const int words = unity::array_length(bits);
    if (words < 32 || words > 128) return {};
    // Native Get indexes 32 bits per word even though the constructor overallocates.
    const uint32_t word = unity::array_value<uint32_t>(bits, absolute_index >> 5);
    return static_cast<int>((word >> (absolute_index & 31)) & 1u);
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
    if (void* user=ref::call_static<void*>(r::mUserDataInstance)) {
        result.current_area_id=ref::rd<int32_t>(user,fUserArea);
        result.party_mini=party_mini(user);
    }
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
    if (const auto* sm = baked::submap_by_map_id(g_observed_map)) {
        observed_index_=baked::submap_index(sm);
        observed_groups_=g_observed_groups;
    }
}
}
}
