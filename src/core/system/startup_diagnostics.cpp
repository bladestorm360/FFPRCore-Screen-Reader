// Bounded boot-boundary diagnostics. Unity disables its own Player.log in these
// releases, so an exception before Splash otherwise leaves only a healthy input
// heartbeat. Observe the original operation; never consume or repair its error.
#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include "../core.h"
#include "../hooks.h"
#include "../log.h"
#include "../refs.h"

namespace ffpr::system::startup_diagnostics {
namespace {
ref::Class kBoot{"", "SceneBoot"};
ref::Method mStart{kBoot,"Start",0},mCreate{kBoot,"CreateInstance",0};
ref::Class kResource{"Last.Management","ResourceManager"};
ref::Method mResource{kResource,"Initialize",0};
using Native=void(*)(void*,const void*);
Native o_start=nullptr,o_create=nullptr,o_resource=nullptr;
unsigned g_exceptions=0;

// FFIII's actual singleton getter searches once, then retains its result even
// when the result is null. Observe that ready storage without CALLING a getter:
// a diagnostic lookup must never cause the initialization failure it studies.
// Metadata is resolved once before enabling hooks. Its process-lifetime class
// pointer stays on native owners; the logger receives only copied scalars.
ref::Class kFade{"System.Fade","FadeManager",game::kFF3};
ref::Class kSteam{"Last.Management","SteamSDKManager",game::kFF3};
struct SingletonWatch {const char* name;ref::Class* type;il2cpp::Class* storage=nullptr;};
SingletonWatch g_watches[]={{"FadeManager",&kFade},{"SteamSDKManager",&kSteam},{"ResourceManager",&kResource}};
void prepare_watches()
{
    if(!game::is(game::Id::FF3))return;
    unsigned prepared=0;
    for(auto& watch:g_watches) {
        watch.storage=nullptr;
        auto* type=watch.type->get();
        for(int depth=0;type&&depth<8;++depth,type=il2cpp::class_parent(type)) {
            const char* name=il2cpp::class_name(type);
            if(!name||std::strncmp(name,"SingletonMonoBehaviour",22)!=0)continue;
            if(il2cpp::field_offset(type,"instance")==0&&il2cpp::field_offset(type,"isAvailable")==8)watch.storage=type;
            break;
        }
        prepared+=watch.storage!=nullptr;
    }
    LOGF("[Startup] singleton metadata prepared=%u/3; getter-free observation",prepared);
}
struct SingletonState {bool valid=false,searched=false,present=false;};
SingletonState singleton_state(const SingletonWatch& watch)
{
    SingletonState state;
    __try {
        if(!watch.storage)return state;
        // FFIII matching native get_Instance: Class.static_fields is0xB8;
        // verified named static fields above are instance0 / isAvailable8.
        auto* fields=*reinterpret_cast<uint8_t**>(reinterpret_cast<uint8_t*>(watch.storage)+0xb8);
        if(!fields)return state;
        state.present=*reinterpret_cast<void**>(fields)!=nullptr;
        state.searched=fields[8]!=0;state.valid=true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
    return state;
}
void record_singletons(const char* phase)
{
    for(const auto& watch:g_watches)if(watch.storage) {
        const auto state=singleton_state(watch);
        LOGF("[Startup] %s singleton=%s valid=%d lookup-done=%d instance-present=%d",phase,watch.name,state.valid,state.searched,state.present);
    }
}

struct TracePrefix {uint32_t rvas[12]{};unsigned count=0,total=0;bool valid=false;};
uintptr_t method_code(uintptr_t method)
{
    __try {return method?*reinterpret_cast<uintptr_t*>(method):0;}
    __except(EXCEPTION_EXECUTE_HANDLER) {return 0;}
}
TracePrefix copy_trace(EXCEPTION_RECORD* record)
{
    TracePrefix prefix;
    if(!game::is(game::Id::FF3))return prefix;
    __try {
        if(!record||record->ExceptionCode!=0xe06d7363||record->NumberParameters<3)return prefix;
        auto* wrapper=reinterpret_cast<uintptr_t*>(record->ExceptionInformation[1]);if(!wrapper)return prefix;
        auto* object=reinterpret_cast<uint8_t*>(*wrapper);if(!object)return prefix;
        // Verified FFIII Exception.PrepareForThrow fills _stackTrace0x38
        // before raising. Native StackTrace.get_trace consumes one MethodInfo*
        // per IntPtr element; this is NOT native_trace_ips0x80 or a stack scan.
        auto* array=*reinterpret_cast<uint8_t**>(object+0x38);if(!array)return prefix;
        const auto length=*reinterpret_cast<uintptr_t*>(array+0x18);
        if(length>4096)return prefix;
        prefix.total=static_cast<unsigned>(length);prefix.valid=true;
        const auto base=timing::game_base.load(),size=timing::game_size.load();
        if(!base||!size)return prefix;
        for(unsigned i=0;i<(std::min)(prefix.total,12u);++i) {
            const auto method=*reinterpret_cast<uintptr_t*>(array+0x20+i*sizeof(uintptr_t));
            const auto code=method_code(method);
            if(code>=base&&code-base<size)prefix.rvas[prefix.count++]=static_cast<uint32_t>(code-base);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {prefix.valid=false;}
    return prefix;
}

// The native Il2CppExceptionWrapper's first field is the exception object.
// All-six generated System.Exception layouts place _message at0x18. Only a
// fixed class/message prefix is copied on the throwing owner; no game API,
// formatting getter, stack walk, worker-native pointer or whole-scene search.
bool copy_managed(EXCEPTION_RECORD* record,char (&name)[65],wchar_t (&message)[193])
{
    __try {
        if(record->ExceptionCode!=0xe06d7363||record->NumberParameters<3)return false;
        auto wrapper=reinterpret_cast<uintptr_t*>(record->ExceptionInformation[1]);
        if(!wrapper)return false;
        auto object=reinterpret_cast<uint8_t*>(*wrapper);
        if(!object)return false;
        auto klass=*reinterpret_cast<uint8_t**>(object);
        auto source=*reinterpret_cast<const char**>(klass+0x10);
        if(!source)return false;
        unsigned count=0;
        for(;count<64&&source[count];++count) {
            const auto ch=static_cast<unsigned char>(source[count]);
            if(ch<0x20||ch>0x7e)return false;
            name[count]=source[count];
        }
        name[count]=0;
        if(!count)return false;
        auto text=*reinterpret_cast<uint8_t**>(object+0x18);
        if(text) {
            const auto length=*reinterpret_cast<int32_t*>(text+0x10);
            if(length<0||length>(1<<20))return false;
            const auto size=(std::min)(length,192);
            for(int i=0;i<size;++i)message[i]=reinterpret_cast<const wchar_t*>(text+0x14)[i];
            message[size]=0;
        }
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {return false;}
}

int exception_filter(const char* boundary,EXCEPTION_POINTERS* error)
{
    // Diagnostic overload bound only; no player speech or text deduplication.
    if(error&&error->ExceptionRecord&&g_exceptions++<4) {
        char name[65]{};wchar_t message[193]{};
        const bool managed=copy_managed(error->ExceptionRecord,name,message);
        LOGF("[Startup] %s exception=0x%08lX managed=%s message=%ls; continuing original exception search",
             boundary,error->ExceptionRecord->ExceptionCode,managed?name:"unavailable",managed?message:L"");
        if(managed) {
            const auto trace=copy_trace(error->ExceptionRecord);
            LOGF("[Startup] %s managed-trace valid=%d total=%u copied=%u",boundary,trace.valid,trace.total,trace.count);
            for(unsigned i=0;i<trace.count;++i)LOGF("[Startup] %s managed-frame=%u method-rva=0x%08X",boundary,i,trace.rvas[i]);
            if(std::strcmp(boundary,"SceneBoot.CreateInstance")==0)record_singletons("CreateInstance fault");
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
void start(void* self,const void* method)
{
    LOGF("[Startup] SceneBoot.Start begin");
    __try {o_start(self,method);}
    __except(exception_filter("SceneBoot.Start",GetExceptionInformation())) {}
    LOGF("[Startup] SceneBoot.Start completed");
}
void create(void* self,const void* method)
{
    LOGF("[Startup] SceneBoot.CreateInstance begin");
    record_singletons("CreateInstance before");
    __try {o_create(self,method);}
    __except(exception_filter("SceneBoot.CreateInstance",GetExceptionInformation())) {}
    LOGF("[Startup] SceneBoot.CreateInstance completed");
    record_singletons("CreateInstance completed");
}
void resource(void* self,const void* method)
{
    LOGF("[Startup] ResourceManager.Initialize begin");
    __try {o_resource(self,method);}
    __except(exception_filter("ResourceManager.Initialize",GetExceptionInformation())) {}
    LOGF("[Startup] ResourceManager.Initialize completed");
}
int install()
{
    prepare_watches();
    return (hooks::attach(mStart,&start,&o_start)?1:0)+
           (hooks::attach(mCreate,&create,&o_create)?1:0)+
           (hooks::attach(mResource,&resource,&o_resource)?1:0);
}
core::Install s_install{"StartupDiagnostics",&install,25};
}
}
