// check-sources:
// Actual scene detours with instrumented particle, position and event owners.
#include "../src/core/field/scene_effect_cues.cpp"
#include <cstdio>
#include <vector>

namespace fixture {
struct Array { void* item; };
struct Effect { bool playing = false; Array* particles; unity::Vec3 position{}; int tile_frame = 0; };
struct Puppet { int core = 1, id = 21, pc = 110; Effect* effect; };
struct Interpreter { bool field = true, working = true, pause = false; Array* cores; };
int particle;
Array particles{&particle};
Effect effect{false, &particles};
Puppet puppet{1, 21, 110, &effect};
Array cores{&puppet};
Interpreter interpreter{true, true, false, &cores};
uint64_t generation = 1;
bool fade_covers = false, drawn = false, enabled = true, first = true, advance_tile = true;
int map_id = 271;
double clock_seconds = 0;
std::wstring scene = L"sc_e_0046";
int forwarded = 0;
std::vector<std::wstring> spoken_cues;
std::vector<frame::Step> observations;

void tick()
{
    for (auto& observe : observations) if (observe && !observe()) observe = {};
}
int show(void*, const void*) { ++forwarded; effect.playing = true; return 2; }
int position(void*, const void*) { ++forwarded; effect.position.x = 5; return 3; }
int move(void*, const void*) { ++forwarded; return 4; }
int cue_anim(void*, const void*) { ++forwarded; if (advance_tile) ++effect.tile_frame; return 5; }
int tile_frame(void*, const void*) { return effect.tile_frame; }
int pc(void*, const void*) { return puppet.pc; }
bool first_clock(void*, const void*) { return first; }
int count(void*, const void*) { return 1; }
}

namespace core { Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {} }
namespace game { Id current() { return Id::FF5; } }
namespace field { int current_map_id() { return fixture::map_id; } }
namespace ffpr::system { uint64_t field_event_generation() { return fixture::generation; } }
namespace ffpr::fieldnav::map_transition { bool is_screen_fading() { return fixture::fade_covers; } }
namespace ffpr::dialogue {
std::wstring drawn_field_actor_art(void*, std::wstring_view pose) {
    return fixture::drawn && pose == L"lookdown_00" ? L"mo_ff5_n024_c00" : L"";
}
}
namespace displayed { bool renderer_visible(void*) { return fixture::drawn; } }
namespace settings { bool get_bool(const char*, bool) { return fixture::enabled; } }
namespace hooks {
bool attach_raw(ref::Method&, void*, void**) { return false; }
void note_fault(const char*) { std::abort(); }
}
namespace modtext { std::wstring T(std::wstring_view key) { return std::wstring(key); } }
namespace ffpr { void say(std::wstring_view key, bool) { fixture::spoken_cues.emplace_back(key); } }
namespace frame {
bool on_main_thread() { return true; }
double now() { return fixture::clock_seconds; }
int every_frame(Step step, const char*, int cap) {
    if (cap != 16 && cap != 320) std::abort();
    fixture::observations.push_back(std::move(step));
    return 1;
}
void after_seconds(double, Task, const char*) { std::abort(); }
}
namespace unity {
bool alive(void* object) { return object != nullptr; }
int array_length(void* array) { return array == &fixture::cores ? 2 : array ? 1 : 0; }
void* array_at(void* array, int) { return static_cast<fixture::Array*>(array)->item; }
void* get_component(void* object, ref::Class&) { return object; }
void* transform(void* object) { return object; }
Vec3 position(void* object) { return static_cast<fixture::Effect*>(object)->position; }
}
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
bool Class::is(void* object) { return object == &fixture::puppet || object == &fixture::effect; }
int32_t Field::get() { return off_; }
il2cpp::Method* Method::get() { return nullptr; }
void* Method::ptr() {
    using namespace ffpr::dialogue;
    if (this == &mFirstClock) return reinterpret_cast<void*>(&fixture::first_clock);
    if (this == &mProgramCounter) return reinterpret_cast<void*>(&fixture::pc);
    if (this == &mParticleCount) return reinterpret_cast<void*>(&fixture::count);
    if (this == &mTileAnimFrame) return reinterpret_cast<void*>(&fixture::tile_frame);
    return nullptr;
}
void* instance(Class&) { return &fixture::interpreter; }
std::wstring rd_str_bounded(void*, Field&, size_t) { return fixture::scene; }
void call_failed_note(const char*) { std::abort(); }
}

