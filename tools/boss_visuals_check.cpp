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
bool visual_descriptions = true;
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
struct Function { Act* act; int value1, value2, value3; Monster* monster; Targets* before; Targets* after; };
struct StatusInfo { Targets* enemies; };
struct Status { StatusInfo* info; };
struct Manager { Status* status; };

int previous_art, next_art, shadow;
Mesh mesh{&previous_art, &mesh};
State state;
Entity entity{&mesh, &state};
Animator animator{&entity, nullptr};
Monster master{188, 202}, open_wings{281, 245};
Unit unit{&entity, &master};
Targets targets{1, &unit};
Act act{&targets};
Function fn{&act, 0, 201, 167, &open_wings, nullptr, nullptr};
Mesh lord_mesh{&next_art, &lord_mesh};
Entity lord_entity{&lord_mesh, nullptr};
Monster lord_master{195, 184};
Unit lord_unit{&lord_entity, &lord_master};
Targets lord_party{1, &lord_unit};
StatusInfo lord_info{&lord_party};
Status lord_status{&lord_info};
Manager lord_manager{&lord_status};
bool lord_drawn = false;
bool outgoing_active = false;
frame::Step lord_step;
int allowed_frames = 0;
const void* method_info = &fn;
const unity::Vec2 face{1, 2}, mouth{3, 4};

int monster_id(void* self, const void*) { return static_cast<Monster*>(self)->id; }
int monster_asset(void* self, const void*) { return static_cast<Monster*>(self)->asset; }
void* plug_instance(const void*) { return &lord_manager; }
void* current_enemies(void*, const void*) { return &targets; }
int32_t move_neo(void* interpreter, const void* mi)
{
    forwarded &= interpreter == &fn && mi == method_info;
    finish_mesh = true;
    return 3;
}
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
void complete_swap(void* self, const void* mi)
{
    forwarded &= self == &fn && mi == method_info;
    finish_mesh = true;
}
void tick_lord()
{
    if (lord_step && !lord_step()) lord_step = {};
}
}

namespace core { Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {} }
namespace game { Id current() { return fixture::title; } }
namespace frame {
bool on_main_thread() { return fixture::main_thread; }
int every_frame(Step step, const char*, int max_frames) {
    fixture::allowed_frames = max_frames;
    fixture::lord_step = std::move(step);
    return 1;
}
void cancel(int) { fixture::lord_step = {}; }
}
namespace settings {
bool get_bool(const char* key, bool fallback)
{
    return std::string_view(key) == ui::prefs::kVisualDescriptions ? fixture::visual_descriptions : fallback;
}
}
namespace events { void on(Kind, Handler) {} }
namespace hooks {
bool attach_raw(ref::Method&, void*, void**) { return false; }
void note_fault(const char*) { std::abort(); }
}
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
bool Class::is(void* object) { return object == &fixture::unit || object == &fixture::lord_unit; }
il2cpp::Class* Class::get() { return reinterpret_cast<il2cpp::Class*>(this); }
int32_t Field::get() { return off_; }
il2cpp::Method* Method::get() { return nullptr; }
void* Method::ptr() {
    if (this == &ffpr::battle::boss_visuals::mId) return reinterpret_cast<void*>(&fixture::monster_id);
    if (this == &ffpr::battle::boss_visuals::mAsset) return reinterpret_cast<void*>(&fixture::monster_asset);
    if (this == &ffpr::battle::boss_visuals::mPlugInstance) return reinterpret_cast<void*>(&fixture::plug_instance);
    if (this == &ffpr::battle::boss_visuals::mEnemyUnits) return reinterpret_cast<void*>(&fixture::current_enemies);
    return nullptr;
}
void call_failed_note(const char*) { std::abort(); }
std::wstring str(void*) { return {}; }
}
namespace il2cpp {
void* read_static_ptr(Class*, const char*) { return &fixture::lord_manager; }
}
namespace unity {
int array_length(void* array) { return array ? static_cast<fixture::Targets*>(array)->length : 0; }
void* array_at(void* array, int) { return static_cast<fixture::Targets*>(array)->unit; }
int list_count(void* list) { return array_length(list); }
void* list_at(void* list, int index) { return array_at(list, index); }
void* transform(void*) { return nullptr; }
Vec3 local_position(void*) { return {}; }
}
namespace displayed {
bool container_visible(void* entity) { return entity == &fixture::entity && fixture::outgoing_active; }
bool renderer_visible(void* renderer) {
    return (renderer == &fixture::mesh && fixture::renderer_visible) ||
           (renderer == &fixture::lord_mesh && fixture::lord_drawn);
}
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

