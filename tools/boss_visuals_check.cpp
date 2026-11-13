// check-sources: json.cpp text.cpp
// Run the production detours against native-layout stand-ins and the shipped catalogs.
#include "../src/core/battle/boss_visuals.cpp"
#include "../src/core/json.h"
#include <array>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string_view>

namespace fixture {
game::Id title = game::Id::FF4;
bool main_thread = true;
bool stage_art = true;
bool renderer_visible = true;
bool finish_mesh = false;
bool forwarded = true;
int spoken = 0;
std::wstring last;

struct Monster { int id, asset; };
struct Mesh { void* sprite; void* renderer; };
struct Animator;
struct State { Animator* animator; };
struct Entity { Mesh* body; State* state; };
struct Animator { Entity* entity; void* staged; };
struct Unit { Entity* entity; Monster* master; };
struct Targets { int length; Unit* unit; };
struct Act { Targets* targets; };
struct Function { Act* act; int value1, value2, value3; Monster* monster; };

int previous_art, next_art, shadow;
Mesh mesh{&previous_art, &mesh};
State state;
Entity entity{&mesh, &state};
Animator animator{&entity, nullptr};
Monster master{188, 202}, open_wings{281, 245};
Unit unit{&entity, &master};
Targets targets{1, &unit};
Act act{&targets};
Function fn{&act, 0, 201, 167, &open_wings};
const void* method_info = &fn;
const unity::Vec2 face{1, 2}, mouth{3, 4};

int monster_id(void* self, const void*) { return static_cast<Monster*>(self)->id; }
int monster_asset(void* self, const void*) { return static_cast<Monster*>(self)->asset; }
void stage(void* self, void* art, void* shadow_sprite, unity::Vec2 first, unity::Vec2 second, const void* mi)
{
    forwarded &= self == &animator && shadow_sprite == &shadow && mi == method_info &&
        first.x == face.x && first.y == face.y && second.x == mouth.x && second.y == mouth.y;
    static_cast<Animator*>(self)->staged = art;
}
void* incoming_art = &next_art;
void create(void* self, const void* mi)
{
    forwarded &= self == &fn && mi == method_info;
    if (stage_art) ffpr::battle::boss_visuals::d_stage_sprite(&animator, incoming_art, &shadow, face, mouth, mi);
}
void commit(void* self, const void* mi)
{
    forwarded &= self == &animator && mi == method_info;
    mesh.sprite = static_cast<Animator*>(self)->staged;
    finish_mesh = true;
}
}

namespace core { Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {} }
namespace game { Id current() { return fixture::title; } }
namespace frame { bool on_main_thread() { return fixture::main_thread; } }
namespace events { void on(Kind, Handler) {} }
namespace hooks {
bool attach_raw(ref::Method&, void*, void**) { return false; }
void note_fault(const char*) { std::abort(); }
}
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
bool Class::is(void* object) { return object == &fixture::unit; }
int32_t Field::get() { return off_; }
il2cpp::Method* Method::get() { return nullptr; }
void* Method::ptr() {
    if (this == &ffpr::battle::boss_visuals::mId) return reinterpret_cast<void*>(&fixture::monster_id);
    if (this == &ffpr::battle::boss_visuals::mAsset) return reinterpret_cast<void*>(&fixture::monster_asset);
    return nullptr;
}
void call_failed_note(const char*) { std::abort(); }
std::wstring str(void*) { return {}; }
}
namespace unity {
int array_length(void* array) { return array ? static_cast<fixture::Targets*>(array)->length : 0; }
void* array_at(void* array, int) { return static_cast<fixture::Targets*>(array)->unit; }
void* transform(void*) { return nullptr; }
Vec3 local_position(void*) { return {}; }
}
namespace displayed {
bool container_visible(void*) { return false; }
bool renderer_visible(void* renderer) { return renderer == &fixture::mesh && fixture::renderer_visible; }
}
namespace ffpr::battle {
ref::Class kBattleEnemyData{"Last.Battle", "BattleEnemyData"};
void* unit_info(void* unit) { return unit; }
void* function_unit(void*) { return nullptr; }
}
namespace modtext { std::wstring T(std::wstring_view key) { return std::wstring(key); } }
namespace ffpr {
void say(std::wstring_view key, bool interrupt) {
    fixture::forwarded &= !interrupt && fixture::finish_mesh;
    ++fixture::spoken;
    fixture::last = key;
}
}

