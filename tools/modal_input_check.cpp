// check-sources: input/bindings.cpp audio/cue_bank.cpp audio/custom_cues.cpp
// Offline regression of the real modal sampler and settings menu; no game/window/control.
#include <windows.h>
#include "bindings_keys_fixture.h"
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <chrono>

namespace fixture {
bool focused = true;
bool transition = false;
int game_id = 2;
bool keys[256]{};
int key_reads = 0;
uint64_t tick = 1;
std::vector<std::wstring> spoken;
std::map<std::string, int> values;
std::vector<std::function<void()>> pending;
}

SHORT fake_key_state(int vk)
{
    ++fixture::key_reads;
    return fixture::keys[vk] ? static_cast<SHORT>(0x8000) : 0;
}

#define GetAsyncKeyState fake_key_state
#include "../src/core/ui/modal_input.cpp"
#undef GetAsyncKeyState
#include "../src/core/ui/mod_menu.cpp"
#include "../src/games/ff5/ff5_internal.h"

namespace core {
Install::Install(const char* n, int (*f)(), int o, uint8_t g) : name(n), fn(f), order(o), games(g) {}
}
namespace actions {
void define(const char*, Fn) {}
bool defined(const char*) { return false; }
bool run(const char*) { return false; }
}
namespace fixture { bool karnak_escape = false; }
namespace queries {
bool is(const char* name) {return std::strcmp(name,"FF5.KarnakEscape.Active")==0 ? fixture::karnak_escape : fixture::transition;}
void define(const char*, std::function<bool()>) {}
}
namespace data { std::string resource(const char*) { return {}; } }
namespace log_ { void write(const char*, ...) {} }
namespace frame {
uint64_t count() { return fixture::tick; }
double now() { return fixture::tick / 60.0; }
void subscribe(void (*)(), const char*) {}
void after_seconds(double, Task task, const char*) { fixture::pending.push_back(std::move(task)); }
}
namespace game { Id current() { return static_cast<Id>(fixture::game_id); } }
namespace fixture { std::vector<int> previews; }
namespace sound { void preview(cues::Id cue) { fixture::previews.push_back(static_cast<int>(cue)); } }
namespace sound::runtime { uint32_t custom_available_mask(){return 0;} }
namespace input {
bool game_window_focused(){return fixture::focused;}
bool key_pressed(Key k){ui::modal_keys::sample();return ui::modal_keys::pressed(fixture::key_vk(k));}
bool key_held(Key k){ui::modal_keys::sample();return ui::modal_keys::held(fixture::key_vk(k));}
bool key_in_use(Key k){return key_held(k);}bool shift_held(){return key_held(Key::LeftShift)||key_held(Key::RightShift);}bool ctrl_held(){return key_held(Key::LeftControl)||key_held(Key::RightControl);}bool alt_held(){return key_held(Key::LeftAlt)||key_held(Key::RightAlt);}
}
namespace modtext {
std::wstring T(std::wstring_view key) { return std::wstring(key); }
std::wstring TF(std::wstring_view key, std::initializer_list<std::wstring> args) {
    std::wstring result(key);
    size_t index = 0;
    for (const auto& arg : args) {
        const std::wstring token = L"{" + std::to_wstring(index++) + L"}";
        const size_t at = result.find(token);
        if (at != result.npos) result.replace(at, token.size(), arg);
    }
    return result;
}
}
namespace ffpr {
void say(std::wstring_view line, bool) { fixture::spoken.emplace_back(line); }
void say_sequence(std::vector<std::wstring> parts, bool) { for (auto& part : parts) fixture::spoken.push_back(std::move(part)); }
void say_sequence(std::vector<std::wstring> parts, bool interrupt, speech::Timing) { say_sequence(std::move(parts), interrupt); }
void say_menu(std::wstring_view line, bool) { fixture::spoken.emplace_back(line); }
void say_menu_sequence(std::vector<std::wstring> parts, bool, speech::Timing) { for (auto& part : parts) fixture::spoken.push_back(std::move(part)); }
}
namespace text {
std::wstring widen(const char* x){return std::wstring(x,x+std::strlen(x));}
std::wstring num(long long value) { return std::to_wstring(value); }
bool is_blank(std::wstring_view value) { return value.empty(); }
}
namespace settings {
int get_int(const char* key, int def) {
    const auto it = fixture::values.find(key);
    return it == fixture::values.end() ? def : it->second;
}
bool get_bool(const char* key, bool def) { return get_int(key, def ? 1 : 0) != 0; }
void set_int(const char* key, int value) { fixture::values[key] = value; }
bool toggle(const char* key, bool def) {
    const bool next = !get_bool(key, def);
    set_int(key, next ? 1 : 0);
    return next;
}
}
namespace ui {
std::wstring menu_position(const std::wstring& line, int, int) { return line; }
namespace text_input { bool is_open() { return false; } bool handle_input() { return false; } }
namespace confirmation { bool is_open() { return false; } bool handle_input() { return false; } void open(const std::wstring&,Callback,Callback){} }
}

