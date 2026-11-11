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
ref::Class kWordUser{"Last.Management","UserDataManager",game::kFF2};
ref::Method mWordInstance{kWordUser,"Instance",0};
ref::Field fWords{kWordUser,"<OwnedKeyWaordList>k__BackingField"};
ref::Class kControlController{"Last.Map","FieldController",game::kFF5};
ref::Method mGetFieldEntity{kControlController,"GetFieldEntity",1};
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
ref::Field fKeyItems{kMiniUser,"normalOwnedItems"}, fKeyCharacterSave{kMiniCharacter,"saveData"};
ref::Method mKeyJob{kMiniCharacter,"get_JobId",0}, mKeyEnabled{kMiniCharacter,"get_IsEnableCorps",0};
ref::Class kKeyItem{"Last.Data.User","OwnedItemData",game::kFF3};
ref::Field fKeyItemSave{kKeyItem,"saveData"};
ref::Method mKeyCount{kKeyItem,"get_Count",0},mKeyItemId{kKeyItem,"get_ItemId",0},mKeyContentId{kKeyItem,"get_ContentId",0};
ref::Class kFF4KeyUser{"Last.Management","UserDataManager",game::kFF4};
ref::Field fFF4KeyItems{kFF4KeyUser,"importantOwendItems"}; // native spelling
ref::Class kFF4KeyItem{"Last.Data.User","OwnedItemData",game::kFF4};
ref::Field fFF4KeySave{kFF4KeyItem,"saveData"};
ref::Method mFF4KeyCount{kFF4KeyItem,"get_Count",0},mFF4KeyContent{kFF4KeyItem,"get_ContentId",0};

