// The puzzle item gates open SelectFieldContentManager, not the ordinary
// ItemWindowController. Observe its ready native selection and Close lifetime
// so Shift+G/I and generic cursor speech use the actual modal owner.
// Exact ABI/body/field evidence: tools/re/ff4_event_item_contract.py and
// docs/re/FF4_PUZZLE_GUIDES_20261101.md. No game pointer leaves this owner.
#include <string>
#include "../../core/core.h"
#include "../../core/hooks.h"
#include "../../core/refs.h"
#include "../../core/say.h"
#include "../../core/modtext.h"
#include "../../core/text.h"
#include "../../core/ui_util.h"
#include "../../core/unity.h"
#include "../../core/status/status.h"
#include "../../core/system/cursor_event.h"
#include "../../core/ui/preferences.h"
#include "../../core/field/scene_events.h"
namespace ff4::event_items {
namespace {
ref::Class base{"Last.UI","SelectFieldContentControllerBase",game::kFF4};
ref::Field contents{base,"contentDataList"};
ref::Field description_enabled{base,"isEnableDescription"};
ref::Class controller_type{"Last.UI.KeyInput","SelectFieldContentController",game::kFF4};
ref::Method select_method{controller_type,"SelectContent",2};
ref::Class manager_type{"Last.UI","SelectFieldContentManager",game::kFF4};
ref::Field manager_controller{manager_type,"controller"};
ref::Method close_method{manager_type,"Close",0};
ref::Class row_type{"Last.UI","SelectFieldContentManager.SelectFieldContentData",game::kFF4};
ref::Field name_id{row_type,"<NameMessageId>k__BackingField"};
ref::Field description_id{row_type,"<DescriptionMessageId>k__BackingField"};
void* owner=nullptr;
std::wstring description;
void clear(){owner=nullptr;description.clear();menu_state::set_active(menu_state::EVENT_ITEM_SELECT,false);}
bool active(){
 if(!owner||!menu_state::is_active(menu_state::EVENT_ITEM_SELECT))return false;
 void* manager=ref::instance(manager_type);
 if(!manager||!unity::alive(owner)||!unity::active_in_hierarchy(owner)||ref::rd<void*>(manager,manager_controller)!=owner){clear();return false;}
 return true;
}
void selected(void* self,int index){
 // One ready native row, finite count and bounded message identifiers. Each
 // native selection remains an announcement, even if its text is unchanged.
 clear();if(!self||index<0||!controller_type.is(self)||!unity::active_in_hierarchy(self))return;
 void* manager=ref::instance(manager_type);if(!manager||ref::rd<void*>(manager,manager_controller)!=self)return;
 void* list=ref::rd<void*>(self,contents);const int count=unity::list_count(list);
 if(count<=0||count>512||index>=count)return;
 void* row=unity::list_at(list,index);if(!row||!row_type.is(row))return;
 const auto key=ref::rd_str_bounded(row,name_id,128);if(key.empty())return;
 auto name=text::trim(text::strip_markup(ffpr::status::game_message(key,true)));if(name.empty())return;
 // The native controller can deliberately hide the description panel. Its
 // row data alone does not grant permission to disclose that hidden text.
 if(description_enabled.get()>=0&&ref::rd<bool>(self,description_enabled)) {
  const auto detail=ref::rd_str_bounded(row,description_id,128);
  if(!detail.empty())description=text::trim(text::strip_markup(ffpr::status::game_message(detail,true)));
 }
 owner=self;menu_state::set_active(menu_state::EVENT_ITEM_SELECT,true);
 if(ui::prefs::auto_detail_enabled()&&!description.empty())name+=L": "+description;
 ffpr::say(ui::menu_position(name,index,count),true);
}
using Select=void(*)(void*,int32_t,int32_t,const void*);Select original_select=nullptr;
using Close=void(*)(void*,const void*);Close original_close=nullptr;
void select_hook(void* self,int32_t index,int32_t scroll,const void* method){
 original_select(self,index,scroll,method);hooks::guarded("FF4.EventItemSelect.Ready",[&]{selected(self,index);});
}
void close_hook(void* self,const void* method){original_close(self,method);hooks::guarded("FF4.EventItemSelect.Close",[]{clear();});}
int install(){
 int n=hooks::attach(select_method,&select_hook,&original_select)?1:0;n+=hooks::attach(close_method,&close_hook,&original_close)?1:0;
 queries::define("EventItemSelectState.IsActive",[]{return active();});queries::define("EventItemSelectState.ShouldSuppress",[]{return active();});
 cursor::add_suppressor("EventItemSelectState.ShouldSuppress");
 actions::define("EventItemSelect.ReadDetails",[]{if(active())ffpr::say(description.empty()?T(L"No description"):description,true);});
 for(auto event:{events::Kind::MapChanging,events::Kind::MapChanged})events::on(event,[]{clear();});
 events::on(events::Kind::SceneLoaded,[]{if(field::scene_name()==L"Title")clear();});return n;
}
core::Install registration{"FF4.EventItemSelect",&install,502,game::kFF4};
}
}
