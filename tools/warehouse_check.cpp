// check-sources: text.cpp ui/navigation_buffer.cpp
// Actual Warehouse reader plus actual rendered-field helpers/frame scheduler;
// reuse the native-owner boundary fixture without executing its separate main.
#define main retained_party_fixture_main
#include "party_esper_check.cpp"
#undef main
#include "../src/core/menus/warehouse.cpp"

namespace warehouse_fixture {
using namespace fixture;
namespace wh=ffpr::menus::warehouse;
void* popup=nullptr;void* controls=nullptr;std::vector<std::string> suppressors;
int originals=0;
struct Screen {
 Node window,view,root,bar,content_cursor,scroll,item,item_view,icon,model,footer;
 Node title=make_text(L"Fat Chocobo"),comment=make_text(L"Hello! What do you want?"),empty=make_text(L""),description=make_text(L"Restores 500 HP.");
 Node store,retrieve,exit,store_view,retrieve_view,exit_view;
 Node store_text=make_text(L"Store"),retrieve_text=make_text(L"Retrieve"),exit_text=make_text(L"Exit");
 Node name=make_text(L"Hi-Potion"),count=make_text(L"11");
 List commands,models,rows;
 Screen(){
  set(window,"view",&view);set(view,"rootObject",&root);set(window,"selectCommandCursor",&bar);set(window,"selectContentCursor",&content_cursor);
  set(view,"titleText",&title);set(view,"commentText",&comment);set(view,"noContensMessageText",&empty);set(view,"descriptionText",&description);set(view,"keyHelpController",&footer);
  set(store,"view",&store_view);set(retrieve,"view",&retrieve_view);set(exit,"view",&exit_view);
  set(store_view,"text",&store_text);set(retrieve_view,"text",&retrieve_text);set(exit_view,"text",&exit_text);
  commands={&store,&retrieve,&exit};set(view,"commandContentList",&commands);
  set(item,"view",&item_view);set(item_view,"IconTextView",&icon);set(icon,"nameText",&name);set(item_view,"countText",&count);
  models={&model};rows={&item};set(window,"contentDataList",&models);set(view,"scrollView",&scroll);set(scroll,"rows",&rows);
 }
}* current=nullptr;
void native_select(void*,int32_t,int32_t,const void*){++originals;native();}
void native_command(void* self,int32_t index,const void*){
 ++originals;native();current->bar.id=index;
 // Native command selection previews the inventory with a separate cursor.
 // The reader must keep the command as producer while state2 owns input.
 wh::d_select(self,0,0,nullptr);
}
void native_open(void* self,bool,void*,const void*){++originals;native();current->window.command=2;current->root.active=true;wh::d_command(self,0,nullptr);}
void native_close(void*,const void*){++originals;native();current->window.command=0;current->root.active=false;}
void native_phase(void*,const void*){++originals;native();}
void native_update(void*,void*,void*,const void*){++originals;native();}
int selected(void* self){return ref::rd<int>(self,wh::fCount);}
void write_count(void* self,void* target,int change){
 ++originals;native();const int value=std::clamp(selected(self)+change,1,11);
 set(*static_cast<Node*>(self),"selectedCount",reinterpret_cast<void*>(static_cast<intptr_t>(value)));
 static_cast<Node*>(target)->value=std::to_wstring(value);
}
void native_add(void* self,void* target,const void*){write_count(self,target,1);}
void native_take(void* self,void* target,const void*){write_count(self,target,-1);}
}
namespace ffpr::dialogue::popup_state {void*active_popup(){fixture::native();return warehouse_fixture::popup;}}
namespace cursor {void add_suppressor(const char* query){warehouse_fixture::suppressors.emplace_back(query);}}
namespace ffpr::system {
void announce_controls_for(void* owner){fixture::native();warehouse_fixture::controls=owner;}
void cancel_controls_reading(){}
}