int main()
{
    using namespace fixture;
    using namespace ffpr::dialogue;
    fCoreId.off_ = offsetof(Puppet, core);
    fActorId.off_ = offsetof(Puppet, id);
    fActor.off_ = offsetof(Puppet, effect);
    fCores.off_ = offsetof(Interpreter, cores);
    fField.off_ = offsetof(Interpreter, field);
    fWorking.off_ = offsetof(Interpreter, working);
    fPause.off_ = offsetof(Interpreter, pause);
    fPlaying.off_ = offsetof(Effect, playing);
    fParticles.off_ = offsetof(Effect, particles);
    native_show = fixture::show;
    native_set_pos = fixture::position;
    native_move = fixture::move;
    native_play = fixture::show;
    native_cue_anim = fixture::cue_anim;
    int failures = 0;
    auto expect = [&](bool passed, const char* why) {
        if (!passed) { ++failures; std::printf("FAIL %s\n", why); }
    };
    expect(ffpr::dialogue::show(&puppet, nullptr) == 2, "The original Show result is preserved");
    tick();
    expect(spoken_cues.empty(), "A show request alone cannot announce black mist");
    drawn = true;
    tick();
    expect(spoken_cues == std::vector<std::wstring>{L"Queen Karnak mist cue"},
           "The visible mist speaks after particles become drawable");
    ffpr::dialogue::show(&puppet, nullptr);
    tick();
    expect(spoken_cues.size() == 1, "Repeated script clocks cannot replay the same cue");
    puppet.id = 8;
    puppet.pc = 117;
    expect(motion<CueTrigger::SetPos, native_set_pos>(&puppet, nullptr) == 3, "SetPos forwards native result");
    tick();
    expect(spoken_cues.size() == 2 && spoken_cues.back() == L"Queen Karnak flames appear cue",
           "The first visible relocated fire speaks after placement");
    puppet.pc = 197;
    expect(motion<CueTrigger::Move, native_move>(&puppet, nullptr) == 4, "Move forwards native result");
    tick();
    expect(spoken_cues.size() == 2, "An already drawn fire with no movement stays quiet");
    effect.position.y = 2;
    tick();
    expect(spoken_cues.size() == 3 && spoken_cues.back() == L"Queen Karnak flames sweep cue",
           "Visible displacement speaks as soon as it commits");
    ++generation;
    motion<CueTrigger::Move, native_move>(&puppet, nullptr);
    generation = 0;
    effect.position.y = 4;
    tick();
    expect(spoken_cues.size() == 3 && forwarded == 5, "Battle transition cancels stale pending movement");
    generation = 3;
    puppet.id = 42;
    puppet.pc = 172;
    scene = L"sc_e_0046_1";
    drawn = false;
    play_anim(&puppet, nullptr);
    for (int i = 1; i <= 40; ++i) { clock_seconds = i * .01; tick(); }
    expect(spoken_cues.size() == 3, "The engine cue waits through late particle emission");
    drawn = true;
    tick();
    expect(spoken_cues.size() == 4 && spoken_cues.back() == L"Fire-Powered Ship engine blasts cue",
           "The first actual blast beyond the old 16-frame limit queues once");
    ++generation;
    play_anim(&puppet, nullptr);
    fade_covers = true;
    clock_seconds = .88;
    tick();
    expect(spoken_cues.size() == 4, "An opaque loading fade still blocks presentation");
    fade_covers = false;
    clock_seconds = 1;
    tick();
    expect(spoken_cues.size() == 4, "An observation cannot linger beyond its verified time window");

    ++generation;
    map_id = 104;
    scene = L"sc_e_0047";
    puppet.id = 204;
    puppet.pc = 517;
    advance_tile = false;
    expect(ffpr::dialogue::cue_anim(&puppet, nullptr) == 5 && spoken_cues.size() == 4, "A requested tile frame alone is silent");
    advance_tile = true;
    ffpr::dialogue::cue_anim(&puppet, nullptr);
    expect(spoken_cues.size() == 5 && spoken_cues.back() == L"Fire Crystal right duct cue", "An applied tile frame has a cue");
    puppet.id = 100;
    puppet.pc = 888;
    drawn = false;
    play_anim(&puppet, nullptr);
    tick();
    expect(spoken_cues.size() == 5, "An actor already hidden cannot prove that a flicker started");
    drawn = true;
    tick();
    expect(spoken_cues.size() == 5, "The first unchanged drawn frame does not describe a flicker");
    drawn = false;
    tick();
    expect(spoken_cues.size() == 6 && spoken_cues.back() == L"Fire Crystal werewolf flickers cue",
           "The visible pose losing its alpha produces the flicker cue");
    std::printf("Scene presentation phases: %d failures\n", failures);
    return failures ? 1 : 0;
}
