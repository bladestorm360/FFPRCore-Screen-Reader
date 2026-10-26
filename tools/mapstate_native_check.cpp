// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Actual native snapshot reader against build-matched IL2CPP layouts; no game loaded.
#include <cstdio>
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
struct User { uint8_t padding[0x7c]{}; int area_id=101;void* corps=nullptr;void* owned=nullptr; } user;
struct MiniMember { int id=0;void* parameter=nullptr; };
struct MiniParameter { void* conditions=nullptr; };
struct MiniCondition { int type=0; };
game::Id running_game=game::Id::FF3;
int mini_id(void*p,const void*){native();return p?static_cast<MiniMember*>(p)->id:0;}
int mini_type(void*p,const void*){native();return p?static_cast<MiniCondition*>(p)->type:0;}
void* current = &storage;
json::Value records;
void* instance(const void*) { native(); return current; }
void* user_instance(const void*) { native(); return &user; }
std::string read(const std::string& path) { std::ifstream f(path, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}}; }
}
namespace core { Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {} }
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
int32_t Field::get() {
    fixture::native();
    if(std::strcmp(cls.name,"UserDataManager")==0){
        if(std::strcmp(name,"<corpsList>k__BackingField")==0)return offsetof(fixture::User,corps);
        if(std::strcmp(name,"<OwnedCharacterList>k__BackingField")==0)return offsetof(fixture::User,owned);
    }
    if(std::strcmp(cls.name,"OwnedCharacterData")==0&&std::strcmp(name,"parameter")==0)return offsetof(fixture::MiniMember,parameter);
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
void* Method::ptr() { fixture::native();if(std::strcmp(name,"get_Id")==0||std::strcmp(name,"get_CharacterId")==0)return reinterpret_cast<void*>(&fixture::mini_id);if(std::strcmp(name,"get_ConditionType")==0)return reinterpret_cast<void*>(&fixture::mini_type); return std::strcmp(name,"get_Instance") == 0 ? reinterpret_cast<void*>(&fixture::instance) : std::strcmp(name,"Instance")==0 ? reinterpret_cast<void*>(&fixture::user_instance) : nullptr; }
il2cpp::Method* Method::get() { fixture::native(); return reinterpret_cast<il2cpp::Method*>(ptr()); }
std::wstring str(void*) { fixture::native(); return {}; }
void call_failed_note(const char*) { fixture::expect(false,"native getter must not fault"); }
}
namespace game {Id current(){return fixture::running_game;}}
namespace unity {
int list_count(void*p){fixture::native();return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}
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
ref::Class kUserDataManager{"Last.Management", "UserDataManager"};
ref::Method mUserDataInstance{kUserDataManager, "Instance", 0};
}
namespace events { void on(Kind, Handler) {} }
namespace data { const json::Value& game_json(const char*) { return fixture::records; } }
int main()
{
    using namespace fixture;
    using namespace field;
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
    b.parameter=&normal;expect(map_state::snapshot().party_mini==0,"one non-Mini active member blocks the Tozus native predicate");b.parameter=&small;
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
