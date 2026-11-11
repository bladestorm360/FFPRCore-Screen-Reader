// check-sources: bestiary/bestiary_reader.cpp field/event_labels.cpp field/npc_names.cpp field/vehicle_identity.cpp text.cpp dedup.cpp json.cpp
// Real interaction/status detours, real all-six policy and labels; fake native
// renderers and parked workers. No game, device, runtime probe or hidden stats.
#define main bestiary_fixture_main
#include "bestiary_check.cpp"
#undef main
#include "../src/core/field/interaction_cue.cpp"
#include "../src/core/battle/enemy_status.cpp"
#include <fstream>
namespace fixture {
bool worker=false,ready=true,battle=true,status_on=true,speech=true;
int wrong_owner=0,native_calls=0,map_id=0;
std::map<int,Node*> vehicle_models;int model_count_override=-1,model_lookups=0;
std::vector<std::function<void()>> jobs;
json::Value labels,names;
std::wstring locale=L"en";
void native(){if(worker)++wrong_owner;}
void integer(Node& n,const char* field,int v){*reinterpret_cast<int*>(reinterpret_cast<char*>(&n)+offset(field))=v;}
void flush(){auto queued=std::move(jobs);jobs.clear();worker=true;for(auto& job:queued)job();worker=false;for(int i=0;i<5;++i)tick();}
void no_play(void*,const void*){++native_calls;}
void no_draw(void*,bool,const void*){++native_calls;}
void no_init(void*,void*,void*,const void*){++native_calls;}
int requested_art=0;
void* loaded_sprite=nullptr;
void init_enemy(void*,void* entity,int32_t,const void* mi){
    if(loaded_sprite)ffpr::battle::enemy_status::d_set_sprite(entity,loaded_sprite,mi);
}
void from_condition(void* self,void* entity,int32_t,const void* mi){
    if(requested_art)ffpr::battle::enemy_status::d_init_enemy(self,entity,requested_art,mi);
}
void set_sprite(void* entity,void* sprite,const void*){
    set(*static_cast<Node*>(displayed::field(entity,"characterSR")),"sprite",sprite);
}
void load(int n){for(auto [kind,file]:{std::pair{&labels,"eventlabels.json"},std::pair{&names,"translation.json"}}){
    std::ifstream in("data/ff"+std::to_string(n)+"/"+file,std::ios::binary);
    const std::string raw{std::istreambuf_iterator<char>(in),{}};
    *kind=json::Value{}; // Match production's fresh per-game resource snapshot.
    if(!json::parse(raw,*kind))std::abort();
}}
}
namespace frame {
bool on_main_thread(){return !fixture::worker;}
void post(Task task){every_frame([task=std::move(task)]{task();return false;},"Post",1);}
}
namespace background {bool post(std::function<void()> job,Lane){fixture::jobs.push_back(std::move(job));return true;}}
namespace settings {bool get_bool(const char* key,bool def){return std::string(key)=="EnemyStatusAnnouncements"?fixture::status_on:std::string(key)=="SpeechEnabled"?fixture::speech:def;}}
namespace data {const json::Value& game_json(const char* kind){return std::string(kind)=="translation"?fixture::names:fixture::labels;}}
namespace modtext {const std::wstring& lang(){return fixture::locale;}}
namespace modtext {LocaleScope::LocaleScope(std::wstring){}LocaleScope::~LocaleScope(){}}
namespace displayed {
bool world_visible(void* p){fixture::native();return container_visible(p)&&static_cast<fixture::Node*>(p)->rendered;}
bool renderer_visible(void* p){fixture::native();return container_visible(p)&&static_cast<fixture::Node*>(p)->rendered;}
}
namespace fixture {int renderer_count=1;void* vehicle=nullptr;int vehicle_id=0;}
namespace unity {std::vector<void*> components_in_children(void* p,ref::Class&,bool){fixture::native();return std::vector<void*>(fixture::renderer_count,p);}void* get_component_in_children(void* p,ref::Class&,bool){fixture::native();return displayed::field(p,"shader");}}
namespace ffpr::system {bool is_field_navigation_ready(){return fixture::ready;}}
namespace field {
bool vehicle_type_of(void* p,int& type,int& id,std::wstring& message){fixture::native();
    if(p!=fixture::vehicle)return vehicle_identity_from_model(p,&fixture::vehicle_models,fixture::map_id,type,id,message);
    type=3;id=fixture::vehicle_id;message=L"Airship";return true;}
std::wstring vehicle_display_name(int,int,const std::wstring& message){fixture::native();return message;}
void* entity_property(void* p){return displayed::field(p,"property");}
std::wstring entity_name_from_property(void* p){return p?static_cast<fixture::Node*>(p)->words:L"";}
npc_names::Label npc_display_label(const std::wstring& name){return npc_names::parse(name);}
int current_map_id_or_user_data(){fixture::native();return fixture::map_id;}
namespace r {
ref::Class kPropertyEntity{"Last.Map","PropertyEntity"};
ref::Field fPropName{kPropertyEntity,"<Name>k__BackingField"};
ref::Field fPropEntityId{kPropertyEntity,"<EntityId>k__BackingField"};
ref::Field fPropObjectType{kPropertyEntity,"<ObjectType>k__BackingField"};
ref::Class kFieldTresureBox{"Last.Entity.Field","FieldTresureBox"};
ref::Class kFieldNonPlayer{"Last.Entity.Field","FieldNonPlayer"};
}
}
namespace unity {
int dict_count(void* p){fixture::native();return fixture::model_count_override>=0?fixture::model_count_override:p?int(static_cast<std::map<int,fixture::Node*>*>(p)->size()):0;}
void* dict_reference(void* p,int32_t id){fixture::native();++fixture::model_lookups;if(!p)return nullptr;
    auto& models=*static_cast<std::map<int,fixture::Node*>*>(p);auto at=models.find(id);return at==models.end()?nullptr:at->second;}
}
namespace ffpr::battle {
ref::Class kBattleEnemyData{"Last.Battle","BattleEnemyData"};
bool in_battle(){return fixture::battle;}
void* unit_info(void* unit){fixture::native();return displayed::field(unit,"info");}
std::wstring enemy_name(void* p){fixture::native();return static_cast<fixture::Node*>(p)->words;}
int condition_type(void* p){fixture::native();return *reinterpret_cast<int*>(reinterpret_cast<char*>(p)+fixture::offset("type"));}
std::wstring condition_name(void* p){fixture::native();return static_cast<fixture::Node*>(p)->words;}
}
int main(){
    using namespace fixture;namespace interaction=field::interaction_cue;namespace status=ffpr::battle::enemy_status;
    int failures=0;auto expect=[&](bool yes,const char* why){if(!yes){++failures;std::printf("FAIL %s\n",why);}};
    // The Class fake normally accepts view types. Test object identity through
    // direct native cue entry below, using the central raw-name label overload.
    interaction::o_near=interaction::o_front=interaction::o_hide=&no_play;interaction::o_draw=&no_draw;
    status::o_init=&no_init;
    status::o_from_condition=&from_condition;status::o_init_enemy=&init_enemy;status::o_set_sprite=&set_sprite;
    struct Form { int type,asset; const wchar_t* label; };
    for(int n=1;n<=6;++n){
        fixture::game=static_cast<game::Id>(n);load(n);
        const Form forms[]{
            {401,n==4?26:n==5?296:0,L"Mini"},
            {402,n==2?129:n==4?131:n==5?297:0,L"Toad"},
            {403,n==4?237:0,L"Pig"},
            {601,n==6?352:0,L"Imp"},
            {10,0,L"Poison"}, {7,0,L"Sleep"}, {8,0,L"Paralysis"}, {4,0,L"Critical"},
        };
        for(const auto& form:forms) {
            status::clear();spoken.clear();jobs.clear();
            Node unit,info,sprite,condition,mesh,body,original,changed,wrong;
            unit.kind="Enemy";unit.words=L"Enemy";condition.words=form.label;
            integer(condition,"type",form.type);
            set(unit,"info",&info);set(info,"<BattleSpriteEntity>k__BackingField",&sprite);
            set(sprite,"characterSR",&mesh);set(mesh,"meshRenderer",&body);set(mesh,"sprite",&original);
            status::d_init(&condition,&unit,&condition,nullptr);
            const auto transform=[&](Node& replacement) {
                loaded_sprite=&replacement;
                status::d_from_condition(nullptr,&sprite,form.type,nullptr);
                loaded_sprite=nullptr;
            };
            requested_art=form.asset?form.asset:999;
            body.rendered=false;transform(changed);tick();flush();
            expect(spoken.empty(),"unseen replacement cannot announce");
            body.rendered=true;tick();
            expect((!jobs.empty())==(form.asset>0),"only a qualified visible enemy form queues owned speech");
            flush();
            expect(form.asset?spoken==std::vector<std::wstring>{std::wstring(L"Enemy: ")+form.label}:spoken.empty(),
                   "the installed condition-to-art policy announces the exact visible form only");
            spoken.clear();
            transform(changed);tick();flush();
            expect(spoken.empty(),"an unchanged sprite cannot announce a form");
            if(!form.asset)continue;
            requested_art=form.asset+1;
            transform(wrong);tick();flush();
            expect(spoken.empty(),"a mismatched asset cannot announce a form");
            loaded_sprite=&original;
            status::d_init_enemy(nullptr,&sprite,form.asset,nullptr);
            loaded_sprite=nullptr;tick();flush();
            expect(spoken.empty(),"ordinary enemy or boss art load cannot announce a form");
            requested_art=form.asset;
            status::d_from_condition(nullptr,&sprite,form.type,nullptr);
            status::d_set_sprite(&sprite,&wrong,nullptr);tick();flush();
            expect(spoken.empty(),"a sprite change after the native load returned cannot announce a form");
            status_on=false;
            transform(changed);tick();flush();
            expect(spoken.empty(),"the existing setting controls form announcements");status_on=true;
            transform(original);tick();
            expect(!jobs.empty(),"visible form is awaiting a worker with owned strings");
            status::d_init_enemy(nullptr,&sprite,1,nullptr);flush();
            expect(spoken.empty(),"ordinary sprite restoration cancels parked speech");
            transform(changed);tick();status::retire();flush();
            expect(spoken.empty(),"battle transition cancels a parked form announcement");
        }
        status::clear();
    }
    expect(wrong_owner==0,"status worker never reads a native object");
    // All-six marker hooks share the same lifecycle; exact FF2 Gate label is
    // tested against the real generated catalog rather than a hardcoded result.
    load(2);fixture::game=game::Id::FF2;spoken.clear();interaction::clear();
    Node icon,object,property,renderer;std::wstring raw=L"鉄格子";
    set(icon,"OwnerInteractiveEntity",&object);set(icon,"spriteRenderer",&renderer);integer(icon,"interactiveIconType",5);integer(icon,"rendererEnable",1);
    set(object,"property",&property);set(property,"<Name>k__BackingField",&raw);
    // Fake class tests are specialized below via named fixture kinds.
    object.kind="Object";icon.kind="Icon";
    interaction::observed(&icon);interaction::observed(&icon);
    expect(jobs.size()==1&&spoken.empty(),"unchanged marker queues one owned cue while worker remains parked");
    interaction::hide(&icon);flush();expect(spoken.empty(),"leaving interaction range cancels parked cue");
    interaction::observed(&icon);flush();expect(spoken.size()==1&&spoken[0].find(L"Iron gate")!=std::wstring::npos,"reappearing marker uses real Iron gate label");
    interaction::observed(&icon);flush();expect(spoken.size()==1,"Near to Front with same visible occurrence does not reannounce");
    interaction::hide(&icon);spoken.clear();object.kind="NPC";object.words=L"ev_e_0013: Jun";
    interaction::observed(&icon);flush();
    expect(spoken==std::vector<std::wstring>({L"Interact: Jun"}),"native NPC interaction cue omits developer script prefix and ordinal");
    for(int n=1;n<=6;++n) {
        fixture::game=static_cast<game::Id>(n);interaction::hide(&icon);spoken.clear();
        object.kind="NPC";object.words=L"NPC";fixture::vehicle=&object;fixture::vehicle_id=n+10;
        interaction::observed(&icon);flush();
        expect(spoken==std::vector<std::wstring>({L"Interact: Airship"}),"vehicle model labels precede generic NPC inheritance in every game's interaction cue");
    }
    fixture::vehicle=nullptr;fixture::game=game::Id::FF2;
    // A freshly recreated world-map vehicle has native identity before the
    // asynchronous discovery cache is populated. Ordinary NPCs can reuse the
    // same ObjectId, so verify the exact actor/map/enabled model relationship.
    Node model,other_actor;std::wstring native_name=L"The Nautilus";
    for(int n=1;n<=6;++n){
        fixture::game=static_cast<game::Id>(n);fixture::map_id=n==3?4:1;
        const int ids[]={3,3,9,3,7,4};const int id=ids[n-1];
        integer(property,"<ObjectId>k__BackingField",id);
        integer(model,"id",id);integer(model,"type",n==6?8:3);
        integer(model,"<StayMapId>k__BackingField",fixture::map_id);integer(model,"<Enable>k__BackingField",1);
        set(model,"<MapObject>k__BackingField",&object);set(model,"messageId",&native_name);
        fixture::vehicle_models={{id,&model}};object.kind="NPC";object.words=L"NPC";
        interaction::hide(&icon);spoken.clear();const int before=fixture::model_lookups;
        interaction::observed(&icon);flush();
        expect(spoken==std::vector<std::wstring>{L"Interact: The Nautilus"}&&fixture::model_lookups==before+1,
            "all-six cold discovery prompts resolve the exact native vehicle with one keyed model lookup");
        for(int invalid=0;invalid<5;++invalid){
            set(model,"<MapObject>k__BackingField",invalid==0?&other_actor:&object);
            integer(model,"<StayMapId>k__BackingField",fixture::map_id+(invalid==1));
            integer(model,"<Enable>k__BackingField",invalid==2?0:1);
            integer(model,"id",invalid==3?id+1:id);fixture::model_count_override=invalid==4?65:-1;
            interaction::hide(&icon);spoken.clear();interaction::observed(&icon);flush();
            expect(spoken==std::vector<std::wstring>{L"Interact: NPC"},"reused ID, other map, disabled/mismatched or oversized native models cannot name an ordinary NPC as a vehicle");
        }
        fixture::model_count_override=-1;fixture::vehicle_models.clear();
    }
    fixture::game=game::Id::FF2;fixture::map_id=0;
    interaction::hide(&icon);object.kind="Object";spoken.clear();
    interaction::observed(&icon);flush();
    raw=L"Unmapped object";integer(property,"<AssetId>k__BackingField",72);
    expect(field::safe_event_label(&object,L"Interactive Object")==L"Iron gate","verified live sprite ID names an unknown developer identifier");
    renderer.visible=false;interaction::hide(&icon);interaction::observed(&icon);flush();expect(spoken.size()==1,"hidden marker cannot announce");
    load(2);
    {
        std::ifstream review_file("tools/event_warp_reviews.json",std::ios::binary);
        const std::string review_bytes{std::istreambuf_iterator<char>(review_file),{}};json::Value reviews;
        expect(json::parse(review_bytes,reviews)&&reviews.arr.size()==16,"exact reviewed travel corpus loads");
        integer(property,"<AssetId>k__BackingField",0);integer(property,"<ObjectType>k__BackingField",2);
        for(const auto& row:reviews.arr) {
            raw=row.get(L"raw")->s;map_id=int(row.get(L"map_id")->n);
            const int id=int(row.get(L"entity_id")->n);integer(property,"<EntityId>k__BackingField",id);
            expect(field::safe_event_label(&object,L"Event")==L"Warp Tile","live native Event label identifies inspected warp pad");
            worker=true;expect(field::safe_event_label(raw,L"Event",map_id,id,2)==L"Warp Tile","copied other-floor label uses the same inspected warp identity without native calls");worker=false;
        }
        map_id=0;
        expect(field::safe_event_label(L"Unreviewed scene",L"Event")==L"Event trigger","unknown scene is not inferred to be a warp from routing alone");
    }
    expect(field::safe_event_label(L"隠し通路",L"Interactive Object",21,36,5)==L"Wall near bed" &&
           field::safe_event_label(L"隠し通路",L"Interactive Object",43,22,5)==L"Wall",
           "Paul's shared hidden-passage identifier names each actual concealing wall");
    expect(field::safe_event_label(L"sc_e_0025:隠し通路の蓋",L"Interactive Object",118,14,5)==L"Rock",
           "Josef's separate rock switch retains its physical identity");
    load(6);
    expect(field::safe_event_label(L"sc_e_0089:隠し通路入口",L"Interactive Object",3,132,5)==L"Rock",
           "Narshe's passage entrance names the stone actually turned in its script");
    expect(field::safe_event_label(L"上の建物で時計を6時10分50秒に合わせると隠し扉",L"Interactive Object")==L"Clock",
           "a clock concealing a passage stays Clock without exposing its solution");
    load(1);
    expect(field::safe_event_label(L"GeneralEventObject",L"Interactive Object",22,55,5)==L"Gravestone",
           "an unnamed FF1 gravestone uses its reviewed native context");
    expect(field::safe_event_label(L"GeneralEventObject",L"Interactive Object",110,19,5)==L"Computer",
           "the same generic identifier can resolve to a different physical object");
    load(4);
    expect(field::safe_event_label(L"MP回復",L"Interactive Object",41,3,5)==L"Pot" &&
           field::safe_event_label(L"MP回復",L"Event",41,3,2)==L"Pot" &&
           field::safe_event_label(L"MP回復",L"Event",153,6,2)==L"Fountain",
           "healing-pot helper is functional and retains its identity independently of the fountain's script context");
    load(5);
    expect(field::safe_event_label(L"sc_e_0362",L"Interactive Object",40,151,5)==L"Barrel" &&
           field::safe_event_label(L"sc_e_0362",L"Interactive Object",78,5,5)==L"Pot",
           "shared item-message script cannot turn barrels and pots into chests");
    locale=L"fr";worker=true;
    expect(field::safe_event_label(L"sc_e_0030:セイレーンと戦闘",L"Interactive Object")==L"Sirène",
           "named actors use the actual game locale on an owned data worker");
    worker=false;locale=L"ja";
    expect(field::safe_event_label(L"sc_e_0030:セイレーンと戦闘",L"Interactive Object")==L"セイレーン",
           "Japanese uses the reviewed proper name without a script outcome");
    locale=L"en";expect(wrong_owner==0,"identity formatting on an owned worker performs no native reads");
    std::printf("Native interaction/enemy presentation cues: %d failures\n",failures);return failures?1:0;
}
