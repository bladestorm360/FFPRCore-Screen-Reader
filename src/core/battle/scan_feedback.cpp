#include "scan_feedback.h"
#include "../core.h"
#include "../game.h"
#include "../hooks.h"
#include "../refs.h"
#include <cstdint>

namespace ffpr::battle::scan_feedback {
namespace {
ref::Class kScan{"Last.Battle.Function", "ScanBattleFunction", game::kFF3};
ref::Method mIsFunctionEnd{kScan, "IsFunctionEnd", 0};
ref::Field fFirst{kScan, "isFirstProcess"};
ref::Field fIndex{kScan, "messageDisplayCount"};
using EndFn=bool (*)(void*,const void*);
EndFn original=nullptr;

// Identity is opaque and kept only to recognize one active/recent producer.
// Its address is never dereferenced after the current native invocation.
struct Event { void* owner=nullptr; int index=-1; bool accepted=false; };
struct Source { void* owner=nullptr; int index=-1; bool valid=false; };
thread_local Event event;
thread_local Source source;
struct Observation { bool valid=false, first=false; int index=-1; };

// A bad native pointer or missing metadata must fall back to ordinary speech.
// Raw byte reads let us distinguish a fault from a legitimate index zero.
bool copy_fields(void* self,int first_offset,int index_offset,Observation* out) {
    if(!self||first_offset<0||index_offset<0||!out)return false;
    __try {
        const auto* p=static_cast<const uint8_t*>(self);
        const uint8_t first=*(p+first_offset);
        const int32_t index=*reinterpret_cast<const int32_t*>(p+index_offset);
        if(first>1||index<0)return false;
        out->first=first!=0;out->index=index;out->valid=true;return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
struct SourceScope {
    Source prior=source;
    SourceScope(void* owner,const Observation& observation) {
        source={owner,observation.index,observation.valid};
        if(!observation.valid) event={};
        if(observation.valid && (event.owner!=owner || observation.first || event.index!=observation.index))
            event={owner,observation.index,false};
    }
    ~SourceScope(){source=prior;}
};
void reset(){event={};}
bool detour(void* self,const void* mi) {
    Observation observed;
    hooks::guarded("Battle.ScanObserve",[&] {
        if(game::is(game::Id::FF3))copy_fields(self,fFirst.get(),fIndex.get(),&observed);
    });
    SourceScope scope(self,observed);
    // Native function and its exact bool result remain authoritative. Scope is
    // stack-owned so nested calls and exceptions restore the previous source.
    const bool done=original(self,mi);
    if(done&&observed.valid&&event.owner==self)reset();
    return done;
}
int install() {
    if(!game::is(game::Id::FF3))return 0;
    events::on(events::Kind::BattleEntering,[]{reset();});
    events::on(events::Kind::BattleEnd,[]{reset();});
    events::on(events::Kind::SceneLoaded,[]{reset();});
    return hooks::attach(mIsFunctionEnd,&detour,&original)?1:0;
}
core::Install registration{"Battle.ScanFeedback",&install,109,game::kFF3};
}
bool accept_message() {
    if(!source.valid)return true;
    if(event.owner!=source.owner||event.index!=source.index)return true;
    if(event.accepted)return false;
    event.accepted=true;return true;
}
}
