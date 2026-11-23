// check-sources: json.cpp text.cpp
#include <array>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string_view>
#include <vector>
#include "../src/core/field/dialogue_appearance.cpp"
#include "../src/core/field/scene_effect_cues.cpp"

namespace {
struct Node {
    std::array<void*, 48> fields{};
    const char* type = "";
    std::wstring value;
    bool visible = true;
};

std::map<std::string_view, size_t> offsets;
Node* manager = nullptr;
Node* player_map = nullptr;
Node* message_client = nullptr;
Node* battle_interpreter = nullptr;
bool owner_thread = true;
bool dialogue_open = true;
bool visual_descriptions = true;
bool in_battle = false;
bool battle_ready = false;
bool transition = false;
bool fading = false;
int current_map = 3;
int actor_reads = 0;
game::Id current_game = game::Id::FF1;
std::wstring spoken;
bool speech_interrupted = false;
json::Value fixture_appearances;
uint64_t event_generation = 0;
int instruction = 0, particle_count = 0, render_budget = 0, original_plays = 0;
bool first_clock = true;
frame::Step observation;
frame::Task reveal;
double reveal_delay = 0;

void put(Node& object, const char* field, void* value)
{
    auto index = offsets.emplace(field, offsets.size()).first->second;
    if (index >= object.fields.size()) std::abort();
    object.fields[index] = value;
}

void* client(const void*) { return message_client; }
void* sprite(void* object, const void*) { return static_cast<Node*>(object)->fields.back(); }
int native_animation(void*, const void*) { ++original_plays; return 3; }
void require(bool result, const char* detail)
{
    if (!result) { std::fprintf(stderr, "FAIL %s\n", detail); std::exit(1); }
}
} // namespace

namespace settings {
bool get_bool(const char* key, bool fallback)
{
    return std::string_view(key) == ui::prefs::kVisualDescriptions ? visual_descriptions : fallback;
}
}

namespace ref {
Class::Class(const char* space, const char* type, uint8_t mask, const char* image)
    : ns(space), name(type), games(mask), assembly(image) {}
Field::Field(Class& owner, const char* field, const char* alternative)
    : cls(owner), name(field), alt(alternative) {}
Method::Method(Class& owner, const char* method, int arity) : cls(owner), name(method), argc(arity) {}
il2cpp::Class* Class::get() { return reinterpret_cast<il2cpp::Class*>(this); }
bool Class::is(void* p)
{
    if (!p) return false;
    const std::string_view type = static_cast<Node*>(p)->type;
    const std::string_view expected = name;
    return type == expected || (type == "EventActionScript" && expected == "EventAction") ||
           (type == "FieldNonPlayer" && expected == "FieldSpriteEntity");
}
int32_t Field::get()
{
    const auto it = offsets.find(name);
    return it != offsets.end() ? static_cast<int32_t>(it->second * sizeof(void*)) : -1;
}
il2cpp::Method* Method::get() { return nullptr; }
void* Method::ptr()
{
    const std::string_view method = name;
    if (method == "Instance") return reinterpret_cast<void*>(&client);
    if (method == "get_pc") return reinterpret_cast<void*>(+[](void*, const void*) { return instruction; });
    if (method == "IsFirstClock") return reinterpret_cast<void*>(+[](void*, const void*) { return first_clock; });
    if (method == "get_particleCount") return reinterpret_cast<void*>(+[](void*, const void*) { return particle_count; });
    return reinterpret_cast<void*>(&sprite);
}
void* instance(Class& cls) { return std::string_view(cls.name) == "Integrator" ? battle_interpreter : manager; }
void call_failed_note(const char*) { std::abort(); }
std::wstring rd_str_bounded(void* object, Field& field, size_t size)
{
    const auto* text = rd<Node*>(object, field);
    return text && text->value.size() <= size ? text->value : L"";
}
} // namespace ref

