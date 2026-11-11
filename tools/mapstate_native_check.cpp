// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Actual native snapshot reader against build-matched IL2CPP layouts; no game loaded.
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include "../src/core/field/map_state.cpp"
namespace fixture {
const DWORD owner = GetCurrentThreadId();
std::atomic<int> wrong_owner{0};
int failures = 0;
bool unresolved_flag_field=false;
bool missing_word_field=false,missing_word_user=false;
bool missing_control=false,bad_control=false,hidden_control=false,hidden_parent=false,dead_control=false;
int native_control_reads=0,current_map=256;
void native() { if (GetCurrentThreadId() != owner) ++wrong_owner; }
void expect(bool ok, const char* label) { if (!ok) { ++failures; std::printf("FAIL %s\n", label); } }
template<class T, size_t N> struct Array {
    uintptr_t header[3]{};
    uintptr_t length = N;
    std::array<T, N> values{};
};
struct Packed { uintptr_t header[2]{}; void* bits = nullptr; int length = 1024; };
struct Storage { uintptr_t header[2]{}; Packed* scenario; Packed* treasure; void* global; void* area; void* map; void* script=nullptr; void* labels=nullptr; int selected=0,item=0,map_id=318; };
Array<uint32_t, 128> scenario, treasure;
Array<int32_t, 16> global;
Array<int32_t, 8> area,local;
Packed scenario_flags{{}, &scenario}, treasure_flags{{}, &treasure};
Storage storage{{}, &scenario_flags, &treasure_flags, &global, &area, &local};
using List=std::vector<void*>;
struct User { uint8_t padding[0x7c]{}; int area_id=101;void* corps=nullptr;void* owned=nullptr;void* words=nullptr;void* normal_items=nullptr; } user;
struct WordList {uintptr_t header[2]{};void* items=nullptr;int size=0;} word_list;
Array<int32_t,32> word_array;
static_assert(offsetof(User,words)==0x90);
struct MiniMember { int id=0;void* parameter=nullptr;void* save=reinterpret_cast<void*>(uintptr_t(1));int job=1;bool enabled=true;int status=1; };
int locke_status_reads=0;bool missing_locke_method=false;
int locke_status(void*p,const void*){native();++locke_status_reads;return static_cast<MiniMember*>(p)->status;}
struct KeyItem {int item=14,content=15,quantity=1;void* save=reinterpret_cast<void*>(uintptr_t(1));};
struct KeyDictionary {std::map<int,void*> values;int size_override=-1;};
bool bad_key_type=false,missing_key_field=false,missing_key_method=false;
int key_lookups=0,key_lookup_id=-1,key_getters=0,job_getters=0;
int key_count(void*p,const void*){native();++key_getters;return static_cast<KeyItem*>(p)->quantity;}
int key_item_id(void*p,const void*){native();++key_getters;return static_cast<KeyItem*>(p)->item;}
int key_content_id(void*p,const void*){native();++key_getters;return static_cast<KeyItem*>(p)->content;}
int key_job(void*p,const void*){native();++job_getters;return static_cast<MiniMember*>(p)->job;}
bool key_enabled(void*p,const void*){native();return static_cast<MiniMember*>(p)->enabled;}
struct MiniParameter { void* conditions=nullptr; };
struct MiniCondition { int type=0; };
struct ControlProperty {int id=13;void* name=nullptr;} control_property;
struct Control {ControlProperty* property=&control_property;bool own_hidden=false;} control;
List controls{&control};
struct Controller {void* entities=&controls;} controller;
struct Handle {void* controller=&fixture::controller;} handle;
std::wstring control_name=L"スイッチ";
void* control_entity(void*,int id,const void*){native();++native_control_reads;if(missing_control)return nullptr;
 for(void* row:controls){auto* actor=static_cast<Control*>(row);if(actor&&actor->property&&actor->property->id==id)return actor;}return nullptr;}
#ifdef FFPR_LIBRARY_OWNER_CHECK
std::map<events::Kind,std::vector<events::Handler>> event_handlers;
#endif
game::Id running_game=game::Id::FF3;
int mini_id(void*p,const void*){native();return p?static_cast<MiniMember*>(p)->id:0;}
int mini_type(void*p,const void*){native();return p?static_cast<MiniCondition*>(p)->type:0;}
void* current = &storage;
json::Value records;
void* instance(const void*) { native(); return current; }
void* user_instance(const void*) { native(); return missing_word_user&&running_game==game::Id::FF2?nullptr:&user; }
std::string read(const std::string& path) { std::ifstream f(path, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}}; }
}
namespace core { Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {} }
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
int32_t Field::get() {
    fixture::native();
    if(fixture::unresolved_flag_field && (std::strcmp(name,"scenario")==0 || std::strcmp(name,"bits")==0 || std::strcmp(name,"length")==0)) return -1;
    if(std::strcmp(name,"fieldController")==0)return offsetof(fixture::Handle,controller);
    if(std::strcmp(name,"entityList")==0)return offsetof(fixture::Controller,entities);
    if(std::strcmp(name,"entity_id")==0)return offsetof(fixture::ControlProperty,id);
    if(std::strcmp(name,"name")==0)return offsetof(fixture::ControlProperty,name);
    if(std::strcmp(cls.name,"UserDataManager")==0){
        if(std::strcmp(name,"normalOwnedItems")==0)return fixture::missing_key_field?-1:offsetof(fixture::User,normal_items);
        if(std::strcmp(name,"<OwnedKeyWaordList>k__BackingField")==0)return fixture::missing_word_field?-1:offsetof(fixture::User,words);
        if(std::strcmp(name,"<corpsList>k__BackingField")==0)return offsetof(fixture::User,corps);
        if(std::strcmp(name,"<OwnedCharacterList>k__BackingField")==0)return offsetof(fixture::User,owned);
    }
    if(std::strcmp(cls.name,"OwnedCharacterData")==0&&std::strcmp(name,"parameter")==0)return offsetof(fixture::MiniMember,parameter);
    if(std::strcmp(cls.name,"OwnedCharacterData")==0&&std::strcmp(name,"saveData")==0)return offsetof(fixture::MiniMember,save);
    if(std::strcmp(cls.name,"Corps")==0&&std::strcmp(name,"saveData")==0)return offsetof(fixture::MiniMember,save);
    if(std::strcmp(cls.name,"OwnedItemData")==0&&std::strcmp(name,"saveData")==0)return offsetof(fixture::KeyItem,save);
    if(std::strcmp(cls.name,"CharacterParameterBase")==0&&std::strcmp(name,"<CurrentConditionList>k__BackingField")==0)return offsetof(fixture::MiniParameter,conditions);
    if (std::strcmp(cls.name,"DataStorage.Flags") == 0) return std::strcmp(name,"bits") == 0 ? 0x10 : 0x18;
    if (std::strcmp(cls.name,"DataStorage") == 0) {
        if (std::strcmp(name,"scenario") == 0) return 0x10;
        if (std::strcmp(name,"treasure") == 0) return 0x18;
        if (std::strcmp(name,"global") == 0) return 0x20;
        if (std::strcmp(name,"area") == 0) return 0x28;
        if (std::strcmp(name,"map") == 0) return 0x30;
        if (std::strcmp(name,"<currentMapId>k__BackingField") == 0) return 0x50;
    }
    if (std::strcmp(cls.name,"UserDataManager")==0&&std::strcmp(name,"<CurrentAreaId>k__BackingField")==0)return 0x7c;
    return -1;
}
void* Method::ptr() { fixture::native();
    if(std::strcmp(name,"get_characterStatusId")==0)return fixture::missing_locke_method?nullptr:reinterpret_cast<void*>(&fixture::locke_status);
    if(std::strcmp(name,"get_Count")==0)return fixture::missing_key_method?nullptr:reinterpret_cast<void*>(&fixture::key_count);
    if(std::strcmp(name,"get_ItemId")==0)return reinterpret_cast<void*>(&fixture::key_item_id);
    if(std::strcmp(name,"get_ContentId")==0)return reinterpret_cast<void*>(&fixture::key_content_id);
    if(std::strcmp(name,"get_JobId")==0)return reinterpret_cast<void*>(&fixture::key_job);
    if(std::strcmp(name,"get_IsEnableCorps")==0)return reinterpret_cast<void*>(&fixture::key_enabled);
    if(std::strcmp(name,"GetFieldEntity")==0)return reinterpret_cast<void*>(&fixture::control_entity);if(std::strcmp(name,"get_Id")==0||std::strcmp(name,"get_CharacterId")==0)return reinterpret_cast<void*>(&fixture::mini_id);if(std::strcmp(name,"get_ConditionType")==0)return reinterpret_cast<void*>(&fixture::mini_type); return std::strcmp(name,"get_Instance") == 0 ? reinterpret_cast<void*>(&fixture::instance) : std::strcmp(name,"Instance")==0 ? reinterpret_cast<void*>(&fixture::user_instance) : nullptr; }
