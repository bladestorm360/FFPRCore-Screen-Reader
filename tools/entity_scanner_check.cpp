// check-sources: field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp field/field_refs.cpp field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp json.cpp text.cpp
// Actual scanner/capture/publication/input flow, with fake native owners only.
#include <windows.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include "../src/core/field/entity_scanner.cpp"
#define contains ff2_placeholder_contains
#include "../src/games/ff2/module.cpp"
#undef contains
namespace fixture {
const DWORD owner = GetCurrentThreadId();
int wrong_owner = 0, map = 1, next_task = 0, native_reads=0;
game::Id selected_game=game::Id::FF2;
bool ready = true, transition = false, route_pending = false;
bool grouped_exits=false, has_goal=false, air_capture=false;
std::optional<field::baked_nav::AirTransitionTarget> air_target;
std::optional<field::baked_nav::AirTransitionTarget> story_target;
std::optional<field::baked_nav::AirTransitionTarget> surface_target;
field::baked_nav::MapGoal goal;
struct TalkProperty { int script = 65; std::wstring name = L"飛竜", message; int action=2; int id=-1; int status=0; std::wstring close; } talk;
bool optional_fields=true;
struct Actor { void* property = &talk; } npc, object;
bool can_action = true, manual_object = false;
bool capture_cells = false;
int captured_object_type=0;
std::string captured_class="FieldEntity";
std::wstring captured_go_name=L"Entity";
std::vector<void*> candidates;
int action_reads=0;
void native();
bool get_can_action(void*, const void*) { native();++action_reads; return can_action; }

unity::Vec3 pose{};
struct Controller { void* player = reinterpret_cast<void*>(1); } controller;
struct Task { frame::Step step; std::string label; };
std::map<int, Task> tasks;
std::vector<frame::Task> navigation, compute, posted;
std::mutex post_lock;
void native() { ++native_reads; if (GetCurrentThreadId() != owner) ++wrong_owner; }
void tick() {
    std::vector<frame::Task> completed;
    { std::lock_guard lock(post_lock); completed.swap(posted); }
    for (auto& task : completed) task();
    std::vector<int> ids;
    for (const auto& [id, task] : tasks) ids.push_back(id);
    for (int id : ids) {
        auto it = tasks.find(id); if (it == tasks.end()) continue;
        auto task = std::move(it->second); tasks.erase(it);
        if (task.step()) tasks.emplace(id, std::move(task));
    }
}
void work() {
    auto jobs = std::move(navigation); navigation.clear();
    auto other=std::move(compute);compute.clear();for(auto& job:other)jobs.push_back(std::move(job));
    std::thread worker([&] { for (auto& job : jobs) job(); }); worker.join();
}
}
namespace ref {
Class::Class(const char* n, const char* c, uint8_t g, const char* a) : ns(n), name(c), games(g), assembly(a) {}
Field::Field(Class& c, const char* n, const char* a) : cls(c), name(n), alt(a) {}
Method::Method(Class& c, const char* n, int a) : cls(c), name(n), argc(a) {}
il2cpp::Class* Class::get() { fixture::native(); return nullptr; }
bool Class::is(void* p) { fixture::native();
 if(p==&fixture::npc)return std::strcmp(name,"FieldNonPlayer")==0||std::strcmp(name,"FieldEntity")==0;
 if(p==&fixture::object)return std::strcmp(name,fixture::manual_object?"FieldMapObjectDefault":"EventTriggerEntity")==0||std::strcmp(name,"FieldEntity")==0;
 if(p==&fixture::talk)return std::strcmp(name,"PropertyEvent")==0||std::strcmp(name,"PropertyTalk")==0;
 return false;
}
int32_t Field::get() { fixture::native(); if (!fixture::optional_fields && (std::strcmp(name,"<ChangeCharacterStatusId>k__BackingField")==0 || std::strcmp(name,"<CloseMessageKey>k__BackingField")==0)) return -1; return std::strcmp(name,"<MessageKey>k__BackingField")==0 || std::strcmp(name,"<CloseMessageKey>k__BackingField")==0 ? 1 : std::strcmp(name,"<ChangeCharacterStatusId>k__BackingField")==0 ? offsetof(fixture::TalkProperty,status) : std::strcmp(name,"fieldPlayer")==0||std::strcmp(name,"<Property>k__BackingField")==0?0:std::strcmp(name,"<ScriptId>k__BackingField")==0?offsetof(fixture::TalkProperty,script):std::strcmp(name,"<ActionId>k__BackingField")==0?offsetof(fixture::TalkProperty,action):std::strcmp(name,"<EntityId>k__BackingField")==0?offsetof(fixture::TalkProperty,id):-1; }
il2cpp::Method* Method::get() { fixture::native(); return std::strcmp(name,"get_CanAction")==0?reinterpret_cast<il2cpp::Method*>(1):nullptr; }
void* Method::ptr() { fixture::native(); return std::strcmp(name,"get_CanAction")==0?reinterpret_cast<void*>(&fixture::get_can_action):nullptr; }
std::wstring str(void*) { fixture::native(); return {}; }
std::wstring rd_str(void* p, Field& f) { fixture::native();if(p!=&fixture::talk)return {};return std::strcmp(f.name,"<MessageKey>k__BackingField")==0?fixture::talk.message:std::strcmp(f.name,"<CloseMessageKey>k__BackingField")==0?fixture::talk.close:fixture::talk.name; }
void* instance(Class&) { fixture::native(); return nullptr; }
void call_failed_note(const char*) {}
void* new_str(const std::wstring&) { fixture::native(); return nullptr; }
}
namespace field::party_entities {bool is_native(void*){fixture::native();return false;}std::wstring companion_name(void*){fixture::native();return {};}}
// Exercise the real route-only identity reader; keep unrelated label stubs
// deterministic for the older scanner tests in this fixture.
namespace fixture {json::Value event_metadata;bool real_event_labels=false;}
namespace data {const json::Value& game_json(const char*){return fixture::event_metadata;}}
namespace field {std::wstring tested_safe_event_label(const std::wstring&,const std::wstring&,int,int,int=-1);}
#define safe_event_label tested_safe_event_label
#include "../src/core/field/event_labels.cpp"
#undef safe_event_label
namespace il2cpp {
Class* object_class(void*) { fixture::native(); return nullptr; }
const char* object_class_name(void*) { fixture::native(); return fixture::captured_class.c_str(); }
Method* find_method_overload_param(Class*, const char*, int, Class*, int) { fixture::native(); return nullptr; }
void* method_pointer(Method*) { fixture::native(); return reinterpret_cast<void*>(&fixture::get_can_action); }
Method* virtual_method(void*, Method* method) { fixture::native(); return method; }
}
namespace unity {
bool alive(void* p) { fixture::native(); return p != nullptr; }
void* game_object(void* p) { fixture::native(); return p; }
void* transform(void* p) { fixture::native(); return p; }
Vec3 local_position(void*) { fixture::native(); return fixture::pose; }
std::wstring name(void*) { fixture::native(); return fixture::captured_go_name; }
bool active_in_hierarchy(void*) { fixture::native(); return true; }
}
namespace frame {
double now() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
void post(Task task) { std::lock_guard lock(fixture::post_lock); fixture::posted.push_back(std::move(task)); }
int every_frame(Step step, const char* label, int) {
    fixture::native(); const int id = ++fixture::next_task;
    fixture::tasks.emplace(id, fixture::Task{std::move(step), label ? label : ""}); return id;
}
void cancel_label(const char* label) {
    fixture::native();
    for (auto it = fixture::tasks.begin(); it != fixture::tasks.end();)
        if (it->second.label.starts_with(label)) it = fixture::tasks.erase(it); else ++it;
}
}
namespace background {
bool post(std::function<void()> task, Lane lane) {
    fixture::native();
    (lane == Lane::Navigation ? fixture::navigation : fixture::compute).push_back(std::move(task)); return true;
}
}
namespace hooks { void note_fault(const char* label) { std::printf("FAIL native fixture fault: %s\n",label);std::fflush(stdout);std::abort(); } }
namespace log_ { void write(const char*, ...) {} }
namespace game { Id current() { return fixture::selected_game; } const Flags& flags() { static Flags value; static bool configured=(kFF2Module.configure(value),true); return value; } const char* key() { return "ff2"; } }
namespace settings {
int get_int(const char*, int fallback) { return fallback; }
bool get_bool(const char* key, bool fallback) { return std::strcmp(key,"MapExitFilter")==0 ? fixture::grouped_exits : fallback; }
}
namespace ffpr { void say(std::wstring_view, bool) { fixture::native(); } }
namespace ffpr::system {
bool is_field_navigation_ready() { fixture::native(); return fixture::ready; }
bool input_transition_blocked() { fixture::native(); return fixture::transition; }
}
namespace ffpr::fieldnav::map_transition { bool is_screen_fading() { fixture::native(); return false; } }
namespace modtext {
LocaleScope::LocaleScope(std::wstring) {}
LocaleScope::~LocaleScope() {}
const std::wstring& lang() { fixture::native(); static std::wstring value = L"en"; return value; }
std::wstring T(std::wstring_view value) { return std::wstring(value); }
std::wstring TF(std::wstring_view value, std::initializer_list<std::wstring> args) {
    auto result = std::wstring(value); int i = 0;
    for (const auto& arg : args) { const auto token = L"{" + std::to_wstring(i++) + L"}"; const auto at = result.find(token); if (at != result.npos) result.replace(at, token.size(), arg); }
    return result;
}
}
namespace field::cache {
void* get(ref::Class& type) { fixture::native(); return std::strcmp(type.name, "FieldPlayerController") == 0 ? &fixture::controller : nullptr; }
void* refresh(ref::Class& type) { return get(type); }
}
namespace field {
int current_map_id_or_user_data() { fixture::native(); return fixture::map; }
int current_map_id() { return current_map_id_or_user_data(); }
void* player() { fixture::native(); return nullptr; }
void* map_handle() { fixture::native(); return nullptr; }
VehicleState current_vehicle() { fixture::native(); return {}; }
bool entity_is_active(void*) { fixture::native(); return true; }
bool entity_is_hidden_by_vehicle(void*) { fixture::native(); return false; }
bool entity_is_scenery(void*) { fixture::native(); return false; }
bool entity_on_current_map(void*, const std::wstring&) { fixture::native(); return true; }
int object_type_of(void*) { fixture::native(); return fixture::captured_object_type; }
int goto_map_destination_id(void*) { fixture::native(); return 0; }
bool vehicle_type_of(void*, int&, int&, std::wstring&) { fixture::native(); return false; }
std::wstring vehicle_display_name(int, int, const std::wstring&) { fixture::native(); return {}; }
std::wstring vehicle_name_from_property(const std::wstring&, const std::wstring&) { fixture::native(); return {}; }
int vehicle_type_from_name(const std::wstring&) { fixture::native(); return 0; }
bool treasure_opened(void*) { fixture::native(); return false; }
std::wstring treasure_contents(void*) { fixture::native(); return {}; }
std::wstring entity_name_from_property(void* p) { fixture::native(); return p==&fixture::npc?L"Wyvern":L""; }
std::wstring clean_object_name(const std::wstring& name, const std::wstring&) { return name; }
std::wstring safe_event_label(void*, const std::wstring& name) { fixture::native();
    return fixture::real_event_labels?tested_safe_event_label(fixture::talk.name,name,fixture::map,fixture::talk.id,fixture::captured_object_type):name;
}
std::wstring safe_event_label(const std::wstring&, const std::wstring&) { return L"Conversation"; }
std::wstring safe_event_label(const std::wstring& raw, const std::wstring& type, int map, int id, int native_type) {
    return fixture::real_event_labels?tested_safe_event_label(raw,type,map,id,native_type):L"Conversation";
}
std::wstring resolve_map_name(int map) { fixture::native();switch(map){case 144:return L"Falgabard";case 242:return L"Temple of Time";case 234:return L"Saronia Catacombs";case 269:return L"Sunken Cave";default:return L"";} }
std::wstring resolve_item_name(int) { fixture::native(); return {}; }
std::wstring current_map_asset_name() { fixture::native(); return {}; }
std::wstring translate_entity_name(const std::wstring& name) { fixture::native(); return name; }
std::wstring translate_entity_name_for_map(const std::wstring& name,int) { fixture::native(); return name; }
npc_names::Label npc_display_label(const std::wstring& name) { fixture::native(); return npc_names::parse(name); }
bool is_decimal_digit(wchar_t c) { return c >= L'0' && c <= L'9'; }
bool EntityCursor::next(void*& entity) { fixture::native();entity=nullptr;if(index>=static_cast<int>(fixture::candidates.size()))return false;entity=fixture::candidates[index++];seen.insert(entity);return true; }
bool Entity::is_alive() const { fixture::native(); return game != reinterpret_cast<void*>(2); }
bool Entity::is_opened() const { fixture::native(); return opened; }
Category Entity::category() const { return kind == Kind::Vehicle ? Category::Vehicles : kind == Kind::TreasureChest ? Category::Chests : kind == Kind::SavePoint ? Category::SavePoints : kind == Kind::NPC ? Category::NPCs : kind == Kind::MapExit ? Category::MapExits : kind == Kind::Event && (event_type==L"Interactive Object"||event_type==L"Interactive") ? Category::Objects : Category::Events; }
unity::Vec3 Entity::position() const { fixture::native(); return pos; }
PathInfo find_path_to_quick(unity::Vec3, unity::Vec3, void*, void*, TargetKind, bool) { fixture::native(); return {}; }
}
namespace field::baked_nav {
std::optional<AirTransitionTarget> air_transition_target(ExitSnapshot&,const std::optional<MapGoal>&,const std::optional<MapGoal>&,std::function<bool()>,bool story,bool) { return story?fixture::story_target:fixture::air_target; }
std::optional<AirTransitionTarget> nautilus_surface_target(ExitSnapshot&,std::function<bool()>,bool) { return fixture::surface_target; }

std::shared_ptr<ExitSnapshot> begin_exit_capture(bool,bool,bool) { fixture::native(); return fixture::air_capture ? std::shared_ptr<ExitSnapshot>(reinterpret_cast<ExitSnapshot*>(1),[](ExitSnapshot*){}) : nullptr; }
void capture_exit_obstacle(ExitSnapshot&,void*) { fixture::native(); }
void finish_exit_capture(ExitSnapshot&) { fixture::native(); }
bool exit_snapshot_world(const ExitSnapshot&) { fixture::native(); return fixture::air_capture; }
int exit_snapshot_vehicle(const ExitSnapshot&) { fixture::native(); return 0; }
std::vector<int> exit_distances(ExitSnapshot&,const std::vector<ExitTarget>&,std::function<bool()>) { return {}; }
std::vector<bool> story_gate_mask(ExitSnapshot&,const std::vector<StoryTarget>& targets,std::function<bool()>) { return std::vector<bool>(targets.size(),false); }
bool available(void*) { fixture::native(); return true; }
bool prepared() { fixture::native(); return true; }
bool cell_at(void*, unity::Vec3 position, baked_route::Cell& cell) { fixture::native();if(!fixture::capture_cells)return false;cell={0,static_cast<int>(position.x),static_cast<int>(position.y)};return true; }
bool pending_goal(MapGoal& goal, TargetKind& kind, std::wstring& name) { fixture::native(); if (!fixture::has_goal) return false; goal=fixture::goal; kind=TargetKind::Transition; name=L"World"; return true; }
bool request_pending() { fixture::native(); return fixture::route_pending; }
void request_reachability(bool, std::function<void()> ready) { fixture::native(); ready(); }
std::vector<RemoteObject> connected_objects_snapshot(int, map_state::Resolver&,std::function<bool()>) { return {}; }
}
namespace field::map_state { Resolver::Resolver() : Resolver(Flags{}, []()->const json::Value&{static const json::Value empty;return empty;}()) { fixture::native(); } }
int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    using namespace field;
    int failures = 0;
    auto expect = [&](bool ok, const char* reason) { if (!ok) { ++failures; std::printf("FAIL %s\n", reason); } };
    fixture::transition = true;
    bool transition_callback = false;
    scanner::when_filter_ready([&] { transition_callback = true; }, true);
    expect(!transition_callback && fixture::tasks.empty() && fixture::navigation.empty(),
        "transition input cannot inspect a map or enqueue scanner readiness work");
    fixture::transition = false;
    Context talk; talk.fe=&fixture::npc; talk.go_name=L"Wyvern"; talk.go_lower=L"wyvern";
    for(int script:{65,189}){fixture::talk.script=script;auto result=npc_detector(talk);expect(result.result==Result::Detected&&result.entity->kind==Kind::NPC&&result.entity->name==L"Wyvern","native-actionable Deist Wyvern dialogue scripts are NPC targets despite placeholder name");}
    // Regression: the actual scanner must omit a blank, effectless Talk actor.
    fixture::talk.action=4;fixture::talk.script=0;fixture::talk.message.clear();
    talk.go_name=L"Elder";talk.go_lower=L"elder";fixture::talk.name=L"Elder";
    expect(npc_detector(talk).result==Result::Skip,"blank effectless Talk is not an NPC destination");
    for (auto game : {game::Id::FF1,game::Id::FF2,game::Id::FF3,game::Id::FF4,game::Id::FF5,game::Id::FF6}) {
        fixture::selected_game=game;
        expect(npc_detector(talk).result==Result::Skip,"all-six blank Talk excluded");
        fixture::talk.message=L" ";expect(npc_detector(talk).result==Result::Detected,"whitespace message retained");fixture::talk.message.clear();
        fixture::talk.script=1;expect(npc_detector(talk).result==Result::Detected,"scripted Talk retained");fixture::talk.script=0;
        fixture::talk.status=1;expect(npc_detector(talk).result==Result::Detected,"status-changing Talk retained");fixture::talk.status=0;
        fixture::talk.close=L"close";expect(npc_detector(talk).result==Result::Detected,"closing message retained");fixture::talk.close.clear();
        fixture::optional_fields=false;expect(npc_detector(talk).result==Result::Detected,"unknown optional field retains actor");fixture::optional_fields=true;
        fixture::talk.message=L"new metadata";expect(npc_detector(talk).result==Result::Detected,"live message update restores actor");fixture::talk.message.clear();
    }
    fixture::selected_game=game::Id::FF2;
    const int native_before=fixture::native_reads;const auto talk_started=std::chrono::steady_clock::now();
    for (int i=0;i<1000;++i) expect(npc_detector(talk).result==Result::Skip,"bounded blank Talk batch");
    const double batch_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-talk_started).count();
    std::printf("Blank Talk fixture: 1000 owner detections, %d fake native calls, %.3f ms cumulative; not native/game-frame latency\n",fixture::native_reads-native_before,batch_ms);
    expect(fixture::wrong_owner==0,"blank Talk reads stay on owner");
    fixture::talk.action=2;talk.go_name=L"Wyvern";talk.go_lower=L"wyvern";fixture::talk.name=L"飛竜";
    fixture::talk.script=0;fixture::talk.message=L"VisibleDialogueKey";
    expect(npc_detector(talk).result==Result::Detected,"message-only animal NPC bypasses the placeholder name filter");
    fixture::talk.message.clear();expect(npc_detector(talk).result==Result::Skip,"scriptless/message-less cutscene or vehicle NPC remains excluded");
    fixture::talk.script=65;fixture::can_action=false;expect(npc_detector(talk).result==Result::Skip,"native CanAction still owns interactability");fixture::can_action=true;
    fixture::can_action=false;fixture::talk.action=9;
    for(int script:{1207,1208,1209,1210,1211,1212}) {
        fixture::talk.script=script;
        expect(npc_detector(talk).result==Result::Detected,"all six Terra defense contact encounters survive the talk-only CanAction guard");
    }
    fixture::talk.script=0;
    expect(npc_detector(talk).result==Result::Skip,"action 9 without a script does not admit a cutscene placeholder");
    fixture::talk.script=0;fixture::talk.action=2;fixture::can_action=true;
    talk.go_name=L"Masked Figure";talk.go_lower=L"masked figure";fixture::talk.name=L"仮面　色変え1";
    expect(npc_detector(talk).result==Result::Skip,"Mysidia's scriptless Masked Figure is staging, not a usable talk target");
    fixture::talk.script=89;expect(npc_detector(talk).result==Result::Detected,"assigning a real script restores a previously staged NPC");
    fixture::talk.script=0;fixture::talk.action=8;expect(npc_detector(talk).result==Result::Detected,"native shop actions remain available without a dialogue script");
    fixture::talk.script=65;fixture::talk.action=2;fixture::can_action=true;
    for(const auto* name:{L"太陽の炎トリガー用",L"女神のベルトリガー用"}) {
        fixture::talk.name=name;fixture::manual_object=true;
        Context object;object.fe=&fixture::object;object.go_name=name;object.go_lower=text::lower(name);
        expect(npc_detector(object).result==Result::NotHandled,"manual keyword objects never become NPCs by dialogue");
        auto manual=interactive_detector(object);
        expect(manual.result==Result::Detected&&manual.entity->event_type==L"Interactive Object"&&manual.entity->category()==Category::Objects,
            "actual manual entity classes put Sunfire/Bell examinations in Objects regardless of trigger names");
        fixture::manual_object=false;object.is_event_trigger=true;
        auto event=event_trigger_detector(object);
        expect(event.result==Result::Detected&&event.entity->category()==Category::Events,"actual walk-on classes retain Events with the same script/name");
    }
    RemoteInput remote;RemoteRow row;row.object.type=baked::ObjType::NPC;row.object.map_id=180;row.object.raw_name=L"飛竜";row.object.scripted=true;row.name=L"Wyvern";row.floor=L"Castle Deist: Wyvern's Room";remote.rows.push_back(row);
    auto built=build_remote(remote);expect(built.size()==1&&built[0]->kind==Kind::NPC&&built[0]->name==L"Wyvern","scripted Wyvern remains an NPC target across floors");
    RemoteBuilder scripted({row.object});scripted.step();expect(scripted.input.rows.size()==1,"remote name capture accepts scripted Wyvern despite placeholder filter");row.object.scripted=false;RemoteBuilder marker({row.object});marker.step();expect(marker.input.rows.empty(),"remote cutscene Wyvern without interaction script remains excluded");
    {
        RemoteInput events;
        for(int id:{32,33}) { RemoteRow event;event.object.type=baked::ObjType::Entity;event.object.event_type=L"Interactive Object";
            event.object.raw_name=L"ペンダント";event.object.entity_id=id;event.object.map_id=184;event.object.cell={0,35,18};
            event.floor=L"Deist Cavern B2";events.rows.push_back(event); }
        std::vector<EntityPtr> objects;
        std::thread worker([&]{objects=build_remote(events);});worker.join();
        expect(objects.size()==2&&objects[0]->kind==Kind::Event&&objects[0]->name==L"Conversation"&&objects[0]->floor_name==L"Deist Cavern B2"&&objects[0]->remote_map_id==184,
            "remote events retain safe labels/category/floor and distinct native identities at the same cell");
        events.cancelled=[]{return true;};expect(build_remote(events).empty(),"retired event-label worker returns no partial destinations");
        auto event=events.rows.front().object;event.raw_name=L"飛竜";RemoteBuilder placeholder_event({event});placeholder_event.step();
        expect(placeholder_event.input.rows.empty(),"remote events preserve the game's existing placeholder exclusions");
    }
    g_last_scanned_map_id = fixture::map;
    {
        g_category=Category::Objects;
        for(int i=0;i<3;++i){auto e=std::make_shared<Entity>();e->kind=Kind::Event;e->event_type=L"Interactive Object";
            e->converted_name=L"Book";e->name=L"Book999";e->entity_id=i;e->pos={float(i),0,0};g_entities.push_back(e);}
        apply_filter();for(int tick=0;g_filter_pending&&tick<100;++tick){fixture::tick();fixture::work();}
        expect(g_entities[0]->name==L"Book1"&&g_entities[2]->name==L"Book3","actual capture/publication numbers original object labels rather than prior output");
        apply_filter();for(int tick=0;g_filter_pending&&tick<100;++tick){fixture::tick();fixture::work();}
        expect(g_entities[0]->name==L"Book1"&&g_entities[2]->name==L"Book3","object suffixes survive actual recapture without accumulation");
        g_entities.clear();g_category=Category::All;
    }
    for (int i = 0; i < 2000; ++i) {
        auto entity = std::make_shared<Entity>(); entity->kind = i % 2 ? Kind::NPC : Kind::MapExit;
        entity->name = entity->converted_name = i % 2 ? L"Soldier" : L"Exit";
        entity->pos = {static_cast<float>(i), 0, 0}; g_entities.push_back(std::move(entity));
    }
    apply_filter();
    int frames = 0;
    while (g_filter_pending && frames++ < 100) { fixture::tick(); fixture::work(); }
    expect(g_views_ready && scanner::count() == 2000, "initial owner capture and worker publication prepare the actual list");
    expect(frames < 100, "2,000-row preparation has no four-entry-per-frame delay");
    expect(fixture::wrong_owner == 0, "worker build makes zero native calls");
    // Production adapter over the existing published pathfinder registry. Its
    // capacity is prepared on the audio worker before native acquisition.
    auto measure_capture=[&](const char* label) {
        sound::scape::Snapshot snapshot; snapshot.rows.reserve(sound::scape::max_rows);
        size_t next=0;uint64_t revision=0;int calls=0,result=0;double peak=0,total=0;
        const double start=frame::now();
        while(result==0 && calls<100) {
            const double begin=frame::now(); result=field::capture_soundscape(snapshot,next,revision);
            const double cost=frame::now()-begin; peak=std::max(peak,cost);total+=cost;++calls;
        }
        expect(result==1&&snapshot.rows.size()==g_audio_entities.size(),"soundscape captures every published row across bounded native slices");
        expect(calls>1&&calls>=int((next+511)/512),"soundscape enforces the per-slice count bound");
        expect(!fixture::wrong_owner,"soundscape never accesses a native pointer off its owner");
        ++g_audio_revision;
        expect(field::capture_soundscape(snapshot,next,revision)==-1,"changed pathfinder registry invalidates an unfinished snapshot");
        std::printf("Soundscape %s %zu: %d native slices, peak %.3f ms, aggregate %.3f ms, copy %.3f ms plus frame scheduling\n",
            label,next,calls,peak*1000,total*1000,(frame::now()-start)*1000);
    };
    measure_capture("normal");
    // Repeated production soundscape captures must never consume navigation
    // selection/category/readiness or retire pending native-owner input work.
    scanner::set_category(Category::NPCs);scanner::next_entity();
    const auto audio_selected=scanner::current_entity();const int audio_index=scanner::current_index();
    scanner::when_filter_ready([]{},true); // one coalesced post-directions refresh is expected
    const auto audio_tasks=fixture::tasks.size();bool audio_browse=false,audio_directions=false;
    for(int repeat=0;repeat<20;++repeat) {
        sound::scape::Snapshot audio;audio.rows.reserve(sound::scape::max_rows);
        size_t next=0;uint64_t revision=0;
        for(int step=0;step<100 && capture_soundscape(audio,next,revision)==0;++step) {}
        scanner::when_filter_ready([&]{audio_browse=true;});
        scanner::when_filter_ready([&]{audio_directions=true;},true);
    }
    expect(audio_browse&&audio_directions&&g_views_ready&&scanner::current_entity()==audio_selected&&
        scanner::current_index()==audio_index&&scanner::category()==Category::NPCs&&fixture::tasks.size()==audio_tasks,
        "continuous production soundscape capture preserves actual scanner browse/directions readiness, selection and category");
    scanner::cancel_navigation_work(); // synthetic rows below have no native collection
    scanner::set_category(Category::All);
    auto saved_audio_view=g_audio_entities;
    g_audio_entities.resize(sound::scape::max_rows,saved_audio_view.front());
    for(auto& e:g_audio_entities) e->audio_hint=std::wstring(512,L'x');
    measure_capture("busy");
    g_audio_entities.push_back(saved_audio_view.front());
    sound::scape::Snapshot invalid;size_t invalid_next=0;uint64_t invalid_revision=0;
    expect(field::capture_soundscape(invalid,invalid_next,invalid_revision)==-1&&invalid.rows.empty(),"over-bound registry fails closed before traversal/allocation");
    g_audio_entities=std::move(saved_audio_view);
    const auto started = frame::now();
    const auto task_count = fixture::tasks.size();
    for (int i = 0; i < 1000; ++i) { scanner::set_category(Category::NPCs); scanner::set_category(Category::MapExits); }
    expect(scanner::count() == 1000 && fixture::tasks.size() == task_count && fixture::navigation.empty(),
        "warm category switches enqueue no capture, name publication or worker job");
    const auto category_us = (frame::now() - started) * 1e6 / 2000;
    fixture::pose = {1000, 0, 0};
    scanner::set_category(Category::NPCs);
    expect(!g_views_ready && g_filter_pending, "movement refreshes category distance ordering without a full entity scan");
    for (int i = 0; g_filter_pending && i < 100; ++i) { fixture::tick(); fixture::work(); }
    expect(g_views_ready && filtered_view()[0]->pos.x == 999, "new category starts at the nearest target from the current position");
    scanner::set_category(Category::MapExits);
    g_sel = filtered_view()[5]; g_sel_has = true; g_index = 5;
    apply_filter(); // leave the refresh worker/capture blocked
    scanner::g_scan_running = true;
    bool spoken = false;
    scanner::when_filter_ready([&] { spoken = true; });
    expect(spoken, "browse readiness completes immediately while both live scan and filter refresh are pending");
    g_sel = filtered_view()[9]; g_index = 9; // input changes while refresh is pending
    for (int i = 0; g_filter_pending && i < 100; ++i) { fixture::tick(); fixture::work(); }
    expect(scanner::current_entity() == g_sel, "refresh publication keeps the most recent input selection");
    filtered_view()[10]->game = reinterpret_cast<void*>(2);
    scanner::next_entity();
    expect(scanner::current_index() == 11, "warm browsing skips a despawned native handle even without reachability filtering");
    g_navigation_filtered=true;g_view_vehicle=0;
    fixture::pose.x+=1;spoken=false;
    const auto selected_started=frame::now();
    scanner::when_filter_ready([&]{spoken=true;},true);
    const auto selected_us=(frame::now()-selected_started)*1e6;
    const auto restarts=std::count_if(fixture::tasks.begin(),fixture::tasks.end(),[](const auto& entry){return entry.second.label=="EntityScanner.RefreshAfterDirections";});
    expect(spoken && g_views_ready && restarts==1,
        "directions to a live selected target after movement do not wait for world-list refiltering");
    apply_filter();spoken=false;fixture::route_pending=true;
    const auto previous_filter=g_filter_work.epoch()->load(),previous_scan=scanner::g_scan_work.epoch()->load();
    scanner::when_filter_ready([&]{spoken=true;},true);
    expect(spoken && g_views_ready && !g_filter_pending &&
        previous_filter!=g_filter_work.epoch()->load() && previous_scan!=scanner::g_scan_work.epoch()->load(),
        "selected route cancels both refresh owners even without movement and keeps its published destination");
    fixture::tick();
    expect(!g_filter_pending&&!scanner::g_scan_running,"no replacement refresh queues ahead of the pending interactive route");
    fixture::route_pending=false;fixture::tick();
    expect(scanner::g_scan_running,"refresh restarts from current native collections after the route completes");
    const auto scan_epoch_before=scanner::g_scan_work.epoch()->load(),filter_epoch_before=g_filter_work.epoch()->load();
    const int reads_before_interrupt=fixture::native_reads;
    const auto tasks_before_interrupt=fixture::tasks.size();
    int callbacks_retired=0;
    scanner::g_scan_work.retain([&]{++callbacks_retired;});
    g_filter_work.retain([&]{++callbacks_retired;});
    scanner::interrupt_navigation_work();
    expect(scanner::g_scan_work.epoch()->load()==scan_epoch_before+1 && g_filter_work.epoch()->load()==filter_epoch_before+1 &&
        !scanner::g_scan_running && !g_filter_pending && fixture::native_reads==reads_before_interrupt &&
        fixture::tasks.size()==tasks_before_interrupt && callbacks_retired==0,
        "battle prefix retires scanner generations without native reads, scanning task queues or destroying owner callbacks");
    scanner::cancel_navigation_work();fixture::pose.x+=16;
    expect(callbacks_retired==2,"ready-phase scanner cleanup releases both interrupted owner registries once");
    spoken=false;
    scanner::when_filter_ready([&]{spoken=true;});
    expect(!spoken&&!g_views_ready&&g_filter_pending,"moving before grouped-exit browse waits for the new nearest-exit generation");
    for(int i=0;!spoken&&i<100;++i){fixture::tick();fixture::work();}
    expect(spoken,"grouped-exit browse resumes after owned worker publication");
    g_views_ready = false;
    spoken=false;scanner::when_filter_ready([&]{spoken=true;},true);
    expect(spoken&&!g_filter_pending,"stale-origin list publication cannot block a still-valid selected destination");
    g_sel.reset(); // cold/unselected context still requires list readiness
    spoken = false;
    scanner::when_filter_ready([&] { spoken = true; }, true);
    fixture::transition = true; fixture::tick(); fixture::transition = false;
    g_views_ready = true; fixture::tick();
    expect(!spoken, "a loading transition retires pending input rather than replaying it on return");
    scanner::invalidate_map();
    spoken = false; scanner::when_filter_ready([&] { spoken = true; },true);
    expect(!spoken, "map invalidation requires a new ready generation");
    ++fixture::map; fixture::tick(); fixture::work(); fixture::tick();
    expect(!spoken && !g_views_ready, "changed map retires pending input and cannot publish stale categories");
    for(int id:{0,32}) {
        scanner::invalidate_map();fixture::map=183;g_last_scanned_map_id=183;g_category=Category::Objects;fixture::capture_cells=true;
        auto make_event=[&](int map,int identity){auto e=std::make_shared<Entity>();e->kind=Kind::Event;e->name=L"Conversation";
            e->event_type=L"Interactive Object";e->entity_id=identity;e->remote_map_id=map;e->cell_x=35;e->cell_y=18;e->floor_name=L"B2";return e;};
        auto target=make_event(184,id),other=make_event(184,id+100),wrong_floor=make_event(185,id);
        g_entities={other,wrong_floor,target};g_sel.reset();g_sel_has=true;g_sel_map=184;g_sel_entity_id=id;
        g_sel_cx=35;g_sel_cy=18;g_sel_category=Category::Objects;
        apply_filter();for(int tick=0;g_filter_pending&&tick<100;++tick){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==target&&g_sel==target,"cross-floor publication restores the exact event and binds its new owner-side handle, including native ID zero");
        fixture::map=184;g_last_scanned_map_id=184;
        auto native=make_event(0,id);native->game=reinterpret_cast<void*>(1);native->pos={35,18,0};
        other=make_event(0,id+100);other->game=reinterpret_cast<void*>(1);other->pos=native->pos;
        g_entities={other,native};apply_filter();for(int tick=0;g_filter_pending&&tick<100;++tick){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==native&&g_sel==native,"returning to the destination floor rebinds the projected event to the exact live event");
        g_entities={other};apply_filter();for(int tick=0;g_filter_pending&&tick<100;++tick){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==nullptr&&g_index==-1,"missing event never silently selects an unrelated or collocated destination");
        scanner::previous_entity();expect(scanner::current_entity()==other,"explicit browsing safely resumes after a target disappears");
    }
    {
        fixture::selected_game=game::Id::FF3;
        scanner::invalidate_map(); scanner::clear_selection(); fixture::map=80; g_last_scanned_map_id=80;
        fixture::capture_cells=true; fixture::grouped_exits=true; fixture::has_goal=true;
        fixture::goal={}; fixture::goal.map_id=77; fixture::goal.cell={1,4,6}; fixture::goal.entity_id=0; fixture::goal.raw_name=L"target";
        g_category=Category::MapExits;
        auto exit=[](int map,int id,int x,const wchar_t* raw){auto e=std::make_shared<Entity>();e->kind=Kind::MapExit;
            e->entity_id=id;e->remote_map_id=map;e->cell_x=x;e->cell_y=6;e->dest_map_id=1;
            e->audio_hint=raw;e->raw_name_known=true;e->name=e->converted_name=L"World";
            if (!map) {e->game=reinterpret_cast<void*>(1);e->pos={static_cast<float>(x),6,0};} return e;};
        auto remote=exit(77,0,4,L"target"), sibling=exit(77,1,5,L"sibling");
        g_entities={remote,sibling}; apply_filter(); for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==remote && g_sel==remote, "grouped pending remote exit is owner selected");
        fixture::map=77;g_last_scanned_map_id=77;
        auto live=exit(0,0,4,L"target"); sibling=exit(0,1,5,L"sibling");
        // Position makes the sibling nearer under the actual fallback comparator.
        fixture::pose={5,6,0}; g_entities={sibling,live}; apply_filter();
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==live && g_sel==live && fixture::goal.entity_id==0 && fixture::goal.raw_name==L"target",
            "floor handoff publishes exact new live handle without changing pending goal");
        // Same-map selection, without a pending goal, survives owner publication
        // when another physical exit becomes the nearer representative.
        fixture::has_goal=false; scanner::clear_selection(); fixture::pose={4,6,0};
        g_entities={live,sibling}; apply_filter();
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==live, "same-map initial grouped exit selected");
        scanner::remember_selection(); // Mirror the actual player selection owner.
        fixture::pose={5,6,0}; apply_filter();
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==live && g_sel==live && g_index>=0,
            "same-map selected exit survives distance flip through owner publication without pending goal");
        fixture::has_goal=true;
        // A cleared pending goal cannot authorize an older worker publication.
        apply_filter(); fixture::tick(); fixture::work();
        fixture::has_goal=false; scanner::clear_selection(); g_index=-1;
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(g_index==-1 && !g_sel, "cleared pending goal cannot restore an older captured target");
        // A map generation change retires the old captured handles.
        fixture::has_goal=true; apply_filter(); fixture::tick(); fixture::work();
        fixture::map=78; scanner::invalidate_map(); fixture::tick(); fixture::work(); fixture::tick();
        expect(!g_filter_pending && !g_sel, "changed map cannot publish a stale live handle");
        fixture::map=77; g_last_scanned_map_id=77; g_entities={sibling,live};
        fixture::has_goal=false; scanner::clear_selection(); apply_filter();
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(filtered_view().size()==1 && filtered_view()[0]==sibling, "manual clear releases grouped pin");
        fixture::grouped_exits=false; fixture::capture_cells=false; fixture::pose={}; scanner::invalidate_map(); scanner::clear_selection();
        fixture::selected_game=game::Id::FF2;
    }
    // Battle/state invalidation rebuilds native handles. Ship and Airship may
    // have the same default entity ID/raw placeholder; model ID is authoritative.
    for(int game=1;game<=6;++game) {
        fixture::selected_game=static_cast<game::Id>(game);
        scanner::invalidate_map();scanner::clear_selection();fixture::map=1;g_last_scanned_map_id=1;
        g_category=Category::Vehicles;fixture::capture_cells=true;
        auto vehicle=[](int id,float x){auto e=std::make_shared<Entity>();e->kind=Kind::Vehicle;
            e->name=id==2?L"Ship":L"Airship";e->audio_hint=L"NPC";e->raw_name_known=true;
            e->entity_id=0;e->transportation_id=id;e->game=reinterpret_cast<void*>(1);e->pos={x,0,0};return e;};
        auto ship=vehicle(2,10),airship=vehicle(3,200);
        g_entities={ship,airship};apply_filter();for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        g_index=static_cast<int>(std::find(filtered_view().begin(),filtered_view().end(),airship)-filtered_view().begin());scanner::remember_selection();
        expect(g_sel==airship&&g_sel_transportation_id==3,"fixture selects the actual Airship after distance sorting");
        for(float distance:{190.f,100.f,20.f,2.f,250.f}) {
            scanner::invalidate_map();g_last_scanned_map_id=1;
            ship=vehicle(2,10);airship=vehicle(3,distance);g_entities={ship,airship};
            expect(!scanner::current_entity(),"retired native list never exposes a different default target during battle restoration");
            apply_filter();for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
            expect(scanner::current_entity()==airship&&g_sel_transportation_id==3,"recreated Airship survives battle and all distance-order changes despite shared default NPC identity");
        }
        g_entities={ship};apply_filter();for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(!scanner::current_entity()&&g_sel_transportation_id==3,"temporarily missing Airship cannot fall back to regular Ship");
        g_entities={ship,airship};apply_filter();for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==airship,"Airship identity restores when its native model becomes available again");
        // Native recreation can be captured while the player changes focus
        // in the still-published list. An old worker index must not win.
        g_index=static_cast<int>(std::find(filtered_view().begin(),filtered_view().end(),ship)-filtered_view().begin());scanner::remember_selection();
        auto replacement_ship=vehicle(2,10),replacement_airship=vehicle(3,250);
        g_entities={replacement_ship,replacement_airship};apply_filter();
        scanner::next_entity();expect(g_sel==airship,"new browse input selects the still-published Airship during recreation");
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()==replacement_airship&&g_sel_transportation_id==3,"a newer selection revision supersedes the older worker's Ship restoration index");
    }
    fixture::selected_game=game::Id::FF2;
    expect(fixture::wrong_owner==0,"event capture/publication and identity matching preserve native thread ownership");
    {
        scanner::invalidate_map();fixture::candidates={&fixture::npc};fixture::can_action=true;fixture::action_reads=0;
        std::printf("Checking native action revalidation\n");
        auto old=std::make_shared<Entity>();old->game=&fixture::npc;old->kind=Kind::NPC;old->name=L"Wyvern";
        g_entity_map={{&fixture::npc,old}};g_entities={old};scanner::g_converted_revision=scanner::g_state_revision;
        g_last_scanned_map_id=fixture::map;g_remote_map_id=fixture::map;scanner::schedule_scan();
        for(int i=0;scanner::g_scan_running&&i<100;++i){fixture::tick();fixture::work();}
        expect(g_entity_map.size()==1&&g_entity_map[0].second==old&&fixture::action_reads==0,
            "ordinary refresh retains a valid known native action without reconverting it on each key");
        scanner::refresh_state();
        for(int i=0;(scanner::g_scan_running||g_filter_pending)&&i<100;++i){fixture::tick();fixture::work();}
        expect(fixture::action_reads>0&&g_entity_map.size()==1&&g_entity_map[0].second->game==&fixture::npc,
            "returning from a reusable native interaction keeps the objective available");
        fixture::action_reads=0;
        fixture::can_action=false;scanner::refresh_state();
        for(int i=0;(scanner::g_scan_running||g_filter_pending)&&i<100;++i){fixture::tick();fixture::work();}
        expect(fixture::action_reads>0&&g_entity_map.empty()&&g_entities.empty(),
            "script return rechecks known native actions and removes consumed entries instead of retaining the cached conversion");
        expect(g_remote_map_id==fixture::map,"script return also rebuilds the other-floor projection from new story/chest flags");
        fixture::candidates.clear();fixture::can_action=true;
    }
    apply_filter();
    const auto filter_epoch=g_filter_work.epoch()->load(),scan_epoch=scanner::g_scan_work.epoch()->load();
    scanner::cancel_navigation_work();
    expect(g_filter_work.epoch()->load()!=filter_epoch && scanner::g_scan_work.epoch()->load()!=scan_epoch &&
        !g_filter_pending && !scanner::g_scan_running,"transition cancellation retires both scan and filter worker epochs before native loading");
    std::printf("Scanner: 2,000 rows prepared in %d capture/publication ticks; warm category %.3f us; %d failures\n",
        frames, category_us, failures);
    {
        std::ifstream file("data/ff2/maps.bin",std::ios::binary);const std::string bytes{std::istreambuf_iterator<char>(file),{}};
        expect(baked::load(bytes.data(),bytes.size()),"scripted exit detector loads matching FFII bake");
        const auto* map=baked::submap_by_map_id(20);const auto& grid=baked::grids(*map);
        const baked::Object* exit=nullptr;
        for(const auto& object:grid.objects)if(baked::world_exit(object)&&object.tx==20&&object.ty==38){exit=&object;break;}
        expect(exit!=nullptr,"Fynn south boundary is a verified scripted world exit");
        if(exit) {
            fixture::map=20;fixture::capture_cells=true;fixture::talk.name=baked::object_identifier(*exit);fixture::talk.id=exit->entity_id();
            Context context;context.fe=&fixture::object;context.is_event_trigger=true;context.is_active=true;
            context.position={20,38,0};context.go_lower=L"event";
            const auto converted=goto_map_detector(context);
            expect(converted.result==Result::Detected&&converted.entity->kind==Kind::MapExit&&converted.entity->dest_map_id==1&&
                converted.entity->name==L"Exit to World Map","actual live detector labels/categories Fynn's script exit before generic Scene trigger");
            baked_nav::RemoteObject source;source.world_exit=true;source.map_id=20;source.type=baked::ObjType::Event;source.dest_map_id=1;
            source.raw_name=fixture::talk.name;source.entity_id=fixture::talk.id;source.cell={0,20,38};
            RemoteBuilder builder({source});builder.step();auto rows=build_remote(builder.input);
            expect(rows.size()==1&&rows[0]->kind==Kind::MapExit&&rows[0]->name==L"Exit to World Map"&&rows[0]->remote_map_id==20,
                "remote scripted exits use the same world destination and Map Exits category");
        }
    }
    scanner::invalidate_map();
    {
        std::ifstream f("data/ff3/maps.bin",std::ios::binary);const std::string bytes((std::istreambuf_iterator<char>(f)),{});
        expect(baked::load(bytes.data(),bytes.size()),"FFIII border audit loads the actual bake");
        std::ifstream labels("data/ff3/eventlabels.json");const std::string json_text((std::istreambuf_iterator<char>(labels)),{});
        expect(json::parse(json_text,fixture::event_metadata),"actual reviewed event identity catalog loads");
        fixture::selected_game=game::Id::FF3;fixture::real_event_labels=true;
        for(const auto& target:std::vector<std::pair<int,const wchar_t*>>{{252,L"Lute"},{290,L"Fang"}}){
            const auto* floor=baked::submap_by_map_id(target.first);const baked::Object* actual=nullptr;
            for(const auto& o:baked::grids(*floor).objects)if(o.type==baked::ObjType::AnimEntity&&o.entity_id()==5&&(o.d&(4|8)))actual=&o;
            expect(actual!=nullptr,"reported Lute/Fang is present as a real animated interactive object");if(!actual)continue;
            baked_nav::RemoteObject source;source.type=actual->type;source.map_id=target.first;source.entity_id=actual->entity_id();
            source.raw_name=baked::object_identifier(*actual);source.group=baked::str(actual->group);source.cell={actual->layer,actual->tx,actual->ty};source.event_type=L"Interactive Object";
            RemoteBuilder builder({source});while(builder.step()){}
            std::vector<EntityPtr> rows;std::thread worker([&]{rows=build_remote(builder.input);});worker.join();
            expect(rows.size()==1&&rows[0]->category()==Category::Objects&&rows[0]->kind==Kind::Event&&rows[0]->name==target.second&&
                rows[0]->remote_map_id==target.first&&rows[0]->entity_id==5&&rows[0]->cell_layer==actual->layer&&
                rows[0]->cell_x==actual->tx&&rows[0]->cell_y==actual->ty&&rows[0]->raw_name_known&&rows[0]->audio_hint==source.raw_name,
                "actual owner capture, label worker and entity publication retain remote animated object identity, localized name and Objects category");
        }
        fixture::real_event_labels=false;
        const auto* world=baked::submap_by_name(L"Map_10010");int pruned=0,scenes=0;
        fixture::selected_game=game::Id::FF3;fixture::capture_cells=true;fixture::manual_object=false;fixture::map=world->first_map_id;
        for(const auto& o:baked::grids(*world).objects)if(o.type==baked::ObjType::Event) {
            Context row;row.fe=&fixture::object;row.is_event_trigger=true;row.position={float(o.tx),float(o.ty),0};fixture::talk.name=baked::object_identifier(o);fixture::talk.id=o.entity_id();
            const auto result=goto_map_detector(row);
            if(event_route_only(fixture::talk.name)){expect(result.result==Result::Skip,"actual detector removes every routine world-border source, independently of event grouping");++pruned;}
            else if(result.result!=Result::Skip)++scenes;
        }
        expect(pruned==1525&&scenes>0,"all three 508-tile border families and passive Chocobo bookkeeping disappear while meaningful scene triggers remain");
        std::printf("FFIII actual discovery: %d routine borders pruned, %d meaningful event sources retained; baked graph objects unchanged\n",pruned,scenes);
        int returns=0;
        for(int map:{2,4}) { fixture::map=map;const auto* surface=baked::submap_by_map_id(map);
            for(const auto& o:baked::grids(*surface).objects)if(const auto* rule=air_transitions::match(3,map,o)) {
                Context row;row.fe=&fixture::object;row.is_event_trigger=true;row.position={float(o.tx),float(o.ty),0};
                fixture::talk.name=baked::object_identifier(o);fixture::talk.id=o.entity_id();fixture::talk.script=rule->script;
                expect(goto_map_detector(row).result==Result::Skip,"reviewed native return trigger is suppressed before generic scene classification");++returns;
            }
        }
        expect(returns==16,"one flooded and fifteen normal return native sources suppressed");fixture::talk.script=65;
        const auto* surface=baked::submap_by_map_id(4);fixture::map=4;
        int entrances=0;
        for(const auto& o:baked::grids(*surface).objects)if(o.type==baked::ObjType::Warp&&o.entity_id()>=888&&o.entity_id()<=890&&baked::object_identifier(o)==L"ファルガバード入口"){
            fixture::talk.name=baked::object_identifier(o);fixture::talk.id=o.entity_id();fixture::talk.script=381;
            fixture::pose={float(o.tx),float(o.ty),0};
            const auto entry=convert(&fixture::object);
            expect(entry&&entry->kind==Kind::MapExit&&entry->category()==Category::MapExits&&entry->dest_map_id==144&&entry->name==L"Exit to Falgabard",
                "complete native detector chain publishes each Falgabard entrance as a named Map Exit, without an Event duplicate");
            baked_nav::RemoteObject source;source.world_exit=true;source.map_id=4;source.type=baked::ObjType::Event;source.dest_map_id=144;
            source.raw_name=fixture::talk.name;source.entity_id=o.entity_id();source.cell={1,o.tx,o.ty};
            RemoteBuilder builder({source});builder.step();auto rows=build_remote(builder.input);
            expect(rows.size()==1&&rows[0]->category()==Category::MapExits&&rows[0]->name==L"Exit to Falgabard",
                "remote physical entrance uses the same actual destination name and exit category");
            ++entrances;
        }
        expect(entrances==3,"all native Falgabard direction-sensitive entrance tiles checked");fixture::talk.script=65;
        fixture::map=3;entrances=0;
        for(const auto& o:baked::grids(*baked::submap_by_map_id(3)).objects)if(baked::submarine_entry(o)) {
            fixture::talk.name=baked::object_identifier(o);fixture::talk.id=o.entity_id();fixture::talk.script=o.entity_id()<6?370:o.entity_id()==6?362:366;
            fixture::pose={float(o.tx),float(o.ty),0};const auto entry=convert(&fixture::object);
            expect(entry&&entry->kind==Kind::MapExit&&entry->category()==Category::MapExits&&entry->dest_map_id==o.a&&entry->name==TF(L"Exit to {0}",{resolve_map_name(o.a)}),
                "complete underwater native detector chain produces a named Map Exit instead of a Scene trigger");
            ++entrances;
        }
        expect(entrances==4,"all4 real underwater entrance tiles pass actual detector/category production");fixture::talk.script=65;
    }
    {
        scanner::invalidate_map();scanner::clear_selection();fixture::selected_game=game::Id::FF3;
        fixture::map=2;g_last_scanned_map_id=2;fixture::air_capture=true;fixture::capture_cells=true;fixture::pose={};
        baked_nav::AirTransitionTarget target;target.goal.map_id=2;target.goal.cell={1,4,6};target.goal.entity_id=0;
        target.goal.raw_name=L"exact";target.position={4,6,0};target.name=L"Exit to Floating Continent";target.distance=3;target.destination_map=0;
        fixture::air_target=target;g_entities.clear();g_category=Category::MapExits;apply_filter();
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        scanner::next_entity();auto selected=scanner::current_entity();
        expect(selected&&selected->aerial_transition&&selected->audio_exit&&selected->local_baked_map_id==2&&
            filtered_view().size()==1&&g_audio_entities.size()==1,"one owned aerial target publishes to exit view and soundscape");
        sound::scape::Snapshot audio;size_t next=0;uint64_t revision=0;int status=0;
        for(int t=0;t<100&&status==0;++t)status=capture_soundscape(audio,next,revision);
        expect(status==1&&audio.rows.size()==1&&audio.rows[0].world_exit&&audio.rows[0].verified_exit&&audio.rows[0].destination==0,
            "unknown surface destination remains a verified world exit in soundscape");
        scanner::remember_selection();apply_filter();for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()&&scanner::current_entity()->aerial_transition&&scanner::current_entity()->entity_id==0,
            "owner restores recreated exact ID-zero aerial selection");
        fixture::map=4;scanner::invalidate_map();expect(!scanner::current_entity(),"map invalidation retires the local baked target");
        fixture::air_target.reset();fixture::map=4;g_last_scanned_map_id=4;scanner::clear_selection();g_entities.clear();
        auto castle=std::make_shared<Entity>();castle->kind=Kind::MapExit;castle->dest_map_id=105;castle->dest_name=L"Castle Saronia";
        castle->name=castle->converted_name=L"Exit to Castle Saronia";castle->pos={83,67,0};g_entities.push_back(castle);
        target.goal.map_id=4;target.goal.cell={1,84,66};target.goal.entity_id=870;target.goal.raw_name=L"飛空艇大破";
        target.position={84,66,0};target.destination_map=105;target.name=L"Saronia Airspace";fixture::story_target=target;g_category=Category::Events;apply_filter();
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        scanner::next_entity();selected=scanner::current_entity();
        expect(selected&&selected->kind==Kind::Event&&selected->category()==Category::Events&&selected->aerial_transition&&
            !selected->audio_exit&&selected->name==L"Saronia Airspace"&&selected->local_baked_map_id==4,
            "actual owner publishes Saronia to Events without any native event candidate");
        sound::scape::Snapshot story_audio;next=0;revision=0;status=0;
        for(int t=0;t<100&&status==0;++t)status=capture_soundscape(story_audio,next,revision);
        expect(status==1&&story_audio.rows.size()==2&&std::count_if(story_audio.rows.begin(),story_audio.rows.end(),[](const auto& row){
            return row.kind==sound::scape::Kind::Event&&!row.world_exit&&!row.verified_exit;
        })==1&&std::count_if(story_audio.rows.begin(),story_audio.rows.end(),[](const auto& row){return row.kind==sound::scape::Kind::Exit;})==1,
            "Current map retains the blocked castle exit while Saronia Airspace keeps independent event soundscape semantics");
        scanner::remember_selection();apply_filter();for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()&&scanner::current_entity()->kind==Kind::Event&&scanner::current_entity()->entity_id==870,
            "Saronia event selection survives owned handle recreation");
        fixture::story_target.reset();scanner::invalidate_map();scanner::clear_selection();fixture::map=3;g_last_scanned_map_id=3;g_entities.clear();
        target.goal.map_id=3;target.goal.cell={1,150,204};target.goal.entity_id=-300014;target.goal.raw_name=L"ffpr:surface:doga_village";
        target.position={150,204,0};target.destination_map=4;target.distance=147;target.name=L"Surface near Doga's Village";
        fixture::surface_target=target;g_category=Category::MapExits;apply_filter();
        for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        scanner::next_entity();selected=scanner::current_entity();
        expect(selected&&surface_target(*selected)&&!selected->aerial_transition&&selected->kind==Kind::MapExit&&
            selected->audio_exit&&selected->local_baked_map_id==3&&selected->dest_name==L"Surface World"&&
            filtered_view().size()==1,"actual owner publishes the manual surface marker as a submarine Map Exit without a native Event");
        sound::scape::Snapshot surface_audio;next=0;revision=0;status=0;
        for(int t=0;t<100&&status==0;++t)status=capture_soundscape(surface_audio,next,revision);
        expect(status==1&&surface_audio.rows.size()==1&&surface_audio.rows[0].world_exit&&surface_audio.rows[0].verified_exit,
            "surfacing marker remains one verified exit in soundscape");
        scanner::remember_selection();apply_filter();for(int t=0;g_filter_pending&&t<100;++t){fixture::tick();fixture::work();}
        expect(scanner::current_entity()&&surface_target(*scanner::current_entity())&&scanner::current_entity()->audio_hint==L"ffpr:surface:doga_village",
            "surfacing selection survives owned-handle recreation with its exact marker identity");
        fixture::map=4;scanner::invalidate_map();expect(!scanner::current_entity(),"surfacing map change retires the marker");
        fixture::surface_target.reset();fixture::air_capture=false;fixture::capture_cells=false;fixture::pose={};fixture::selected_game=game::Id::FF2;
    }
    {
        // Every real scripted world event from the IV/V/VI source inventory
        // passes the actual native-class detector chain, including Torna and
        // the underworld/airship action pads. Geometry/reachability is separately
        // checked by actual owned planner fixtures, never this host's route stub.
        std::ifstream input("../tools/world_event_capture_20261029/inventory.json",std::ios::binary);
        std::string inventory{std::istreambuf_iterator<char>(input),{}};json::Value facts;
        expect(json::parse(inventory,facts),"IV/V/VI exact world-event inventory is available");
        int scripted=0;
        for(int number:{4,5,6}){
            std::ifstream file("data/ff"+std::to_string(number)+"/maps.bin",std::ios::binary);
            std::string bytes{std::istreambuf_iterator<char>(file),{}};expect(baked::load(bytes.data(),bytes.size()),"world-event native detector loads matching real bake");
            std::ifstream labels("data/ff"+std::to_string(number)+"/eventlabels.json",std::ios::binary);
            std::string metadata{std::istreambuf_iterator<char>(labels),{}};expect(json::parse(metadata,fixture::event_metadata),"world-event native detector loads matching pruning metadata");
            fixture::selected_game=static_cast<game::Id>(number);
            const auto* game=facts.get(std::to_wstring(number));if(!game)continue;
            const auto* rows=game->get(L"rows");if(!rows)continue;
            for(const auto& row:rows->arr){
                fixture::map=int(row.get(L"map_id")->n);fixture::talk.id=int(row.get(L"entity_id")->n);
                fixture::talk.script=int(row.get(L"script")->n);fixture::talk.action=int(row.get(L"action")->n);fixture::talk.name=row.get(L"raw")->s;
                fixture::captured_object_type=int(row.get(L"type")->n);fixture::manual_object=fixture::captured_object_type==21;
                fixture::captured_class=fixture::manual_object?"FieldMapObjectDefault":"EventTriggerEntity";
                fixture::captured_go_name=text::widen(fixture::captured_class.c_str());fixture::capture_cells=true;
                fixture::pose={float(row.get(L"cell")->arr[0].n),float(row.get(L"cell")->arr[1].n),0};
                auto entity=convert(&fixture::object);
                expect(entity&&entity->entity_id==fixture::talk.id&&entity->kind!=Kind::Vehicle,"scripted world event survives actual native detector/pruning and keeps its identity");
                ++scripted;
            }
        }
        expect(scripted==221,"all221 IV/V/VI scripted world occurrences exercise the actual scanner");
        fixture::manual_object=fixture::capture_cells=false;fixture::captured_object_type=0;fixture::captured_class="FieldEntity";fixture::captured_go_name=L"Entity";
        std::printf("IV/V/VI native scanner: %d actual scripted world events\n",scripted);
    }
    std::printf("Selected destination after movement: %.3f us native-fixture dispatch, zero list-readiness ticks (offline)\n",selected_us);
    // Retire intentionally blocked fixture work before C++ static registries
    // tear down. Production workers/queues live for the entire game process.
    scanner::invalidate_map();
    std::fflush(stdout);
    return failures ? 1 : 0;
}