namespace field::r {
ref::Class kFieldMap{"", "FieldMap"}, kFieldController{"Last.Map", "FieldController"};
ref::Class kFieldNonPlayer{"Last.Entity.Field", "FieldNonPlayer"}, kPropertyTalk{"", "PropertyTalk"};
ref::Field fEntityList{kFieldController, "entityList"}, fFieldController{kFieldMap, "fieldController"};
ref::Field fEventScriptId{kPropertyTalk, "scriptId"}, fTalkMessageKey{kPropertyTalk, "messageKey"};
ref::Field fPropEntityId{kPropertyTalk, "entityId"};
}
namespace field {
ref::Field fixture_property{r::kFieldNonPlayer, "Property"};
void* entity_property(void* actor) { return ref::rd<void*>(actor, fixture_property); }
bool live_entity(void* actor) { return unity::alive(actor); }
int current_map_id() { return current_map; }
}
namespace field::cache { void* get(ref::Class&) { return player_map; } }
namespace unity {
bool alive(void* p) { return p != nullptr; }
std::wstring name(void* p) { return p ? static_cast<Node*>(p)->value : L""; }
int list_count(void* p) { return p ? static_cast<int>(static_cast<std::vector<void*>*>(p)->size()) : 0; }
void* list_at(void* p, int n) { ++actor_reads; return (*static_cast<std::vector<void*>*>(p))[n]; }
int array_length(void* p) { return list_count(p); }
void* array_at(void* p, int n) { return (*static_cast<std::vector<void*>*>(p))[n]; }
void* get_component(void* p, ref::Class&) { return p; }
}
namespace displayed {
bool visible(void* p) { return p && static_cast<Node*>(p)->visible; }
bool renderer_visible(void* p) { return visible(p); }
}
namespace frame {
bool on_main_thread() { return owner_thread; }
void after_seconds(double delay, Task task, const char*)
{
    require(!reveal && !observation, "the authored fade queues one task without frame reads");
    reveal_delay = delay;
    reveal = std::move(task);
}
int every_frame(Step step, const char*, int frames)
{
    require(!observation, "only one pending observation per fixture episode");
    observation = std::move(step);
    render_budget = frames;
    return 1;
}
}
namespace core { Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace hooks {
bool attach_raw(ref::Method&, void*, void**) { return false; }
void note_fault(const char*) { std::abort(); }
}
namespace queries { bool is(const char*) { return transition; } }
namespace ffpr::fieldnav::map_transition {
bool is_state_changing() {return transition;}
bool is_screen_fading() {return fading;}
}
namespace game { Id current() { return current_game; } }
namespace data { const json::Value& game_json(const char*) { return fixture_appearances; } }
namespace modtext {
bool has(std::wstring_view key)
{
    return key == L"Field appearance mo_ff1_n101_c00" || key == L"Field appearance mo_ff1_n038_c00" ||
           key == L"Field appearance mo_ff1_n010_c00" || key == L"Field appearance mo_ff1_n012_c00" ||
           key == L"Field appearance mo_ff1_n017_c00" || key == L"Field appearance mo_ff1_n034_c00" ||
           key == L"Field appearance mo_ff1_n035_c00" ||
           key == L"Field appearance mo_ff2_n005_c00" || key == L"Field appearance mo_ff2_n022_c00" ||
           key == L"Field appearance mo_ff2_p004_c00" || key == L"Field appearance mo_ff4_n003_c02" ||
           key == L"Field appearance mo_ff4_n006_c02" || key == L"Field appearance mo_ff3_n008_c00" ||
           key == L"Field appearance mo_ff5_n012_c00" || key == L"Field appearance mo_ff6_n035_c00" ||
           key == L"Field appearance mo_ff6_p005";
}
std::wstring T(std::wstring_view key) { return std::wstring(key); }
}
namespace ffpr { void say(std::wstring_view text, bool interrupt) { spoken = text; speech_interrupted = interrupt; } }
namespace ffpr::system { uint64_t field_event_generation() { return transition ? 0 : event_generation; } }
namespace ffpr::dialogue { bool in_dialogue() { return dialogue_open; } }
namespace ffpr::battle {
bool in_battle() { return ::in_battle; }
bool battle_presentation_ready() { return battle_ready; }
}

void check_effect_cues()
{
    using namespace ffpr::dialogue;
    Node interpreter, core, effect, particle, script, card, card_renderer, card_sprite, card_texture;
    core.type = "PuppetCore";
    effect.type = "FieldEffectParticleEntity";
    card.type = "FieldSpriteEntity";
    card_renderer.fields.back() = &card_sprite;
    card_sprite.fields.back() = &card_texture;
    put(card, "spriteRenderer", &card_renderer);
    std::vector<void*> particles{&particle};
    std::vector<void*> cores{nullptr, &core};
    put(interpreter, "isFieldScript", reinterpret_cast<void*>(1));
    put(interpreter, "working", reinterpret_cast<void*>(1));
    put(interpreter, "<scriptName>k__BackingField", &script);
    put(interpreter, "cores", &cores);
    put(core, "<coreID>k__BackingField", reinterpret_cast<void*>(1));
    put(core, "<entity>k__BackingField", &effect);
    put(effect, "isPlaying", reinterpret_cast<void*>(1));
    put(effect, "particleArray", &particles);
    battle_interpreter = &interpreter;
    native_play = native_show = native_animation;
    for (const auto& cue : cues) {
        current_game = cue.title;
        current_map = cue.map;
        script.value = cue.script;
        instruction = cue.instruction;
        const bool is_sprite = !cue.art.empty();
        const auto dispatch = cue.trigger == CueTrigger::Show ? show : play_anim;
        put(core, "<entity>k__BackingField", is_sprite ? &card : &effect);
        put(core, "<entityID>k__BackingField", reinterpret_cast<void*>(static_cast<uintptr_t>(cue.actor)));
        ++event_generation;
        spoken.clear();
        visual_descriptions = false;
        require(dispatch(&core, nullptr) == 3 && !observation && !reveal,
                "descriptions Off keeps the native instruction and schedules no cue");
        visual_descriptions = true;
        transition = true;
        require(dispatch(&core, nullptr) == 3 && !observation, "transition preserves the native instruction without scheduling");
        transition = false;
        first_clock = false;
        dispatch(&core, nullptr);
        require(!observation, "an ordinary repeat clock cannot claim a cue");
        first_clock = true;
        ++instruction;
        dispatch(&core, nullptr);
        require(!observation, "another instruction on the same actor is silent");
        --instruction;
        dispatch(&core, nullptr);
        dispatch(&core, nullptr);
        if (cue.settle_seconds > 0) {
            require(reveal && reveal_delay == cue.settle_seconds && !observation && spoken.empty(),
                    "the pre-fade effect waits for its authored delay");
            auto due = std::move(reveal);
            due();
        }
        require(observation && render_budget == 16, "the exact binding arms one finite owner observation");
        fading = true;
        require(observation() && spoken.empty(), "faded footage is silent");
        fading = false;
        if (is_sprite) {
            card_texture.value = L"sactx-64x64-Uncompressed-MO_FF1_E000_C00-12345678";
            require(observation() && spoken.empty(), "a different card atlas is silent");
            std::wstring art(cue.art);
            for (auto& ch : art) if (ch >= L'a' && ch <= L'z') ch -= L'a' - L'A';
            card_texture.value = L"sactx-64x64-Uncompressed-" + art + L"-12345678";
            card_renderer.visible = false;
            require(observation() && spoken.empty(), "the exact card waits until actually visible");
            card_renderer.visible = true;
            card_sprite.value = L"Default_00";
            if (!cue.pose.empty()) {
                require(observation() && spoken.empty(), "the same atlas in an idle pose cannot claim a motion cue");
                card_sprite.value = std::wstring(cue.pose) + L"(Clone)";
            }
            require(!observation() && spoken == cue.text && !speech_interrupted,
                    "the exact visible sprite queues one description without interrupting");
        } else {
            particle_count = 0;
            require(observation() && spoken.empty(), "a playing system without particles is silent");
            particle_count = 3;
            particles.resize(67, &particle);
            require(observation() && spoken.empty(), "an unexpectedly large emitter list cannot claim a cue");
            particles.resize(1);
            particle.visible = false;
            require(observation() && spoken.empty(), "hidden particles cannot claim the cue");
            Node drawn_emitter;
            particles.assign(66, &particle);
            particles.back() = &drawn_emitter;
            for (int slice = 0; slice < 8; ++slice)
                require(observation() && spoken.empty(), "a larger effect yields after each slice of eight emitters");
            require(!observation() && spoken == cue.text && !speech_interrupted,
                    "a later visible emitter queues the exact description without interrupting");
            particle.visible = true;
            particles.assign(1, &particle);
        }
        observation = {};
        ++event_generation;
        dispatch(&core, nullptr);
        ++event_generation;
        spoken.clear();
        if (reveal) {
            auto due = std::move(reveal);
            due();
            require(!observation && spoken.empty(), "changing events prevents the delayed render observation");
        } else {
            require(observation && !observation() && spoken.empty(), "new event generation cancels an old observation");
        }
        observation = {};
        ++event_generation;
        dispatch(&core, nullptr);
        visual_descriptions = false;
        if (reveal) {
            auto due = std::move(reveal);
            due();
            require(!observation && spoken.empty(), "descriptions Off cancels the delayed observation");
        } else {
            require(observation && !observation() && spoken.empty(), "descriptions Off cancels the pending cue");
        }
        visual_descriptions = true;
        observation = {};
    }
    require(original_plays == static_cast<int>(cues.size()) * 8, "every captured call keeps the native return path");
    battle_interpreter = nullptr;
}

int main()
{
    Node game_map, controller, observer, procedure, action, actor, actor2, property;
    Node window_manager, window, view, message, client, id, renderer, body, texture;
    controller.type = "FieldController";
    procedure.type = "EventProcedure";
    action.type = "EventAction";
    actor.type = actor2.type = "FieldNonPlayer";
    property.type = "PropertyTalk";
    manager = &window_manager;
    player_map = &game_map;
    message_client = &client;
    id.value = L"MSG_FIELD_DIALOGUE";
    texture.value = L"sactx-128x64-Uncompressed-MO_FF1_N101_C00-cd76ed62";
    renderer.fields.back() = &body;
    body.fields.back() = &texture;
    std::vector<void*> actors{&actor};
    put(game_map, "fieldController", &controller);
    put(controller, "entityList", &actors);
    put(controller, "interactiveTargetObserver", &observer);
    put(controller, "eventHandle", &procedure);
    put(procedure, "runAction", &action);
    put(observer, "interactiveTargetEntity", &actor);
    put(window_manager, "currentWindowController", &window);
    put(window_manager, "fieldWindowController", &window);
    put(window, "view", &view);
    put(view, "messageText", &message);
    put(client, "<CurrentMessageId>k__BackingField", &id);
    put(property, "messageKey", &id);
    put(property, "scriptId", nullptr);
    for (auto* candidate : {&actor, &actor2}) {
        put(*candidate, "Property", &property);
        put(*candidate, "spriteRenderer", &renderer);
    }

    const auto read = &ffpr::dialogue::read_dialogue_appearance;
    require(read() && spoken == L"Field appearance mo_ff1_n101_c00" && actor_reads == 0, "native target and current atlas");
    spoken.clear();
    visual_descriptions = false;
    require(!read() && spoken.empty(), "descriptions Off suppresses the native speaker's appearance");
    visual_descriptions = true;
    renderer.visible = false;
    require(!read(), "hidden actor");
    renderer.visible = true;
    message.visible = false;
    require(!read(), "hidden message");
    message.visible = true;
    put(window_manager, "currentWindowController", &actor);
    require(!read(), "other current window");
    put(window_manager, "currentWindowController", &window);
    action.type = "EventActionScript";
    require(!read(), "script action does not borrow direct speaker");
    action.type = "EventAction";
    Node other_id;
    other_id.value = L"MSG_OTHER";
    put(client, "<CurrentMessageId>k__BackingField", &other_id);
    require(!read(), "different message key");
    put(client, "<CurrentMessageId>k__BackingField", &id);
    put(observer, "interactiveTargetEntity", nullptr);
    require(read(), "unique active key after native indicator is cleared");
    actors.push_back(&actor2);
    require(!read(), "ambiguous direct key with no target");
    actors.resize(65, &actor);
    actor_reads = 0;
    require(!read() && actor_reads == 0, "bounded fallback");
    put(observer, "interactiveTargetEntity", &actor);
    transition = true;
    require(!read(), "transition");
    transition = false;
    owner_thread = false;
    require(!read(), "thread ownership");
    owner_thread = true;
    dialogue_open = false;
    require(!read(), "closed dialogue");
    dialogue_open = true;
    texture.value = L"sactx-128x64-Uncompressed-MO_FF1_N101_C01-12345678";
    require(!read(), "variant without wording");
    texture.value = L"non-atlas-MO_FF1_N101_C00";
    require(!read(), "unrelated texture with similar name");

    Node script;
    script.value = L"sc_e_0003";
    put(action, "scriptName", &script);
    put(property, "entityId", reinterpret_cast<void*>(32));
    action.type = "EventActionScript";
    current_map = 72;
    actors.assign(1, &actor);
    id.value = L"MSG_HELPSERA_02";
    texture.value = L"sactx-32x32-Uncompressed-MO_FF1_N038_C00-12345678";
    require(read() && spoken == L"Field appearance mo_ff1_n038_c00", "scripted speaker and live costume");
    current_map = 3;
    require(!read(), "same message key after a map change");
    current_map = 72;
    script.value = L"sc_e_0003_3";
    require(!read(), "same message key in another script");

    current_map = 3;
    script.value = L"sc_e_0002_1";
    id.value = L"MSG_EKKEN_02";
    put(property, "entityId", reinterpret_cast<void*>(12));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF1_N010_C00-12345678";
    require(read() && spoken == L"Field appearance mo_ff1_n010_c00", "king owns the audience in its active stage");
    script.value = L"sc_e_0004_1";
    id.value = L"MSG_LUTE_02";
    put(property, "entityId", reinterpret_cast<void*>(7));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF1_N012_C00-12345678";
    require(read() && spoken == L"Field appearance mo_ff1_n012_c00", "Sarah owns the direct Lute scene");
    script.value = L"sc_e_0004_2";
    require(read(), "Sarah also owns the alternate Lute scene");

    current_map = 13;
    script.value = L"sc_e_0009";
    id.value = L"MSG_SHIP_01";
    put(property, "entityId", reinterpret_cast<void*>(38));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF1_N017_C00-12345678";
    require(read() && spoken == L"Field appearance mo_ff1_n017_c00", "Bikke owns the challenge before the pirate battle");
    id.value = L"MSG_SHIP_02";
    require(!read(), "the pirate crew page does not borrow Bikke's appearance");
    script.value = L"sc_e_0009_2";
    id.value = L"MSG_SHIP_03";
    require(read(), "Bikke also owns the post-battle exchange");
    const auto cue = &ffpr::dialogue::action_cue_at_dialogue;
    Node interpreter, battle_script;
    battle_interpreter = &interpreter;
    put(interpreter, "<scriptName>k__BackingField", &battle_script);
    put(interpreter, "isFieldScript", nullptr);
    current_game = game::Id::FF6;
    in_battle = true;
    id.value = L"E0460_01_606_b_04";
    battle_script.value = L"sc_b_0460_1";
    require(cue().empty(), "battle loading gate");
    battle_ready = true;
    require(cue() == L"Kefka wags finger cue", "reached Kefka's exact staged page");
    id.value = L"E0460_01_606_b_05";
    require(cue().empty(), "the next page stays native");
    id.value = L"E1004_00_571_b_01";
    require(cue().empty(), "same kind of window with a different battle script");
    battle_script.value = L"sc_b_2022";
    require(cue() == L"Kefka winged descent cue", "descent page after the final tier");
    id.value = L"E0547_01_425_b_10";
    battle_script.value = L"sc_b_0547";
    require(cue() == L"Relm joins Ultros scene cue", "Relm's first line after entering");
    id.value = L"E0555_05_180_b_03";
    battle_script.value = L"sc_b_0555_2";
    require(cue() == L"Kefka turns forward cue", "page following the walk and turn");
    message.visible = false;
    require(cue().empty(), "battle cue requires a visible page");
    message.visible = true;
    put(interpreter, "isFieldScript", reinterpret_cast<void*>(1));
    require(cue().empty(), "field interpreter cannot supply a battle cue");
    in_battle = battle_ready = false;
    current_game = game::Id::FF1;
    id.value = L"MSG_SHIP_03";
    require(cue() == L"Pravoka pirate defeat cue", "the surrender page follows both crew animations");
    visual_descriptions = false;
    require(cue().empty(), "descriptions Off omits the cue prefix from an otherwise valid page");
    visual_descriptions = true;
    script.value = L"sc_e_0009";
    require(cue().empty(), "the fight opener cannot narrate the surrender");

    current_map = 33;
    script.value = L"sc_e_0011";
    id.value = L"MSG_ASTOS_01";
    put(property, "entityId", reinterpret_cast<void*>(105));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF1_N034_C00-12345678";
    require(read() && cue().empty() && spoken == L"Field appearance mo_ff1_n034_c00",
            "the first page describes only the currently visible crowned figure");
    id.value = L"MSG_ASTOS_02";
    put(property, "entityId", reinterpret_cast<void*>(106));
    require(cue().empty(), "the reveal requires the transformed sprite");
    texture.value = L"sactx-64x64-Uncompressed-MO_FF1_N035_C00-12345678";
    require(read() && spoken == L"Field appearance mo_ff1_n035_c00" &&
            cue() == L"Western Keep transformation cue", "the next page follows the actual form change");
    id.value = L"MSG_ASTOS_03";
    require(read() && cue().empty(), "the transformed appearance remains available after the one cue");

    current_game = game::Id::FF2;
    require(!read(), "an FF1 speaker row cannot match a different game");
    current_map = 7;
    script.value = L"sc_e_0001_3";
    id.value = L"E0001_03_007_a_04";
    put(property, "entityId", reinterpret_cast<void*>(15));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF2_P004_C00-12345678";
    require(read() && spoken == L"Field appearance mo_ff2_p004_c00", "Minwu owns the healing dialogue");
    id.value = L"E0001_03_007_a_05";
    require(!read(), "the next healing scene line belongs to Hilda");
    put(property, "entityId", reinterpret_cast<void*>(14));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF2_N005_C00-12345678";
    require(read() && spoken == L"Field appearance mo_ff2_n005_c00", "Hilda owns the next visible page");
    current_map = 13;
    script.value = L"sc_e_0016";
    id.value = L"E0016_00_013_a_01";
    put(property, "entityId", reinterpret_cast<void*>(9));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF2_N022_C00-12345678";
    require(read() && spoken == L"Field appearance mo_ff2_n022_c00", "Tobul's initial stage placement");
    id.value = L"E0016_00_013_f_06";
    require(!read(), "initial placement is rejected after the forging scene");
    put(property, "entityId", reinterpret_cast<void*>(6));
    require(read(), "Tobul's post-forging stage placement");
    current_map = 104;
    script.value = L"sc_e_0016_1";
    id.value = L"N022_C00_013_02_01";
    put(property, "entityId", reinterpret_cast<void*>(8));
    require(read(), "Tobul's second smithy");

    current_map = 6;
    script.value = L"sc_e_0001_5";
    id.value = L"E0001_04_003_a_05";
    put(property, "entityId", reinterpret_cast<void*>(49));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF2_N005_C00-12345678";
    require(cue() == L"Altair audience bow cue", "the first audience page follows the visible bow");
    script.value = L"sc_e_0016_1";
    require(cue().empty(), "same audience key under another script");
    script.value = L"sc_e_0001_5";
    id.value = L"E0001_04_006_e_09";
    require(cue().empty(), "second bow has no repeated cue");
    id.value = L"E0001_04_003_a_05";
    renderer.visible = false;
    require(cue().empty(), "hidden audience actor");
    renderer.visible = true;

    current_game = game::Id::FF4;
    current_map = 8;
    script.value = L"sc_e_0001_11";
    id.value = L"E0001_11_008_a_01";
    put(property, "entityId", reinterpret_cast<void*>(19));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF4_N003_C02-12345678";
    require(read() && spoken == L"Field appearance mo_ff4_n003_c02", "Baron audience uses Baigan's field costume");
    id.value = L"E0001_11_008_a_02";
    require(!read(), "the next Baron page belongs to the king");
    put(property, "entityId", reinterpret_cast<void*>(18));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF4_N006_C02-12345678";
    require(read() && spoken == L"Field appearance mo_ff4_n006_c02", "Baron audience uses the king's red outfit");

    struct OpeningSpeaker {
        game::Id game;
        int map, entity;
        const wchar_t* script;
        const wchar_t* message;
        const wchar_t* texture;
        const wchar_t* key;
    };
    const OpeningSpeaker openings[] = {
        {game::Id::FF3, 12, 6, L"sc_e_0010", L"E0010_00_020_a_03",
         L"sactx-64x64-Uncompressed-MO_FF3_N008_C00-12345678", L"Field appearance mo_ff3_n008_c00"},
        {game::Id::FF5, 71, 32, L"sc_e_0001", L"E0001_00_071_a_03",
         L"sactx-64x64-Uncompressed-MO_FF5_N012_C00-12345678", L"Field appearance mo_ff5_n012_c00"},
        {game::Id::FF6, 17, 14, L"sc_e_0093", L"E0093_00_017_a_01",
         L"sactx-64x64-Uncompressed-MO_FF6_N035_C00-12345678", L"Field appearance mo_ff6_n035_c00"},
    };
    for (const auto& stage : openings) {
        current_game = stage.game;
        current_map = stage.map;
        script.value = stage.script;
        id.value = stage.message;
        texture.value = stage.texture;
        put(property, "entityId", reinterpret_cast<void*>(static_cast<intptr_t>(stage.entity)));
        require(read() && spoken == stage.key, "opening dialogue resolves its exact field costume");
        script.value = L"sc_e_0001_99";
        require(!read(), "the same title, map and actor in another scene do not match");
    }

    current_game = game::Id::FF1;
    id.value = L"MSG_BRIDGE_01";
    put(property, "entityId", reinterpret_cast<void*>(1));
    texture.value = L"sactx-128x64-Uncompressed-MO_FF1_N101_C00-cd76ed62";
    require(cue().empty(), "bridge message outside the construction map");
    current_map = 121;
    require(cue() == L"Cornelia bridge construction cue", "first construction dialogue and visible soldier");
    id.value = L"MSG_BRIDGE_02";
    require(cue().empty(), "later construction dialogue does not repeat the cue");
    id.value = L"MSG_BRIDGE_01";
    renderer.visible = false;
    require(cue().empty(), "construction sprite is hidden");
    renderer.visible = true;
    transition = true;
    require(cue().empty(), "construction cue during a native transition");

    for (const auto& stage : ffpr::dialogue::staged_cues) {
        current_game = stage.title;
        current_map = stage.map;
        auto& owning_script = stage.continuation ? battle_script : script;
        owning_script.value = stage.script;
        if (stage.continuation) {
            script.value = L"sc_e_0072";
            put(interpreter, "isFieldScript", reinterpret_cast<void*>(1));
        }
        id.value = stage.message;
        put(property, "entityId", reinterpret_cast<void*>(static_cast<intptr_t>(stage.actor)));
        std::wstring atlas(stage.art);
        for (auto& ch : atlas) if (ch >= L'a' && ch <= L'z') ch -= L'a' - L'A';
        texture.value = L"sactx-64x64-Uncompressed-" + atlas + L"-12345678";
        body.value = stage.pose;
        require(cue().empty(), "staged cue remains closed during transition");
        transition = false;
        require(cue() == stage.text, "the ready stage adds its cue to the exact first page");
        if (!stage.pose.empty()) {
            body.value = std::wstring(stage.pose) + L"(Clone)";
            require(cue() == stage.text, "an atlas clone retains the same exact pose identity");
        }
        owning_script.value = L"sc_e_9999";
        require(cue().empty(), "the same message in an unrelated scene never triggers a cue");
        owning_script.value = stage.script;
        if (stage.continuation) {
            put(interpreter, "isFieldScript", nullptr);
            require(cue().empty(), "a continuation cue needs the active field interpreter");
            put(interpreter, "isFieldScript", reinterpret_cast<void*>(1));
            visual_descriptions = false;
            require(cue().empty(), "descriptions Off omits the continuation cue");
            visual_descriptions = true;
            id.value = L"E0072_01_249_a_02";
            require(cue().empty(), "an unannotated page in the CPU aftermath stays native");
        }
        if (stage.actor) {
            renderer.visible = false;
            require(cue().empty(), "a hidden stage actor vetoes narration");
            renderer.visible = true;
            if (!stage.pose.empty()) {
                body.value = L"Default_00";
                require(cue().empty(), "the same costume in an ordinary pose remains silent");
            }
            texture.value = L"sactx-64x64-Uncompressed-MO_FF1_N101_C00-12345678";
            require(cue().empty(), "a replaced actor costume vetoes narration");
        }
        transition = true;
    }
    current_game = game::Id::FF4;
    current_map = 153;
    script.value = L"sc_e_0069";
    transition = false;
    id.value = L"E0069_00_153_a_01";
    require(cue().empty(), "Crystal Palace welcome precedes the reveal");
    id.value = L"E0069_00_153_a_02";
    require(cue() == L"Crystal Palace reveal cue", "Cecil's question follows the reveal");
    visual_descriptions = false;
    require(cue().empty(), "descriptions Off leaves Cecil's native line alone");
    visual_descriptions = true;
    id.value = L"E0069_00_153_a_03";
    require(cue().empty(), "FuSoYa's introduction does not repeat the reveal cue");

    const auto scroll_cue = &ffpr::dialogue::action_cue_at_scroll;
    current_game = game::Id::FF1;
    current_map = 124;
    script.value = L"sc_e_0006_1";
    transition = true;
    require(scroll_cue(0, 90, 52).empty(), "native map change vetoes the bridge scroll cue");
    transition = false;
    dialogue_open = false;
    require(scroll_cue(0, 90, 52) == L"Cornelia prologue birds cue", "the bridge scroll recaps the completed bird shot");
    visual_descriptions = false;
    require(scroll_cue(0, 90, 52).empty(), "descriptions Off leaves the bridge scroll's native text alone");
    visual_descriptions = true;
    require(scroll_cue(0, 90, 40).empty(), "another scroll in the same event cannot claim the bird shot");
    fading = true;
    require(scroll_cue(0, 90, 52).empty(), "a native fade keeps scroll cues out of loading");
    fading = false;
    script.value = L"sc_e_0044_3";
    require(scroll_cue(0, 90, 52).empty(), "credits do not repeat the bridge prologue cue");
    const auto fade_cue = &ffpr::dialogue::action_cue_at_fade;
    current_game = game::Id::FF3;
    put(interpreter, "isFieldScript", reinterpret_cast<void*>(1));
    current_map = 76;
    battle_script.value = L"sc_e_0001_3";
    id.value = L"E0001_00_999_a_01";
    put(property, "entityId", reinterpret_cast<void*>(3));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF3_N023_C00-12345678";
    require(fade_cue(52) == L"Gulgan prologue cue", "first Gulgan narration follows the visible prologue costume");
    visual_descriptions = false;
    require(fade_cue(52).empty(), "descriptions Off leaves the prologue caption's native text alone");
    visual_descriptions = true;
    require(fade_cue(40).empty(), "another native caption style cannot claim the scene");
    id.value = L"E0001_00_999_a_02";
    require(fade_cue(52).empty(), "different prologue line keeps native narration");
    current_map = 340;
    battle_script.value = L"sc_e_0001";
    id.value = L"E0001_00_999_a_05";
    texture.value = L"sactx-64x64-Uncompressed-MO_FF3_B002_C00-12345678";
    require(fade_cue(52) == L"Crystal prologue cue", "the crystal stage checks the currently drawn atlas");
    renderer.visible = false;
    require(fade_cue(52).empty(), "hidden crystal abstains");
    renderer.visible = true;
    current_map = 1;
    battle_script.value = L"sc_e_0001_4";
    id.value = L"E0001_00_999_a_06";
    require(fade_cue(52) == L"Coast prologue cue", "world prologue describes the shoreline");
    current_map = 158;
    battle_script.value = L"sc_e_0001_5";
    id.value = L"E0001_00_999_a_08";
    require(fade_cue(52) == L"Cavern prologue cue", "cavern prologue describes the separated adventurers");
    transition = true;
    require(fade_cue(52).empty(), "no prologue cue during native loading");
    transition = false;
    fading = true;
    require(fade_cue(52).empty(), "no prologue cue behind the map fade");
    fading = false;
    dialogue_open = true;
    current_game = game::Id::FF6;
    current_map = 21;
    id.value = L"E0036_00_021_a_01";
    script.value = L"sc_e_0035";
    battle_script.value = L"sc_e_0036";
    put(property, "entityId", reinterpret_cast<void*>(553));
    texture.value = L"sactx-64x64-Uncompressed-MO_FF6_P005-12345678";
    require(json::parse(R"({"field_dialogue":{"21":{"sc_e_0036":{"E0036_00_021_a_01":553}}}})",
                        fixture_appearances), "load the actual Figaro speaker binding");
    require(read() && spoken == L"Field appearance mo_ff6_p005", "a continuation describes the current script's named actor");
    renderer.visible = false;
    require(!read(), "the named speaker is still required to be visible");
    renderer.visible = true;
    battle_script.value = L"sc_e_0035";
    require(!read(), "a stale caption key cannot claim a different active script");
    battle_script.value = L"sc_e_0036";
    put(interpreter, "isFieldScript", nullptr);
    require(!read(), "battle interpreter cannot satisfy a field description");
    check_effect_cues();
    std::puts("Dialogue appearance checks passed");
}