    fixture::title = Id::FF6;
    open();
    expect(!revealed_painting_art(300) && !revealed_painting_art(317), "A new encounter has no remembered reveal");
    painting_asset = 300;
    expect(revealed_painting_art(300) && !revealed_painting_art(317), "Only the currently revealed painting qualifies");
    close();
    expect(!revealed_painting_art(300), "Battle teardown retires the last observed painting");
}

void check_asura_faces(const json::Value& catalog)
{
    using namespace fixture;
    constexpr struct Face { int asset, mode, reset_mode; const wchar_t* key; } faces[] = {
        {213, 0, 1, L"BossVisual.AsuraAngry"},
        {214, 2, 0, L"BossVisual.AsuraGrinning"},
        {212, 1, 2, L"BossVisual.AsuraCalm"},
    };
    for (const auto& face : faces) check_key(catalog, face.key);

    title = Id::FF4;
    master.id = 193;
    int sprites[3]{};
    mesh.sprite = &sprites[2]; // Asura starts with the calm face.
    open();
    for (const int effect : {0, 167}) {
        for (size_t index = 0; index < std::size(faces); ++index) {
            const auto& face = faces[index];
            const int begin = spoken;
            fn.value1 = face.mode;
            fn.value2 = face.asset;
            fn.value3 = effect;
            incoming_art = &sprites[index];
            finish_mesh = false;
            d_create_sprite(&fn, method_info);
            expect(spoken == begin, "Asura's face is silent until the new sprite is committed");
            d_commit_sprite(&animator, method_info);
            expect(spoken == begin + 1 && last == face.key, "Asura announces the actual incoming face");

            fn.value1 = face.reset_mode;
            fn.value3 = 0;
            d_create_sprite(&fn, method_info);
            d_commit_sprite(&animator, method_info);
            expect(spoken == begin + 1, "Changing Asura's mode without changing her face is silent");
        }
    }
    close();
    mesh.sprite = &previous_art;
    incoming_art = &next_art;
    expect(forwarded, "Asura's face cues queue after native sprite completion");
}

void check_lord_swaps()
{
    using namespace fixture;
    fixture::title = Id::FF4;
    fOldUnits.off_ = offsetof(Function, before);
    fNewUnits.off_ = offsetof(Function, after);
    fLordStatus.off_ = offsetof(Manager, status);
    fLordStatusInfo.off_ = offsetof(Status, info);
    fLordEnemies.off_ = offsetof(StatusInfo, enemies);
    o_start_end = &complete_swap;
    fn.before = &targets;
    fn.after = &lord_party;
    open();

    constexpr struct Swap { int party, outgoing, incoming; const wchar_t* cue; } swaps[] = {
        {541, 194, 195, L"BossVisual.LordWater"},
        {542, 195, 228, L"BossVisual.LordWind"},
        {540, 228, 227, L"BossVisual.LordFire"},
    };
    for (const auto& swap : swaps) {
        fn.value1 = swap.party;
        master.id = swap.outgoing;
        lord_master.id = swap.incoming;
        const int begin = spoken;
        outgoing_active = true;
        lord_drawn = false;
        finish_mesh = false;
        d_start_end(&fn, method_info);
        expect(finish_mesh && spoken == begin && lord_step && allowed_frames == 4,
               "Native swap completion can precede outgoing destruction and incoming draw");
        tick_lord();
        expect(spoken == begin && lord_step, "An undrawn incoming body cannot speak");
        outgoing_active = false;
        lord_drawn = true;
        tick_lord();
        tick_lord();
        expect(spoken == begin + 1 && last == swap.cue && !lord_step,
               "Each replacement speaks once after its exact body is drawn");
    }

    const int begin = spoken;
    d_start_end(&fn, method_info);
    lord_info.enemies = &targets;
    tick_lord();
    expect(spoken == begin && !lord_step, "A new roster cannot publish the retired form");
    lord_info.enemies = &lord_party;

    lord_drawn = false;
    d_start_end(&fn, method_info);
    for (int frames = 0; frames < allowed_frames; ++frames) tick_lord();
    expect(spoken == begin && !lord_step && !lord_read_task, "A hidden replacement has a finite read budget");

    lord_drawn = true;
    d_start_end(&fn, method_info);
    visual_descriptions = false;
    tick_lord();
    d_start_end(&fn, method_info);
    expect(spoken == begin && !lord_step, "Turning descriptions off retires a queued and a new reveal");
    visual_descriptions = true;
    d_start_end(&fn, method_info);
    close();
    tick_lord();
    expect(spoken == begin && !lord_step, "Battle teardown cancels the deferred reveal");
    expect(forwarded, "Replacement speech stays queued after native swap completion");
}

