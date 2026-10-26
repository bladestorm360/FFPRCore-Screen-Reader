// Fat Chocobo/shared warehouse UI. Native state and exact serialized fields own
// command/list/quantity focus; no inventory reads, writes or scene discovery.
// Evidence, controls, lifecycle and bounds: docs/re/WAREHOUSE_20261026.md.
#include "../core.h"
#include "../refs.h"
#include "../hooks.h"
#include "../frame.h"
#include "../displayed_ui.h"
#include "../unity.h"
#include "../ui_util.h"
#include "../modtext.h"
#include "../say.h"
#include "../text.h"
#include "../ui/preferences.h"
#include "../system/system.h"
#include "../system/cursor_event.h"
#include "../dialogue/dialogue.h"

namespace ffpr::menus::warehouse {
namespace {
ref::Class kWindow{"Last.UI.KeyInput","WarehouseController"};
ref::Class kBase{"Last.UI","WarehouseControlleBase"}; // native spelling
ref::Field fView{kWindow,"view"},fCommandCursor{kWindow,"selectCommandCursor"};
ref::Field fState{kBase,"stateMachine"},fCursor{kBase,"selectContentCursor"},fData{kBase,"contentDataList"};
ref::Class kScroll{"Last.UI","InfiniteScrollView"};
ref::Method mContent{kScroll,"GetContentObject",1};
ref::Class kItem{"Last.UI.KeyInput","ItemListContentController"};
ref::Class kPopup{"Last.UI.KeyInput","WarehousePopupController"};
ref::Class kPopupBase{"Last.UI","WarehousePopupControllerBase"};
ref::Field fPopupView{kPopup,"view"},fCount{kPopupBase,"selectedCount"};
ref::Method mOpen{kWindow,"Open",2},mClose{kWindow,"Close",0};
ref::Method mCommand{kWindow,"SelectCommand",1},mSelect{kWindow,"SelectContent",2},mUpdate{kWindow,"ContentUpdate",2};
ref::Method mCommandInit{kWindow,"SelectingCommandInit",0},mStoreInit{kWindow,"SelectingEntrustTargetInit",0},mRetrieveInit{kWindow,"SelectingWithdrawTargetInit",0};
ref::Method mAdd{kPopupBase,"AddSelectedCount",1},mTake{kPopupBase,"TakeSelectedCount",1};
void* owner=nullptr;
unsigned generation=0;
bool opening=false,entry_pending=false;
constexpr int kListBound=4096,kCommandBound=8;

void* view(){return ref::rd<void*>(owner,fView);}
int phase(){return owner?ui::state_tag(owner,fState):0;}
bool active(){
    if(!owner||!kWindow.is(owner)||!unity::alive(owner))return false;
    if(opening)return true; // claim before nested native cursor creation
    const int tag=phase();
    return tag>0&&tag<=10&&displayed::container_visible(displayed::field(view(),"rootObject"));
}
bool list_phase(int tag){return tag==3||tag==4||tag==5||tag==6;}
std::wstring quantity_value(void* popup){
    if(!kPopup.is(popup)||!unity::alive(popup))return {};
    void* panel=ref::rd<void*>(popup,fPopupView);
    if(!displayed::container_visible(panel))return {};
    const auto number=displayed::text_field(panel,"targetCountText");
    if(number.empty())return {};
    const auto label=displayed::text_field(panel,"targetCountNameText");
    return label.empty()?number:label+L": "+number;
}
std::wstring popup_line(){
    void* popup=dialogue::popup_state::active_popup();
    const auto amount=quantity_value(popup);if(amount.empty())return {};
    void* panel=ref::rd<void*>(popup,fPopupView);
    std::vector<std::wstring> parts{displayed::text_field(panel,"commentText"),
        displayed::text_field(displayed::field(panel,"targetItemView"),"nameText")};
    const auto stock=displayed::text_field(panel,"possessionCountText");
    const auto label=displayed::text_field(panel,"fixedPossessionText");
    if(!stock.empty())parts.push_back(label.empty()?stock:label+L": "+stock);
    parts.push_back(amount);return text::join(parts,L". ");
}
void* selected_data(){
    void* cursor=ref::rd<void*>(owner,fCursor);void* list=ref::rd<void*>(owner,fData);
    const int count=unity::list_count(list),index=ui::cursor_index(cursor);
    return count>0&&count<=kListBound&&index>=0&&index<count?unity::list_at(list,index):nullptr;
}
void* selected_object(){
    if(!selected_data())return nullptr;
    return ref::call<void*>(mContent,displayed::field(view(),"scrollView"),ui::cursor_index(ref::rd<void*>(owner,fCursor)));
}
void read_focus(){
    if(!active())return;
    const int tag=phase();
    std::vector<std::wstring> parts;
    if(tag==1||tag==2){
        void* commands=displayed::field(view(),"commandContentList");
        const int count=unity::list_count(commands),index=ui::cursor_index(ref::rd<void*>(owner,fCommandCursor));
        if(count<1||count>kCommandBound||index<0||index>=count)return;
        auto label=displayed::text_field(displayed::field(unity::list_at(commands,index),"view"),"text");
        if(label.empty())return;
        if(entry_pending){
            parts.push_back(displayed::text_field(view(),"titleText"));
            parts.push_back(displayed::text_field(view(),"commentText"));entry_pending=false;
        }
        parts.push_back(ui::menu_position(label,index,count));
        parts.push_back(displayed::text_field(view(),"noContensMessageText"));
    }else if(list_phase(tag)){
        const auto empty=displayed::text_field(view(),"noContensMessageText");
        if(!empty.empty()){ffpr::say(empty,true);return;}
        void* row=unity::get_component(selected_object(),kItem);
        void* row_view=displayed::field(row,"view");
        if(!displayed::container_visible(row_view))return;
        auto name=displayed::text_field(displayed::field(row_view,"IconTextView"),"nameText");
        if(name.empty())return; // wait for ContentUpdate, never poll hidden data
        const auto count=displayed::text_field(row_view,"countText");
        if(!count.empty())name+=L", "+TF(L"quantity {0}",{count});
        parts.push_back(std::move(name));
        if(ui::prefs::auto_detail_enabled())parts.push_back(displayed::text_field(view(),"descriptionText"));
    }else return; // quantity popup owns its independent typed reader
    ffpr::say_sequence(std::move(parts),true);
}
void schedule(){
    const auto token=++generation;frame::cancel_label("Warehouse.Focus");
    frame::next_frame([token]{if(token==generation)hooks::guarded("Warehouse.ReadFocus",[]{read_focus();});},"Warehouse.Focus");
}
void retire(){
    ++generation;frame::cancel_label("Warehouse.Focus");owner=nullptr;opening=entry_pending=false;
}
using Open=void(*)(void*,bool,void*,const void*);Open o_open;
void d_open(void* self,bool vegetable,void* cancel,const void* mi){
    hooks::guarded("Warehouse.Claim",[=]{if(kWindow.is(self)){retire();owner=self;opening=entry_pending=true;}});
    o_open(self,vegetable,cancel,mi);
    hooks::guarded("Warehouse.Opened",[=]{if(owner==self){opening=false;schedule();}});
}
using Void=void(*)(void*,const void*);Void o_close,o_command_init,o_store_init,o_retrieve_init;
void d_close(void* self,const void* mi){if(owner==self){retire();system::cancel_controls_reading();}o_close(self,mi);}
void d_command_init(void* self,const void* mi){o_command_init(self,mi);hooks::guarded("Warehouse.CommandReady",[=]{if(owner==self)schedule();});}
void d_store_init(void* self,const void* mi){o_store_init(self,mi);hooks::guarded("Warehouse.StoreReady",[=]{if(owner==self)schedule();});}
void d_retrieve_init(void* self,const void* mi){o_retrieve_init(self,mi);hooks::guarded("Warehouse.RetrieveReady",[=]{if(owner==self)schedule();});}
using Command=void(*)(void*,int32_t,const void*);Command o_command;
void d_command(void* self,int32_t index,const void* mi){o_command(self,index,mi);hooks::guarded("Warehouse.CommandFocus",[=]{if(owner==self)schedule();});}
using Select=void(*)(void*,int32_t,int32_t,const void*);Select o_select;
void d_select(void* self,int32_t index,int32_t scroll,const void* mi){
    o_select(self,index,scroll,mi);
    hooks::guarded("Warehouse.ItemFocus",[=]{if(owner==self&&list_phase(phase()))schedule();});
}
using Update=void(*)(void*,void*,void*,const void*);Update o_update;
void d_update(void* self,void* data,void* object,const void* mi){
    o_update(self,data,object,mi);
    hooks::guarded("Warehouse.RenderedItem",[=]{
        // Virtual-scroll callbacks for unrelated pooled rows cannot repeat the
        // focused row. Match native model AND rendered object identity.
        if(owner==self&&list_phase(phase())&&selected_data()==data&&selected_object()==object)schedule();
    });
}
using Count=void(*)(void*,void*,const void*);Count o_add,o_take;
void changed_count(void* self,int before){
    if(dialogue::popup_state::active_popup()!=self||ref::rd<int>(self,fCount)==before)return;
    const auto line=quantity_value(self);if(!line.empty())ffpr::say(line,true);
}
void d_add(void* self,void* text,const void* mi){
    const int before=ref::rd<int>(self,fCount);o_add(self,text,mi);
    hooks::guarded("Warehouse.QuantityIncrease",[=]{changed_count(self,before);});
}
void d_take(void* self,void* text,const void* mi){
    const int before=ref::rd<int>(self,fCount);o_take(self,text,mi);
    hooks::guarded("Warehouse.QuantityDecrease",[=]{changed_count(self,before);});
}
int install(){
    queries::define("Warehouse.IsActive",[]{return active();});
    queries::define_text("Warehouse.QuantityPopup",[]{return popup_line();});
    actions::define("Warehouse.ReadDetails",[]{
        if(!active())return;
        if(phase()>=7){const auto line=popup_line();if(!line.empty())ffpr::say(line,true);}
        else {const auto line=displayed::text_field(view(),"descriptionText");if(!line.empty())ffpr::say(line,true);}
    });
    actions::define("Warehouse.ReadControls",[]{if(active())system::announce_controls_for(displayed::field(view(),"keyHelpController"));});
    cursor::add_suppressor("Warehouse.IsActive");
    events::on(events::Kind::SceneLoaded,[]{retire();});events::on(events::Kind::TitleScreen,[]{retire();});
    return (hooks::attach(mOpen,&d_open,&o_open)?1:0)+(hooks::attach(mClose,&d_close,&o_close)?1:0)+
        (hooks::attach(mCommand,&d_command,&o_command)?1:0)+(hooks::attach(mSelect,&d_select,&o_select)?1:0)+
        (hooks::attach(mUpdate,&d_update,&o_update)?1:0)+(hooks::attach(mCommandInit,&d_command_init,&o_command_init)?1:0)+
        (hooks::attach(mStoreInit,&d_store_init,&o_store_init)?1:0)+(hooks::attach(mRetrieveInit,&d_retrieve_init,&o_retrieve_init)?1:0)+
        (hooks::attach(mAdd,&d_add,&o_add)?1:0)+(hooks::attach(mTake,&d_take,&o_take)?1:0);
}
core::Install s_install{"Warehouse",&install,119};
}
}
