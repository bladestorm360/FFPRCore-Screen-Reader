// Actual singleton producer; fake runtime only. Native first-lookup cache
// semantics are established separately from the real FFIII getter disassembly.
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "../src/core/refs.cpp"
namespace fixture {
game::Id id=game::Id::FF3;int invokes=0;bool ready=false,searched=false,present=false,method_missing=false;
std::vector<std::string> lines;int token=0;
}
namespace game {Id current(){return fixture::id;}const char* label(){return "fixture";}}
namespace log_ {void write(const char*fmt,...){char line[1024];va_list args;va_start(args,fmt);vsnprintf(line,sizeof(line),fmt,args);va_end(args);fixture::lines.emplace_back(line);}}
namespace il2cpp {
Class* find_class(const char*,const char*,const char* name){return std::strcmp(name,"Missing")==0?nullptr:&fixture::token;}
bool is_a(Object* object,Class*){return object!=nullptr;}
int32_t field_offset(Class*,const char*){return 0;}
Method* find_method(Class*,const char*,int){return fixture::method_missing?nullptr:&fixture::token;}
void* method_pointer(Method*){return nullptr;}
String* new_string(const wchar_t*){return nullptr;}
void* invoke_ptr(Method*,void*,void**,int){++fixture::invokes;if(!fixture::searched){fixture::present=fixture::ready;fixture::searched=true;}return fixture::present?&fixture::token:nullptr;}
}
ref::Class observed{"Last.Management","ResourceManager"},missing{"Last.Management","Missing"};
int main(){
 int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 timing::frame=0;ref::g_boot_lookup_records=0;
 expect(ref::instance(observed)==nullptr&&fixture::invokes==1,"cold native singleton result is preserved with exactly one original call");
 expect(fixture::lines.size()==2&&fixture::lines[0].find("mod singleton request=1 class=Last.Management.ResourceManager")!=std::string::npos&&fixture::lines[1].find("instance-present=0")!=std::string::npos,
  "before-first-frame producer publishes the actual requested type and null result");
 fixture::ready=true;
 expect(ref::instance(observed)==nullptr&&fixture::invokes==2,"diagnostics never retry/reset/fabricate the native getter's cached null");
 fixture::searched=false;
 expect(ref::instance(observed)==&fixture::token&&fixture::invokes==3,"already-created native manager result is returned unchanged");
 for(int i=0;i<40;++i)ref::instance(observed);
 expect(fixture::lines.size()==32&&fixture::invokes==43,"only16 early observations are logged while every native getter continues normally");
 const auto count=fixture::lines.size();timing::frame=1;
 for(int i=0;i<1000;++i)ref::instance(observed);
 expect(fixture::lines.size()==count&&fixture::invokes==1043,"normal running input frames add no startup logs");
 timing::frame=0;ref::g_boot_lookup_records=0;fixture::id=game::Id::FF2;
 ref::instance(observed);expect(fixture::lines.size()==count,"unverified other games receive no early-lookup trace");
 fixture::id=game::Id::FF3;ref::g_boot_lookup_records=0;fixture::method_missing=true;
 const int before=fixture::invokes;expect(ref::instance(observed)==nullptr&&fixture::invokes==before,"missing getter retains the original no-invoke result");
 expect(ref::instance(missing)==nullptr&&fixture::invokes==before,"missing class retains the original no-invoke result");
 std::printf("Actual early singleton producer: %d failures; no game runtime\n",failures);return failures?1:0;
}