void check_neo_reveal()
{
    using namespace fixture;
    title = Id::FF5;
    o_neo_move = &move_neo;
    const int begin = spoken;
    close();
    master.id = 388;
    expect(d_neo_move(&fn, method_info) == 3 && spoken == begin, "The native result passes through while loading");
    open();
    master.id = 389;
    d_neo_move(&fn, method_info);
    expect(spoken == begin, "An invisible Neo Exdeath part cannot represent the visible boss");
    master.id = 388;
    fixture::renderer_visible = false;
    d_neo_move(&fn, method_info);
    expect(spoken == begin, "The actual boss must be drawn");
    fixture::renderer_visible = true;
    visual_descriptions = false;
    d_neo_move(&fn, method_info);
    expect(spoken == begin, "The appearance switch silences the reveal");
    visual_descriptions = true;
    d_neo_move(&fn, method_info);
    d_neo_move(&fn, method_info);
    expect(spoken == begin + 1 && last == L"BossVisual.NeoExdeath" && forwarded,
           "The visible new form queues once after the original movement");
    close();
}
}

int main()
{
    fixture::visual_descriptions = false;
    ffpr::battle::boss_visuals::speak(L"BossVisual.CloakOpen");
    expect(fixture::spoken == 0, "descriptions Off suppresses an automatic boss appearance cue");
    fixture::visual_descriptions = true;
    const auto iv=catalog(4), v=catalog(5), vi=catalog(6);
    for (const auto& cue : {
         scripted_sprite(Id::FF4,178,1,191,167),scripted_sprite(Id::FF4,178,2,190,167),
         scripted_sprite(Id::FF4,165,1,176,167),scripted_sprite(Id::FF4,163,4,168,167),
         scripted_sprite(Id::FF4,171,3,182,167),scripted_sprite(Id::FF4,188,0,201,167),
         scripted_sprite(Id::FF4,188,0,202,167),scripted_sprite(Id::FF4,189,0,204,0),
         scripted_sprite(Id::FF4,189,0,203,0),
         replacement(Id::FF4,540,228,227),replacement(Id::FF4,541,194,195),
         replacement(Id::FF4,542,195,228)}) check_key(iv,cue);
    for (const auto& cue : {
         deformation(Id::FF5,281,282,246,1,101),deformation(Id::FF5,282,281,245,0,101),
         scripted_sprite(Id::FF5,281,2,246,0),
         replacement(Id::FF5,445,291,290),replacement(Id::FF5,654,290,291),
         replacement(Id::FF5,655,291,292),L"BossVisual.SandwormMoved",L"BossVisual.NeoExdeath"}) check_key(v,cue);
    for (const auto& cue : {
         no_select_pose(271,0,4,true),no_select_pose(271,0,7,false),
         no_select_pose(272,0,12,true),no_select_pose(272,0,11,false),
         no_select_pose(278,0,7,false),no_select_pose(279,0,7,false),
         no_select_pose(311,0,18,true),no_select_pose(311,0,19,false),
         painting(317,300),painting(300,317),L"Enemy appearance mn_ff6_127_c00",
         L"Enemy appearance mn_ff6_144_c00"}) check_key(vi,cue);

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
    check_asura_faces(iv);
    check_lord_swaps();
    check_neo_reveal();
    std::printf("Boss visual form, native text and locale checks: %d failures\n",failures);
    return failures?1:0;
}
