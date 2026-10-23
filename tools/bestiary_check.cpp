// check-sources: bestiary/bestiary_reader.cpp text.cpp dedup.cpp
// Real Bestiary detours/capture; fake native views and scheduler. No game launch.
#include <map>
#include <cstdio>
#include <cstddef>
#include "../src/core/bestiary/bestiary_patches.cpp"
namespace fixture {
struct Node { void* slots[64]{}; bool alive=true,visible=true,rendered=true;std::wstring words;std::string kind; };
using List=std::vector<void*>;
std::map<std::string,int>& fields(){static std::map<std::string,int> v;return v;}
int offset(const char* name){auto& f=fields();auto [i,added]=f.emplace(name,int(f.size()));return int(offsetof(Node,slots))+i->second*int(sizeof(void*));}
void set(Node& node,const char* name,void* value){*reinterpret_cast<void**>(reinterpret_cast<char*>(&node)+offset(name))=value;}
struct Task {frame::Step run;std::string label;};
std::vector<Task> tasks;uint64_t ticks=0;double clock=0;
std::vector<ffpr::bestiary::StatEntry> entries;std::vector<std::wstring> spoken;int published=0;
game::Id game=game::Id::FF2;
void tick(){++ticks;auto work=std::move(tasks);tasks.clear();for(auto& task:work)if(task.run())tasks.push_back(std::move(task));}
void native_data(void*,void*,const void*){}
void* synchronous_content=nullptr;
void synchronous_data(void*,void* data,const void* mi){ffpr::bestiary::d_LibContentUpdate(synchronous_content,data,mi);}
void native_update(void*,const void*){}
void native_state(void*,int32_t,const void*){}
}
namespace core {Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace ref {
Class::Class(const char* n,const char* c,uint8_t g,const char* a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class& c,const char* n,const char* a):cls(c),name(n),alt(a){}
Method::Method(Class& c,const char* n,int a):cls(c),name(n),argc(a){}
int32_t Field::get(){return fixture::offset(name);}
il2cpp::Class* Class::get(){return nullptr;}
bool Class::is(void* p){
    if(!p)return false;const auto& kind=static_cast<fixture::Node*>(p)->kind;
    if(std::string(name)=="BattleEnemyData")return kind=="Enemy";
    if(std::string(name)=="FieldTresureBox")return kind=="Chest";
    if(std::string(name)=="FieldNonPlayer")return kind=="NPC";
    return true;
}
void* Method::ptr(){return nullptr;}il2cpp::Method* Method::get(){return nullptr;}
std::wstring str(void* p){return p?*static_cast<std::wstring*>(p):L"";}
std::wstring rd_str(void* p,Field& f){return str(rd<void*>(p,f));}
void call_failed_note(const char*){std::abort();}
}
namespace unity {
bool alive(void* p){return p&&static_cast<fixture::Node*>(p)->alive;}
bool active_in_hierarchy(void* p){return alive(p)&&static_cast<fixture::Node*>(p)->visible;}
int list_count(void* p){return p?int(static_cast<fixture::List*>(p)->size()):0;}
void* list_at(void* p,int i){return p&&i>=0&&i<list_count(p)?(*static_cast<fixture::List*>(p))[i]:nullptr;}
int array_length(void* p){return list_count(p);}void* array_at(void* p,int i){return list_at(p,i);}
}
namespace displayed {
void* field(void* p,const char* name){if(!p)return nullptr;return *reinterpret_cast<void**>(reinterpret_cast<char*>(p)+fixture::offset(name));}
bool container_visible(void* p){return unity::active_in_hierarchy(p);}
std::wstring text(void* p){return container_visible(p)?static_cast<fixture::Node*>(p)->words:L"";}
std::wstring text_field(void* p,const char* name){return text(field(p,name));}
}
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return false;}void note_fault(const char*){std::abort();}}
namespace log_ {void write(const char*,...) {}}
namespace game {Id current(){return fixture::game;}}
namespace actions {void define(const char*,Fn){}bool run(const char*){return false;}}
namespace queries {void define(const char*,std::function<bool()> ){}bool is(const char*){return false;}}
namespace events {void on(Kind,Handler){}}
namespace menu_state {void reset(const char*){}void set_active(const char*,bool){}bool is_active(const char*){return false;}}
namespace field::cache {void* get(ref::Class&){return nullptr;}void register_object(ref::Class&,void*){}}
namespace modtext {
std::wstring T(std::wstring_view s){return std::wstring(s);}
std::wstring TF(std::wstring_view s,std::initializer_list<std::wstring> args){auto out=std::wstring(s);int i=0;for(const auto& a:args){auto key=L"{"+std::to_wstring(i++)+L"}";auto at=out.find(key);if(at!=out.npos)out.replace(at,key.size(),a);}return out;}
}
namespace ffpr {void say(std::wstring_view s,bool){fixture::spoken.emplace_back(s);}}
namespace ffpr::status {std::wstring game_message(const std::wstring&,bool){return {};}void* master_entry(ref::Class&,int){return nullptr;}}
namespace ffpr::bestiary {
void nav_initialize(std::vector<StatEntry> e){fixture::entries=std::move(e);++fixture::published;}
void nav_reset(){fixture::entries.clear();}bool nav_is_active(){return !fixture::entries.empty();}
}
namespace frame {
uint64_t count(){return fixture::ticks;}double now(){return fixture::clock+=.0001;}
int every_frame(Step step,const char* label,int){fixture::tasks.push_back({std::move(step),label?label:""});return int(fixture::tasks.size());}
void cancel_label(const char* label){std::erase_if(fixture::tasks,[=](const auto& t){return t.label.starts_with(label);});}
void next_frame(Task task,const char* label){every_frame([task=std::move(task)]{task();return false;},label,1);}
void after_frames(int,Task task,const char* label){next_frame(std::move(task),label);}
}
int main(){
    using namespace fixture;using namespace ffpr::bestiary;
    int failures=0;auto expect=[&](bool yes,const char* why){if(!yes){++failures;std::printf("FAIL %s\n",why);}};
    o_LibInfoSetData=&native_data;o_LibContentUpdate=&native_data;o_LibInfoUpdate=&native_update;o_ChangeState=&native_state;
    for(int n=1;n<=6;++n)for(bool config:{false,true}) {
        fixture::game=static_cast<game::Id>(n);tracker().reset();published=0;spoken.clear();
        Node controller,outer,window,window_view,content_controller,content,data,label,value,name,stat;
        label.words=L"-";value.words=L"00000";name.words=L"Leg Eater";
        set(controller,"view",&outer);set(outer,"windowController",&window);
        set(window,"view",&window_view);set(window_view,"nameText",&name);
        set(window,"contentController",&content_controller);set(content_controller,"view",&content);
        List stats{&stat};set(content,"statusTable",&stats);set(stat,"paramText",&label);set(stat,"valueText",&value);
        if(config)config_handle_state_change(n==1?20:n==2?19:18);else d_ChangeState(nullptr,4,nullptr);
        d_LibInfoSetData(&controller,&data,nullptr);
        for(int i=0;i<48;++i){tick();d_LibInfoUpdate(&controller,nullptr);}
        expect(published==0,"SetData plus48 updates cannot publish prefab placeholders");
        name.visible=false;label.words=L"HP";value.words=L"12";
        d_LibContentUpdate(&content,&data,nullptr);d_LibInfoUpdate(&controller,nullptr);tick();
        expect(published==0,"populated but hidden native panel waits for rendered header");
        name.visible=true;d_LibInfoUpdate(&controller,nullptr);
        for(int i=0;i<30&&!published;++i)tick();
        expect(published==1&&entries.size()==2&&entries[1].to_string()==L"HP: 12","both entry paths publish current rendered values in all6 games");
        for(int i=0;i<10;++i)d_LibInfoUpdate(&controller,nullptr);
        expect(published==1,"unchanged native updates do not rebuild or reannounce detail");
        d_LibInfoSetData(&controller,&data,nullptr);d_LibContentUpdate(&content,&data,nullptr);d_LibInfoUpdate(&controller,nullptr);
        tracker().reset();for(int i=0;i<30;++i)tick();
        expect(published==1,"closing cancels a partially copied detail without stale publication");
        synchronous_content=&content;o_LibInfoSetData=&synchronous_data;
        d_LibInfoSetData(&controller,&data,nullptr);d_LibInfoUpdate(&controller,nullptr);
        for(int i=0;i<30&&published==1;++i)tick();
        expect(published==2,"synchronous nested native population belongs to the new selection generation");
        o_LibInfoSetData=&native_data;tracker().reset();
    }
    Node content,row,label,value;List rows(65,&row);
    set(content,"statusTable",&rows);set(row,"paramText",&label);set(row,"valueText",&value);
    label.words=L"HP";value.words=std::wstring(100000,L'x');StatCapture bounded;bounded.content=&content;
    for(int i=0;i<10&&!capture_stat_step(bounded);++i){}
    expect(bounded.entries.empty(),"corrupt oversized native table is rejected before indexing");
    rows.resize(64);bounded={};bounded.content=&content;
    for(int i=0;i<100&&!capture_stat_step(bounded);++i){}
    expect(bounded.characters<=65536&&bounded.entries.size()<64,"oversized text respects total capture retention bound");
    expect(!bounded.entries.empty()&&bounded.entries[0].value.size()==4096,"each retained native text field is bounded");
    std::printf("Bestiary native-ready/capture lifecycle: %d failures\n",failures);return failures?1:0;
}
