// Read the current native keyboard configuration only when opening mod settings.
// Native pointers never leave this owner call. Picker/capture admission uses a
// small owned array, adds no hook or per-frame read, and never changes game keys.
#include "bindings.h"
#include "native_keycodes.h"
#include "../core.h"
#include "../frame.h"
#include "../refs.h"
#include "../unity.h"
#include "../modtext.h"
#include <array>
#include <algorithm>
namespace input::native_keyboard {
namespace {
ref::Class kListener{"System.Input","InputListener"};
ref::Field fConfig{kListener,"<KeyConfig>k__BackingField"};
ref::Class kConfig{"System.Input.KeyConfig","KeyConfig"};
ref::Field fMappings{kConfig,"config"};
ref::Class kValue{"System.Input.KeyConfig","KeyValue"};
ref::Field fType{kValue,"inputType"};
ref::Field fDevice{kValue,"deviceType"};
ref::Field fKey{kValue,"keyCode"};
struct Binding {int code=0;std::wstring action;};
std::array<Binding,128> mapped;
size_t count=0;
bool known=false;
void refresh() {
    if(!frame::on_main_thread())return;
    known=false;count=0;
    if(!kListener.get()||!fConfig||!fMappings||!fType||!fDevice||!fKey)return;
    void* owner=il2cpp::read_static_ptr(kListener.get(),"instance");
    void* config=ref::rd<void*>(owner,fConfig);
    void* devices=ref::rd<void*>(config,fMappings);
    const int total=unity::dict_count(devices);
    if(!devices||total<1||total>8)return;
    void* keys=nullptr;
    for(int i=0;i<total;++i){uint64_t device=0,value=0;if(!unity::dict_entry(devices,i,device,value,4,8))continue;if(int(device)!=0)continue;if(keys)return;keys=reinterpret_cast<void*>(value);}
    const int actions=unity::dict_count(keys);if(!keys||actions<1||actions>64)return;
    for(int i=0;i<actions;++i) {
        uint64_t key=0,value=0;if(!unity::dict_entry(keys,i,key,value,8,8))continue;
        wchar_t name[65]{};if(!key||!il2cpp::read_string(reinterpret_cast<il2cpp::String*>(key),name,65)||!name[0])return;
        void* list=reinterpret_cast<void*>(value);const int n=unity::list_count(list);if(!list||n<1||n>16)return;
        for(int j=0;j<n;++j) {
            void* row=unity::list_at(list,j);if(!row||!kValue.is(row))return;
            if(ref::rd<int32_t>(row,fDevice)!=0||ref::rd<int32_t>(row,fType)!=1)return;
            const int code=ref::rd<int32_t>(row,fKey);if(code==0)continue;
            if(code<0||count==mapped.size())return;
            mapped[count++]={code,name};
        }
    }
    known=true;
}
bindings::NativeKeyboardResult check(const bindings::Entry& e,int primary,int modifier,std::wstring& conflict) {
    using Result=bindings::NativeKeyboardResult;
    // Modal keys are exclusive to mod UI. Original approved defaults include
    // native speed/encounter reports and owned information-buffer navigation;
    // preserve their deliberate sharing when restoring the original chord.
    if(e.scope<0||(primary==e.default_primary&&modifier==e.default_modifier))return Result::Clear;
    if(!known)return Result::Unavailable;
    const auto matches=[&](int code) {
        if(kUnityKeys[primary]>=0&&code==kUnityKeys[primary])return true;
        return ((modifier&1)&&(code==kUnityKeys[int(Key::LeftShift)]||code==kUnityKeys[int(Key::RightShift)]))||
            ((modifier&2)&&(code==kUnityKeys[int(Key::LeftControl)]||code==kUnityKeys[int(Key::RightControl)]))||
            ((modifier&4)&&(code==kUnityKeys[int(Key::LeftAlt)]||code==kUnityKeys[int(Key::RightAlt)]));
    };
    for(size_t i=0;i<count;++i)if(matches(mapped[i].code)){conflict=mapped[i].action;return Result::Conflict;}
    return Result::Clear;
}
int install() {
    bindings::native_keyboard_check(&check);
    actions::define("Input.RefreshKeyboardConflicts",&refresh);
    return 0;
}
core::Install s_install{"Input.NativeKeyboardConflicts",&install,59};
}
}
