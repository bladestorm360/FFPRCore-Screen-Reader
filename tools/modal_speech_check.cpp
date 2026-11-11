// check-sources: input/bindings.cpp text.cpp
// Actual modal producers, front end, worker queue and Prism adapter; fake SDK only.
#include <windows.h>
#include "bindings_keys_fixture.h"
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include <cstdarg>
#include "../src/core/say.cpp"
#include "../src/core/speech.cpp"
#include "../src/core/ui/text_input.cpp"
#include "../src/core/ui/confirmation_dialog.cpp"
#include "../src/core/ui/initial_speech.h"
#include "../src/core/battle/battle_message_line.h"
#include "../src/core/dialogue/scroll_message.cpp"
#define install install_popup_fixture
#define s_install popup_installation_fixture
#include "../src/core/dialogue/popup.cpp"
#undef s_install
#undef install
namespace fixture {
struct Output { std::string text; bool interrupt; double at; };
std::vector<Output> output;
struct Trace { std::string feature; double request, queue, age, sdk; int result; };
std::vector<Trace> timing;
uint64_t tick = 0;
int pressed = 0, delayed = 0;
bool game_speech = true;
HANDLE entered = nullptr, release = nullptr;
std::atomic<bool> block{false};
using StoryLines = std::vector<std::wstring>;
struct StoryData { StoryLines* messages = nullptr; };
struct StoryWindow { StoryData data; StoryData* execution = &data; int index = 0; };
struct PopupCommand { std::wstring* text = nullptr; };
struct PopupWindow {
    int* cursor = nullptr;
    std::vector<PopupCommand*>* commands = nullptr;
    std::vector<void*> texts;
};
std::vector<PopupCommand*>* command_list = nullptr;
frame::Step popup_ready;
bool popup_active = false;
std::function<void()> clear_story;
void story_set(void* window, void* lines, uint32_t, float, float, float, const void*) {
    auto& owner = *static_cast<StoryWindow*>(window);
    owner.data.messages = static_cast<StoryLines*>(lines);
    owner.index = 0;
}
void story_play(void*, const void*) {}
double now() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
PrismConfig config() { return {}; }
PrismContext* init(PrismConfig*) { return reinterpret_cast<PrismContext*>(1); }
void shutdown(PrismContext*) {}
PrismBackend* acquire(PrismContext*) { return reinterpret_cast<PrismBackend*>(2); }
PrismError initialize(PrismBackend*) { return PRISM_OK; }
const char* name(PrismBackend*) { return "Fixture"; }
uint64_t features(PrismBackend*) { return PRISM_BACKEND_SUPPORTS_SPEAK | PRISM_BACKEND_SUPPORTS_OUTPUT; }
PrismError speak(PrismBackend*, const char* text, bool interrupt) {
    if (block.exchange(false)) { SetEvent(entered); WaitForSingleObject(release, 2000); }
    output.push_back({text, interrupt, now()}); return PRISM_OK;
}
PrismError braille(PrismBackend*, const char*) { return PRISM_OK; }
PrismError stop(PrismBackend*) { return PRISM_OK; }
void free(PrismBackend*) {}
const char* error(PrismError) { return "fixture"; }
const char* version() { return "0.18.3"; }
speech::prism::Api api() {
    speech::prism::Api result;
    result.config_init=config; result.init=init; result.shutdown=shutdown; result.registry_acquire_best=acquire;
    result.backend_initialize=initialize; result.backend_name=name; result.backend_get_features=features;
    result.backend_output=speak; result.backend_speak=speak; result.backend_braille=braille; result.backend_stop=stop;
    result.backend_free=free; result.error_string=error; result.version_string=version; return result;
}
void consume() {
    g_backend.load(api());
    speech::drain_backend();
    g_backend.close();
}
void drain() { std::thread worker(&consume); worker.join(); }
}
namespace core { Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {} }
namespace actions { void define(const char*, Fn) {} bool defined(const char*) { return false; } bool run(const char*) { return false; } }
namespace queries { void define(const char*, std::function<bool()>) {} std::wstring text(const char*) { return {}; } }
namespace log_ {
void write(const char* format, ...) {
    if (std::strncmp(format,"[SpeechTiming]",14)!=0) return;
    va_list args; va_start(args,format);
    fixture::Trace trace; trace.feature=va_arg(args,const char*);
    (void)va_arg(args,unsigned long long);
    trace.request=va_arg(args,double); trace.queue=va_arg(args,double);
    trace.age=va_arg(args,double); trace.sdk=va_arg(args,double); trace.result=va_arg(args,int);
    fixture::timing.push_back(std::move(trace)); va_end(args);
}
void writeW(const wchar_t*, const wchar_t*) {}
}
namespace settings {
int get_int(const char*,int d){return d;}void set_int(const char*,int){}
bool get_bool(const char* key, bool def) { return std::strcmp(key, "SpeechEnabled") == 0 ? fixture::game_speech : def; }
void save_pending() {}
}
namespace frame {
uint64_t count() { return fixture::tick; }
bool on_main_thread() { return true; }
void cancel_label(const char*) { fixture::popup_ready = {}; }
int every_frame(Step step, const char*, int) { fixture::popup_ready = std::move(step); return 1; }
void after_seconds(double, Task, const char*) { ++fixture::delayed; }
void after_frames(int, Task, const char*) { ++fixture::delayed; }
}
namespace game { Id current() { return Id::FF4; } }
namespace events { void on(Kind kind, Handler action) { if (kind == Kind::MapChanging) fixture::clear_story = std::move(action); } }
namespace dedup { bool text(const char*, std::wstring_view, Scope) { return true; } bool index(const char*, long long, Scope) { return true; } void reset(const char*) {} }
namespace menu_state {
bool is_active(const char* name) { return std::strcmp(name, POPUP) == 0 && fixture::popup_active; }
void set_active(const char*, bool active) { fixture::popup_active = active; }
void reset(const char*) { fixture::popup_active = false; }
}
namespace cursor { const CursorEvent& current() { static CursorEvent event; return event; } void add_suppressor(const char*) {} }
namespace ui { int cursor_index(void* cursor) { return *static_cast<int*>(cursor); } }
namespace ffpr::fieldnav::location_message { bool should_announce_fade_message(std::wstring_view) { return true; } }
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
bool Class::is(void*) { return std::strcmp(name, "JobChangePopup") == 0; }
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
int32_t Field::get() {
    if (std::strcmp(name, "targetIndex") == 0) return offsetof(fixture::StoryWindow, index);
    if (std::strcmp(name, "executionData") == 0) return offsetof(fixture::StoryWindow, execution);
    if (std::strcmp(name, "selectCursor") == 0) return offsetof(fixture::PopupWindow, cursor);
    if (std::strcmp(name, "commandList") == 0) return offsetof(fixture::PopupWindow, commands);
    if (std::strcmp(name, "text") == 0) return offsetof(fixture::PopupCommand, text);
    return offsetof(fixture::StoryData, messages);
}
std::wstring str(void* value) { return value ? *static_cast<std::wstring*>(value) : std::wstring(); }
}
namespace hooks {
bool attach_raw(ref::Method& method, void*, void** original) {
    if (std::strcmp(method.name, "SetData") == 0) *original = reinterpret_cast<void*>(&fixture::story_set);
    else if (std::strcmp(method.name, "PlayInit") == 0) *original = reinterpret_cast<void*>(&fixture::story_play);
    else return false;
    return true;
}
void note_fault(const char*) { std::abort(); }
}
namespace unity {
int list_count(void* value) {
    if (!value) return 0;
    if (value == fixture::command_list) return static_cast<int>(fixture::command_list->size());
    return static_cast<int>(static_cast<fixture::StoryLines*>(value)->size());
}
void* list_at(void* value, int index) {
    if (index < 0 || index >= list_count(value)) return nullptr;
    if (value == fixture::command_list) return (*fixture::command_list)[index];
    return &(*static_cast<fixture::StoryLines*>(value))[index];
}
int array_length(void* value) { return list_count(value); }
void* array_at(void* value, int index) { return list_at(value, index); }
bool alive(void* value) { return value != nullptr; }
std::vector<void*> texts(void* value, bool) { return static_cast<fixture::PopupWindow*>(value)->texts; }
std::wstring ui_text(void* value) { return ref::str(value); }
}
namespace displayed {
void* field(void* value, const char* name) {
    if (std::strcmp(name, "commandList") == 0) return static_cast<fixture::PopupWindow*>(value)->commands;
    if (std::strcmp(name, "text") == 0) return static_cast<fixture::PopupCommand*>(value)->text;
    return nullptr;
}
bool visible(void* value) { return value != nullptr; }
std::wstring text(void* value) { return ref::str(value); }
std::wstring text_field(void*, const char*) { return {}; }
}
namespace ui::modal_keys {
void seed() {} void sample() {} bool pressed(int key) { return key == fixture::pressed; } bool held(int) { return false; }
}
namespace modtext {
std::wstring T(std::wstring_view value) { return std::wstring(value); }
std::wstring TF(std::wstring_view value, std::initializer_list<std::wstring> args) {
    auto result=std::wstring(value); int i=0;
    for (const auto& arg:args) { const auto token=L"{"+std::to_wstring(i++)+L"}"; const auto at=result.find(token); if(at!=result.npos) result.replace(at,token.size(),arg); }
    return result;
}
}
namespace input {
bool game_window_focused(){return true;}
bool key_pressed(Key k){return fixture::key_vk(k)==fixture::pressed;}bool key_held(Key){return false;}bool key_in_use(Key){return false;}
bool shift_held(){return false;}bool ctrl_held(){return false;}bool alt_held(){return false;}
bool gamepad_available(){return false;}bool button_held(int){return false;}bool button_pressed(int){return false;}
float left_trigger(){return 0;}float right_trigger(){return 0;}float left_stick_x(){return 0;}float left_stick_y(){return 0;}float right_stick_x(){return 0;}float right_stick_y(){return 0;}
std::wstring button_label(int n){return L"Button "+std::to_wstring(n);}const char* key_name(Key){return "Key";}
namespace gamepad {
bool lstick_up_pressed(){return false;}bool lstick_down_pressed(){return false;}bool lstick_left_pressed(){return false;}bool lstick_right_pressed(){return false;}
bool rstick_up_pressed(){return false;}bool rstick_down_pressed(){return false;}bool rstick_left_pressed(){return false;}bool rstick_right_pressed(){return false;}
}
}

