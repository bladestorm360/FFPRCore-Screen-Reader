// One bounded native timer observation. The hidden room timer is a path-validity
// dependency, covered by the owner's pathfinding identification approval. No
// native timer is changed, and no pointer survives these owner-thread calls.
#include "zone_eater_roof.h"
#include "../../core/hooks.h"
#include "../../core/il2cpp.h"
#include "../../core/core.h"
#include "../../core/unity.h"
#include "../../core/frame.h"
#include <cmath>
#include <cwchar>
#include <iterator>
#include <atomic>

namespace ffpr::ff6::story {
namespace {
ref::Class kRoofManager{"Last.Management","TimerInstanceManager",game::kFF6};
ref::Class kRoofGlobal{"Last.Timer","GrobalTimer",game::kFF6};
ref::Class kRoofStructure{"Last.Timer","TimerStructure",game::kFF6};
ref::Class kRoofClock{"Last.Timer","Timer",game::kFF6};
ref::Field fRoofGlobal{kRoofManager,"<GrobalTimer>k__BackingField"};
ref::Field fRoofScripts{kRoofGlobal,"<ScriptList>k__BackingField"};
ref::Field fRoofClock{kRoofStructure,"<Timer>k__BackingField"};
ref::Field fRoofRequired{kRoofClock,"requiredSec"},fRoofElapsed{kRoofClock,"elapsedSec"},fRoofFinished{kRoofClock,"isFinished"};
ref::Method mRoofSet{kRoofManager,"SetGrobalTimer",4};
using SetTimer=void(*)(void*,float,void*,void*,bool,const void*);
SetTimer o_roof_set=nullptr;
void(*g_roof_reset)()=nullptr;
bool(*g_roof_wanted)()=nullptr;
std::atomic<bool> g_roof_available{false};
bool roof_script(void* scripts) {
    if(unity::array_length(scripts)!=1)return false;
    void* name=unity::array_at(scripts,0);wchar_t text[16]{};
    if(!name||il2cpp::read_field<int32_t>(name,il2cpp::kStringLengthOff)!=9)return false;
    return il2cpp::read_string(name,text,std::size(text))&&std::wcscmp(text,L"sc_e_0850")==0;
}
void d_roof_set(void* self,float seconds,void* states,void* scripts,bool visible,const void* mi) {
    o_roof_set(self,seconds,states,scripts,visible,mi);
    if(!frame::on_main_thread())return;
    // Most game timers never enter the observation path. The caller only asks
    // for a notification while one explicitly requested roof leg is waiting.
    hooks::guarded("FF6.Roof.TimerReset",[&] {
        if(g_roof_wanted&&g_roof_wanted()&&seconds==5.0f&&roof_script(scripts)&&g_roof_reset)g_roof_reset();
    });
}
}
void capture_roof_timer(Snapshot& s) {
    if(s.map!=504||!g_roof_available.load(std::memory_order_acquire)||!frame::on_main_thread())return;
    if(fRoofGlobal.get()<0||fRoofScripts.get()<0||fRoofClock.get()<0||fRoofRequired.get()<0||fRoofElapsed.get()<0||fRoofFinished.get()<0)return;
    // Read the existing static field; do not invoke a lazy singleton getter.
    void* manager=il2cpp::read_static_ptr(kRoofManager.get(),"instance");
    if(!kRoofManager.is(manager))return;
    void* global=ref::rd<void*>(manager,fRoofGlobal);
    if(!kRoofGlobal.is(global)||!roof_script(ref::rd<void*>(global,fRoofScripts)))return;
    void* timer=ref::rd<void*>(global,fRoofClock);if(!kRoofClock.is(timer))return;
    const float required=ref::rd<float>(timer,fRoofRequired),elapsed=ref::rd<float>(timer,fRoofElapsed);
    if(required!=5.0f||!std::isfinite(elapsed)||elapsed<0||elapsed>required||ref::rd<bool>(timer,fRoofFinished))return;
    s.roof_seconds=required-elapsed;s.roof_frozen=queries::is("TimerHelper.TimersFrozen");
}
bool install_roof_timer(void(*reset)(),bool(*wanted)()) {
    g_roof_reset=reset;g_roof_wanted=wanted;
    const bool available=hooks::attach(mRoofSet,&d_roof_set,&o_roof_set);
    g_roof_available.store(available,std::memory_order_release);return available;
}
}