namespace input {
bool gamepad_available(){return false;}bool button_held(int){return false;}bool button_pressed(int){return false;}
float left_trigger(){return 0;}float right_trigger(){return 0;}float left_stick_x(){return 0;}float left_stick_y(){return 0;}float right_stick_x(){return 0;}float right_stick_y(){return 0;}
std::wstring button_label(int n){return L"Button "+std::to_wstring(n);}const char* key_name(Key){return "Key";}
namespace gamepad {
bool lstick_up_pressed(){return false;}bool lstick_down_pressed(){return false;}bool lstick_left_pressed(){return false;}bool lstick_right_pressed(){return false;}
bool rstick_up_pressed(){return false;}bool rstick_down_pressed(){return false;}bool rstick_left_pressed(){return false;}bool rstick_right_pressed(){return false;}
}
}

int main()
{
    ui::mod_menu::install();
    int failures = 0;
    auto expect = [&](bool ok, const char* message) {
        if (!ok) { ++failures; std::printf("FAIL %s\n", message); }
    };
    auto tick = [&] { ++fixture::tick; ui::handle_modal_input(); };
    auto press = [&](int key) {
        fixture::keys[key] = true; tick();
        fixture::keys[key] = false; tick();
    };

    fixture::values[ui::prefs::kStatDeltaMode] = 1;
    const std::map<std::string, std::wstring> moved_settings = {
        {ui::prefs::kAutowalk, L"Navigation"},
        {ui::prefs::kEventGroupFilter, L"Navigation"},
        {ui::prefs::kEntranceSuffixes, L"Speech"},
        {ui::prefs::kSpeechEnabled, L"Speech"},
        {ui::prefs::kResultNotificationsOnce, L"Speech"},
        {ui::prefs::kStorylineSpoilers, L"Speech"},
        {ui::prefs::kVisualDescriptions, L"Speech"},
        {ui::prefs::kSpeakAutowalkPath, L"Navigation"},
        {ui::prefs::kNavigationKeymap, L"Navigation"},
        {ui::prefs::kTeleportShortcuts, L"Navigation"},
        {ui::prefs::kPathfindingFilterMode, L"Navigation"},
        {ui::prefs::kPuzzleGuideVerbosity, L"Navigation"},
        {ui::prefs::kNavigationPuzzleGuidance, L"Navigation"},
        {ui::prefs::kOtherPuzzleGuidance, L"Navigation"},
        {ui::prefs::kMapExitFilter, L"Navigation"},
        {ui::prefs::kToLayerFilter, L"Navigation"},
        {ui::prefs::kStickClickNormalization, L"Navigation"},
        {ui::prefs::kWallTones, L"Navigation audio"},
        {ui::prefs::kWallToneVolume, L"Navigation audio"},
        {ui::prefs::kWallBumpVolume, L"Navigation audio"},
        {ui::prefs::kFootsteps, L"Navigation audio"},
        {ui::prefs::kFootstepVolume, L"Navigation audio"},
        {ui::prefs::kAudioBeacons, L"Navigation audio"},
        {ui::prefs::kBeaconVolume, L"Navigation audio"},
        {ui::prefs::kBeaconWithDirections, L"Navigation audio"},
        {ui::prefs::kLandingPings, L"Navigation audio"},
        {ui::prefs::kLandingPingVolume, L"Navigation audio"},
        {ui::prefs::kExpCounter, L"Navigation audio"},
        {ui::prefs::kExpCounterVolume, L"Navigation audio"},
    };
    for (int n = 1; n <= 6; ++n) {
        fixture::values.erase(ui::prefs::kModWarnings);
        fixture::values.erase(ui::prefs::kNavigationKeymap);
        fixture::values.erase(ui::prefs::kTeleportShortcuts);
        fixture::game_id = n; ui::mod_menu::initialize();
        std::wstring section; int modes = 0; int warning_rows = 0; int speech_rows = 0;
        bool first_speech_item = true;
        std::vector<std::wstring> headers;
        std::map<std::string, int> moved_counts;
        const auto& items = ui::mod_menu::st().items;
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            if (item.kind == ui::mod_menu::Kind::Header) { section = item.name; headers.push_back(section); }
            else if (section == L"Speech" && first_speech_item) {
                first_speech_item = false;
                expect(item.key && std::string(item.key) == ui::prefs::kSpeechEnabled &&
                       item.name == L"Game speech" && item.kind == ui::mod_menu::Kind::Toggle && item.bool_def,
                       "Game speech is the first Speech row and defaults on");
            }
            if (item.key) {
                if (std::string(item.key)==ui::prefs::kEventGroupFilter || std::string(item.key)==ui::prefs::kEntranceSuffixes) {
                    expect(item.kind==ui::mod_menu::Kind::Toggle && !item.bool_def,"new navigation naming preferences are independent default-off toggles");
                    fixture::values.erase(item.key);
                    ui::mod_menu::flip(item);
                    expect(settings::get_bool(item.key,false),"new preference persists through the actual menu toggle");
                    ui::mod_menu::flip(item);
                    expect(!settings::get_bool(item.key,true),"new preference can be disabled independently");
                }
                if (std::string(item.key) == ui::prefs::kSpeechEnabled) {
                    ++speech_rows;
                    expect(section == L"Speech" && item.kind == ui::mod_menu::Kind::Toggle && item.bool_def,
                           "Game speech is a default-on Speech toggle");
                }
                if (std::string(item.key) == ui::prefs::kModWarnings) {
                    ++warning_rows;
                    expect(section == L"Speech" && item.kind == ui::mod_menu::Kind::Toggle && item.bool_def,
                           "Mod warnings is a default-on Speech toggle");
                }
                if (std::string(item.key) == ui::prefs::kStorylineSpoilers) {
                    fixture::values.erase(item.key);
                    expect(item.kind == ui::mod_menu::Kind::Toggle && !item.bool_def &&
                           !ui::prefs::storyline_spoilers_enabled(),
                           "Storyline spoilers default off with no saved setting");
                    ui::mod_menu::flip(item);
                    expect(ui::prefs::storyline_spoilers_enabled(), "the menu stores the spoiler opt-in");
                    ui::mod_menu::flip(item);
                    expect(!ui::prefs::storyline_spoilers_enabled(), "the menu stores an explicit spoiler opt-out");
                }
                if (std::string(item.key) == ui::prefs::kVisualDescriptions) {
                    fixture::values.erase(item.key);
                    expect(item.kind == ui::mod_menu::Kind::Toggle && item.bool_def &&
                           ui::prefs::visual_descriptions_enabled(), "existing saves retain visual descriptions by default");
                    ui::mod_menu::flip(item);
                    expect(!ui::prefs::visual_descriptions_enabled() && ui::prefs::speech_enabled(),
                           "the menu saves descriptions Off independently of game speech");
                    ui::mod_menu::flip(item);
                    expect(ui::prefs::visual_descriptions_enabled(), "the menu saves descriptions On again");
                }
                if (std::string(item.key) == ui::prefs::kResultNotificationsOnce) {
                    expect(section == L"Speech" && item.kind == ui::mod_menu::Kind::Toggle && item.bool_def &&
                        i > 0 && items[i-1].key && std::string(items[i-1].key) == ui::prefs::kAutoDetail,
                        "all-six notifications-once option follows Auto Detail and defaults on");
                    fixture::values.erase(item.key);
                    expect(ui::prefs::result_notifications_once_enabled(),"notifications default to once per battle");
                    ui::mod_menu::flip(item);
                    expect(!ui::prefs::result_notifications_once_enabled(),"explicit saved Off overrides the default");
                    ui::mod_menu::flip(item);
                }
                const auto expected = moved_settings.find(item.key);
                if (expected != moved_settings.end()) {
                    ++moved_counts[item.key];
                    expect(section == expected->second, "moved setting is under its requested category");
                }
            }
            auto adjacent_volume = [&](const char* toggle_key, const char* volume_key) {
                if (!item.key || std::string(item.key) != toggle_key) return;
                expect(item.kind == ui::mod_menu::Kind::Toggle && i + 1 < items.size() &&
                       items[i + 1].kind == ui::mod_menu::Kind::Volume && items[i + 1].key &&
                       std::string(items[i + 1].key) == volume_key && section == L"Navigation audio",
                       "audio toggle and its volume remain adjacent in Navigation audio");
            };
            adjacent_volume(ui::prefs::kWallTones, ui::prefs::kWallToneVolume);
            adjacent_volume(ui::prefs::kFootsteps, ui::prefs::kFootstepVolume);
            adjacent_volume(ui::prefs::kAudioBeacons, ui::prefs::kBeaconVolume);
            adjacent_volume(ui::prefs::kExpCounter, ui::prefs::kExpCounterVolume);
            adjacent_volume(ui::prefs::kLandingPings, ui::prefs::kLandingPingVolume);
        }
        expect(headers == std::vector<std::wstring>{L"Speech", L"Navigation", L"Navigation audio", L"Battle"},
               "all-six main category order is Speech, Navigation, Navigation audio, Battle");
        expect(speech_rows == 1, "all-six menus expose one Game speech row");
        const auto audio_header = std::find_if(items.begin(), items.end(), [](const auto& item) {
            return item.kind == ui::mod_menu::Kind::Header && item.name == L"Navigation audio";
        });
        expect(audio_header != items.end() && items.back().name == L"Close Menu" && items.back().parent == -1,
               "all-six root keeps four categories and Close Menu");
        if (audio_header != items.end()) {
            const int audio_index = static_cast<int>(audio_header - items.begin());
            const auto scape_page=std::find_if(items.begin(),items.end(),[&](const auto& item){return item.name==L"Soundscape settings"&&item.kind==ui::mod_menu::Kind::Submenu&&item.parent==audio_index;});
            const auto scape_toggle=std::find_if(items.begin(),items.end(),[&](const auto& item){return item.name==L"Soundscape"&&item.kind==ui::mod_menu::Kind::Toggle&&item.parent==audio_index;});
            expect(audio_index + 2 < static_cast<int>(items.size()) &&
                   items[audio_index + 1].name == L"Navigation audio master" &&
                   items[audio_index + 1].kind == ui::mod_menu::Kind::Toggle && items[audio_index + 1].bool_def &&
                   items[audio_index + 2].name == L"HRTF" && items[audio_index + 2].kind == ui::mod_menu::Kind::Toggle &&
                   !items[audio_index + 2].bool_def &&
                   scape_toggle!=items.end()&&!scape_toggle->bool_def&&scape_page!=items.end()&&
                   items[audio_index + 1].parent == audio_index,
                   "Navigation audio retains its independent master, HRTF, Soundscape switch and settings submenu");
            const int soundscape_index = int(scape_page-items.begin());
            const auto distance = std::find_if(items.begin(), items.end(), [&](const auto& item) {
                return item.parent == soundscape_index && item.key && std::string(item.key) == "SoundscapeDistance";
            });
            expect(distance != items.end() && distance->kind == ui::mod_menu::Kind::Range &&
                   distance->int_def == 10 && distance->int_min == 5 && distance->int_max == 25 && distance->int_step == 5,
                   "Soundscape distance defaults to 10 and uses five-step bounds from 5 to 25");
            if (distance != items.end()) {
                fixture::values.erase("SoundscapeDistance");
                expect(ui::mod_menu::int_value(*distance) == 10, "Soundscape distance reads its default value");
                ui::mod_menu::adjust(*distance, -1);
                expect(fixture::values["SoundscapeDistance"] == 5, "Soundscape distance clamps downward at 5");
                ui::mod_menu::adjust(*distance, -1);
                expect(fixture::values["SoundscapeDistance"] == 5, "Soundscape distance cannot move below 5");
                ui::mod_menu::adjust(*distance, 1);
                expect(fixture::values["SoundscapeDistance"] == 10, "Soundscape distance adjusts in five-step increments");
                fixture::values["SoundscapeDistance"] = 25;
                ui::mod_menu::adjust(*distance, 1);
                expect(fixture::values["SoundscapeDistance"] == 25, "Soundscape distance clamps upward at 25");
            }

            int cue_count = 0;
            const auto preview_count_before = fixture::previews.size();
            for (size_t cue_index = sound::cues::first_entity; cue_index < sound::cues::count; ++cue_index) {
                const auto& cue = sound::cues::definitions[cue_index];
                const auto toggle = std::find_if(items.begin(), items.end(), [&](const auto& item) {
                    return item.parent == soundscape_index && item.key && std::string(item.key) == cue.toggle;
                });
                const auto volume = std::find_if(items.begin(), items.end(), [&](const auto& item) {
                    return item.parent == soundscape_index && item.key && std::string(item.key) == cue.volume;
                });
                const std::wstring label = cue.label;
                const auto preview = std::find_if(items.begin(), items.end(), [&](const auto& item) {
                    return item.parent == soundscape_index && item.kind == ui::mod_menu::Kind::Action &&
                           item.name == L"Preview " + label;
                });
                expect(toggle != items.end() && toggle->kind == ui::mod_menu::Kind::Toggle && toggle->bool_def &&
                       toggle->name == label && volume != items.end() && volume->kind == ui::mod_menu::Kind::Volume &&
                       volume->name == label + L" volume" && volume->int_def == 50 && volume->int_step == 5 &&
                       preview != items.end() && preview->cue == static_cast<sound::cues::Id>(cue_index),
                       "each shared cue has an independent toggle, volume and matching preview");
                if (preview != items.end()) ui::mod_menu::activate(*preview);
                ++cue_count;
            }
            expect(cue_count == static_cast<int>(sound::cues::entity_count) &&
                   fixture::previews.size() == preview_count_before + sound::cues::entity_count,
                   "every entity preview dispatches through the shared cue bank");

            auto& menu_state = ui::mod_menu::st();
            const auto preview = std::find_if(items.begin(), items.end(), [](const auto& item) {
                return item.cue == sound::cues::Id::NPC;
            });
            if (preview != items.end()) {
                fixture::values["NavigationAudio"] = 0;
                fixture::values["SoundscapeNPC"] = 0;
                const auto before = fixture::previews.size();
                ui::mod_menu::activate(*preview);
                ui::mod_menu::adjust(*preview, 1);
                expect(fixture::previews.size() == before + 2 && fixture::values["NavigationAudio"] == 0 &&
                       fixture::values["SoundscapeNPC"] == 0,
                       "both deliberate preview actions dispatch with the master and individual cue off");
                fixture::values.erase("NavigationAudio");
                fixture::values.erase("SoundscapeNPC");
            } else {
                expect(false, "the NPC preview is available in every game");
            }
            menu_state.open = true; menu_state.page = -1; menu_state.index = audio_index;
            ui::mod_menu::enter_page(soundscape_index);
            expect(menu_state.page == soundscape_index && menu_state.index >= 0 &&
                   items[static_cast<size_t>(menu_state.index)].parent == soundscape_index,
                   "Soundscape settings submenu opens on its first child row");
            ui::mod_menu::back();
            expect(menu_state.page == audio_index && menu_state.index == soundscape_index,
                   "Back returns to Navigation audio with Soundscape settings focused");
            menu_state.open = false; menu_state.page = -1;
        }
        for (const auto& [key, category] : moved_settings) {
            const bool conditional = (key == ui::prefs::kExpCounter || key == ui::prefs::kExpCounterVolume) ? n != 2 :
                                     (key == ui::prefs::kLandingPings || key == ui::prefs::kLandingPingVolume) ? n != 4 && n != 6 : true;
            expect(moved_counts[key] == (conditional ? 1 : 0), "moved settings retain one entry and optional-game availability");
        }
        auto moved_toggle = std::find_if(items.begin(), items.end(), [&](const auto& item) {
            return item.key && std::string(item.key) == ui::prefs::kAutowalk;
        });
        expect(moved_toggle != items.end(), "Autowalk remains available after its category move");
        if (moved_toggle != items.end()) {
            fixture::values[ui::prefs::kAutowalk] = 0;
            ui::mod_menu::flip(*moved_toggle);
            expect(fixture::values[ui::prefs::kAutowalk] == 1, "moved toggle still changes its existing persistent setting key");
        }
        for (const auto& item : ui::mod_menu::st().items) {
            if (item.kind == ui::mod_menu::Kind::Header) section = item.name;
            if (!item.key || std::string(item.key) != ui::prefs::kStatDeltaMode) continue;
            ++modes;
            expect(section == L"Speech" && item.kind == ui::mod_menu::Kind::Enum &&
                   item.options.size() == 2 && std::wstring(item.options[0]) == L"Verbose" &&
                   std::wstring(item.options[1]) == L"Brief" && item.int_def == 0 && item.int_max == 1,
                   "all-six Stat delta detail is in Speech with unchanged options/defaults");
        }
        expect(modes == 1 && ui::prefs::stat_delta_mode() == 1,
               "moving the setting preserves the saved Brief preference without duplicate entries");
        int keymap_rows = 0, teleport_rows = 0, beacon_rows=0;
        for (const auto& item : items) {
            if (item.kind == ui::mod_menu::Kind::Header) section = item.name;
            if (!item.key) continue;
            if(std::string(item.key)==ui::prefs::kBeaconMode) {
                ++beacon_rows;
                expect(section==L"Navigation audio"&&item.kind==ui::mod_menu::Kind::Enum&&item.options.size()==2&&
                    std::wstring(item.options[0])==L"Follow route"&&std::wstring(item.options[1])==L"Object location"&&
                    item.int_def==0&&item.int_max==1,"all six games expose one persistent route/object beacon choice");
                fixture::values.erase(ui::prefs::kBeaconMode);
                expect(ui::prefs::beacon_mode()==0,"beacon mode preserves current route behavior by default");
                ui::mod_menu::adjust(item,1);expect(ui::prefs::beacon_mode()==1,"menu mode switch selects object location");
                fixture::values["HRTF"]=1;expect(ui::prefs::beacon_mode()==1,"beacon behavior is shared across HRTF volume banks");
                fixture::values["HRTF"]=0;
            }
            if (std::string(item.key) == ui::prefs::kNavigationKeymap) {
                ++keymap_rows;
                expect(section == L"Navigation" && item.kind == ui::mod_menu::Kind::Enum &&
                       item.options.size() == 2 && std::wstring(item.options[0]) == L"Letters" &&
                       std::wstring(item.options[1]) == L"Brackets and backslash" &&
                       item.int_def == 0 && item.int_max == 1,
                       "all-six Navigation keymap row has translated choices and default Letters");
                expect(ui::prefs::navigation_keymap() == 0, "Navigation keymap defaults to Letters");
                ui::mod_menu::adjust(item, 1);
                expect(fixture::values[ui::prefs::kNavigationKeymap] == 1 && ui::prefs::navigation_keymap() == 1,
                       "Navigation keymap enum adjustment persists Brackets and backslash");
                fixture::values[ui::prefs::kNavigationKeymap] = 9;
                expect(ui::prefs::navigation_keymap() == 1, "Navigation keymap getter clamps stored values");
            }
            if (std::string(item.key) == ui::prefs::kTeleportShortcuts) {
                ++teleport_rows;
                expect(section == L"Navigation" && item.kind == ui::mod_menu::Kind::Toggle && !item.bool_def,
                       "all-six Teleport shortcuts row is under Navigation and defaults off");
                expect(!ui::prefs::teleport_shortcuts_enabled(), "Teleport shortcuts defaults off");
                ui::mod_menu::flip(item);
                expect(fixture::values[ui::prefs::kTeleportShortcuts] == 1 && ui::prefs::teleport_shortcuts_enabled(),
                       "Teleport shortcuts toggle persists its enabled value");
            }
        }
        // Every existing volume row changes bank in place, including the nested
        // master/categories. Range/enums/toggles are deliberately not banked.
        const auto row_count=items.size();
        for(const auto& item:items) if(item.kind==ui::mod_menu::Kind::Volume) {
            fixture::values["HRTF"]=0;ui::mod_menu::set_int(item,30);
            fixture::values.erase(std::string("HRTF.")+item.key);
            fixture::values["HRTF"]=1;
            expect(ui::mod_menu::int_value(item)==50,"new HRTF bank has independent default volume in each existing row");
            ui::mod_menu::set_int(item,85);
            expect(ui::mod_menu::int_value(item)==85&&fixture::values[item.key]==30,"HRTF adjustment preserves stereo bank");
            fixture::values["HRTF"]=0;
            expect(ui::mod_menu::int_value(item)==30,"switching HRTF off restores saved stereo volume");
            ui::mod_menu::set_int(item,15);fixture::values["HRTF"]=1;
            expect(ui::mod_menu::int_value(item)==85,"stereo adjustment cannot change HRTF volume");
            ui::mod_menu::set_int(item,-1);expect(ui::mod_menu::int_value(item)==0,"HRTF volume preserves zero mute");
            ui::mod_menu::set_int(item,101);expect(ui::mod_menu::int_value(item)==100,"HRTF slider stays within original bounds");
        }
        const auto shared_range=std::find_if(items.begin(),items.end(),[](const auto& item){return item.kind==ui::mod_menu::Kind::Range&&item.key&&std::string(item.key)=="SoundscapeDistance";});
        if(shared_range!=items.end()) {
            fixture::values["HRTF"]=1;ui::mod_menu::set_int(*shared_range,15);fixture::values["HRTF"]=0;
            expect(ui::mod_menu::int_value(*shared_range)==15&&!fixture::values.contains("HRTF.SoundscapeDistance"),"soundscape range stays shared across HRTF modes");
        }
        fixture::values["HRTF"]=0;
        expect(items.size()==row_count,"mode-specific volumes add no menu rows or configuration screens");
        expect(keymap_rows == 1 && teleport_rows == 1 && beacon_rows==1, "all six games expose one row for each navigation setting");
        expect(warning_rows == 1 && ui::prefs::mod_warnings_enabled(),
               "Mod warnings has one Speech row and defaults on when no saved preference exists");
        fixture::values[ui::prefs::kModWarnings] = 0;
        expect(!ui::prefs::mod_warnings_enabled(), "Mod warnings reads its saved off preference");
    }
    fixture::values.clear(); fixture::spoken.clear(); fixture::pending.clear(); fixture::game_id = 2;
    ui::mod_menu::initialize();
    fixture::keys[VK_F8] = true;
    ui::mod_menu::open();
    expect(fixture::spoken.size() == 2 && fixture::spoken[0] == L"Mod menu" && fixture::spoken[1] == L"Speech" && fixture::pending.empty(),
        "mod-menu entry queues its heading then initial item immediately, with no delayed stale speech");
    for (auto& task : fixture::pending) task();
    fixture::pending.clear();
    tick();
    expect(ui::mod_menu::is_open(), "opening key held does not close the menu");
    fixture::keys[VK_F8] = false; tick();
    press(VK_RETURN);
    expect(fixture::spoken[fixture::spoken.size()-2] == L"Speech", "submenu headline precedes its initial setting");
    const auto initial_name = fixture::spoken.back();
    const size_t speech_count = fixture::spoken.size();
    fixture::transition = true;
    fixture::keys[VK_DOWN] = fixture::keys[VK_RETURN] = true;
    tick(); ui::mod_menu::handle_input();
    expect(fixture::spoken.size() == speech_count && fixture::values.empty(),
        "loading blocks main, late and direct modal handlers without changing settings");
    fixture::transition = false; tick();
    expect(fixture::spoken.size() == speech_count && fixture::values.empty(),
        "held modal input is seeded during loading and cannot replay on return");
    fixture::keys[VK_DOWN] = fixture::keys[VK_RETURN] = false; tick();

    fixture::focused = false;
    const int read_count = fixture::key_reads;
    for (int key : {VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RETURN, VK_SPACE, static_cast<int>('I'), VK_ESCAPE, VK_F8}) {
        press(key);
    }
    expect(fixture::key_reads == read_count, "background modal never reads hardware keys");
    expect(fixture::spoken.size() == speech_count && fixture::values.empty() && ui::mod_menu::is_open(),
           "background arrows, confirm, details and close keys have no menu effects");
    expect(!ui::modal_keys::held(VK_DOWN), "background held state is cleared");

    fixture::keys[VK_DOWN] = true;
    fixture::keys[VK_RETURN] = true;
    tick();
    fixture::focused = true; tick(); tick();
    expect(fixture::spoken.size() == speech_count && fixture::values.empty(),
           "keys held on focus return neither move nor change a setting");
    fixture::keys[VK_DOWN] = false;
    fixture::keys[VK_RETURN] = false; tick();
    press(VK_RETURN);
    expect(fixture::values.size() == 1 && fixture::spoken.back() != initial_name,
           "a fresh focused confirm changes the retained setting");
    const size_t once = fixture::spoken.size();
    fixture::keys[VK_DOWN] = true; tick();
    const size_t moved = fixture::spoken.size();
    ui::handle_modal_input();
    ui::mod_menu::handle_input();
    expect(moved == once + 1 && fixture::spoken.size() == moved,
           "main input, late tick and direct handler act only once per frame");
    fixture::keys[VK_DOWN] = false; tick();

    // The shared gate also protects a modal opened while the game is already unfocused.
    ui::mod_menu::close();
    fixture::focused = false;
    fixture::keys[VK_ESCAPE] = true;
    const int before_seed = fixture::key_reads;
    ui::mod_menu::open();
    expect(fixture::key_reads == before_seed, "background modal open does not sample hardware");
    tick();
    fixture::focused = true; tick();
    expect(ui::mod_menu::is_open(), "held Escape on focus return does not close a new modal");
    fixture::keys[VK_ESCAPE] = false; tick();
    press(VK_ESCAPE);
    expect(!ui::mod_menu::is_open(), "fresh focused Escape closes normally");
    // Actual nested custom menu, conditional rebuild and persisted typed keys.
    double menu_total_ms=0,menu_peak_ms=0;
    for(int game=1;game<=6;++game) {
        fixture::game_id=game;fixture::values.clear();
        const auto menu_began=std::chrono::steady_clock::now();ui::mod_menu::initialize();
        const auto menu_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-menu_began).count();
        menu_total_ms+=menu_ms;menu_peak_ms=std::max(menu_peak_ms,menu_ms);
        auto find_key=[&](const char* key) {const auto& items=ui::mod_menu::st().items;for(size_t n=0;n<items.size();++n)if(items[n].key&&std::string(items[n].key)==key)return int(n);return -1;};
        auto find_name=[&](const wchar_t* name) {const auto& items=ui::mod_menu::st().items;for(size_t n=0;n<items.size();++n)if(items[n].name==name)return int(n);return -1;};
        auto focus=[&](int n){auto& s=ui::mod_menu::st();s.index=n;s.page=s.items[n].parent;};
        expect(!ui::prefs::puzzle_guide_verbose(),"all-six unset guide verbosity defaults to Minimal");
        expect(ui::prefs::navigation_puzzle_guidance()&&ui::prefs::other_puzzle_guidance(),"all-six missing type preferences preserve both guide kinds");
        const auto nav_key=ui::prefs::kNavigationPuzzleGuidance,other_key=ui::prefs::kOtherPuzzleGuidance;
        expect(find_key(nav_key)>=0&&find_key(other_key)>=0,"all-six Navigation menu contains both independent type toggles");
        focus(find_key(nav_key));ui::mod_menu::adjust_current_item(-1);
        expect(!ui::prefs::navigation_puzzle_guidance()&&ui::prefs::other_puzzle_guidance(),"navigation toggle preserves non-navigation guidance");
        ui::mod_menu::initialize();
        expect(!ui::prefs::navigation_puzzle_guidance()&&ui::prefs::other_puzzle_guidance(),"type settings survive a menu rebuild");
        focus(find_key(other_key));ui::mod_menu::adjust_current_item(-1);
        expect(!ui::prefs::navigation_puzzle_guidance()&&!ui::prefs::other_puzzle_guidance(),"both guide types can be disabled together");
        focus(find_key(nav_key));ui::mod_menu::adjust_current_item(1);
        expect(ui::prefs::navigation_puzzle_guidance()&&!ui::prefs::other_puzzle_guidance(),"navigation guidance can remain enabled alone");
        focus(find_key(other_key));ui::mod_menu::adjust_current_item(1);
        const int guide_verbosity=find_key(ui::prefs::kPuzzleGuideVerbosity);
        expect(guide_verbosity>=0,"all-six Navigation menu exposes guide verbosity");
        focus(guide_verbosity);ui::mod_menu::adjust_current_item(1);
        expect(ui::prefs::puzzle_guide_verbose()&&fixture::values[ui::prefs::kPuzzleGuideVerbosity]==1,"guide Verbose selection persists through the shared settings owner");
        ui::mod_menu::initialize();
        expect(ui::prefs::puzzle_guide_verbose(),"menu rebuild retains the explicit Verbose selection");
        focus(find_key(ui::prefs::kPuzzleGuideVerbosity));ui::mod_menu::adjust_current_item(-1);
        expect(!ui::prefs::puzzle_guide_verbose()&&fixture::values[ui::prefs::kPuzzleGuideVerbosity]==0,"guide Minimal selection restores the requested behavior");
        const int custom_page=find_name(L"Custom sound settings");size_t cues=0;
        for(const auto& item:ui::mod_menu::st().items)if(item.parent==custom_page)++cues;
        expect(custom_page>=0&&cues==sound::cues::count-((game==2||game==4||game==6)?1:0),"all-six custom submenu has only applicable cue IDs");
        const auto& npc=sound::custom::keys(sound::cues::Id::NPC);
        focus(find_key(npc.repeat));ui::mod_menu::adjust_current_item(1);
        expect(fixture::values[npc.repeat]==200,"first custom timing adjustment starts at200ms");
        for(int n=0;n<60;++n)ui::mod_menu::adjust_current_item(1);
        expect(fixture::values[npc.repeat]==5000,"custom timing cannot exceed5s");
        for(int n=0;n<60;++n)ui::mod_menu::adjust_current_item(-1);
        expect(fixture::values[npc.repeat]==200,"custom timing cannot fall below200ms");
        ui::mod_menu::toggle_current_item();expect(fixture::values[npc.repeat]==0,"Confirm restores original timing");
        focus(find_key(npc.enabled));ui::mod_menu::toggle_current_item();expect(fixture::values[npc.enabled]==0,"per-sound default fallback is saved independently");
        const auto& wall=sound::custom::keys(sound::cues::Id::WallNorth);
        expect(find_key(wall.repeat)<0,"looped wall does not expose unused interval");
        focus(find_key(wall.loop));ui::mod_menu::toggle_current_item();
        expect(find_key(wall.repeat)>=0&&ui::mod_menu::st().items[ui::mod_menu::st().index].key==wall.loop,"wall loop toggle rebuilds timing row and retains focus");
        focus(find_key(sound::custom::enabled_key));ui::mod_menu::toggle_current_item();
        expect(find_name(L"Custom sound settings")<0&&ui::mod_menu::st().items[ui::mod_menu::st().index].key==sound::custom::enabled_key,"global off hides custom submenu without stale selection");
        ui::mod_menu::toggle_current_item();expect(find_name(L"Custom sound settings")>=0,"global on restores custom submenu");
    }
    {
        namespace b=input::bindings;
        const int route=b::add("key.field.modal.fixture","Route fixture",false,1,int(input::Key::F12));
        b::native_keyboard_check(+[](const b::Entry&e,int,int,std::wstring&){return e.scope>=0?b::NativeKeyboardResult::Unavailable:b::NativeKeyboardResult::Clear;});
        ui::mod_menu::initialize();auto& s=ui::mod_menu::st();s.open=true;
        const auto row=std::find_if(s.items.begin(),s.items.end(),[&](const auto&i){return i.binding==route&&i.kind==ui::mod_menu::Kind::BindingInput;});
        expect(row!=s.items.end(),"actual keyboard editor lists the registered gameplay action");
        if(row!=s.items.end()){s.index=int(row-s.items.begin());s.page=row->parent;const auto before=b::entries()[route].primary;const auto said=fixture::spoken.size();ui::mod_menu::adjust_current_item(1);const bool explained=std::any_of(fixture::spoken.begin()+said,fixture::spoken.end(),[](const auto&line){return line.find(L"unavailable")!=std::wstring::npos;});expect(b::entries()[route].primary==before&&explained,"unavailable game configuration preserves binding instead of cycling to Unbound");}
        b::native_keyboard_check(nullptr);s.open=false;
    }
    {
        fixture::game_id=5;
        fixture::values.erase(ff5::karnak::preference);
        auto route_row=[] {
            const auto& items=ui::mod_menu::st().items;
            return std::find_if(items.begin(),items.end(),[](const auto& item) {
                return item.key && std::strcmp(item.key,ff5::karnak::preference)==0;
            });
        };
        auto& menu=ui::mod_menu::st();
        ui::mod_menu::initialize();
        expect(route_row()==menu.items.end(),"Karnak route is hidden before the timer starts");
        fixture::karnak_escape=true;
        ui::mod_menu::open();
        const auto row=route_row();
        expect(row!=menu.items.end()&&row->options.size()==3,"timed Karnak exposes all three route choices");
        if (row!=menu.items.end()) {
            menu.index=int(row-menu.items.begin());menu.page=row->parent;
            expect(ui::mod_menu::value_string(*row)==L"Equipment then Elixirs","Karnak defaults to equipment then Elixirs");
            ui::mod_menu::adjust_current_item(1);
            expect(ui::mod_menu::value_string(*row)==L"Equipment only","the second choice collects only equipment");
            ui::mod_menu::adjust_current_item(1);
            expect(ui::mod_menu::value_string(*row)==L"All chests","the third choice attempts all chests");
        }
        using ffpr::ff5::timer_update_state;
        for (int state : {3,4,13})
            expect(timer_update_state(state,ui::mod_menu::is_open())==5,"the open mod menu forwards the native menu state to timer logic");
        for (int state : {0,1,2,5,8,11,12,14})
            expect(timer_update_state(state,ui::mod_menu::is_open())==state,"loading, transitions and native paused states keep their timer state");
        ui::mod_menu::close();
        for (int state : {3,4,13})
            expect(timer_update_state(state,ui::mod_menu::is_open())==state,"closing the mod menu immediately restores native timer-state admission");
        fixture::karnak_escape=false;ui::mod_menu::initialize();
        expect(route_row()==menu.items.end()&&settings::get_int(ff5::karnak::preference,0)==2,
               "the saved route persists when the context setting is hidden");
        fixture::game_id=4;fixture::karnak_escape=true;ui::mod_menu::initialize();
        expect(route_row()==menu.items.end(),"the Karnak option stays out of other games");
        fixture::karnak_escape=false;
    }
    std::printf("All-six full menu construction: mean %.3f ms, peak %.3f ms; fixture settings/resource cap, no game or speech latency claim\n",menu_total_ms/6,menu_peak_ms);
    std::printf("Modal focus/custom menu regression: %s\n", failures ? "FAILED" : "PASS");
    return failures ? 1 : 0;
}
