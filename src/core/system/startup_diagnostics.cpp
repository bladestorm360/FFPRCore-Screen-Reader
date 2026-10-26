// Bounded boot-boundary diagnostics. Unity disables its own Player.log in these
// releases, so an exception before Splash otherwise leaves only a healthy input
// heartbeat. Observe the original operation; never consume or repair its error.
#include <windows.h>
#include <algorithm>
#include <cstdint>
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
    __try {o_create(self,method);}
    __except(exception_filter("SceneBoot.CreateInstance",GetExceptionInformation())) {}
    LOGF("[Startup] SceneBoot.CreateInstance completed");
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
    return (hooks::attach(mStart,&start,&o_start)?1:0)+
           (hooks::attach(mCreate,&create,&o_create)?1:0)+
           (hooks::attach(mResource,&resource,&o_resource)?1:0);
}
core::Install s_install{"StartupDiagnostics",&install,25};
}
}
