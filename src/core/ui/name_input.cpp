// The native KeyboardManager opens Last.UI.KeyInput.InputPopup in all six PC
// games. Observe only its actual Unity InputField edits; no key polling, injected
// text, replacement keyboard or retained engine pointer on a worker.
#include "name_input_echo.h"
#include "../core.h"
#include "../hooks.h"
#include "../refs.h"
#include "../unity.h"
#include "../modtext.h"
#include "../say.h"
#include "preferences.h"

namespace ui::name_input {
namespace {
ref::Class kPopup{"Last.UI.KeyInput","InputPopup"};
ref::Class kField{"UnityEngine.UI","InputField"};
ref::Class kEvent{"UnityEngine","Event"};
ref::Method mOpen{kPopup,"Open",0},mKey{kField,"KeyPressed",1},mDeselect{kField,"OnDeselect",1};
ref::Method mCode{kEvent,"get_keyCode",0};
ref::Field fInput{kPopup,"inputField"},fText{kField,"m_Text"},fCaret{kField,"m_CaretSelectPosition"};
void* owner=nullptr; // native-owner identity; never dereferenced after a close
using Open=void(*)(void*,const void*);
using Key=int32_t(*)(void*,void*,const void*);
using Deselect=void(*)(void*,void*,const void*);
Open o_open;Key o_key;Deselect o_deselect;
void d_open(void* self,const void* mi) {
    o_open(self,mi);
    hooks::guarded("NameInput.Open",[=]{if(kPopup.is(self))owner=ref::rd<void*>(self,fInput);});
}
void d_deselect(void* self,void* event,const void* mi) {
    if(self==owner)owner=nullptr;
    o_deselect(self,event,mi);
}
int32_t d_key(void* self,void* event,const void* mi) {
    std::wstring before;int caret=-1,key=0;bool observe=false;
    hooks::guarded("NameInput.BeforeKey",[&]{
        if(self!=owner||!ui::prefs::speech_enabled()||!unity::active_in_hierarchy(self)||!kField.is(self))return;
        key=ref::call<int32_t>(mCode,event);
        if(key!=8&&(key<273||key>276))return;
        before=ref::rd_str_bounded(self,fText,128);
        caret=ref::rd<int32_t>(self,fCaret);observe=true;
    });
    const auto result=o_key(self,event,mi);
    hooks::guarded("NameInput.AfterKey",[&]{
        if(!observe||self!=owner)return;
        auto spoken=echo(before,ref::rd_str_bounded(self,fText,128),caret,ref::rd<int32_t>(self,fCaret),key);
        if(spoken==L" ")spoken=T(L"space");
        if(!spoken.empty())ffpr::say(spoken,true);
    });
    return result;
}
int install() {
    events::on(events::Kind::SceneLoaded,[]{owner=nullptr;});
    events::on(events::Kind::TitleScreen,[]{owner=nullptr;});
    int count=hooks::attach(mOpen,&d_open,&o_open)?1:0;
    count+=hooks::attach(mKey,&d_key,&o_key)?1:0;
    return count+(hooks::attach(mDeselect,&d_deselect,&o_deselect)?1:0);
}
core::Install s_install{"UI.NameInput",&install,117};
}
}
