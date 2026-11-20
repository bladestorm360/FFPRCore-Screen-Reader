#include <array>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string_view>
#include <vector>
#include "../src/core/field/dialogue_appearance.cpp"

namespace {
struct Node {
    std::array<void*, 32> fields{};
    const char* type = "";
    std::wstring value;
    bool visible = true;
};

std::map<std::string_view, size_t> offsets;
Node* manager = nullptr;
Node* player_map = nullptr;
Node* message_client = nullptr;
bool owner_thread = true;
bool dialogue_open = true;
bool transition = false;
int current_map = 3;
int actor_reads = 0;
game::Id current_game = game::Id::FF1;
std::wstring spoken;

void put(Node& object, const char* field, void* value)
{
    auto index = offsets.emplace(field, offsets.size()).first->second;
    if (index >= object.fields.size()) std::abort();
    object.fields[index] = value;
}

void* client(const void*) { return message_client; }
void* sprite(void* object, const void*) { return static_cast<Node*>(object)->fields.back(); }
void require(bool result, const char* detail)
{
    if (!result) { std::fprintf(stderr, "FAIL %s\n", detail); std::exit(1); }
}
} // namespace

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
    return std::string_view(name) == "Instance" ? reinterpret_cast<void*>(&client) : reinterpret_cast<void*>(&sprite);
}
void* instance(Class&) { return manager; }
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
}
namespace displayed {
bool visible(void* p) { return p && static_cast<Node*>(p)->visible; }
bool renderer_visible(void* p) { return visible(p); }
}
namespace frame { bool on_main_thread() { return owner_thread; } }
namespace queries { bool is(const char*) { return transition; } }
namespace game { Id current() { return current_game; } }
namespace modtext {
bool has(std::wstring_view key)
{
    return key == L"Field appearance mo_ff1_n101_c00" || key == L"Field appearance mo_ff1_n038_c00" ||
           key == L"Field appearance mo_ff1_n010_c00" || key == L"Field appearance mo_ff1_n012_c00" ||
           key == L"Field appearance mo_ff1_n017_c00" ||
           key == L"Field appearance mo_ff2_n005_c00" || key == L"Field appearance mo_ff2_n022_c00" ||
           key == L"Field appearance mo_ff2_p004_c00" || key == L"Field appearance mo_ff4_n003_c02" ||
           key == L"Field appearance mo_ff4_n006_c02";
}
std::wstring T(std::wstring_view key) { return std::wstring(key); }
}
namespace ffpr { void say(std::wstring_view text, bool) { spoken = text; } }
namespace ffpr::dialogue { bool in_dialogue() { return dialogue_open; } }

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
    require(cue() == L"Pravoka pirate defeat cue", "the surrender page follows both crew animations");
    script.value = L"sc_e_0009";
    require(cue().empty(), "the fight opener cannot narrate the surrender");
    script.value = L"sc_e_0009_2";

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
    std::puts("Dialogue appearance checks passed");
}
