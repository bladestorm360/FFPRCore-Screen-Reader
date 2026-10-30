// Actual boot wrappers; fake native operations only, never launches a game.
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <string>
#include <vector>
#include "../src/core/system/startup_diagnostics.cpp"
namespace fixture {
int calls=0,metadata_reads=0;std::vector<std::string> lines;game::Id game_id=game::Id::FF3;
alignas(8) uint8_t child_class[0xc0]{},parent_class[0xc0]{};
struct StaticFields {void* instance=nullptr;bool searched=false;} fields;
}
struct ManagedClass {void* image=nullptr;void* gc=nullptr;const char* name="NullReferenceException";};
struct ManagedText {void*klass=nullptr;void*monitor=nullptr;int length=12;wchar_t value[193]=L"boot failure";};
struct ManagedError {ManagedClass*klass;void*monitor=nullptr;void*class_name=nullptr;ManagedText*message;void*unused[3]{};void*trace=nullptr;};
static_assert(offsetof(ManagedError,trace)==0x38);
struct TraceArray {void*klass=nullptr;void*monitor=nullptr;void*bounds=nullptr;uintptr_t count=20;uintptr_t items[20]{};};
static_assert(offsetof(TraceArray,items)==0x20);
struct NativeWrapper {ManagedError*error;};
ManagedClass managed_class;ManagedText managed_text;ManagedError managed_error{&managed_class,nullptr,nullptr,&managed_text};
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace game {Id current(){return fixture::game_id;}}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
il2cpp::Class* Class::get(){++fixture::metadata_reads;return reinterpret_cast<il2cpp::Class*>(fixture::child_class);}
}
namespace il2cpp {
Class* class_parent(Class* type){++fixture::metadata_reads;return type==reinterpret_cast<Class*>(fixture::child_class)?reinterpret_cast<Class*>(fixture::parent_class):nullptr;}
const char* class_name(Class* type){++fixture::metadata_reads;return type==reinterpret_cast<Class*>(fixture::parent_class)?"SingletonMonoBehaviour`1":"ObservedManager";}
int32_t field_offset(Class*,const char* name){++fixture::metadata_reads;return std::strcmp(name,"instance")==0?0:std::strcmp(name,"isAvailable")==0?8:-1;}
}
namespace hooks {bool attach_raw(ref::Method&,void*,void**){return false;}}
namespace log_ {void write(const char*fmt,...){char text[1024];va_list a;va_start(a,fmt);vsnprintf(text,sizeof(text),fmt,a);va_end(a);fixture::lines.emplace_back(text);}}
void normal(void*,const void*){++fixture::calls;}
void fault(void*,const void*){++fixture::calls;RaiseException(0xe1234567,0,0,nullptr);}
void cpp_fault(void*,const void*){++fixture::calls;throw NativeWrapper{&managed_error};}
bool invoke_fault(ffpr::system::startup_diagnostics::Native fn)
{
    __try {fn(nullptr,nullptr);}
    __except(GetExceptionCode()==0xe1234567?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH) {return true;}
    return false;
}
int main()
{
    using namespace ffpr::system::startup_diagnostics;
    int failures=0;auto expect=[&](bool yes,const char*why){if(!yes){++failures;std::printf("FAIL %s\n",why);}};
    o_start=o_create=o_resource=&normal;fixture::calls=0;
    start(nullptr,nullptr);create(nullptr,nullptr);resource(nullptr,nullptr);
    expect(fixture::calls==3&&fixture::lines.size()==6,"ordinary boot operations run exactly once with completed boundaries");
    o_start=o_create=o_resource=&fault;fixture::lines.clear();fixture::calls=0;g_exceptions=0;
    expect(invoke_fault(&start)&&invoke_fault(&create)&&invoke_fault(&resource),"native errors propagate to the original outer handler");
    expect(fixture::calls==3&&fixture::lines.size()==6,"fault boundaries are recorded without false completion or swallowed operations");
    const auto before=fixture::lines.size();
    for(int i=0;i<8;++i)expect(invoke_fault(&create),"diagnostic capacity never changes native error propagation");
    expect(g_exceptions==11&&fixture::lines.size()==before+9,"only four exception records retained; native boundaries continue");
    char name[65]{};wchar_t message[193]{};EXCEPTION_RECORD record{};record.ExceptionCode=0xe06d7363;record.NumberParameters=3;record.ExceptionInformation[1]=1;
    expect(!copy_managed(&record,name,message),"an unrelated or malformed C++ exception cannot break its observer");
    NativeWrapper wrapper{&managed_error};record.ExceptionInformation[1]=reinterpret_cast<ULONG_PTR>(&wrapper);
    expect(copy_managed(&record,name,message)&&std::string(name)=="NullReferenceException"&&std::wstring(message)==L"boot failure","bounded managed exception class and message copy use the verified layout");
    o_start=&cpp_fault;g_exceptions=0;fixture::lines.clear();bool caught=false;
    try {start(nullptr,nullptr);}catch(const NativeWrapper& original){caught=original.error==&managed_error;}
    bool named=false;for(const auto& line:fixture::lines)named|=line.find("NullReferenceException")!=std::string::npos;
    expect(caught&&fixture::lines.size()==3&&named,"real MSVC exception propagates unchanged after its diagnostic prefix and unavailable trace are copied");
    TraceArray trace;uintptr_t methods[20]{};
    timing::game_base=0x10000000;timing::game_size=0x100000;
    for(unsigned i=0;i<20;++i){methods[i]=0x10001000+i*0x100;trace.items[i]=reinterpret_cast<uintptr_t>(&methods[i]);}
    managed_error.trace=&trace;
    auto prefix=copy_trace(&record);
    expect(prefix.valid&&prefix.total==20&&prefix.count==12&&prefix.rvas[0]==0x1000&&prefix.rvas[11]==0x1b00,
        "ready native-produced MethodInfo array copies only12 module-relative method addresses");
    methods[0]=0x90000000;prefix=copy_trace(&record);
    expect(prefix.valid&&prefix.count==11,"foreign-module values are not mislabeled as game methods");
    trace.count=4097;prefix=copy_trace(&record);expect(!prefix.valid&&prefix.count==0,"oversized trace headers are rejected before reading elements");
    trace.count=1;trace.items[0]=1;prefix=copy_trace(&record);expect(prefix.valid&&prefix.count==0,"malformed MethodInfo pointers cannot break error propagation");
    managed_error.trace=reinterpret_cast<void*>(1);expect(!copy_trace(&record).valid,"malformed native trace arrays remain guarded");
    managed_error.trace=&trace;trace.count=20;trace.items[0]=reinterpret_cast<uintptr_t>(&methods[0]);methods[0]=0x10001000;
    auto* fields=&fixture::fields;std::memcpy(fixture::parent_class+0xb8,&fields,sizeof(fields));
    prepare_watches();const int ready_reads=fixture::metadata_reads;
    auto state=singleton_state(g_watches[0]);expect(state.valid&&!state.searched&&!state.present,"cold singleton observation does not perform the first lookup");
    fixture::fields.searched=true;state=singleton_state(g_watches[0]);
    expect(state.valid&&state.searched&&!state.present,"failed cached singleton lookup is distinguishable from an unsearched singleton");
    fixture::fields.instance=&trace;state=singleton_state(g_watches[0]);
    expect(state.valid&&state.present&&state.searched,"available singleton is observed without calling its getter");
    o_create=&normal;fixture::lines.clear();create(nullptr,nullptr);
    expect(fixture::lines.size()==8&&fixture::metadata_reads==ready_reads&&fixture::fields.instance==&trace&&fixture::fields.searched,
        "before/completed snapshots copy scalars only, without metadata calls, searches or native-state writes");
    o_create=&cpp_fault;fixture::lines.clear();g_exceptions=0;caught=false;
    try{create(nullptr,nullptr);}catch(const NativeWrapper& original){caught=original.error==&managed_error;}
    bool frame_seen=false,fault_state=false,completed=false;
    for(const auto& line:fixture::lines){frame_seen|=line.find("method-rva=0x00001000")!=std::string::npos;fault_state|=line.find("CreateInstance fault singleton=")!=std::string::npos;completed|=line.find("CreateInstance completed")!=std::string::npos;}
    expect(caught&&frame_seen&&fault_state&&!completed&&fixture::metadata_reads==ready_reads,
        "actual failing wrapper publishes bounded method/state evidence and preserves the same original exception");
    fixture::game_id=game::Id::FF2;expect(!copy_trace(&record).valid,"unverified other-game trace layouts are not read");
    fixture::game_id=game::Id::FF3;
    EXCEPTION_POINTERS error{&record,nullptr};double aggregate=0,peak=0;
    for(int episode=0;episode<100;++episode){fixture::lines.clear();g_exceptions=0;
        const auto began=std::chrono::steady_clock::now();
        const int result=exception_filter("SceneBoot.CreateInstance",&error);
        const double elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
        aggregate+=elapsed;peak=(std::max)(peak,elapsed);
        expect(result==EXCEPTION_CONTINUE_SEARCH,"bounded failure evidence never consumes the native exception");}
    expect(fixture::metadata_reads==ready_reads,"repeated exception capture never invokes metadata or singleton APIs");
    std::printf("100 independent fake-owner failure captures: %.3f ms aggregate, %.3f ms peak; production limit4/process, 12 methods/prefix; native/game/audio unmeasured\n",aggregate,peak);
    std::printf("Actual boot diagnostics: original calls/error propagation/capacity, %d failures\n",failures);return failures?1:0;
}
