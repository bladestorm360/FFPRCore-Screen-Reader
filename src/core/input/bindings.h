#pragma once
#include "input.h"
#include <vector>

// Physical input stays physical. Only mod command dispatch uses this catalog.
namespace input::bindings {
struct Entry {
    std::string id;
    std::wstring label;
    bool controller = false;
    int scope = 0, layout = -1;
    int primary = -1, modifier = 0, default_primary = -1, default_modifier = 0;
};
int add(const char* id, const char* label, bool controller, int scope, int primary, int modifier = 0, int layout = -1);
void initialize_keyboard_controls();
const std::vector<Entry>& entries();
std::wstring value(int index);
std::wstring input_name(bool controller, int primary);
std::wstring scope_name(int scope);
bool set(int index, int primary, int modifier, std::wstring& conflict);
// Prepared on opening the mod settings, never during frame dispatch/picker
// iteration. The native owner installs this one cached, read-only admission.
enum class NativeKeyboardResult { Clear, Conflict, Unavailable };
using NativeKeyboardCheck=NativeKeyboardResult(*)(const Entry&,int,int,std::wstring&);
void native_keyboard_check(NativeKeyboardCheck check);
std::wstring conflict_message(const std::wstring& conflict);
bool native_controls_unavailable();
void reset(int index);
void reset_all();
bool pressed(int index);
bool held(int index);
bool keyboard_in_use(Context context);
void context(Context context);
// Owner-cached battle-only scope, independent of the current native information
// buffer. Native input hooks read the prepared ownership bit, never scan rows.
void battle_party_context(bool active);
bool battle_party_context();
bool battle_party_keyboard_in_use();
bool remapped_keyboard_in_use();
// A native-owned scripted puzzle may expose only the two read-only guide
// commands while ordinary field controls remain unavailable. No engine reads.
void mark_event_guide(int index);
void event_guide_context(bool active);
bool event_guide_context();
// Cached once per input tick. Kept separate from Opera's native performance
// qualification, so a static choice/cancel can never select the Opera module.
void owned_guide_context(bool repeat, bool stop);
bool guide_command_context();
bool guide_next_allowed();
bool guide_stop_allowed();
bool guide_binding_allowed(int index);
bool event_guide_keyboard_in_use();
bool keyboard_action_pressed(const char* action);
void begin_frame();
bool pad(const char* id, bool release = false);
bool pad_held(const char* id);
bool consumed(int physical_input);
bool navigation_request_pressed();
bool navigation_controller_input(int physical_input);
void keyboard_allowed(bool allowed);
// Editor is hosted inside the speech-only mod menu; capture suppresses router dispatch.
void capture(int index);
bool capturing();
bool update_capture();
void cancel_capture();
constexpr int kLeftTrigger = kButtonCount, kRightTrigger = kButtonCount + 1;
constexpr int kLeftUp = kButtonCount + 2, kLeftDown = kButtonCount + 3,
    kLeftLeft = kButtonCount + 4, kLeftRight = kButtonCount + 5,
    kRightUp = kButtonCount + 6, kRightDown = kButtonCount + 7,
    kRightLeft = kButtonCount + 8, kRightRight = kButtonCount + 9;
constexpr int kPadInputs = kButtonCount + 10;
}
