// Actual boot wrappers; fake native operations only, never launches a game.
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "../src/core/system/startup_diagnostics.cpp"
namespace fixture {int calls=0;std::vector<std::string> lines;}
struct ManagedClass {void* image=nullptr;void* gc=nullptr;const char* name="NullReferenceException";};
struct ManagedText {void*klass=nullptr;void*monitor=nullptr;int length=12;wchar_t value[193]=L"boot failure";};
struct ManagedError {ManagedClass*klass;void*monitor=nullptr;void*class_name=nullptr;ManagedText*message;};
struct NativeWrapper {ManagedError*error;};
ManagedClass managed_class;ManagedText managed_text;ManagedError managed_error{&managed_class,nullptr,nullptr,&managed_text};
namespace core {Install::Install(const char*n,int(*f)(),int o,uint8_t g):name(n),fn(f),order(o),games(g){}}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
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
    expect(caught&&fixture::lines.size()==2&&fixture::lines.back().find("NullReferenceException")!=std::string::npos,"real MSVC exception propagates unchanged after its diagnostic prefix is copied");
    std::printf("Actual boot diagnostics: original calls/error propagation/capacity, %d failures\n",failures);return failures?1:0;
}
