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
int actor_reads = 0;
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
    return type == name || (type == "EventActionScript" && std::string_view(name) == "EventAction");
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
}
namespace field {
ref::Field fixture_property{r::kFieldNonPlayer, "Property"};
void* entity_property(void* actor) { return ref::rd<void*>(actor, fixture_property); }
bool live_entity(void* actor) { return unity::alive(actor); }
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
namespace modtext {
bool has(std::wstring_view key) { return key == L"Field appearance mo_ff1_n101_c00"; }
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
    std::puts("Dialogue appearance checks passed");
}
