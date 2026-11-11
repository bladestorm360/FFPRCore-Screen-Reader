// Matching FFVI53F760 callback shows the old collider, hides the next, then
// writes map-local3. Native tables85/86/87 correspond to left/middle/right.
// Only an explicitly armed guide receives an owned notification after phase0.
#include "owzer_doors.h"
#include "../../core/hooks.h"
#include "../../core/frame.h"
#include "../../core/unity.h"
namespace ffpr::ff6::story {
namespace {
ref::Class kCycle{"Last.Interpreter.Instructions.SystemCall","Current.<>c",game::kFF6};
ref::Method mCycle{kCycle,"<SetGimmickTimerOwzer>b__742_0",0};
ref::Class kDoorStorage{"Last.Interpreter","DataStorage",game::kFF6};
ref::Field fDoorMap{kDoorStorage,"<currentMapId>k__BackingField"},fDoorLocals{kDoorStorage,"map"};
using Cycle=void(*)(void*,const void*);
Cycle o_cycle=nullptr;
bool(*g_wanted)()=nullptr;void(*g_opened)()=nullptr;
void d_cycle(void* self,const void* mi) {
    o_cycle(self,mi);
    if(!frame::on_main_thread()||!g_wanted||!g_wanted())return;
    hooks::guarded("FF6.Owzer.LeftDoor",[]{
        // Read the already existing singleton and exactly eight owned ints.
        // Never invoke a lazy getter or retain a native storage/array pointer.
        if(fDoorMap.get()<0||fDoorLocals.get()<0)return;
        void* storage=il2cpp::read_static_ptr(kDoorStorage.get(),"instance");
        if(!kDoorStorage.is(storage)||ref::rd<int32_t>(storage,fDoorMap)!=445)return;
        void* values=ref::rd<void*>(storage,fDoorLocals);
        if(unity::array_length(values)!=8||unity::array_value<int32_t>(values,2)!=1||
           unity::array_value<int32_t>(values,3)!=0)return;
        if(g_opened)g_opened();
    });
}
}
bool install_owzer_doors(bool(*wanted)(),void(*opened)()) {
    g_wanted=wanted;g_opened=opened;return hooks::attach(mCycle,&d_cycle,&o_cycle);
}
}
