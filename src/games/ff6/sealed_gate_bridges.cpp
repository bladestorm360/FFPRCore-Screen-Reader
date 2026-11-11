// The two distinct native callbacks finish their real phase/actor updates before
// notifying an explicitly armed guide. Copy eight existing ints, no lazy getter,
// hierarchy search, timer write, native pointer retention or worker native call.
#include "sealed_gate_bridges.h"
#include "../../core/hooks.h"
#include "../../core/frame.h"
#include "../../core/unity.h"
namespace ffpr::ff6::story {
namespace {
ref::Class kBridgeCycle{"Last.Interpreter.Instructions.SystemCall","Current.<>c",game::kFF6};
ref::Method mBridgeA{kBridgeCycle,"<SetGimmickTimerEvent0443>b__734_0",0};
ref::Method mBridgeB{kBridgeCycle,"<SetGimmickTimerEvent0444>b__737_0",0};
ref::Class kBridgeStorage{"Last.Interpreter","DataStorage",game::kFF6};
ref::Field fBridgeMap{kBridgeStorage,"<currentMapId>k__BackingField"},fBridgeLocals{kBridgeStorage,"map"};
using BridgeCycle=void(*)(void*,const void*);
BridgeCycle o_bridge_a=nullptr,o_bridge_b=nullptr;
bool(*bridge_wanted)()=nullptr;void(*bridge_pulse)(int,int)=nullptr;
void after_bridge(int expected_mode) {
    if(!frame::on_main_thread()||!bridge_wanted||!bridge_wanted())return;
    hooks::guarded("FF6.SealedGate.Bridge",[=]{
        int mode=0,phase=-1;
        if(sealed_bridge_phase(mode,phase)&&mode==expected_mode&&bridge_pulse)bridge_pulse(mode,phase);
    });
}
void d_bridge_a(void* self,const void* mi){o_bridge_a(self,mi);after_bridge(1);}
void d_bridge_b(void* self,const void* mi){o_bridge_b(self,mi);after_bridge(2);}
}
bool sealed_bridge_phase(int& mode,int& phase) {
    if(!frame::on_main_thread()||fBridgeMap.get()<0||fBridgeLocals.get()<0)return false;
    void* storage=il2cpp::read_static_ptr(kBridgeStorage.get(),"instance");
    if(!kBridgeStorage.is(storage)||ref::rd<int32_t>(storage,fBridgeMap)!=407)return false;
    void* values=ref::rd<void*>(storage,fBridgeLocals);
    if(unity::array_length(values)!=8)return false;
    mode=unity::array_value<int32_t>(values,1);
    if(mode!=1&&mode!=2)return false;
    phase=unity::array_value<int32_t>(values,mode==1?2:3);
    return phase==0||phase==1;
}
bool install_sealed_bridges(bool(*wanted)(),void(*pulse)(int,int)) {
    bridge_wanted=wanted;bridge_pulse=pulse;
    const bool a=hooks::attach(mBridgeA,&d_bridge_a,&o_bridge_a);
    const bool b=hooks::attach(mBridgeB,&d_bridge_b,&o_bridge_b);
    return a&&b;
}
}
