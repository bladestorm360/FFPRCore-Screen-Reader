// check-sources: text.cpp ui/navigation_buffer.cpp
// Actual renderer, battle party, Esper readers and review worker; fake native owner only.
#include <map>
#include <array>
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include "../src/core/frame.cpp"
#include "../src/core/background.cpp"
#include "../src/core/displayed_ui.cpp"
#include "../src/core/battle/battle_party.cpp"
#include "../src/core/battle/party_status.cpp"
#include "../src/core/ui/character_row.cpp"
#include "../src/core/ui/review_buffer.cpp"
#include "../src/games/ff6/ability_espers.cpp"
namespace fixture {
struct Node {std::array<void*,128> fields{};int command=0,kind=0,id=0;bool alive=true,active=true,enabled=true,owned=true,focused=true;float alpha=1,r=1,g=1,b=1;Node* parent=nullptr;void* sprite=nullptr;std::wstring value;};
using List=std::vector<void*>;
std::map<std::string,int> offsets;std::vector<std::wstring> spoken;std::map<std::string,void*> owners;
std::map<std::string,void*> extra_methods;
std::map<std::string,std::wstring> guards;
std::map<std::string,std::function<std::wstring()>> text_queries;
std::map<std::string,actions::Fn> party_actions;
std::map<events::Kind,std::vector<events::Handler>> status_events;
std::map<input::Key,std::string> party_keys;
std::set<std::string> active_menus;
DWORD owner=GetCurrentThreadId();int wrong=0;uint64_t native_calls=0;bool autodetail=false;game::Id game=game::Id::FF6;
bool main_menu=true,palette_ready=true;
int red_palette=static_cast<int>(0xff4040ffu);
int(*party_status_install)()=nullptr;
using FormHook=void(*)(void*,void*,int32_t,int32_t,int32_t,const void*);
FormHook form_hook=nullptr;
using ConditionTextHook=void(*)(void*,void*,const void*);
ConditionTextHook condition_text_hook=nullptr;
void* next_motions=nullptr;
void native(){++native_calls;if(GetCurrentThreadId()!=owner)++wrong;}
int offset(const char*n){auto [i,inserted]=offsets.emplace(n,static_cast<int>(offsets.size()));return static_cast<int>(offsetof(Node,fields))+i->second*static_cast<int>(sizeof(void*));}
void set(Node&n,const char*k,void*v){*reinterpret_cast<void**>(reinterpret_cast<char*>(&n)+offset(k))=v;}
void* get(Node&n,const char*k){return *reinterpret_cast<void**>(reinterpret_cast<char*>(&n)+offset(k));}
bool enabled(void*p,const void*){native();return static_cast<Node*>(p)->enabled;}
displayed::Color color(void*p,const void*){native();auto& n=*static_cast<Node*>(p);return {n.r,n.g,n.b,n.alpha};}
void* renderer(void*,const void*){native();return nullptr;}
void* sprite(void*p,const void*){native();return static_cast<Node*>(p)->sprite;}
void* content(void*p,int index,const void*){native();auto*l=static_cast<List*>(get(*static_cast<Node*>(p),"rows"));return l&&index>=0&&index<static_cast<int>(l->size())?(*l)[index]:nullptr;}
void phase(void*,const void*){}void update(void*,void*,bool,const void*){}void focus(void*,void*,bool,const void*){}
std::function<void()> on_details_show;
void show(void*p,void*,void*,void*,const void*){static_cast<Node*>(p)->active=true;if(on_details_show)on_details_show();}void details_update(void*,void*,void*,const void*){}void list_select(void*,int,int,const void*){}
void tick(){const auto until=GetTickCount64()+5000;do{frame::run_tick();std::this_thread::yield();}while(ui::review::g_build_pending&&GetTickCount64()<until);}
Node make_text(const wchar_t*v){Node n;n.kind=1;n.value=v;return n;}
void native_form(void*,void* entity,int32_t,int32_t,int32_t,const void*){
 native();auto* animation=static_cast<Node*>(get(*static_cast<Node*>(entity),"spriteAnimBattle"));
 if(animation)set(*animation,"<battleCharaSpriteList>k__BackingField",next_motions);
}
void native_condition_text(void* self,void* condition,const void*){
 native();auto* view=static_cast<Node*>(get(*static_cast<Node*>(self),"view"));
 auto* text=static_cast<Node*>(get(*view,"conditionText"));text->value=static_cast<Node*>(condition)->value;
}
Node sprite_node;
Node image(){Node n;n.kind=2;n.sprite=&sprite_node;return n;}
bool contains(std::wstring_view s){for(auto&v:spoken)if(v.find(s)!=v.npos)return true;return false;}
}
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){if(std::string(n)=="Battle.PartyStatus")fixture::party_status_install=f;}}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
il2cpp::Class* Class::get(){fixture::native();return reinterpret_cast<il2cpp::Class*>(this);}
bool Class::is(void*p){fixture::native();if(!p)return false;auto&n=*static_cast<fixture::Node*>(p);std::string k=name;if(k=="BattlePlayerEntity")return n.kind==4;if(k=="ChangeSkinColorConditionFunctionBase")return n.kind==5;if(k=="ConditionEffectEntity")return n.kind==6;if(k=="Renderer"||k=="SpriteRenderer")return n.kind==8;if(k=="StatusWindowContentViewBase")return n.kind==3;if(k=="Image")return n.kind==2;if(k=="Graphic")return n.kind==1||n.kind==2;if(k=="Behaviour")return true;return true;}
void*Method::ptr(){fixture::native();std::string n=name;if(auto it=fixture::extra_methods.find(n);it!=fixture::extra_methods.end())return it->second;if(n=="get_isVisible"||n=="get_isPlaying")return reinterpret_cast<void*>(&fixture::enabled);if(n=="get_enabled")return reinterpret_cast<void*>(&fixture::enabled);if(n=="get_color")return reinterpret_cast<void*>(&fixture::color);if(n=="get_canvasRenderer")return reinterpret_cast<void*>(&fixture::renderer);if(n=="get_overrideSprite")return reinterpret_cast<void*>(&fixture::sprite);if(n=="GetContentObject")return reinterpret_cast<void*>(&fixture::content);return nullptr;}
il2cpp::Method*Method::get(){return reinterpret_cast<il2cpp::Method*>(ptr());}
int32_t Field::get(){fixture::native();if(std::string(cls.name)=="StatusWindowContentViewBase"&&std::string(cls.ns)=="Serial.Template.UI.KeyInput")return fixture::offset(std::string(name)=="defaultCursorParent"?"nativeDefaultParent":"nativeFocusParent");if(std::string(name)=="<IsFocus>k__BackingField")return offsetof(fixture::Node,focused);if(std::string(name)=="callerCommandId")return offsetof(fixture::Node,command);if(std::string(name)=="<ContentId>k__BackingField")return offsetof(fixture::Node,id);if(std::string(name)=="<IsOwned>k__BackingField")return offsetof(fixture::Node,owned);return fixture::offset(name);}
void call_failed_note(const char*){std::abort();}
}
namespace il2cpp {bool read_static_int(Class*,const char* name,int& value){fixture::native();value=std::string(name)=="Gray"?static_cast<int>(0xff999999u):fixture::red_palette;return fixture::palette_ready&&(std::string(name)=="Red"||std::string(name)=="Gray");}Class*object_class(Object*){fixture::native();return reinterpret_cast<Class*>(1);}int32_t field_offset(Class*,const char*n){fixture::native();return fixture::offset(n);}void*resolve_icall(const char*){return nullptr;}}
namespace unity {
bool alive(void*p){fixture::native();return p&&static_cast<fixture::Node*>(p)->alive;}bool active_in_hierarchy(void*p){return alive(p)&&static_cast<fixture::Node*>(p)->active&&(!static_cast<fixture::Node*>(p)->parent||active_in_hierarchy(static_cast<fixture::Node*>(p)->parent));}
void*transform(void*p){fixture::native();return p;}void*parent(void*p){fixture::native();return p?static_cast<fixture::Node*>(p)->parent:nullptr;}void*get_component(void*p,ref::Class& k){fixture::native();if(!p)return nullptr;if(std::string(k.name)=="EffectReactionInspector")return fixture::get(*static_cast<fixture::Node*>(p),"inspector");if(std::string(k.name)=="Renderer")return static_cast<fixture::Node*>(p)->kind==8?p:fixture::get(*static_cast<fixture::Node*>(p),"renderer");return p;}void*get_component_in_parent(void*p,ref::Class&){return parent(p);}
std::wstring ui_text(void*p){fixture::native();return static_cast<fixture::Node*>(p)->value;}
std::vector<void*>texts(void*,bool){return {};}std::vector<void*>components_in_children(void*,ref::Class&,bool){return {};}
void*get_component_in_children(void*p,ref::Class&,bool){fixture::native();return p?fixture::get(*static_cast<fixture::Node*>(p),"child_renderer"):nullptr;}
int list_count(void*p){fixture::native();return p?static_cast<int>(static_cast<fixture::List*>(p)->size()):0;}void*list_at(void*p,int i){fixture::native();return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}
}
namespace unity {int array_length(void*p){return list_count(p);}void*array_at(void*p,int i){return list_at(p,i);}}
namespace unity {bool enumerate_bounded(void*p,std::vector<void*>&out,int bound){fixture::native();out.clear();if(!p||list_count(p)>bound)return false;out=*static_cast<fixture::List*>(p);return true;}}
namespace displayed {std::wstring conditions(void*p){return p&&visible(p)?static_cast<fixture::Node*>(p)->value:std::wstring();}}
namespace hooks {bool attach_raw(ref::Method& method,void* hook,void** original){
 if(std::string(method.cls.name)=="InstantiateManager"&&std::string(method.name)=="ChangePlayerMotions"){
  fixture::form_hook=reinterpret_cast<fixture::FormHook>(hook);*original=reinterpret_cast<void*>(&fixture::native_form);return true;
 }
 if(std::string(method.cls.ns)=="Last.UI"&&std::string(method.cls.name)=="BattleConditionController"&&std::string(method.name)=="SetConditionText"){
  fixture::condition_text_hook=reinterpret_cast<fixture::ConditionTextHook>(hook);*original=reinterpret_cast<void*>(&fixture::native_condition_text);return true;
 }
 return false;
}void note_fault(const char*){std::abort();}}
namespace game {Id current(){return fixture::game;}}
namespace field::cache {void*get(ref::Class&c){return fixture::owners[c.name];}void register_object(ref::Class&c,void*p){fixture::owners[c.name]=p;}}
namespace ffpr::battle {ref::Class kBattlePlayerData{"Last.Battle","BattlePlayerData"};bool in_battle(){return true;}void*player_owned_character(void*p){return displayed::field(p,"owned");}int character_id(void*p){return p?static_cast<fixture::Node*>(p)->id:0;}void*current_actor(){return nullptr;}std::wstring unit_name(void*){return {};}
void* unit_info(void*p){return displayed::field(p,"info");}
void* info_functions(void*p){return displayed::field(p,"functions");}
void* function_condition(void*p){return displayed::field(p,"condition");}
int condition_id(void*p){return p?static_cast<fixture::Node*>(p)->id:0;}
int condition_type(void*p){return p?static_cast<fixture::Node*>(p)->command:0;}
std::wstring condition_name(void*p){return p?static_cast<fixture::Node*>(p)->value:std::wstring();}
}
namespace speech {void request_load(){}}
namespace sound {void initialize(){}}
namespace ffpr::ff6 {bool auto_detail(){return fixture::autodetail;}void set_ability_focus(void*,const std::wstring&,const std::wstring&){} }
namespace ffpr {void say(std::wstring_view v,bool){fixture::spoken.emplace_back(v);}void say_parts(std::vector<std::wstring>v,bool){fixture::spoken.push_back(text::join(v,L". "));}void say_sequence(std::vector<std::wstring>v,bool b){say_parts(std::move(v),b);}}
namespace modtext {std::wstring T(std::wstring_view v){return std::wstring(v);}std::wstring TF(std::wstring_view v,std::initializer_list<std::wstring>a){return text::format(v,a);}}
namespace ui {int state_tag(void*p,ref::Field&){fixture::native();return p?static_cast<fixture::Node*>(p)->command:-1;}std::wstring menu_position(const std::wstring&s,int,int){return s;}int cursor_index(void*p){return p?static_cast<fixture::Node*>(p)->id:-1;}}
namespace actions {void define(const char*name,Fn fn){fixture::party_actions[name]=std::move(fn);}bool run(const char*){return false;}}
namespace queries {void define(const char*,std::function<bool()>){} }
namespace queries {void define_text(const char* name,std::function<std::wstring()> fn){fixture::text_queries[name]=std::move(fn);}}
namespace ffpr::system {bool is_main_menu_state(){return fixture::main_menu;}}
namespace events {void on(Kind kind,Handler fn){fixture::status_events[kind].push_back(std::move(fn));} }
namespace menu_state {bool is_active(const char* name){return fixture::active_menus.contains(name);}}
namespace input {void add_binding(Key key,Mod,Context context,const char*action,const char*){if(context==Context::Battle)fixture::party_keys[key]=action;}}
namespace settings {bool get_bool(const char*,bool){return fixture::autodetail;}int get_int(const char*,int d){return d;}}
namespace dedup {bool text(const char*k,std::wstring_view v,Scope){if(fixture::guards[k]==v)return false;fixture::guards[k]=v;return true;}void reset(const char*k){fixture::guards.erase(k);}}
namespace log_ {void write(const char*,...){} }
int main(){using namespace fixture;background::prepare();int failed=0;auto expect=[&](bool ok,const char*s){if(!ok){++failed;std::printf("FAIL %s\n",s);}};
 // Actual character formatter/detail consumer, with the eight current-only MP
 // cells shown in FF1/III. Native cursor ownership retires I without comparing
 // speech strings or borrowing a previous character's values.
 {
  Node r,v,cursor,anchor,other,window,cursor_image=image();cursor.parent=&anchor;set(r,"view",&v);set(v,"cursorParent",&other);set(v,"frontParent",&other);set(v,"backParent",&other);v.kind=3;set(v,"nativeDefaultParent",&other);set(v,"nativeFocusParent",&anchor);set(cursor,"image",&cursor_image);
  window.command=2;cursor.focused=false;set(window,"selectCursor",&cursor);
  Node n=make_text(L"Corrad"),h=make_text(L"36"),mx=make_text(L"47"),scalar=make_text(L"999"),job=make_text(L"Warrior"),
       next_label=make_text(L"Next Level in"),next=make_text(L"54"),lv_label=make_text(L"Lv."),lv=make_text(L"3"),
       jl_label=make_text(L"Job Level"),jl=make_text(L"1");
  set(v,"nameText",&n);set(v,"currentHpText",&h);set(v,"maxHpText",&mx);set(v,"currentMpText",&scalar);
  set(v,"jobNameText",&job);set(v,"fixedNextText",&next_label);set(v,"nextExpText",&next);
  set(v,"fixedLevelText",&lv_label);set(v,"currentLevelText",&lv);set(v,"fixedJobLevel",&jl_label);set(v,"jobLevelText",&jl);
  std::array<Node,8> counts;List charges;
  for(auto& cell:counts){cell=make_text(L"0");charges.push_back(&cell);}set(v,"currentLvMpTextList",&charges);
  for(auto id:{game::Id::FF1,game::Id::FF3}){
   fixture::game=id;autodetail=false;spoken.clear();
   auto brief=displayed::character_overview(&r,&window);
   expect(brief==L"Corrad, HP 36/47, Warrior, Next Level in 54, Lv. 3, Job Level 1","short overview puts job/TNL after vitals and excludes all MP charges/scalar");
   ui::character_row::announce(&r,&cursor,brief,&window);
   expect(spoken==std::vector<std::wstring>{brief},"Autodetail off produces one short row");
   auto detail=ui::character_row::details();
   expect(detail.starts_with(L"Level 1 MP 0")&&detail.ends_with(L"Level 8 MP 0")&&detail.find(L"Corrad")==detail.npos&&detail.find(L"HP")==detail.npos&&detail.find(L"999")==detail.npos,"I reads only current MP levels, without repeating the short status line");
   counts[7].value=L"5";expect(ui::character_row::details().ends_with(L"Level 8 MP 5"),"I reads fresh native counts without stale text cache");
   counts[7].active=false;expect(ui::character_row::details().find(L"Level 8")==detail.npos,"hidden charge remains absent");counts[7].active=true;
   autodetail=true;spoken.clear();ui::character_row::announce(&r,&cursor,brief,&window);
   expect(spoken.size()==1&&spoken[0].starts_with(brief+L". ")&&spoken[0].ends_with(L"Level 8 MP 5"),"Autodetail queues the charge tail after overview");
   expect(spoken[0].find(brief,brief.size())==std::wstring::npos,"Autodetail never repeats the full status line");
   expect(ui::character_row::details().find(brief)==std::wstring::npos,"manual I remains MP-only with Autodetail enabled");
   List actual_rows{&r};expect(displayed::cursor_row(&cursor,&actual_rows)==&r,"native FocusCursorParent resolves the displayed overview row");
   Node unrelated;cursor.parent=&unrelated;expect(ui::character_row::details().empty(),"cursor moving to another row releases old details");cursor.parent=&anchor;
   set(v,"nativeFocusParent",nullptr);set(v,"nativeDefaultParent",&anchor);expect(!ui::character_row::details().empty(),"native DefaultCursorParent is also a valid character focus anchor");set(v,"nativeFocusParent",&anchor);set(v,"nativeDefaultParent",&other);
   cursor.active=false;expect(ui::character_row::details().empty(),"hidden native cursor releases details");cursor.active=true;
   cursor.focused=false;expect(!ui::character_row::details().empty(),"native Select clears IsFocus on the active character cursor; I still works");
   window.command=1;expect(ui::character_row::details().empty(),"passive List overview cannot steal item/spell I");window.command=0;expect(ui::character_row::details().empty(),"closed None overview cannot steal details");window.command=2;
   set(window,"selectCursor",&other);expect(ui::character_row::details().empty(),"a different active native cursor retires the old overview");set(window,"selectCursor",&cursor);
   ui::character_row::announce(&r,&cursor,brief);expect(!ui::character_row::details().empty(),"visible item/spell target does not misuse native IsFocus either");ui::character_row::announce(&r,&cursor,brief,&window);
   cursor_image.alpha=0;expect(ui::character_row::details().empty(),"hidden cursor image retires details even with live controller");cursor_image.alpha=1;
   main_menu=false;expect(ui::character_row::details().empty(),"menu exit/preload cannot expose character details");main_menu=true;
   ui::character_row::clear();expect(ui::character_row::details().empty(),"context retirement clears borrowed owners");counts[7].value=L"0";
  }
  expect(displayed::target(&r)==L"Corrad, HP 36/47","item/spell targets retain concise name and HP");
  charges.push_back(&scalar);expect(displayed::charge_lines(&v).empty(),"invalid over-eight charge count is rejected");
  autodetail=false;fixture::game=game::Id::FF6;
 }
 Node party,ordinary,target,row,view,unit,copy,owned;party.active=false;owned.id=4;set(unit,"owned",&owned);set(copy,"owned",&owned);List empty,rows{&row};set(party,"contentList",&empty);set(party,"targetSelectContentList",&rows);set(row,"view",&view);set(row,"<TargetData>k__BackingField",&unit);
 Node name=make_text(L"Rydia"),hp=make_text(L"30"),maximum=make_text(L"30"),mp=make_text(L"10");set(view,"nameText",&name);set(view,"currentHpText",&hp);set(view,"maxHpText",&maximum);set(view,"currentMpText",&mp);row.enabled=false;
 for(int n=1;n<=6;++n){fixture::game=static_cast<game::Id>(n);ffpr::battle::observe_target_party(&party);expect(ffpr::battle::displayed_player(&copy)==L"Rydia, HP 30/30, MP 10","all-six actual party reader matches rendered target by native character identity with hidden controller");expect(displayed::target(&row)==L"Rydia, HP 30/30, MP 10","disabled target scripts do not hide their rendered name/vitals");}
 // A different live but hidden target controller can be the latest native
 // observation. Attack still renders the ordinary owner's matching party row.
 Node hidden_party,hidden_row,hidden_view;hidden_view.active=false;List hidden_rows{&hidden_row};set(hidden_party,"targetSelectContentList",&hidden_rows);set(hidden_row,"view",&hidden_view);set(hidden_row,"<TargetData>k__BackingField",&unit);
 for(int n=1;n<=6;++n){fixture::game=static_cast<game::Id>(n);ffpr::battle::observe_target_party(&hidden_party);expect(ffpr::battle::displayed_player(&copy)==L"Rydia, HP 30/30, MP 10","a newer hidden owner cannot silence the ally rendered in the ordinary Attack party panel");}
 Node command_text=make_text(L"Defend");set(view,"commandText",&command_text);name.value=L"Firion";hp.value=L"1363";maximum.value=L"1605";mp.value=L"10";
 expect(ffpr::battle::displayed_player(&copy)==L"Firion, HP 1363/1605, MP 10, Defend","Attack ally speech contains exactly the displayed name, HP, current MP and command without inventing maximum MP");
 Node hidden_mp_max=make_text(L"999");hidden_mp_max.active=false;set(view,"maxMpText",&hidden_mp_max);
 hp.value=maximum.value=L"1900";command_text.value=L"Attack";
 ffpr::battle::g_execution_player=&unit;
 for(int n=1;n<=6;++n){fixture::game=static_cast<game::Id>(n);spoken.clear();ffpr::battle::announce_active_status();ffpr::battle::announce_active_status();expect(spoken==std::vector<std::wstring>({L"Firion, HP 1900/1900, MP 10",L"Firion, HP 1900/1900, MP 10"}),"all-six H status reads current/max HP and only displayed current MP, without appending the separate command or suppressing repeated requests");expect(ffpr::battle::displayed_player(&copy)==L"Firion, HP 1900/1900, MP 10, Attack","ally targeting still includes the displayed command column independently from H status");}
 // All-six danger colors and visible body/effect status review. This is a
 // native-boundary fixture, not a parallel status implementation or game launch.
 expect(party_status_install&&party_status_install()==2&&form_hook&&condition_text_hook,"completed native form producer is installed");
 Node info,entity,body,body_renderer,body_sprite,animation,ordinary_motions,changed_motions,condition,function,effects_owner;
 entity.kind=4;body_renderer.kind=8;
 set(unit,"info",&info);set(info,"<BattleSpriteEntity>k__BackingField",&entity);
 set(entity,"characterSR",&body);set(body,"meshRenderer",&body_renderer);set(body,"sprite",&body_sprite);
 set(entity,"spriteAnimBattle",&animation);set(animation,"<battleCharaSpriteList>k__BackingField",&ordinary_motions);
 set(function,"condition",&condition);List functions{&function};set(info,"functions",&functions);
 set(entity,"ConditionEffectEntityController",&effects_owner);
 for(int n=1;n<=6;++n){
  fixture::game=static_cast<game::Id>(n);name.value=L"Rae";hp.value=L"21";maximum.value=L"297";
  hp.r=1;hp.g=hp.b=64.0f/255;spoken.clear();ffpr::battle::g_execution_player=&unit;
  ffpr::battle::announce_active_status();ffpr::battle::announce_active_status();
  expect(spoken==std::vector<std::wstring>({L"Rae, HP 21/297, MP 10, Critical",L"Rae, HP 21/297, MP 10, Critical"}),"all-six H translates the actual red HP cue and preserves repeat requests");
  expect(ffpr::battle::displayed_player(&copy)==L"Rae, HP 21/297, MP 10, Attack, Critical","ally targeting includes the same visible low-health equivalent");
  spoken.clear();ffpr::battle::announce_slot(1);expect(contains(L"Critical"),"number row includes visible danger outside targeting");
  hp.r=hp.g=hp.b=153.0f/255;hp.value=L"0";spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Critical")&&contains(L"KO"),"KO gray zero HP has its own visible equivalent, not Critical");
  if(n==6){
   hp.value=L"21";hp.r=1;hp.g=hp.b=64.0f/255;
   Node ui_conditions,ui_view,ui_text=make_text(L""),critical=make_text(L"Critical");critical.command=4;
   set(ui_conditions,"view",&ui_view);set(ui_view,"conditionText",&ui_text);set(row,"condtionController",&ui_conditions);
   condition_text_hook(&ui_conditions,&critical,nullptr);
   auto extra=ffpr::battle::party_status::extra_lines(&unit,&view,false,&ui_conditions);
   expect(extra.empty(),"FFVI native Critical text owns its label without text-content deduplication");
   ui_text.active=false;extra=ffpr::battle::party_status::extra_lines(&unit,&view,false,&ui_conditions);
   expect(extra==std::vector<std::wstring>{L"Critical"},"hidden rotating condition text yields to the visible HP cue");set(row,"condtionController",nullptr);
  }
  hp.r=hp.g=hp.b=1;hp.value=L"21";spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Critical"),"low numeric HP alone is not renderer evidence");
  hp.r=1;hp.g=hp.b=64.0f/255;hp.alpha=0;spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Critical"),"transparent HP cannot disclose a cached danger cue");hp.alpha=1;
  palette_ready=false;spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Critical"),"unavailable native palette fails closed");palette_ready=true;hp.r=hp.g=hp.b=1;
  // Mini is genuinely supported in II-V; VI has Imp, I has neither transform.
  if(n!=1){
   condition.command=n==6?601:401;condition.id=condition.command;condition.value=n==6?L"Imp":L"Mini";
   next_motions=&changed_motions;form_hook(nullptr,&entity,1,1,condition.command,nullptr);
   spoken.clear();ffpr::battle::announce_active_status();expect(contains(condition.value),"H reads a completed visible ally form while the command HUD hides status icons");
   spoken.clear();ffpr::battle::announce_slot(1);expect(contains(condition.value),"number row reads the inactive visible ally form");
   expect(ffpr::battle::displayed_player(&copy).ends_with(condition.value),"target reader shares form fallback if no condition column is rendered");
   body_renderer.enabled=false;spoken.clear();ffpr::battle::announce_active_status();expect(!contains(condition.value),"hidden body cannot prove a form");body_renderer.enabled=true;
   next_motions=&ordinary_motions;form_hook(nullptr,&entity,1,1,0,nullptr);spoken.clear();ffpr::battle::announce_active_status();expect(!contains(condition.value),"native form restoration retires its old status");
  }
  // Tint flags and actual visible effect renderers are independent witnesses.
  condition.command=10;condition.id=10;condition.value=n==2?L"Venom":L"Poison";function.kind=5;
  set(function,"changedSkinColor",reinterpret_cast<void*>(1));set(function,"effectRender",&body_renderer);
  spoken.clear();ffpr::battle::announce_active_status();expect(contains(condition.value),"all-six H includes a currently applied visible poison tint");
  set(function,"changedSkinColor",nullptr);spoken.clear();ffpr::battle::announce_active_status();expect(!contains(condition.value),"unapplied tint cannot expose the raw condition list");function.kind=0;
  Node effect,effect_renderer,particle;effect.kind=6;effect_renderer.kind=8;condition.command=8;condition.id=8;condition.value=L"Paralysis";
  set(effect,"conditionType",reinterpret_cast<void*>(8));set(particle,"renderer",&effect_renderer);List particles{&particle},effects{&effect};
  set(effect,"particleSystems",&particles);set(effects_owner,"conditionEffectList",&effects);
  spoken.clear();ffpr::battle::announce_active_status();expect(contains(L"Paralysis"),"visible playing particles identify the ally's native paralysis effect");
  effect_renderer.enabled=false;spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Paralysis"),"disabled effect renderer cannot expose an internal paralysis flag");effect_renderer.enabled=true;
  particle.enabled=false;spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Paralysis"),"stopped particles are not an active visual cue");particle.enabled=true;
  effects.assign(65,&effect);spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Paralysis"),"over-bound effect storage is rejected before traversal");effects.clear();
  functions.assign(129,&function);spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Paralysis"),"over-bound function storage is rejected before traversal");functions.assign(1,&function);
  Node inspector,reaction;List target_meshes{&body};condition.command=19;condition.id=19;condition.value=L"Protect";
  effects={&effect};set(effect,"conditionType",reinterpret_cast<void*>(19));set(effect,"particleSystems",nullptr);
  set(effect,"inspector",&inspector);set(inspector,"currentType",reinterpret_cast<void*>(13));
  set(inspector,"isStart",reinterpret_cast<void*>(1));set(inspector,"currentReaction",&reaction);set(reaction,"entityMeshRenderers",&target_meshes);
  spoken.clear();ffpr::battle::announce_active_status();expect(contains(L"Protect"),"all-six H reads a started native shader reaction bound to the actual visible ally mesh");
  set(inspector,"isEnd",reinterpret_cast<void*>(1));spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Protect"),"completed shader reactions cannot expose a lingering condition flag");set(inspector,"isEnd",nullptr);
  target_meshes={&body_sprite};spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Protect"),"another renderer's shader cannot be borrowed as ally evidence");
  if(n==6){
   set(effect,"inspector",nullptr);set(effect,"conditionType",reinterpret_cast<void*>(6));set(effect,"effectId",reinterpret_cast<void*>(68));
   condition.command=6;condition.id=6;condition.value=L"Silence";set(effect,"child_renderer",&effect_renderer);
   spoken.clear();ffpr::battle::announce_active_status();expect(contains(L"Silence"),"FFVI's verified bubble prefab is read through its one visible child sprite renderer");
   effect_renderer.enabled=false;spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Silence"),"hidden FFVI silence bubble cannot disclose its condition");effect_renderer.enabled=true;
   set(effect,"effectId",reinterpret_cast<void*>(69));spoken.clear();ffpr::battle::announce_active_status();expect(!contains(L"Silence"),"unreviewed prefabs do not receive a hierarchy lookup");
  }
  set(effects_owner,"conditionEffectList",nullptr);
 }
 // The entry boundary precedes native initial forms; BattleStart must preserve
 // them. End/return-to-title retire observations, even if an address is reused.
 fixture::game=game::Id::FF3;condition.command=401;condition.id=401;condition.value=L"Mini";
 for(auto boundary:{events::Kind::BattleEntering,events::Kind::BattleEnd,events::Kind::TitleScreen}){
  next_motions=&changed_motions;set(animation,"<battleCharaSpriteList>k__BackingField",&ordinary_motions);form_hook(nullptr,&entity,1,1,401,nullptr);
  for(auto& fn:status_events[events::Kind::BattleStart])fn();
  expect(!ffpr::battle::party_status::extra_lines(&unit,&view,true,nullptr).empty(),"BattleStart preserves a form loaded during battle entry");
  for(auto& fn:status_events[boundary])fn();
  expect(ffpr::battle::party_status::extra_lines(&unit,&view,true,nullptr).empty(),"entry/end/title boundaries retire prior battle form identities");
 }
 // Compare the complete H formatting/dispatch path with the same fake native
 // owner. These timings exclude real Unity/render/audio cost; counts expose
 // added work, while the wall-clock numbers are harness measurements only.
 auto measure_h=[&](const char* label){
  const auto calls=native_calls;const auto start=std::chrono::steady_clock::now();double peak=0;
  for(int i=0;i<1000;++i){spoken.clear();const auto begin=std::chrono::steady_clock::now();ffpr::battle::announce_active_status();peak=std::max(peak,std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count());}
  std::printf("H harness %s: 1000 requests %.3f ms aggregate, %.3f us peak; %llu fake-native boundary calls/request; audible latency unmeasured\n",label,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count(),peak,static_cast<unsigned long long>((native_calls-calls)/1000));
 };
 set(unit,"info",nullptr);measure_h("text baseline");set(unit,"info",&info);
 next_motions=&changed_motions;set(animation,"<battleCharaSpriteList>k__BackingField",&ordinary_motions);form_hook(nullptr,&entity,1,1,401,nullptr);measure_h("visible Mini");
 functions.assign(128,&function);Node dense_effect;dense_effect.kind=6;List dense_effects(64,&dense_effect);set(effects_owner,"conditionEffectList",&dense_effects);measure_h("128 functions/64 effects, total renderer budget64");
 set(effects_owner,"conditionEffectList",nullptr);functions.assign(1,&function);
 // Leave the shared party fixture in its original ready state.
 set(unit,"info",nullptr);name.value=L"Firion";hp.value=maximum.value=L"1900";hp.r=hp.g=hp.b=1;
 ffpr::battle::g_execution_player=nullptr;
 // Exercise the registered battle I query with the native active member/target.
 ffpr::battle::install();
 std::array<Node,8> battle_counts;List battle_charges;
 for(auto& cell:battle_counts){cell=make_text(L"2");battle_charges.push_back(&cell);}set(view,"currentLvMpTextList",&battle_charges);
 for(auto id:{game::Id::FF1,game::Id::FF3}) {
  fixture::game=id;active_menus={menu_state::BATTLE_COMMAND};ffpr::battle::g_execution_player=&unit;
  for(bool auto_on:{false,true}) {
   autodetail=auto_on;auto details=text_queries.at("BattleParty.ChargeDetails")();
   expect(details.starts_with(L"Level 1 MP 2")&&details.ends_with(L"Level 8 MP 2")&&details.find(L"Firion")==details.npos&&details.find(L"HP")==details.npos&&details.find(L"Attack")==details.npos,"battle I reads only displayed MP levels with either Autodetail setting");
  }
  active_menus={menu_state::BATTLE_TARGET};ffpr::battle::g_focused_player=&unit;
  expect(text_queries.at("BattleParty.ChargeDetails")().starts_with(L"Level 1 MP 2"),"ally-target I uses the same MP-only payload");
 }
 active_menus.clear();ffpr::battle::g_execution_player=nullptr;ffpr::battle::g_focused_player=nullptr;autodetail=false;set(view,"currentLvMpTextList",nullptr);
 hp.value=L"1363";maximum.value=L"1605";command_text.value=L"Defend";
 command_text.active=false;expect(ffpr::battle::displayed_player(&copy)==L"Firion, HP 1363/1605, MP 10","hidden command text is not disclosed");name.value=L"Rydia";hp.value=maximum.value=L"30";
 Node cursor,anchor,empty_row;cursor.parent=&anchor;set(view,"cursorParent",&anchor);List raw_slots{&empty_row,&row};
 expect(displayed::cursor_row(&cursor,&raw_slots)==&row,"field/formation focus follows native cursor anchor despite empty first slot and separate row GameObject");
 name.alpha=0;expect(ffpr::battle::displayed_player(&unit).empty(),"fully transparent names do not disclose a target");name.alpha=1;
 Node window,portrait_owner,portrait=image(),front,back;window.command=5;portrait.parent=&front;set(row,"menuCharacterController",&portrait_owner);set(portrait_owner,"characterImageView",&portrait);set(view,"frontParent",&front);set(view,"backParent",&back);
 expect(displayed::formation_row(&window,&row)==L"Front Row","Formation row uses displayed portrait parent");portrait.parent=&back;expect(displayed::formation_row(&window,&row)==L"Back Row","whole FF4 formation shift reports actual resulting row");window.command=2;expect(displayed::formation_row(&window,&row).empty(),"magic/item/status selections never announce row");
 fixture::game=game::Id::FF6;Node esper,list_view,scroll,stone,stone_view,icon,equipped=image(),master=image();Node stone_name=make_text(L"Alexander"),desc=make_text(L"Divine Judgment: Sears all enemies with a beam of holy light."),cost_label=make_text(L"MP Cost"),cost=make_text(L"45");List stone_rows{&stone};List data{&unit};set(esper,"view",&list_view);set(esper,"contentDataList",&data);set(list_view,"scrollView",&scroll);set(scroll,"rows",&stone_rows);set(stone,"view",&stone_view);set(stone_view,"IconTextView",&icon);set(icon,"nameText",&stone_name);set(stone_view,"equipIcon",&equipped);set(stone_view,"masterImage",&master);set(list_view,"descriptionText",&desc);set(list_view,"fixedMpText",&cost_label);set(list_view,"mpValueText",&cost);
 Node ability_window,status_controller,status_view,terra_name=make_text(L"Terra"),job=make_text(L"Magitek Elite"),current_mp=make_text(L"585"),max_mp=make_text(L"713"),hidden_level=make_text(L"61");hidden_level.active=false;
 owners["AbilityWindowController"]=&ability_window;set(ability_window,"magicStoneListController",&esper);set(ability_window,"statusController",&status_controller);set(status_controller,"view",&status_view);set(status_view,"nameText",&terra_name);set(status_view,"jobNameText",&job);set(status_view,"currentMpText",&current_mp);set(status_view,"maxMpText",&max_mp);set(status_view,"currentLevelText",&hidden_level);
 ffpr::ff6::o_ListSelectContent=&list_select;ffpr::ff6::d_ListSelectContent(&esper,0,0,nullptr);tick();expect(contains(L"Alexander, Equipped, Mastered, MP Cost 45")&&ui::review::active(),"Esper list reads equip marker through native virtual-scroll lookup and builds current panel buffer");spoken.clear();ui::review::details();expect(contains(L"Divine Judgment")&&contains(L"45")&&!contains(L"Terra")&&!contains(L"61"),"Esper-only list buffer excludes active-character header and retains current description/cost");
 Node details,detail_view,spell,spell_view,spell_icon,holy=make_text(L"Holy"),progress=make_text(L"100"),percent=make_text(L"%"),rate=make_text(L"2"),times=make_text(L"x"),progress_label=make_text(L"Progress"),rate_label=make_text(L"Acquisition Rate"),level_label=make_text(L"At level up:"),strength=make_text(L"Strength"),plus=make_text(L"+"),bonus=make_text(L"2");List spells{&spell};set(details,"view",&detail_view);set(detail_view,"nameText",&stone_name);set(detail_view,"targetEquipeedIcon",&equipped);set(detail_view,"masterImage",&master);set(detail_view,"skillLevelText",&progress_label);set(detail_view,"acquisitionRatetext",&rate_label);set(detail_view,"contents",&spells);set(spell,"view",&spell_view);set(spell_view,"magicNameText",&spell_icon);set(spell_icon,"nameText",&holy);set(spell_view,"skillLevelText",&progress);set(spell_view,"percentText",&percent);set(spell_view,"acquisitionRateText",&rate);set(spell_view,"multiplicationText",&times);set(spell_view,"masterImage",&master);set(detail_view,"levelUpText",&level_label);set(detail_view,"parameterNameText",&strength);set(detail_view,"parameterFixText",&plus);set(detail_view,"parameterValueText",&bonus);set(detail_view,"descriptionText",&desc);set(detail_view,"fixedMpText",&cost_label);set(detail_view,"mpValueText",&cost);
 set(ability_window,"magicStoneDetailsController",&details);
 ffpr::ff6::o_DetailsShow=&show;ffpr::ff6::o_DetailsUpdateView=&details_update;spoken.clear();ffpr::ff6::d_DetailsShow(&details,nullptr,nullptr,nullptr,nullptr);ffpr::ff6::d_DetailsUpdateView(&details,nullptr,nullptr,nullptr);tick();tick();spoken.clear();ui::review::details();expect(contains(L"Alexander, Equipped")&&contains(L"Holy, 100%, x 2, Mastered")&&contains(L"At level up: Strength + 2")&&contains(L"MP Cost 45"),"acquired page buffer includes every rendered progress/rate/bonus/description/cost field");spoken.clear();ui::review::next();expect(contains(L"Progress, Acquisition Rate"),"arrows browse the assembled page without changing native Esper selection");
 Node replacement;replacement.id=2;stone_name.value=L"Ramuh";spoken.clear();ffpr::ff6::d_DetailsUpdateView(&details,nullptr,&replacement,nullptr);tick();expect(contains(L"Ramuh, Equipped"),"native details highlight change announces the newly selected Esper and its equip marker");

 // Same cursor, two native pages: return must rebuild without a SelectContent callback.
 Node list_cursor;set(esper,"selectCursor",&list_cursor);
 ffpr::ff6::o_details_ready=&phase;
 esper.active=false;spoken.clear();ffpr::ff6::d_DetailsShow(&details,nullptr,nullptr,&replacement,nullptr);
 ffpr::ff6::d_DetailsUpdateView(&details,nullptr,&replacement,nullptr);
 ffpr::ff6::d_details_ready(&details,nullptr);tick();tick();spoken.clear();ui::review::details();
 expect(contains(L"Holy, 100%, x 2")&&!contains(L"Terra"),"details native ready phase rebuilds on Q with the unchanged Esper identity and excludes active character");
 on_details_show=[&]{ffpr::ff6::d_DetailsUpdateView(&details,nullptr,&replacement,nullptr);ffpr::ff6::d_details_ready(&details,nullptr);};
 spoken.clear();ffpr::ff6::d_DetailsShow(&details,nullptr,nullptr,&replacement,nullptr);tick();tick();
 expect(contains(L"Ramuh, Equipped")&&ui::review::active(),"Show cannot cancel its nested native state-entry UpdateView/ready publication");on_details_show={};
 ffpr::ff6::reset_esper_list_guard();details.active=false;esper.active=true;spoken.clear();
 ffpr::ff6::refresh_esper_list(&ability_window);tick();tick();spoken.clear();ui::review::details();
 expect(contains(L"Divine Judgment")&&!contains(L"Holy, 100%")&&!contains(L"Terra"),"return to list rebuilds current panel without moving the native cursor");
 // An unowned, rendered blank slot must replace stale/pending content, including I.
 Node blank_data,blank_row,blank_view,blank_icon,blank_name=make_text(L"");blank_data.owned=false;
 set(blank_row,"view",&blank_view);set(blank_view,"IconTextView",&blank_icon);set(blank_icon,"nameText",&blank_name);
 data.push_back(&blank_data);data.push_back(&blank_data);stone_rows.push_back(&blank_row);stone_rows.push_back(&blank_row);
 spoken.clear();ffpr::ff6::d_ListSelectContent(&esper,1,0,nullptr);tick();
 expect(spoken==std::vector<std::wstring>{L"Empty"},"unowned visible blank speaks Empty once");
 spoken.clear();ui::review::details();expect(spoken==std::vector<std::wstring>{L"Empty"}&&ffpr::ff6::esper_details_reannounce_text()==L"Empty","blank buffer and manual details contain only Empty, without hidden Esper name or prior footer");
 spoken.clear();ffpr::ff6::d_ListSelectContent(&esper,2,0,nullptr);tick();expect(spoken==std::vector<std::wstring>{L"Empty"},"moving to another blank slot remains an announcement event");
 spoken.clear();ffpr::ff6::d_ListSelectContent(&esper,0,0,nullptr);tick();expect(contains(L"Ramuh, Equipped"),"blank to former Esper speaks its name again despite the retained local text guard");
 ffpr::ff6::d_ListSelectContent(&esper,0,0,nullptr);ffpr::ff6::d_ListSelectContent(&esper,1,0,nullptr);tick();spoken.clear();ui::review::details();
 expect(spoken==std::vector<std::wstring>{L"Empty"},"obsolete queued named buffer cannot replace the latest blank");
 stone_rows[1]=nullptr;spoken.clear();ffpr::ff6::d_ListSelectContent(&esper,1,0,nullptr);tick();
 expect(spoken.empty()&&!ui::review::active()&&ffpr::ff6::esper_details_reannounce_text().empty(),"unready virtual row cancels stale information without prematurely announcing Empty");
 stone_rows[1]=&blank_row;ffpr::ff6::d_ListSelectContent(&esper,1,0,nullptr);tick();
 ffpr::ff6::reset_esper_list_guard();ffpr::ff6::refresh_esper_list(&ability_window);ffpr::ff6::reset_esper_list_guard();tick();expect(!ui::review::active(),"rapid page return then close cancels pending native reads");
 ffpr::ff6::reset_esper_list_guard();expect(!ui::review::active(),"closing Esper context cancels queued buffer work");
 // Reproduce the four rendered rows from the Medusa report. The registration
 // is production Battle.Party; input dispatch/catalog are checked separately.
 Node four_owner;std::array<Node,4> four_rows,four_views,four_names,four_hp,four_max;List four;
 const wchar_t* names[]={L"Corrad",L"Rae",L"Robyn",L"Ryco"};
 const wchar_t* current[]={L"187",L"239",L"219",L"255"};
 const wchar_t* maxima[]={L"254",L"239",L"245",L"255"};
 for(int i=0;i<4;++i) {
  four_names[i]=make_text(names[i]);four_hp[i]=make_text(current[i]);four_max[i]=make_text(maxima[i]);
  set(four_rows[i],"view",&four_views[i]);set(four_views[i],"nameText",&four_names[i]);
  set(four_views[i],"currentHpText",&four_hp[i]);set(four_views[i],"maxHpText",&four_max[i]);four.push_back(&four_rows[i]);
 }
 set(four_owner,"contentList",&four);set(four_owner,"targetSelectContentList",&empty);
 ffpr::battle::g_party_owners.clear();ffpr::battle::observe_target_party(&four_owner);autodetail=false;
 for(int n=1;n<=6;++n)for(int i=0;i<9;++i) {
  fixture::game=static_cast<game::Id>(n);spoken.clear();
  const auto action=party_keys.at(static_cast<input::Key>(int(input::Key::Alpha1)+i));
  expect(action=="BattleParty.Slot"+std::to_string(i+1),"production party registration maps each number to its own slot action");
  party_actions.at(action)();
  expect(i<4 ? spoken==std::vector<std::wstring>{std::wstring(names[i])+L", HP "+current[i]+L"/"+maxima[i]} : spoken.empty(),
         "all-six registered number actions read the requested rendered party row and reject absent slots");
 }
 four_views[1].active=false;spoken.clear();party_actions.at("BattleParty.Slot2")();party_actions.at("BattleParty.Slot4")();
 expect(spoken==std::vector<std::wstring>{L"Ryco, HP 255/255"},"hidden slot2 stays silent without renumbering visible slot4");
 expect(wrong==0,"renderer/native pointers remain exclusively on their owner thread");std::printf("Party/Esper actual-source renderer, targeting, formation and page buffers: %d failures\n",failed);return failed?1:0;
}