il2cpp::Method* Method::get() { fixture::native(); return reinterpret_cast<il2cpp::Method*>(ptr()); }
bool Class::is(void*p){fixture::native();return p&&!fixture::bad_control&&!(std::strcmp(name,"OwnedItemData")==0&&fixture::bad_key_type);}
std::wstring str(void*p) { fixture::native(); return p?*static_cast<std::wstring*>(p):L""; }
std::wstring rd_str_bounded(void*p,Field& f,size_t cap) {fixture::native();const auto value=str(il2cpp::read_field<void*>(p,f.get()));return value.size()<=cap?value:L"";}
void call_failed_note(const char*) { fixture::expect(false,"native getter must not fault"); }
}
namespace game {Id current(){return fixture::running_game;}}
namespace unity {
int dict_count(void*p){fixture::native();if(!p)return 0;const auto& d=*static_cast<fixture::KeyDictionary*>(p);return d.size_override>=0?d.size_override:static_cast<int>(d.values.size());}
void* dict_reference(void*p,int key){fixture::native();++fixture::key_lookups;fixture::key_lookup_id=key;if(!p)return nullptr;const auto& d=*static_cast<fixture::KeyDictionary*>(p);const auto it=d.values.find(key);return it==d.values.end()?nullptr:it->second;}
void* game_object(void*p){fixture::native();return p;}
void* transform(void*p){fixture::native();return p;}
void* parent(void*p){fixture::native();return p;}
bool active_self(void*p){fixture::native();return p&&!fixture::hidden_control&&!static_cast<fixture::Control*>(p)->own_hidden;}
bool active_in_hierarchy(void*){fixture::native();return !fixture::hidden_parent;}
int list_count(void*p){fixture::native();if(p==&fixture::word_list)return fixture::word_list.size;return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}
void* list_at(void*p,int i){fixture::native();return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}
int array_length(void* array) { fixture::native(); return il2cpp::read_field<int32_t>(array, kArrayLengthOff); }
}
namespace hooks {
bool attach_raw(ref::Method&, void*, void**) { return false; }
void note_fault(const char*) { fixture::expect(false,"map-state callback must not fault"); }
}
namespace field::baked_nav { void invalidate() {} void reset_transition_history() {} }
namespace field::scanner { void invalidate_map() {} }
namespace field::r {
ref::Class kFieldController{"Last.Map", "FieldController"};
ref::Field fMapManager{kFieldController, "mapManager"};
ref::Field fFieldController{kFieldController,"fieldController"},fEntityList{kFieldController,"entityList"};
ref::Field fPropEntityId{kFieldController,"entity_id"},fPropName{kFieldController,"name"};
ref::Class kUserDataManager{"Last.Management", "UserDataManager"};
ref::Method mUserDataInstance{kUserDataManager, "Instance", 0};
}
namespace field {
int current_map_id(){fixture::native();return fixture::current_map;}
void* map_handle(){fixture::native();return &fixture::handle;}
bool live_entity(void*p){fixture::native();return p&&!fixture::dead_control;}
void* entity_property(void*p){fixture::native();return p?static_cast<fixture::Control*>(p)->property:nullptr;}
}
namespace events { void on(Kind kind, Handler handler) {
#ifdef FFPR_LIBRARY_OWNER_CHECK
 fixture::event_handlers[kind].push_back(std::move(handler));
#endif
} }
namespace data { const json::Value& game_json(const char*) { return fixture::records; } }
int main()
{
    using namespace fixture;
    using namespace field;
    running_game=game::Id::FF2;word_list.items=&word_array;user.words=&word_list;
    expect(map_state::keyword_learned(927)==false,"initialized empty native word list proves the keyword is not learned");
    word_array.values[0]=926;word_array.values[1]=927;word_array.values[2]=931;word_list.size=3;
    expect(map_state::keyword_learned(927)==true&&map_state::keyword_learned(931)==true&&map_state::keyword_learned(928)==false,
        "native-shaped learned-word storage resolves exact content IDs, not menu indices");
    missing_word_user=true;expect(!map_state::keyword_learned(927).has_value(),"missing native user is unknown");missing_word_user=false;
    missing_word_field=true;expect(!map_state::keyword_learned(927).has_value(),"unresolved typed word field is unknown");missing_word_field=false;
    user.words=nullptr;expect(!map_state::keyword_learned(927).has_value(),"missing word list is unknown");user.words=&word_list;
    word_list.items=nullptr;expect(!map_state::keyword_learned(927).has_value(),"missing word backing storage is unknown");word_list.items=&word_array;
    word_list.size=-1;expect(!map_state::keyword_learned(927).has_value(),"negative word size is rejected");
    word_list.size=33;expect(!map_state::keyword_learned(927).has_value(),"over-budget word size is rejected before walking it");word_list.size=3;
    word_array.length=2;expect(!map_state::keyword_learned(927).has_value(),"short backing array cannot certify a learned word");
    word_array.length=129;expect(!map_state::keyword_learned(927).has_value(),"over-budget capacity is unknown");word_array.length=32;
    expect(!map_state::keyword_learned(0).has_value(),"invalid keyword ID is rejected");
    for(auto id:{game::Id::FF1,game::Id::FF3,game::Id::FF4,game::Id::FF5,game::Id::FF6}){running_game=id;expect(!map_state::keyword_learned(927).has_value(),"other games do not query FFII word storage");}
    running_game=game::Id::FF3;
    // Recovered CheckChild checks all occupied active corps slots, not only the
    // leader or every character ever owned. No interpreter state is written.
    MiniCondition mini{401};List mini_conditions{&mini},normal_conditions;
    MiniParameter small{&mini_conditions},normal{&normal_conditions};
    MiniMember a{1,&small},b{2,&small},inactive{3,&normal};
    List active{&a,&b},owned{&a,&b,&inactive};user.corps=&active;user.owned=&owned;
    expect(map_state::snapshot().party_mini==1,"all active Mini members satisfy the copied native predicate; inactive characters do not veto it");
    small.conditions=nullptr;expect(!map_state::snapshot().party_mini,"uninitialized condition storage stays unknown");small.conditions=&mini_conditions;
    mini_conditions={nullptr};expect(!map_state::snapshot().party_mini,"null condition entries stay unknown");mini_conditions={&mini};
    active={nullptr};expect(!map_state::snapshot().party_mini,"null corps entries stay unknown");active={&a,&b};
    expect(map_state::party_condition_state(401)==1&&map_state::party_condition_state(402)==0,"explicit condition reader distinguishes all Mini from no Frog");
    b.parameter=&normal;expect(map_state::snapshot().party_mini==0,"one non-Mini active member blocks the Tozus native predicate");
    expect(map_state::party_condition_state(401)==2,"mixed Mini is distinct from all and none");b.parameter=&small;
    MiniCondition frog{402};List frog_conditions{&frog};MiniParameter transformed{&frog_conditions};
    a.parameter=b.parameter=&transformed;
    expect(map_state::party_condition_state(402)==1&&map_state::party_condition_state(401)==0,"all Frog does not borrow the Mini condition");
    b.parameter=&normal;expect(map_state::party_condition_state(402)==2,"one normal member cannot certify all-party Frog removal");
    a.parameter=&normal;expect(map_state::party_condition_state(402)==0,"all normal members certify no Frog");
    a.parameter=b.parameter=&small;
    expect(!map_state::party_condition_state(403),"unreviewed condition IDs cannot use the FFIII guide reader");
    small.conditions=nullptr;expect(!map_state::party_condition_state(401),"unknown native condition storage cannot complete a transformation step");small.conditions=&mini_conditions;
    auto sig1=map_state::Resolver().signature();b.parameter=&normal;auto sig2=map_state::Resolver().signature();expect(sig1!=sig2,"party-form changes invalidate route state signatures without a story-flag write");b.parameter=&small;
    owned.erase(owned.begin());expect(!map_state::snapshot().party_mini,"missing active character data remains unknown");owned.insert(owned.begin(),&a);
    active.assign(10,&a);expect(!map_state::snapshot().party_mini,"over-bound corps rejected before reading members");active={&a,&b};
    mini_conditions.assign(129,&mini);expect(!map_state::snapshot().party_mini,"over-bound condition list remains unknown");mini_conditions={&mini};
    for(auto id:{game::Id::FF1,game::Id::FF2,game::Id::FF4,game::Id::FF5,game::Id::FF6}){running_game=id;expect(!map_state::snapshot().party_mini,"other games do not query FFIII's Mini contract");}running_game=game::Id::FF3;
    user.corps=user.owned=nullptr;
    // Recovered Flags.Get: bits[index >> 5] & (1u << (index & 31)). The
    // constructors in ALL SIX games nevertheless allocate ceil(bitLength / 8)
    // uints: 128, not 32, for 1024 logical flags. Tail words are never flags.
    for (size_t i = 0; i < 32; ++i) { scenario.values[i] = 0x80000001u; treasure.values[i] = ~scenario.values[i]; }
    for (size_t i = 32; i < 128; ++i) { scenario.values[i] = 0xdeadbeefu; treasure.values[i] = 0xffffffffu; }
    for (size_t i = 0; i < 16; ++i) global.values[i] = static_cast<int>(i) - 8;
    for(size_t i=0;i<8;++i){area.values[i]=static_cast<int>(i)-3;local.values[i]=static_cast<int>(i)+20;}
    scenario.values[0]=1u; scenario.values[6]=1u; scenario.values[31]=0x80000000u;
    expect(map_state::scenario_flag(0)==1 && map_state::scenario_flag(192)==1 && map_state::scenario_flag(1023)==1,"single-bit reader selects first, circuit and last bits");
    expect(map_state::scenario_flag(190)==0 && !map_state::scenario_flag(-1) && !map_state::scenario_flag(1024),"single-bit reader handles clear and out of range");
    current=nullptr;expect(!map_state::scenario_flag(192),"missing singleton is unknown");current=&storage;
    storage.scenario=nullptr;expect(!map_state::scenario_flag(192),"missing bank is unknown");storage.scenario=&scenario_flags;
    scenario_flags.bits=nullptr;expect(!map_state::scenario_flag(192),"missing words are unknown");scenario_flags.bits=&scenario;
    scenario.length=31;expect(!map_state::scenario_flag(192),"short backing array is unknown");scenario.length=128;
    scenario_flags.length=1000;expect(!map_state::scenario_flag(192),"invalid bank length is unknown");scenario_flags.length=1024;
    unresolved_flag_field=true;expect(!map_state::scenario_flag(192),"unresolved native fields are unknown");unresolved_flag_field=false;
    expect(map_state::local_value(318,1)==21,"single local value retains exact current storage map");
    expect(!map_state::local_value(386,1)&&!map_state::local_value(318,-1)&&!map_state::local_value(318,8),"foreign map and invalid local index remain unknown");
    local.length=7;expect(!map_state::local_value(318,1),"malformed local array cannot certify a clock result");local.length=8;
    current=nullptr;expect(!map_state::local_value(318,1),"missing local storage remains unknown");current=&storage;
    running_game=game::Id::FF5;control_property.name=&control_name;
    expect(map_state::entity_hidden(256,13,control_name)==false,"live visible ship control is pending");
    hidden_control=true;expect(map_state::entity_hidden(256,13,control_name)==true,"native Hide on retained exact actor certifies the flagless control");
    hidden_parent=true;expect(!map_state::entity_hidden(256,13,control_name),"a hidden parent cannot imitate a completed control");hidden_parent=false;
    expect(!map_state::entity_hidden(255,13,control_name)&&!map_state::entity_hidden(256,0,control_name)&&!map_state::entity_hidden(256,13,L"other"),"foreign floor, special zero identity and raw-name mismatch stay unknown");
    missing_control=true;expect(!map_state::entity_hidden(256,13,control_name),"absent actor does not establish Hide");missing_control=false;
    dead_control=true;expect(!map_state::entity_hidden(256,13,control_name),"destroyed actor remains unknown");dead_control=false;
    bad_control=true;expect(!map_state::entity_hidden(256,13,control_name),"invalid native controller is rejected");bad_control=false;
    const auto before_control_reads=native_control_reads;controls.assign(65,&control);
    expect(!map_state::entity_hidden(256,13,control_name)&&native_control_reads==before_control_reads,"over-budget list is refused before native lookup");controls={&control};
    running_game=game::Id::FF6;expect(!map_state::entity_hidden(256,13,control_name),"other games cannot borrow the reviewed FFV native lookup");running_game=game::Id::FF3;
    for (size_t i = 0; i < 32; ++i) scenario.values[i] = 0x80000001u;
    auto captured = map_state::snapshot();
    bool exact = true;
    for (size_t i = 0; i < 1024; ++i) {
        exact &= captured.scenario[i] == std::optional<int>((scenario.values[i / 32] >> (i % 32)) & 1);
        exact &= captured.treasure[i] == std::optional<int>((treasure.values[i / 32] >> (i % 32)) & 1);
    }
    expect(exact,"real 128-word allocation preserves all 1024 scenario/treasure flags");
    expect(captured.global[0] == -8 && captured.global[15] == 7,"global variables retain signed values");
    expect(captured.area[0]==-3&&captured.area[7]==4&&captured.local[0]==20&&captured.local[7]==27&&captured.current_map_id==318&&captured.current_area_id==101,
        "bounded native area/map locals retain signed state and exact ownership IDs");
    area.length=7;local.length=9;
    expect(!map_state::snapshot().area[0]&&!map_state::snapshot().local[0],"malformed local arrays stay unknown without partial reads");
    area.length=local.length=8;
    treasure.length = 32;
    expect(map_state::snapshot().treasure == captured.treasure,"minimal complete backing allocation has the same logical bits");
    for (auto length : {0, 31, 129}) {
        treasure.length = length;
        const auto missing = map_state::snapshot();
        expect(!missing.treasure[0] && !missing.treasure[1023],"invalid backing lengths remain unknown");
        expect(missing.scenario == captured.scenario,"one invalid flag bank cannot discard another valid bank");
    }
    treasure.length = 128; treasure_flags.length = 1023;
    expect(!map_state::snapshot().treasure[0],"wrong logical flag count remains unknown");
    treasure_flags.length = 1024; treasure_flags.bits = nullptr;
    expect(!map_state::snapshot().treasure[0],"missing backing array remains unknown");
    treasure_flags.bits = &treasure; current = nullptr;
    expect(!map_state::snapshot().treasure[0],"absent storage remains unknown"); current = &storage;

    for (int game = 1; game <= 6; ++game) {
        const auto root = "data/ff" + std::to_string(game) + "/";
        const auto maps = read(root + "maps.bin");
        expect(baked::load(maps.data(), maps.size()),"matching real bake loads");
        expect(json::parse(read(root + "mapstate.json"),records),"matching real saved-flag records parse");
        for (int i = 0; i < baked::submap_count(); ++i) baked::grids(*baked::submap_at(i));
        scenario.values.fill(0); treasure.values.fill(0);
        // A real later additive group is not a complete native group inventory.
        // Check every game against its actual initializer and native snapshot.
        bool additive_case=false;
        for(int i=0;i<baked::submap_count()&&!additive_case;++i) {
            const auto& sm=*baked::submap_at(i);if(sm.first_map_id<=0)continue;
            map_state::Resolver projected(map_state::snapshot(),records);
            for(const auto& object:baked::grids(sm).objects) {
                const auto group=baked::str(object.group);
                if(group.empty()||group==L"entity_default"||group==L"default"||!projected.active(sm,object))continue;
                const auto& objects=baked::grids(sm).objects;
                auto later=std::find_if(objects.begin(),objects.end(),[&](const auto& o){const auto g=baked::str(o.group);return !g.empty()&&g!=L"entity_default"&&g!=L"default"&&g!=group;});
                if(later==objects.end())continue;
                map_state::observe_current(sm.first_map_id,baked::str(later->group));map_state::Resolver observed;
                map_state::reset_observed();bool retained=false;
                std::thread observed_worker([&]{retained=observed.active(sm,object);});observed_worker.join();
                expect(retained,"later additive native observation preserves initialized groups on worker after owner reset");
                std::printf("FF%d additive observation: map%d initialized group retained=%d\n",game,sm.first_map_id,retained);
                additive_case=true;break;
            }
        }
        expect(additive_case,"real initialized/observed group witness exists for every game");
        if(game==3) {
            map_state::observe_current(4,L"ev_e_0185");
            map_state::Resolver observed;
            const auto* world=baked::submap_by_map_id(4);
            const auto& objects=baked::grids(*world).objects;
            const auto at=std::find_if(objects.begin(),objects.end(),[](const auto& o){return o.entity_id()==213&&o.type==baked::ObjType::Event;});
            expect(at!=objects.end()&&observed.scenario_value(73)==0,"actual native default capture retains Saronia prerequisite despite observed group updates");
            expect(at!=objects.end()&&observed.active(*world,*at),"native observed chain group cannot erase initialized Saronia scene group");
            std::printf("Actual native observed-group cache: Saronia flag73=%d, ordinary group active=%d (entry must use its reviewed prerequisite)\n",observed.scenario_value(73).value_or(-1),at!=objects.end()&&observed.active(*world,*at));
            map_state::reset_observed();
        }
        map_state::Resolver closed;
        treasure.values.fill(0xffffffffu);
        map_state::Resolver opened;
        int mapped = 0, unknown = 0;
        // Worker receives only owned copied flags and immutable baked data;
        // changing the native source after capture cannot change either resolver.
        std::thread worker([&] {
            for (int i = 0; i < baked::submap_count(); ++i) {
                const auto& sm = *baked::submap_at(i);
                for (const auto& object : baked::grids(sm).objects) {
                    if (object.type != baked::ObjType::TreasureBox) continue;
                    const auto before = closed.treasure_opened(sm,object);
                    const auto after = opened.treasure_opened(sm,object);
                    if (before.has_value()) { ++mapped; expect(!*before && after == true,"actual native snapshot drives cross-floor closed/open chest state"); }
                    else { ++unknown; expect(!after.has_value(),"unmapped chest remains unknown"); }
                }
            }
        }); worker.join();
        expect(mapped > 0,"each game resolves real chest records from native-shaped storage");
        std::printf("FF%d: %d native-snapshot chest mappings; %d unmapped remain unknown\n",game,mapped,unknown);
    }
    expect(wrong_owner == 0,"worker never calls native flag or reflection APIs");
    const auto start = std::chrono::steady_clock::now();
    double peak = 0;
    for (int i = 0; i < 100; ++i) {
        const auto begin = std::chrono::steady_clock::now();
        captured = map_state::snapshot();
        peak = std::max(peak,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());
    }
    std::printf("100 fake-native snapshots: %.3f ms aggregate, %.3f ms peak; zero worker native calls\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count(),peak);
    user.corps=&active;user.owned=&owned;running_game=game::Id::FF3;
    const auto mini_start=std::chrono::steady_clock::now();peak=0;
    for(int i=0;i<100;++i){const auto begin=std::chrono::steady_clock::now();captured=map_state::snapshot();peak=std::max(peak,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());}
    expect(captured.party_mini==1,"timed complete snapshot retains active Mini predicate");
    std::printf("100 fake-native snapshots including two active Mini members: %.3f ms aggregate, %.3f ms peak; actual game/native/audible cost unmeasured\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-mini_start).count(),peak);
    std::printf("%s: %d failures\n",failures ? "FAIL" : "PASS",failures);
    return failures ? 1 : 0;
}