namespace {
using namespace ffpr::battle::boss_visuals;
using game::Id;
int failures = 0;

void expect(bool passed, const char* why)
{
    if (passed) return;
    ++failures;
    std::printf("FAIL %s\n",why);
}

json::Value catalog(int title)
{
    std::ifstream input("data/ff"+std::to_string(title)+"/mod_text.json",std::ios::binary);
    const std::string bytes(std::istreambuf_iterator<char>(input),{});
    json::Value result;
    expect(json::parse(bytes,result),"Game mod text parses");
    return result;
}

void check_key(const json::Value& catalog, const wchar_t* key)
{
    const auto* entry = key ? catalog.get(key) : nullptr;
    expect(entry != nullptr,"Audited visual has a localized cue");
    if (!entry) return;
    constexpr std::array locales{L"en",L"ja",L"fr",L"it",L"de",L"es",L"ko",L"zht",L"zhc",L"ru",L"th",L"pt"};
    for (auto* language : locales) {
        const auto* text=entry->get(language);
        expect(text&&text->is_string()&&!text->s.empty(),"Cue is present in each game language");
    }
}

void check_detours()
{
    using namespace fixture;
    fAct.off_ = offsetof(Function, act);
    fTargets.off_ = offsetof(Act, targets);
    fEntity.off_ = offsetof(Unit, entity);
    fMaster.off_ = offsetof(Unit, master);
    fValue1.off_ = offsetof(Function, value1);
    fValue2.off_ = offsetof(Function, value2);
    fValue3.off_ = offsetof(Function, value3);
    fDeformedMonster.off_ = offsetof(Function, monster);
    fAnimState.off_ = offsetof(Entity, state);
    fAnimator.off_ = offsetof(State, animator);
    fAnimatorEntity.off_ = offsetof(Animator, entity);
    fBody.off_ = offsetof(Entity, body);
    fArt.off_ = offsetof(Mesh, sprite);
    fRenderer.off_ = offsetof(Mesh, renderer);
    state.animator = &animator;
    o_create_sprite = o_create_deformation = &create;
    o_stage_sprite = &stage;
    o_commit_sprite = o_exit_sprite = &commit;

    d_create_sprite(&fn, method_info);
    d_commit_sprite(&animator, method_info);
    expect(spoken == 0, "Battle entrance cannot queue a form cue");
    open();
    mesh.sprite = &previous_art;
    finish_mesh = false;
    d_create_sprite(&fn, method_info);
    expect(spoken == 0 && pending == 1, "FFIV type 45 stages the cloak opening without early speech");
    // Native ChangeSprite displays the previous art before ExecuteChangeSprite.
    mesh.sprite = &previous_art;
    d_commit_sprite(&animator, method_info);
    d_commit_sprite(&animator, method_info);
    expect(spoken == 1 && last == L"BossVisual.CloakOpen", "A completed cloak opening is spoken once");

    fn.value2 = 202;
    incoming_art = &previous_art;
    finish_mesh = false;
    d_create_sprite(&fn, method_info);
    d_commit_sprite(&animator, method_info);
    expect(spoken == 2 && last == L"BossVisual.CloakClosed", "A completed cloak closure is spoken");
    fn.value3 = 0;
    d_create_sprite(&fn, method_info);
    d_commit_sprite(&animator, method_info);
    expect(spoken == 2, "A mode reset that keeps the closed cloak stays quiet");

    fn.value2 = 201;
    fn.value3 = 167;
    incoming_art = &next_art;
    stage_art = false;
    d_create_sprite(&fn, method_info);
    d_commit_sprite(&animator, method_info);
    expect(spoken == 2 && pending == 0, "A condition can make the game skip staging");
    stage_art = true;
    d_create_sprite(&fn, method_info);
    d_stage_sprite(&animator, &previous_art, &shadow, face, mouth, method_info);
    d_commit_sprite(&animator, method_info);
    expect(spoken == 2 && pending == 0, "An unrelated restage cancels the old request");

    master.id = 226;
    d_create_sprite(&fn, method_info);
    d_commit_sprite(&animator, method_info);
    expect(spoken == 2, "The scripted encounter keeps its native Rubicante dialogue");

    fixture::title = Id::FF5;
    master.id = 281;
    fn.value1 = 2;
    fn.value2 = 246;
    fn.value3 = 0;
    incoming_art = &previous_art;
    finish_mesh = false;
    fixture::renderer_visible = false;
    d_create_sprite(&fn, method_info);
    d_exit_sprite(&animator, method_info);
    expect(spoken == 2, "Hidden art stays quiet");
    fixture::renderer_visible = true;
    d_commit_sprite(&animator, method_info);
    expect(spoken == 3 && last == L"BossVisual.WingsClosed", "FFV commits a staged type-45 wing fold");

    fn.value1 = 281;
    fn.value2 = 101;
    fn.value3 = 3;
    incoming_art = &next_art;
    finish_mesh = false;
    d_create_deformation(&fn, method_info);
    d_exit_sprite(&animator, method_info);
    expect(spoken == 4 && last == L"BossVisual.WingsOpen", "FFV type 73 can reopen the wings in mode 3");

    fn.value1 = 2;
    fn.value2 = 246;
    fn.value3 = 0;
    incoming_art = &previous_art;
    d_create_sprite(&fn, method_info);
    close();
    d_commit_sprite(&animator, method_info);
    expect(spoken == 4 && pending == 0, "Battle teardown cancels pending form speech");
    expect(forwarded, "Native action arguments and original-before-speech ordering are preserved");
}
}

