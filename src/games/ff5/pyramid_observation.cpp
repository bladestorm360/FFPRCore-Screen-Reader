// Two exact native floor-change producers. Copy only a scalar phase boundary;
// guide capture/routing remains on its existing ready continuation and worker.
#include "pyramid_model.h"
#include "../../core/core.h"
#include "../../core/frame.h"
#include "../../core/hooks.h"
#include "../../core/field/field_internal.h"
namespace ff5::pyramid {
namespace {
ref::Class kCurrent{"Last.Interpreter.Instructions.SystemCall","Current",game::kFF5};
ref::Method mPhase0{kCurrent,"SwitchPyramid8FGimmick1",0},mPhase1{kCurrent,"SwitchPyramid8FGimmick2",0};
using Phase=void(*)(const void*);Phase original0=nullptr,original1=nullptr;
int observed_phase=-1;double started=0;
void observed(int phase){
 observed_phase=phase;started=frame::now();
 if(field::current_map_id()==424&&queries::is("GameModule.PuzzleGuideActive"))actions::run("GameModule.FieldStateChanged");
}
void phase0(const void* m){original0(m);hooks::guarded("FF5.Pyramid.Phase0",[]{observed(0);});}
void phase1(const void* m){original1(m);hooks::guarded("FF5.Pyramid.Phase1",[]{observed(1);});}
int install(){int n=0;n+=hooks::attach(mPhase0,&phase0,original0);n+=hooks::attach(mPhase1,&phase1,original1);return n;}
core::Install installed{"FF5.PyramidPhases",&install,517,game::kFF5};
}
bool phase_fresh(int map,int phase){
 // Each native phase lasts three seconds. A new crossing is offered only in
 // its first second,leaving two seconds for the short spoken/cardinal leg.
 // Native/menu pauses can only make this conservatively wait for a fresh
 // boundary;the explicit frozen-timer preference is handled by the caller.
 const double age=frame::now()-started;return map==424&&phase==observed_phase&&age>=0&&age<=1;
}
}