std::optional<unsigned> ff4_door_keys(void* user)
{
    if (fFF4KeyItems.get()<0 || fFF4KeySave.get()<0 || !mFF4KeyCount || !mFF4KeyContent) return {};
    void* items=ref::rd<void*>(user,fFF4KeyItems);
    const int count=unity::dict_count(items);
    if (!items || count<0 || count>4096) return {};
    unsigned owned=0;
    for (size_t i=0;i<ff4_door_key_contents.size();++i) {
        const int content=ff4_door_key_contents[i];
        void* item=unity::dict_reference(items,content);
        if (!item) continue;
        if (!kFF4KeyItem.is(item) || !ref::rd<void*>(item,fFF4KeySave) ||
            ref::call<int32_t>(mFF4KeyContent,item)!=content) return {};
        const int quantity=ref::call<int32_t>(mFF4KeyCount,item);
        if (quantity<0) return {};
        if (quantity>0) owned|=1u<<i;
    }
    return owned;
}
// SearchOwnedItem6454B0 uses normalOwnedItems by content ID. Magic Key item14
// is content15, not dictionary key14. One native bucket lookup avoids scanning
// inventory on the navigation hot path; all pointers stay on the native owner.
std::optional<int> magic_key_count(void* user) {
    if(!game::is(game::Id::FF3)||!user||fKeyItems.get()<0||fKeyItemSave.get()<0||
       !mKeyCount||!mKeyItemId||!mKeyContentId)return {};
    void* items=ref::rd<void*>(user,fKeyItems);if(!items)return {};
    const int count=unity::dict_count(items);if(count<0||count>4096)return {};
    void* item=unity::dict_reference(items,15);if(!item)return 0;
    if(!kKeyItem.is(item)||!ref::rd<void*>(item,fKeyItemSave)||
       ref::call<int32_t>(mKeyItemId,item)!=14||ref::call<int32_t>(mKeyContentId,item)!=15)return {};
    const int quantity=ref::call<int32_t>(mKeyCount,item);
    return quantity>=0?std::optional<int>(quantity):std::nullopt;
}
// Current.CheckThiefIsInCorps writes map-local0 and allocates/logs through its
// visitor. Reproduce only its predicate: the enabled owned character matching
// each active corps ID has job3. No native clone, wrapper, inventory enumeration,
// HP inference or game-data write. Bounds match the existing Mini snapshot.
std::optional<bool> party_has_thief(void* user) {
    if(!game::is(game::Id::FF3)||!user||fMiniCorps.get()<0||fMiniOwned.get()<0||
       fKeyCharacterSave.get()<0||!mMiniMember||!mMiniId||!mKeyJob||!mKeyEnabled)return {};
    void* corps=ref::rd<void*>(user,fMiniCorps);void* owned=ref::rd<void*>(user,fMiniOwned);
    const int nc=unity::list_count(corps),no=unity::list_count(owned);
    if(nc<1||nc>9||no<1||no>16)return {};
    int members=0;bool thief=false;
    for(int i=0;i<nc;++i){
        void* slot=unity::list_at(corps,i);if(!slot)return {};
        const int id=ref::call<int32_t>(mMiniMember,slot);if(id<=0)continue;
        void* member=nullptr;
        for(int j=0;j<no;++j){
            void* candidate=unity::list_at(owned,j);
            if(!candidate||!kMiniCharacter.is(candidate)||!ref::rd<void*>(candidate,fKeyCharacterSave))return {};
            if(ref::call<int32_t>(mMiniId,candidate)==id){member=candidate;break;}
        }
        if(!member||!ref::call<bool>(mKeyEnabled,member))return {};
        thief|=ref::call<int32_t>(mKeyJob,member)==3;++members;
    }
    return members?std::optional<bool>(thief):std::nullopt;
}
ref::Class kLockeUser{"Last.Management","UserDataManager",game::kFF6};
ref::Class kLockeCorps{"Last.Data.User","Corps",game::kFF6};
ref::Class kLockeCharacter{"Last.Data.User","OwnedCharacterData",game::kFF6};
ref::Field fLockeCorps{kLockeUser,"<corpsList>k__BackingField"},fLockeOwned{kLockeUser,"<OwnedCharacterList>k__BackingField"};
ref::Field fLockeCorpSave{kLockeCorps,"saveData"},fLockeCharacterSave{kLockeCharacter,"saveData"};
ref::Method mLockeMember{kLockeCorps,"get_CharacterId",0},mLockeId{kLockeCharacter,"get_Id",0},mLockeStatus{kLockeCharacter,"get_characterStatusId",0};
std::optional<unsigned> active_party_roles(void* user,std::optional<int>* member_count=nullptr) {
    if(!game::is(game::Id::FF6)||!user||fLockeCorps.get()<0||fLockeOwned.get()<0||
       fLockeCorpSave.get()<0||fLockeCharacterSave.get()<0||!mLockeMember||!mLockeId||!mLockeStatus)return {};
    // Native BranchByCharacter uses the current corps and canonical status ID.
    // GetMemberData/clone helpers allocate; read their same bounded lists and
    // match owned ID directly instead. No native wrapper, write or clone runs.
    void* corps=ref::rd<void*>(user,fLockeCorps);void* owned=ref::rd<void*>(user,fLockeOwned);
    const int nc=unity::list_count(corps),no=unity::list_count(owned);
    if(nc<1||nc>16||no<1||no>64)return {};
    unsigned roles=0;int members=0;
    for(int i=0;i<nc;++i) {
        void* slot=unity::list_at(corps,i);
        if(!slot||!kLockeCorps.is(slot)||!ref::rd<void*>(slot,fLockeCorpSave))return {};
        const int id=ref::call<int32_t>(mLockeMember,slot);if(id<=0)continue;
        void* member=nullptr;
        for(int j=0;j<no;++j) {
            void* candidate=unity::list_at(owned,j);
            if(!candidate||!kLockeCharacter.is(candidate)||!ref::rd<void*>(candidate,fLockeCharacterSave))return {};
            if(ref::call<int32_t>(mLockeId,candidate)==id){member=candidate;break;}
        }
        if(!member)return {};
        const int status=ref::call<int32_t>(mLockeStatus,member);
        if(status==2)roles|=1u;
        if(status==8)roles|=2u;
        if(status==6)roles|=4u;
        if(status==12)roles|=8u;
        if(status==1)roles|=16u;
        if(status==3)roles|=32u;
        if(status==11)roles|=64u;
        ++members;
    }
    // Native BranchByNumOfMember iterates the same current corps and counts
    // nonzero resolved character IDs. Reuse the completed pass, never clone or
    // call that interpreter on the game thread. Failure leaves both unknown.
    if(member_count&&members)*member_count=members;
    return members?std::optional<unsigned>(roles):std::nullopt;
}
std::optional<int> party_condition(void* user,int condition_type,bool distinguish_mixed) {
    if(!game::is(game::Id::FF3)||!user)return {};
    if(!mMiniMember||!mMiniId||!mMiniType||fMiniCorps.get()<0||fMiniOwned.get()<0||
        fMiniParameter.get()<0||fMiniConditions.get()<0)return {};
    void* corps=ref::rd<void*>(user,fMiniCorps);void* owned=ref::rd<void*>(user,fMiniOwned);
    const int nc=unity::list_count(corps),no=unity::list_count(owned);
    if(nc<1||nc>9||no<1||no>16)return {};
    int members=0,work=512;bool all=true,any=false;
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
            mini=ref::call<int32_t>(mMiniType,condition)==condition_type;
        }
        all&=mini;any|=mini;++members;
    }
    return members?std::optional<int>(all?1:(distinguish_mixed&&any?2:0)):std::nullopt;
}
std::optional<int> party_mini(void* user) {return party_condition(user,401,false);}
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
std::optional<bool> keyword_learned(int content_id)
{
    // Native UserDataClient.AddKeyWord5DFC10 checks type6 and Contains before
    // appending to this exact typed field (FFII UserDataManager offset90).
    // Only explicit guide input reaches here; no hook/poll/worker owns this
    // pointer. Current master data has a finite word set; cap both size and
    // backing capacity before reading integers, and fail unknown on corruption.
    if(!game::is(game::Id::FF2)||content_id<=0||!mWordInstance||fWords.get()<0)return {};
    void* user=ref::call_static<void*>(mWordInstance);if(!user)return {};
    void* words=ref::rd<void*>(user,fWords);if(!words)return {};
    const int count=unity::list_count(words);if(count<0||count>32)return {};
    if(!count)return false;
    void* items=il2cpp::read_field<void*>(words,unity::kListItemsOff);if(!items)return {};
    const int capacity=unity::array_length(items);if(capacity<count||capacity>128)return {};
    for(int i=0;i<count;++i)if(unity::array_value<int32_t>(items,i)==content_id)return true;
    return false;
}
std::optional<int> party_condition_state(int condition_type)
{
    // CheckChild(401)/CheckFrog(402) share native CheckConditionCommon91B450.
    // Guidance also distinguishes mixed from none: one human does not prove
    // that every party member has returned to normal. Explicit guide requests
    // only; no native SysCall, mutation or movement-time condition scan.
    if(!game::is(game::Id::FF3)||(condition_type!=401&&condition_type!=402)||!r::mUserDataInstance)return {};
    return party_condition(ref::call_static<void*>(r::mUserDataInstance),condition_type,true);
}
std::optional<int> local_value(int map_id,int index)
{
    if(map_id<=0||index<0||index>=8||!mInstance||fStorageMap.get()<0||fLocal.get()<0)return {};
    void* storage=ref::call_static<void*>(mInstance);
    if(!storage||ref::rd<int32_t>(storage,fStorageMap)!=map_id)return {};
    void* values=ref::rd<void*>(storage,fLocal);
    if(!values||unity::array_length(values)!=8)return {};
    return unity::array_value<int32_t>(values,index);
}
std::optional<bool> entity_hidden(int map_id,int entity_id,const std::wstring& raw_name)
{
    if(!game::is(game::Id::FF5)||map_id!=current_map_id()||entity_id<=0||raw_name.empty()||
       !mGetFieldEntity||r::fFieldController.get()<0||r::fEntityList.get()<0||
       r::fPropEntityId.get()<0||r::fPropName.get()<0)return {};
    void* handle=map_handle();if(!handle)return {};
    void* controller=ref::rd<void*>(handle,r::fFieldController);
    if(!controller||!r::kFieldController.is(controller))return {};
    void* list=ref::rd<void*>(controller,r::fEntityList);
    const int count=unity::list_count(list);
    // Exact positive-ID native path scans this list without filtering Hide.
    // Keep this explicit-request observation finite; never use it per frame.
    if(!list||count<1||count>64)return {};
    void* actor=ref::call<void*>(mGetFieldEntity,controller,entity_id);
    if(!live_entity(actor))return {};
    void* property=entity_property(actor);
    if(!property||ref::rd<int32_t>(property,r::fPropEntityId)!=entity_id||
       ref::rd_str_bounded(property,r::fPropName,raw_name.size())!=raw_name)return {};
    void* object=unity::game_object(actor);if(!object)return {};
    void* transform=unity::transform(actor);
    void* parent=transform?unity::parent(transform):nullptr;
    if(parent&&!unity::active_in_hierarchy(parent))return {};
    return !unity::active_self(object);
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
        if(game::is(game::Id::FF3)) {
            result.magic_keys=magic_key_count(user);
            result.party_thief=party_has_thief(user);
        }
        if(game::is(game::Id::FF4)) result.ff4_door_keys=ff4_door_keys(user);
        if(game::is(game::Id::FF6))if(const auto roles=active_party_roles(user,&result.active_party_members)) {
            result.party_locke=(*roles&1u)!=0;
            result.party_strago=(*roles&2u)!=0;
            result.party_sabin=(*roles&4u)!=0;
            result.party_gau=(*roles&8u)!=0;
            result.party_terra=(*roles&16u)!=0;
            result.party_cyan=(*roles&32u)!=0;
            result.party_mog=(*roles&64u)!=0;
        }
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
