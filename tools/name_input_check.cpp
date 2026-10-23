// Native name-popup edit detours with fake engine owners; no game or OS input.
#include "../src/core/ui/name_input.cpp"
#include <cstdio>
#include <cstring>
#include <map>

namespace fixture {
struct Node { const char* type="InputField";void* input=nullptr;std::wstring text;int caret=0,key=0;bool active=true; };
std::vector<std::wstring> speech;std::map<events::Kind,std::vector<std::function<void()>>> listeners;
int original=0,reads=0;bool speech_on=true;
void open(void*,const void*){++original;}
void deselect(void*,void*,const void*){++original;}
int keycode(void* p,const void*){return static_cast<Node*>(p)->key;}
int key(void* p,void* event,const void*) {
    ++original;auto& n=*static_cast<Node*>(p);const int code=static_cast<Node*>(event)->key;
    if(code==276&&n.caret>0)--n.caret;
    if(code==275&&n.caret<static_cast<int>(n.text.size()))++n.caret;
    if(code==8&&n.caret>0){n.text.erase(static_cast<size_t>(--n.caret),1);}
    return 17;
}
}
namespace core {Install::Install(const char* n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){} }
namespace ref {
Class::Class(const char* n,const char* c,uint8_t g,const char* a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class& c,const char* n,const char* a):cls(c),name(n),alt(a){}
Method::Method(Class& c,const char* n,int a):cls(c),name(n),argc(a){}
bool Class::is(void* p){++fixture::reads;return p&&std::strcmp(static_cast<fixture::Node*>(p)->type,name)==0;}
int32_t Field::get(){++fixture::reads;return std::strcmp(name,"inputField")==0?offsetof(fixture::Node,input):std::strcmp(name,"m_CaretSelectPosition")==0?offsetof(fixture::Node,caret):-1;}
void* Method::ptr(){++fixture::reads;return std::strcmp(name,"get_keyCode")==0?reinterpret_cast<void*>(&fixture::keycode):nullptr;}
il2cpp::Method* Method::get(){return nullptr;}
std::wstring rd_str(void* p,Field&){++fixture::reads;return static_cast<fixture::Node*>(p)->text;}
std::wstring rd_str_bounded(void* p,Field& f,size_t maximum){auto s=rd_str(p,f);return s.size()<=maximum?s:std::wstring();}
void call_failed_note(const char*){std::abort();}
}
namespace unity {bool active_in_hierarchy(void* p){++fixture::reads;return p&&static_cast<fixture::Node*>(p)->active;} }
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return false;}void note_fault(const char*){std::abort();} }
namespace events {void on(Kind kind,std::function<void()> handler){fixture::listeners[kind].push_back(std::move(handler));} }
namespace settings {bool get_bool(const char*,bool){return fixture::speech_on;} }
namespace modtext {std::wstring T(std::wstring_view value){return std::wstring(value);} }
namespace ffpr {void say(std::wstring_view value,bool){fixture::speech.emplace_back(value);} }

int main() {
    namespace names=ui::name_input;using fixture::speech;
    int failures=0;const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    names::o_open=&fixture::open;names::o_key=&fixture::key;names::o_deselect=&fixture::deselect;names::install();
    fixture::Node popup,input,event,other;popup.type="InputPopup";popup.input=&input;
    input.text=L"Terra";input.caret=5;names::d_open(&popup,nullptr);
    const auto press=[&](int key){event.key=key;return names::d_key(&input,&event,nullptr);};
    expect(press(276)==17&&speech==std::vector<std::wstring>({L"a"}),"left arrow reads the native crossed letter and preserves return value");
    press(276);press(276);expect(speech.size()==3&&speech[1]==L"r"&&speech[2]==L"r","repeated equal letter moves are not deduplicated");
    press(275);expect(speech.back()==L"r","right arrow reads the crossed letter");
    press(8);expect(speech.back()==L"r"&&input.text==L"Tera","backspace speaks only the erased letter");
    input.text=L"aa";input.caret=2;speech.clear();press(8);press(8);press(8);
    expect(speech==std::vector<std::wstring>({L"a",L"a"}),"equal deletions each speak once; deleting empty field stays silent");
    input.text=L" ";input.caret=1;speech.clear();press(8);expect(speech==std::vector<std::wstring>({L"space"}),"deleted space has a spoken localized name");
    other.text=L"Other";other.caret=5;event.key=8;speech.clear();names::d_key(&other,&event,nullptr);
    expect(speech.empty()&&other.text==L"Othe","unrelated InputField retains native editing without name echo");
    input.text=L"Terra";input.caret=0;speech.clear();press(276);expect(speech.empty(),"native caret boundary is silent");
    fixture::speech_on=false;press(275);expect(speech.empty(),"speech setting suppresses name echo");fixture::speech_on=true;
    names::d_deselect(&input,nullptr,nullptr);press(8);expect(speech.empty()&&!names::owner,"native deselection retires the field identity");
    names::d_open(&popup,nullptr);for(auto& handler:fixture::listeners[events::Kind::SceneLoaded])handler();
    expect(!names::owner,"scene changes retire owner identity");
    expect(names::echo(L"a中b",L"a中b",2,1,276)==L"中","non-Latin caret navigation retains the actual letter");
    expect(names::echo(L"abcd",L"ad",3,1,8)==L"bc","selection deletion echoes only the removed text");
    std::printf("Native name-entry edits: %d native calls, %d failures\n",fixture::original,failures);return failures?1:0;
}