int main() {
    ui::text_input::install();ui::confirmation::install();
    int failures=0;
    auto expect=[&](bool ok,const char* reason) { if(!ok) { ++failures; std::printf("FAIL %s\n",reason); } };
    speech::init();
    double start=fixture::now();
    ui::confirmation::open(L"First question?", [] {
        ui::confirmation::open(L"Second question?", [] {});
    });
    fixture::drain();
    const double cold_ms=(fixture::output[0].at-start)*1000;
    expect(fixture::output.size()==2 && fixture::output[0].text=="First question?" && fixture::output[0].interrupt &&
        fixture::output[1].text=="Yes or No" && !fixture::output[1].interrupt,
        "ready prompt and initial answer choices traverse actual worker/adapter in order");
    fixture::output.clear(); fixture::pressed='Y'; ++fixture::tick; ui::confirmation::handle_input(); fixture::drain();
    expect(fixture::output.size()==3 && fixture::output[0].text=="Yes" && fixture::output[0].interrupt &&
        fixture::output[1].text=="Second question?" && !fixture::output[1].interrupt && !fixture::output[2].interrupt && ui::confirmation::is_open(),
        "chained question cannot interrupt the selected answer");
    fixture::pressed=0; ui::confirmation::close(); fixture::output.clear();
    ui::text_input::open(L"Enter waypoint name", L"Chest", [](const std::wstring&) { ffpr::say(L"Waypoint renamed"); });
    fixture::drain();
    expect(fixture::output.size()==2 && fixture::output[0].text=="Enter waypoint name" && fixture::output[1].text=="Chest" &&
        fixture::output[0].interrupt && !fixture::output[1].interrupt,
        "text-entry prompt precedes the initial text with no settling timer");
    fixture::output.clear(); fixture::pressed=VK_RETURN; ++fixture::tick; ui::text_input::handle_input(); fixture::drain();
    expect(fixture::output.size()==2 && fixture::output[0].text=="Confirmed: Chest" && fixture::output[0].interrupt &&
        fixture::output[1].text=="Waypoint renamed" && !fixture::output[1].interrupt && !ui::text_input::is_open(),
        "completion callback output queues behind confirmation and closes input immediately");
    expect(fixture::delayed==0, "ready modal speech and completion contain no deferred stale announcements");
    expect(ffpr::last_spoken()==L"Waypoint renamed", "repeat state retains callback output");
    fixture::output.clear();
    ui::InitialSpeech entry;
    entry.begin();
    expect(entry.capture(L"Firion") && entry.capture(L"Firion"), "nested native entry keeps every ready focus event without text deduplication");
    entry.ready(L"Select name for character"); fixture::drain();
    expect(fixture::output.size()==3 && fixture::output[0].text=="Select name for character" && fixture::output[0].interrupt &&
        fixture::output[1].text=="Firion" && !fixture::output[1].interrupt && fixture::output[2].text=="Firion" && !fixture::output[2].interrupt,
        "native focus observed during initialization queues after its parent's ready heading");
    expect(!entry.capture(L"Maria"), "later navigation is not captured by an ended native-entry event");
    fixture::pressed=0; fixture::output.clear();
    double worst_warm=0;
    for(int i=0;i<50;++i) {
        start=fixture::now();
        ffpr::say_sequence({L"Category: NPCs",L"Soldier1"},true,{speech::Feature::EntityCategory,start,start}); fixture::drain();
        worst_warm=std::max(worst_warm,(fixture::output[fixture::output.size()-2].at-start)*1000);
    }
    expect(cold_ms<100 && worst_warm<100, "cold/warm request through enqueue, worker and fake SDK stays millisecond-scale");
    expect(fixture::timing.size()==50 && fixture::timing.back().feature=="category" && fixture::timing.back().request>=0 &&
        fixture::timing.back().queue>=0 && fixture::timing.back().result==PRISM_OK,
        "production dispatcher records first SDK submission/queue/return per ordered event");
    fixture::output.clear();
    fixture::timing.clear();
    fixture::entered=CreateEventW(nullptr,TRUE,FALSE,nullptr); fixture::release=CreateEventW(nullptr,TRUE,FALSE,nullptr);
    start=fixture::now();
    ffpr::say_sequence({L"Old category",L"Old destination"},true,{speech::Feature::EntityCategory,start,start}); fixture::block=true;
    std::thread blocked(&fixture::consume);
    expect(WaitForSingleObject(fixture::entered,2000)==WAIT_OBJECT_0,"fake SDK really holds the speech owner");
    const double requested=fixture::now();
    for(int i=0;i<2000;++i) ffpr::say_sequence({L"New category",L"New destination"},true,
        {speech::Feature::EntityCategory,requested,requested-0.010});
    const double publish_ms=(fixture::now()-requested)*1000;
    expect(publish_ms<100,"rapid owner requests do not wait for the blocked backend or full queue");
    SetEvent(fixture::release); blocked.join();
    expect(fixture::output.size()==3 && fixture::output[1].text=="New category" && fixture::output[1].interrupt &&
        fixture::output[2].text=="New destination" && !fixture::output[2].interrupt,
        "busy SDK/rapid category changes discard obsolete tails and retain latest headline/focus");
    const double busy_ms=(fixture::output[1].at-requested)*1000;
    expect(fixture::timing.size()==2 && fixture::timing.back().age>=10 && fixture::timing.back().queue>=0,
        "pressure recovery retains owned snapshot age and excludes obsolete/tail first-output reports");
    fixture::output.clear();
    fixture::game_speech=false;
    const std::wstring last_before_mute=ffpr::last_spoken();
    ffpr::say(L"Muted game line",true);
    ffpr::say_parts({L"Muted game parts"},true);
    ffpr::say_sequence({L"Muted game sequence"},true);
    expect(ffpr::last_spoken()==last_before_mute && fixture::output.empty(),
        "muted say, parts and sequence leave repeat state untouched and publish nothing");
    fixture::drain();
    expect(fixture::output.empty(), "muted gameplay messages do not reach the backend");

    ffpr::say_menu(L"Game speech: Off",true); fixture::drain();
    ffpr::say_menu_sequence({L"Mod menu",L"Game speech: Off"},true); fixture::drain();
    ffpr::say_menu(L"Mod menu closed",true); fixture::drain();
    expect(fixture::output.size()==4 && fixture::output[0].text=="Game speech: Off" &&
        fixture::output[1].text=="Mod menu" && fixture::output[1].interrupt &&
        fixture::output[2].text=="Game speech: Off" && !fixture::output[2].interrupt &&
        fixture::output[3].text=="Mod menu closed" && ffpr::last_spoken()==L"Mod menu closed",
        "explicit menu row, ordered sequence and close remain spoken with gameplay speech off");
    ffpr::say(L"Muted after menu",true);
    ffpr::say_sequence({L"Muted sequence after menu"},true);
    expect(ffpr::last_spoken()==L"Mod menu closed", "menu exemption does not leak to later ordinary speech");
    fixture::drain();
    expect(fixture::output.size()==4, "ordinary speech immediately after menu remains muted");

    fixture::output.clear();
    fixture::game_speech=true;
    ffpr::say_sequence({L"Queued gameplay heading",L"Obsolete gameplay tail"},true);
    fixture::game_speech=false;
    ffpr::say_menu(L"Game speech: Off",true);
    fixture::drain();
    expect(fixture::output.size()==1 && fixture::output[0].text=="Game speech: Off" && fixture::output[0].interrupt,
        "turning the master off with menu confirmation cancels queued gameplay speech");

    fixture::game_speech=true;
    fixture::output.clear();
    ffpr::say(L"Repeated line",true);
    ffpr::say(L"Repeated line",false);
    fixture::drain();
    expect(fixture::output.size()==2 && fixture::output[0].text=="Repeated line" &&
        fixture::output[1].text=="Repeated line" && fixture::output[0].interrupt && !fixture::output[1].interrupt,
        "re-enabling restores gameplay speech and identical queued lines remain distinct");
    fixture::output.clear();
    {ffpr::battle::message_line::OpeningPresentation opening;
     ffpr::battle::message_line::game_message(L"Ambushed!",false);}
    ffpr::say_parts({L"Enemy formation",L"3 Goblin",L"2 Hornet"},false);
    ffpr::say(L"Firion's turn",false);ffpr::say(L"Attack",false);fixture::drain();
    expect(fixture::output.size()==4&&fixture::output[0].text=="Ambushed!"&&
        fixture::output[1].text=="Enemy formation. 3 Goblin. 2 Hornet"&&
        fixture::output[2].text=="Firion's turn"&&fixture::output[3].text=="Attack"&&
        std::none_of(fixture::output.begin(),fixture::output.end(),[](const auto& out){return out.interrupt;}),
        "opening condition, roster, turn and command traverse real front end/worker/SDK in order without interruption");
    fixture::output.clear();
    {ffpr::battle::message_line::OpeningPresentation opening;
     ffpr::battle::message_line::game_message(L"Pre-emptive strike!",false);}
    ffpr::say_parts({L"Enemy formation",L"3 Goblin"},false);ffpr::silence();fixture::drain();
    expect(fixture::output.empty(),"explicit Control/silence still cancels pending opening condition and formation");
    fixture::game_speech=false;
    {ffpr::battle::message_line::OpeningPresentation opening;
     ffpr::battle::message_line::game_message(L"Ambushed!",false);}
    ffpr::say_parts({L"Enemy formation",L"3 Goblin"},false);fixture::drain();
    expect(fixture::output.empty(),"opening source classification obeys the central gameplay speech master");
    fixture::game_speech=true;
    expect(fixture::delayed==0,"opening condition speech adds no timer or action-pair delay");
    {
        using namespace ffpr::dialogue;
        expect(install() == 2, "FFIV registers SetData and the per-line presentation hook");
        fixture::StoryWindow owner, unrelated;
        fixture::StoryLines lines{L"First visible line\ncontinued on this page", L"", L"Repeated line", L"Repeated line"};
        const auto set = [&](fixture::StoryLines& pages) { d_LineFadeSetData(&owner, &pages, 0, 3.5f, 0.7f, 5.0f, nullptr); };
        set(lines); fixture::drain();
        expect(fixture::output.empty(), "FFIV legend SetData never speaks the future lines together");
        d_LineFadePlayInit(&unrelated, nullptr);
        d_LineFadePlayInit(&owner, nullptr);
        d_LineFadePlayInit(&owner, nullptr);
        fixture::drain();
        expect(fixture::output.size() == 1 && fixture::output[0].text == "First visible line continued on this page" &&
            ffpr::last_spoken() == lines[0] && !fixture::output[0].interrupt,
            "first native fade queues one page and preserves its line break for repeat");
        for (int index : {1, 2, 3}) {
            owner.index = index;
            d_LineFadePlayInit(&owner, nullptr);
            fixture::drain();
        }
        expect(fixture::output.size() == 3 && fixture::output[1].text == "Repeated line" &&
            fixture::output[2].text == "Repeated line", "blank fade is quiet and equal text at distinct native indices speaks twice");
        fixture::output.clear();
        set(lines); fixture::clear_story(); d_LineFadePlayInit(&owner, nullptr); fixture::drain();
        expect(fixture::output.empty(), "a scene departure retires lines that never entered the renderer");
        set(lines); d_LineFadePlayInit(&owner, nullptr); fixture::drain();
        expect(fixture::output.size() == 1, "replaying the same presentation begins a new sequence");
        fixture::output.clear();
        fixture::StoryLines defeat{L"You were defeated."};
        set(defeat); d_LineFadePlayInit(&owner, nullptr); fixture::drain();
        expect(fixture::output.size() == 1 && fixture::output[0].text == "You were defeated." &&
            fixture::output[0].interrupt, "ordinary single-line defeat feedback retains its timing");
        expect(fixture::delayed == 0, "legend pacing adds no timers or polling");
    }
    {
        using namespace ffpr::dialogue;
        fixture::output.clear();
        std::wstring job = L"Monk", question = L"Are you sure you want to change jobs?", yes = L"Yes", no = L"No";
        fixture::PopupCommand yes_command{&yes}, no_command{&no};
        std::vector<fixture::PopupCommand*> commands{&yes_command, &no_command};
        fixture::command_list = &commands;
        int index = 1;
        fixture::PopupWindow popup{&index, &commands, {&job, &question, &yes, &no}};
        popup_open(&popup);
        expect(fixture::popup_ready() && fixture::output.empty(), "the native popup waits for its rendered body and initial cursor");
        expect(!fixture::popup_ready(), "ready job popup queues its initial announcement exactly once");
        fixture::drain();
        expect(fixture::output.size() == 1 && fixture::output[0].text == "Monk. Are you sure you want to change jobs? No",
            "job confirmation reads its body and focused No in that order");
        fixture::output.clear();
        index = 0;
        read_current_button(&index); fixture::drain();
        expect(fixture::output.size() == 1 && fixture::output[0].text == "Yes", "later native movement still speaks only the new option");
        fixture::output.clear();
        popup_open(&popup);
        fixture::popup_ready(); fixture::popup_ready(); fixture::drain();
        expect(fixture::output.size() == 1 && fixture::output[0].text == "Monk. Are you sure you want to change jobs? Yes",
            "reopening uses the actual cursor instead of a fixed No default");
        popup_close();
        fixture::command_list = nullptr;
    }
    std::printf("Speech request to fake SDK: cold %.3f ms, worst warm %.3f ms; 2,000 blocked-SDK publications %.3f ms, latest output %.3f ms; %d failures\n",
        cold_ms,worst_warm,publish_ms,busy_ms,failures);
    CloseHandle(fixture::entered); CloseHandle(fixture::release);
    return failures?1:0;
}