int main(){
 using namespace fixture;using namespace warehouse_fixture;
 background::prepare();int failed=0;auto expect=[&](bool ok,const char* why){if(!ok){++failed;std::printf("FAIL %s\n",why);}};
 wh::install();wh::o_open=&native_open;wh::o_close=&native_close;wh::o_command=&native_command;wh::o_select=&native_select;wh::o_update=&native_update;
 wh::o_command_init=wh::o_store_init=wh::o_retrieve_init=&native_phase;wh::o_add=&native_add;wh::o_take=&native_take;
 expect(std::find(suppressors.begin(),suppressors.end(),"Warehouse.IsActive")!=suppressors.end(),"warehouse owns the generic cursor while its native window is open");
 for(int number=1;number<=6;++number){
  fixture::game=static_cast<game::Id>(number);Screen screen;current=&screen;autodetail=false;
  spoken.clear();wh::d_open(&screen.window,false,nullptr,nullptr);tick();
  expect(contains(L"Fat Chocobo")&&contains(L"Store")&&!contains(L"Hi-Potion"),"all-six entry announces title/greeting/Store instead of the preview inventory");
  expect(spoken.size()==1&&wh::active(),"nested native preview callbacks produce one current command announcement");
  for(int index:{1,2,0}){spoken.clear();wh::d_command(&screen.window,index,nullptr);tick();const wchar_t* names[]{L"Store",L"Retrieve",L"Exit"};expect(spoken.size()==1&&contains(names[index])&&!contains(L"Hi-Potion"),"Store/Retrieve/Exit own their actual native command focus");}
  screen.empty.value=L"No stored\nitems.";spoken.clear();wh::d_command(&screen.window,1,nullptr);tick();
  expect(contains(L"Retrieve")&&contains(L"No stored items."),"visible empty-storage text is read on Retrieve preview with normal whitespace cleanup");screen.empty.value.clear();
  for(int phase:{4,6})for(bool details:{false,true}){
   screen.window.command=phase;autodetail=details;spoken.clear();wh::d_select(&screen.window,0,0,nullptr);tick();
   expect(spoken.size()==1&&contains(L"Hi-Potion, quantity 11")&&!contains(L"Store")&&!contains(L"Retrieve")&&contains(L"Restores 500 HP.")==details,"both native list modes read rendered item/count and optional description without mode prefixes");
   spoken.clear();party_actions.at("Warehouse.ReadDetails")();expect(contains(L"Restores 500 HP."),"manual I reads the live native description with either Autodetail setting");
  }
  autodetail=false;screen.window.command=4;
  screen.count.value=L"10";spoken.clear();wh::d_update(&screen.window,&screen.model,&screen.item,nullptr);tick();
  expect(contains(L"quantity 10"),"a completed transfer/native row update reads the refreshed visible count");
  Node unrelated;spoken.clear();wh::d_update(&screen.window,&unrelated,&screen.item,nullptr);tick();expect(spoken.empty(),"unrelated pooled row updates cannot repeat the selected item");
  screen.item_view.active=false;spoken.clear();wh::d_select(&screen.window,0,0,nullptr);tick();expect(spoken.empty(),"hidden item renderers cannot borrow model quantities");screen.item_view.active=true;
  screen.content_cursor.id=2;spoken.clear();wh::d_select(&screen.window,2,0,nullptr);tick();expect(spoken.empty(),"invalid virtual-scroll index fails closed");screen.content_cursor.id=0;
  // Existing Popup.Warehouse Show uses this typed query as its sole initial
  // prompt producer. Count-change hooks then read only the changed live amount.
  Node popup_owner,popup_view,popup_item;
  Node prompt=make_text(L"How many are you giving me?"),popup_name=make_text(L"Hi-Potion"),stock_label=make_text(L"Stock"),stock=make_text(L"11"),amount_label=make_text(L"Stored"),amount=make_text(L"10");
  popup=&popup_owner;set(popup_owner,"view",&popup_view);set(popup_view,"commentText",&prompt);set(popup_view,"targetItemView",&popup_item);set(popup_item,"nameText",&popup_name);
  set(popup_view,"fixedPossessionText",&stock_label);set(popup_view,"possessionCountText",&stock);set(popup_view,"targetCountNameText",&amount_label);set(popup_view,"targetCountText",&amount);set(popup_owner,"selectedCount",reinterpret_cast<void*>(10));
  screen.window.command=8;auto initial=text_queries.at("Warehouse.QuantityPopup")();
  expect(initial==L"How many are you giving me?. Hi-Potion. Stock: 11. Stored: 10","quantity Show reads prompt/item/stock/selected amount from exact rendered fields");
  spoken.clear();wh::d_add(&popup_owner,&amount,nullptr);expect(spoken==std::vector<std::wstring>{L"Stored: 11"},"quantity increase reads the new displayed amount only");
  spoken.clear();wh::d_add(&popup_owner,&amount,nullptr);expect(spoken.empty(),"native maximum clamp creates no false quantity-change event");
  spoken.clear();wh::d_take(&popup_owner,&amount,nullptr);expect(spoken==std::vector<std::wstring>{L"Stored: 10"},"quantity decrease is spoken");
  amount_label.value=L"Retrieved";screen.window.command=10;spoken.clear();wh::d_take(&popup_owner,&amount,nullptr);expect(spoken==std::vector<std::wstring>{L"Retrieved: 9"},"Retrieve uses its own displayed amount label");
  set(popup_owner,"selectedCount",reinterpret_cast<void*>(1));amount.value=L"1";spoken.clear();wh::d_take(&popup_owner,&amount,nullptr);expect(spoken.empty(),"native minimum clamp is respected");
  popup_view.active=false;spoken.clear();wh::d_add(&popup_owner,&amount,nullptr);expect(spoken.empty()&&text_queries.at("Warehouse.QuantityPopup")().empty(),"hidden popup quantities cannot speak or populate initial details");popup_view.active=true;
  popup=nullptr;spoken.clear();wh::d_add(&popup_owner,&amount,nullptr);expect(spoken.empty(),"closed/foreign popup callbacks cannot publish a quantity");
  screen.window.command=4;party_actions.at("Warehouse.ReadControls")();expect(controls==&screen.footer,"Shift+I selects this warehouse's exact serialized native footer");
  screen.window.command=2;spoken.clear();wh::d_command(&screen.window,1,nullptr);wh::d_close(&screen.window,nullptr);tick();expect(spoken.empty()&&!wh::active(),"close cancels pending focus and releases generic ownership");
  wh::d_open(&screen.window,false,nullptr,nullptr);tick();screen.root.active=false;expect(!wh::active(),"a hidden preloaded warehouse cannot claim other screens");wh::retire();
 }
 expect(wrong==0&&originals>0,"every original handler executes and native reads remain on the owning thread");
 std::printf("Warehouse command/list/empty/quantity/details/footer/lifetime: %d failures; %d original callbacks\n",failed,originals);return failed?1:0;
}