int main()
{
    const auto iv=catalog(4), v=catalog(5), vi=catalog(6);
    for (const auto& cue : {
         scripted_sprite(Id::FF4,178,1,191,167),scripted_sprite(Id::FF4,178,2,190,167),
         scripted_sprite(Id::FF4,165,1,176,167),scripted_sprite(Id::FF4,163,4,168,167),
         scripted_sprite(Id::FF4,171,3,182,167),scripted_sprite(Id::FF4,188,0,201,167),
         scripted_sprite(Id::FF4,188,0,202,167),scripted_sprite(Id::FF4,189,0,204,0),
         scripted_sprite(Id::FF4,189,0,203,0),scripted_sprite(Id::FF4,193,2,214,0),
         replacement(Id::FF4,540,228,227),replacement(Id::FF4,541,194,195),
         replacement(Id::FF4,542,195,228)}) check_key(iv,cue);
    for (const auto& cue : {
         deformation(Id::FF5,281,282,246,1,101),deformation(Id::FF5,282,281,245,0,101),
         scripted_sprite(Id::FF5,281,2,246,0),
         replacement(Id::FF5,445,291,290),replacement(Id::FF5,654,290,291),
         replacement(Id::FF5,655,291,292),L"BossVisual.SandwormMoved"}) check_key(v,cue);
    for (const auto& cue : {
         no_select_pose(271,0,4,true),no_select_pose(271,0,7,false),
         no_select_pose(272,0,12,true),no_select_pose(272,0,11,false),
         no_select_pose(278,0,7,false),no_select_pose(279,0,7,false),
         no_select_pose(311,0,18,true),no_select_pose(311,0,19,false),
         painting(317,300),painting(300,317)}) check_key(vi,cue);

    expect(!scripted_sprite(Id::FF4,162,1,166,167)&&!scripted_sprite(Id::FF4,162,2,165,167),
           "Mist Dragon uses native text in both directions");
    expect(!scripted_sprite(Id::FF4,171,0,182,167)&&!scripted_sprite(Id::FF4,171,2,184,167),
           "Cagnazzo retains the native lightning and buildup messages");
    expect(!scripted_sprite(Id::FF4,163,14,173,167)&&!scripted_sprite(Id::FF4,226,0,201,167),
           "The scripted Octomammoth and Rubicante text takes priority");
    expect(!replacement(Id::FF5,602,301,302)&&!replacement(Id::FF5,654,291,291),
           "Identical art and unrelated replacements are silent");
    expect(!scripted_sprite(Id::FF6,178,1,191,167)&&!shell(269,true),
           "The shell itself and the wrong title cannot produce a form cue");
    expect(!scripted_sprite(Id::FF4,163,5,168,0)&&!scripted_sprite(Id::FF4,193,1,213,0)&&
           !deformation(Id::FF5,281,282,246,2,0),
           "Mode updates carrying the existing image do not qualify");
    expect(no_select_pose(271,0,12,true)&&no_select_pose(271,0,4,true)&&
           no_select_pose(272,0,11,false)&&no_select_pose(272,0,7,false),
           "Shell cues accept both verified hide and emergence animations");
    expect(!no_select_pose(271,12,12,true)&&!no_select_pose(272,12,6,false)&&
           !no_select_pose(311,0,7,false)&&!no_select_pose(280,0,7,false),
           "Repeated and unrelated standing states are silent");
    expect(!painting(0,317)&&!painting(317,317)&&painting(317,300)&&painting(300,317),
           "Opening dialogue and recurring painting forms are distinguished");
    const SpriteChange wind{1,2,scripted_sprite(Id::FF4,178,1,191,167)};
    expect(wind.presented(2,true)&&!wind.presented(1,true)&&!wind.presented(2,false),
           "The expected sprite must actually become visible");
    expect(!SpriteChange{2,2,wind.key}.presented(2,true)&&!SpriteChange{}.presented(2,true),
           "A repeated sprite or missing request cannot produce another cue");
    check_detours();
    std::printf("Boss visual form, native text and locale checks: %d failures\n",failures);
    return failures?1:0;
}
