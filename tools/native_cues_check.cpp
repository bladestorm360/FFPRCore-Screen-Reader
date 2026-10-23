// check-sources: bestiary/bestiary_reader.cpp field/event_labels.cpp field/npc_names.cpp text.cpp dedup.cpp json.cpp
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
int wrong_owner=0,native_calls=0;
std::vector<std::function<void()>> jobs;
json::Value visuals,labels,names;
std::wstring locale=L"en";
void native(){if(worker)++wrong_owner;}
void integer(Node& n,const char* field,int v){*reinterpret_cast<int*>(reinterpret_cast<char*>(&n)+offset(field))=v;}
void flush(){auto queued=std::move(jobs);jobs.clear();worker=true;for(auto& job:queued)job();worker=false;for(int i=0;i<5;++i)tick();}
void no_play(void*,const void*){++native_calls;}
void no_draw(void*,bool,const void*){++native_calls;}
void no_init(void*,void*,void*,const void*){++native_calls;}
void effect_play(void* self,void* effect,int32_t,int32_t type,bool,const void*){
    ++native_calls;if(!effect)return;
    auto* list=static_cast<List*>(displayed::field(self,"conditionEffectList"));
    for(void* p:*list)if(ref::rd<int32_t>(p,ffpr::battle::enemy_status::fType)==type)return;
    integer(*static_cast<Node*>(effect),"conditionType",type);list->push_back(effect);
}
void effect_play3(void* self,void* effect,int32_t asset,int32_t type,const void* mi){effect_play(self,effect,asset,type,true,mi);}
void init_enemy(void*,void*,int32_t,const void*){}
void from_condition(void* self,void* entity,int32_t type,const void* mi){ffpr::battle::enemy_status::d_init_enemy(self,entity,type,mi);}
void set_sprite(void* entity,void* sprite,const void*){set(*static_cast<Node*>(displayed::field(entity,"characterSR")),"sprite",sprite);}
void skin_start(void* self,const void*){integer(*static_cast<Node*>(self),"changedSkinColor",1);}
void skin_remove(void* self,const void*){integer(*static_cast<Node*>(self),"changedSkinColor",0);}
void status_play(void* self,void* effect,int32_t asset,int32_t type,bool enemy,const void* mi){
    if(fixture::game==game::Id::FF3)ffpr::battle::enemy_status::d_play3(self,effect,asset,type,mi);
    else ffpr::battle::enemy_status::d_play(self,effect,asset,type,
        int(fixture::game)>=4?false:enemy,mi); // FF4–6 isCopyEffect, not isEnemy
}
void load(int n){for(auto [kind,file]:{std::pair{&visuals,"condition_visuals.json"},std::pair{&labels,"eventlabels.json"},std::pair{&names,"translation.json"}}){
    std::ifstream in("data/ff"+std::to_string(n)+"/"+file,std::ios::binary);
    const std::string raw{std::istreambuf_iterator<char>(in),{}};
    if(!json::parse(raw,*kind))std::abort();
}}
}
namespace frame {
bool on_main_thread(){return !fixture::worker;}
void post(Task task){every_frame([task=std::move(task)]{task();return false;},"Post",1);}
}
namespace background {bool post(std::function<void()> job,Lane){fixture::jobs.push_back(std::move(job));return true;}}
namespace settings {bool get_bool(const char* key,bool def){return std::string(key)=="EnemyStatusAnnouncements"?fixture::status_on:std::string(key)=="SpeechEnabled"?fixture::speech:def;}}
namespace data {const json::Value& game_json(const char* kind){return std::string(kind)=="conditionvisuals"?fixture::visuals:std::string(kind)=="translation"?fixture::names:fixture::labels;}}
namespace modtext {const std::wstring& lang(){return fixture::locale;}}
namespace modtext {LocaleScope::LocaleScope(std::wstring){}LocaleScope::~LocaleScope(){}}
namespace displayed {
bool world_visible(void* p){fixture::native();return container_visible(p)&&static_cast<fixture::Node*>(p)->rendered;}
bool renderer_visible(void* p){fixture::native();return container_visible(p)&&static_cast<fixture::Node*>(p)->rendered;}
}
namespace fixture {int renderer_count=1;}
namespace unity {std::vector<void*> components_in_children(void* p,ref::Class&,bool){fixture::native();return std::vector<void*>(fixture::renderer_count,p);}void* get_component_in_children(void* p,ref::Class&,bool){fixture::native();return displayed::field(p,"shader");}}
namespace ffpr::system {bool is_field_navigation_ready(){return fixture::ready;}}
namespace field {
void* entity_property(void* p){return displayed::field(p,"property");}
std::wstring entity_name_from_property(void* p){return p?static_cast<fixture::Node*>(p)->words:L"";}
npc_names::Label npc_display_label(const std::wstring& name){return npc_names::parse(name);}
int current_map_id_or_user_data(){fixture::native();return 0;}
namespace r {
ref::Class kPropertyEntity{"Last.Map","PropertyEntity"};
ref::Field fPropName{kPropertyEntity,"<Name>k__BackingField"};
ref::Field fPropEntityId{kPropertyEntity,"<EntityId>k__BackingField"};
ref::Field fPropObjectType{kPropertyEntity,"<ObjectType>k__BackingField"};
ref::Class kFieldTresureBox{"Last.Entity.Field","FieldTresureBox"};
ref::Class kFieldNonPlayer{"Last.Entity.Field","FieldNonPlayer"};
}
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
    status::o_init=&no_init;status::o_play=&effect_play;status::o_play3=&effect_play3;
    status::o_from_condition=&from_condition;status::o_init_enemy=&init_enemy;status::o_set_sprite=&set_sprite;
    status::o_skin_start=&skin_start;status::o_skin_remove=&skin_remove;
    for(int n=1;n<=6;++n){
        fixture::game=static_cast<game::Id>(n);load(n);status::clear();spoken.clear();jobs.clear();
        Node unit,info,sprite,controller,condition,effect;List list;
        unit.kind="Enemy";unit.words=L"Enemy";condition.words=L"Poison";integer(condition,"type",10);
        set(unit,"info",&info);set(info,"<BattleSpriteEntity>k__BackingField",&sprite);
        set(sprite,"ConditionEffectEntityController",&controller);set(controller,"conditionEffectList",&list);
        int poison=0,shared=0,shared_type=0;
        for(const auto& [id,value]:visuals.obj){auto* types=value.get(L"types");if(types&&types->arr.size()==1&&types->arr[0].n==10)poison=std::stoi(id);if(types&&types->arr.size()>1){shared=std::stoi(id);shared_type=int(types->arr[0].n);}}
        expect(poison>0&&shared>0,"real all-six policy supplies unique poison and ambiguous effect cases");
        status::d_init(&condition,&unit,&condition,nullptr);
        renderer_count=n==5?36:1; // actual maximum serialized FF5 status prefab size
        effect.rendered=false;status_play(&controller,&effect,poison,10,true,nullptr);renderer_count=1;for(int i=0;i<5;++i)tick();flush();
        expect(spoken.empty(),"hidden effect cannot disclose an enemy condition");
        effect.rendered=true;tick();expect(!jobs.empty()&&spoken.empty(),"visible native effect submits owned text without waiting for parked worker");
        flush();expect(spoken.size()==1&&spoken[0]==L"Enemy: Poison","unique visible effect identifies actual condition across all6 policies");
        status_play(&controller,&effect,poison,10,true,nullptr);tick();flush();expect(spoken.size()==1,"native repeated Play guard cannot duplicate a status announcement");
        Node ambiguous;integer(condition,"type",shared_type);condition.words=L"Hidden exact condition";status::d_init(&condition,&unit,&condition,nullptr);
        status_play(&controller,&ambiguous,shared,shared_type,true,nullptr);tick();flush();
        expect(spoken.back()==L"Enemy: Status effect","shared visual asset cannot reveal hidden exact condition identity");
        spoken.clear();integer(condition,"type",4);condition.words=L"Critical";status::d_init(&condition,&unit,&condition,nullptr);tick();flush();
        expect(spoken.empty(),"Critical data without a native visible effect never announces");
        status_on=false;list.clear();Node disabled;status_play(&controller,&disabled,poison,10,true,nullptr);tick();flush();expect(spoken.empty(),"optional status toggle off suppresses automatic effect speech");status_on=true;
        Node mesh,body,original_sprite,frog_sprite,skin;
        set(sprite,"characterSR",&mesh);set(mesh,"meshRenderer",&body);set(mesh,"sprite",&original_sprite);
        condition.words=L"Frog";integer(condition,"type",402);status::d_init(&condition,&unit,&condition,nullptr);
        body.rendered=false;status::d_from_condition(nullptr,&sprite,402,nullptr);status::d_set_sprite(&sprite,&frog_sprite,nullptr);tick();flush();
        expect(spoken.empty(),"native transformation intent without a rendered replacement stays silent");
        body.rendered=true;tick();flush();expect(spoken.size()==1&&spoken.back()==L"Enemy: Frog","actual enemy sprite replacement supports zero-effect-asset transformations in every adapter");spoken.clear();
        status::d_from_condition(nullptr,&sprite,402,nullptr);status::d_set_sprite(&sprite,&frog_sprite,nullptr);tick();flush();expect(spoken.empty(),"unchanged native sprite is not another transformation");
        status::d_set_sprite(&sprite,&original_sprite,nullptr);tick();status::d_init_enemy(nullptr,&sprite,1,nullptr);flush();expect(spoken.empty(),"native base-sprite restoration cancels a parked transformation announcement");
        set(skin,"<BattleUnitData>k__BackingField",&unit);set(skin,"effectRender",&body);
        status::d_skin_start(&skin,nullptr);tick();flush();expect(spoken.size()==1&&spoken.back()==L"Enemy: Status effect","actual native material change with visible enemy renderer has conservative speech");spoken.clear();
        status::d_skin_remove(&skin,nullptr);status::d_skin_start(&skin,nullptr);tick();status::d_skin_remove(&skin,nullptr);flush();expect(spoken.empty(),"native condition removal cancels material speech without retaining a managed function pointer");
        list.clear();Node cancelled;status_play(&controller,&cancelled,poison,10,true,nullptr);tick();status::retire();flush();expect(spoken.empty(),"battle transition cancels a parked status formatter");status::clear();
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
    interaction::hide(&icon);object.kind="Object";spoken.clear();
    interaction::observed(&icon);flush();
    raw=L"Unmapped object";integer(property,"<AssetId>k__BackingField",72);
    expect(field::safe_event_label(&object,L"Interactive Object")==L"Iron gate","verified live sprite ID names an unknown developer identifier");
    renderer.visible=false;interaction::hide(&icon);interaction::observed(&icon);flush();expect(spoken.size()==1,"hidden marker cannot announce");
    load(1);
    expect(field::safe_event_label(L"GeneralEventObject",L"Interactive Object",22,55,5)==L"Gravestone",
           "an unnamed FF1 gravestone uses its reviewed native context");
    expect(field::safe_event_label(L"GeneralEventObject",L"Interactive Object",110,19,5)==L"Computer",
           "the same generic identifier can resolve to a different physical object");
    load(4);
    expect(field::safe_event_label(L"MP回復",L"Interactive Object",41,3,5)==L"Pot" &&
           field::safe_event_label(L"MP回復",L"Event",41,3,2)==L"Scene trigger",
           "a healing pot and its scripted helper retain distinct native object types");
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
