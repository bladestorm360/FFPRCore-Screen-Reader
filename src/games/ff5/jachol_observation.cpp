// Exact original-first scalar phase boundaries. No scan,flag read,worker wait
// or native pointer leaves this owner. The accepted guide owner coalesces work.
#include "jachol_model.h"
#include "../../core/core.h"
#include "../../core/frame.h"
#include "../../core/hooks.h"
#include "../../core/field/field_internal.h"
namespace ff5::jachol {
namespace {
ref::Class current{"Last.Interpreter.Instructions.SystemCall","Current",game::kFF5};
ref::Method set_switch{current,"SetSwitchJacholCave",0},reset_switch{current,"ResetSwitchJacholCave",0};
using Phase=void(*)(const void*);Phase original_set=nullptr,original_reset=nullptr;
int phase=-1;double started=0;
void observed(int value){phase=value;started=frame::now();if(field::current_map_id()==284&&queries::is("GameModule.PuzzleGuideActive"))actions::run("FF5.Jachol.PhaseChanged");}
void on_set(const void* m){original_set(m);hooks::guarded("FF5.Jachol.Show",[]{observed(1);});}
void on_reset(const void* m){original_reset(m);hooks::guarded("FF5.Jachol.Hide",[]{observed(0);});}
int install(){int n=0;n+=hooks::attach(set_switch,&on_set,original_set);n+=hooks::attach(reset_switch,&on_reset,original_reset);return n;}
core::Install installed{"FF5.JacholPhases",&install,518,game::kFF5};
}
bool phase_fresh(int map,int wanted){return map==284&&phase==wanted&&frame::now()>=started&&frame::now()-started<1.;}
}
