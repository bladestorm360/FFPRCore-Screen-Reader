// check-sources: text.cpp
// Both actual game adapters, fake typed native owners; no game launch.
#define main retained_ff3_event_item_main
#include "event_item_select_check.cpp"
#undef main
#include "../src/games/ff4/event_item_select.cpp"
namespace ref {
std::wstring rd_str_bounded(void* p,Field& f,size_t cap){auto text=rd_str(p,f);return text.size()<=cap?text:L"";}
}
namespace field {const std::wstring& scene_name(){static std::wstring scene=L"Field";return scene;}}
int main(){
 int failures=retained_ff3_event_item_main(),cases=0;using namespace fixture;namespace event=ff4::event_items;
 const auto expect=[&](bool okay,const char* why){++cases;if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 event::install();event::original_select=&fixture::select;event::original_close=&fixture::close;
 Node controller,key,blank,other;key.name=L"Baron Key";key.description=L"Opens the locked doors.";blank.name=L"Luca's Necklace";
 List rows{&key,&blank};controller.list=&rows;manager.list=&controller;manager_exists=true;autodetail=false;
 const auto details=[]{fixture::actions.at("EventItemSelect.ReadDetails")();};
 spoken.clear();event::select_hook(&controller,0,0,nullptr);
 expect(spoken==std::vector<std::wstring>{key.name}&&event::active(),"FFIV ready native item row owns speech and modal state");
 details();details();expect(spoken.size()==3&&spoken.back()==key.description,"every explicit I request reads current owned details");
 const auto count=spoken.size();for(int i=0;i<100;++i)event::select_hook(&controller,0,0,nullptr);
 expect(spoken.size()==count+100,"legitimate equal-text native selection events are never deduplicated");
 autodetail=true;spoken.clear();event::select_hook(&controller,0,0,nullptr);
 expect(spoken==std::vector<std::wstring>{key.name+L": "+key.description},"FFIV Autodetail respects the independent existing preference");
 controller.description_enabled=false;spoken.clear();event::select_hook(&controller,0,0,nullptr);details();
 expect(spoken==std::vector<std::wstring>{key.name,L"No description"},"native disabled description panel prevents disclosure through Autodetail and I");controller.description_enabled=true;
 event::select_hook(&controller,1,0,nullptr);details();expect(spoken.back()==L"No description","blank row clears prior details");
 spoken.clear();event::close_hook(&manager,nullptr);details();expect(spoken.empty()&&!event::active(),"native manager Close retires speech and cached detail");
 event::select_hook(&controller,0,0,nullptr);manager.list=&other;expect(!event::active(),"replacement native manager/controller rejects stale ownership");manager.list=&controller;
 event::select_hook(&controller,0,0,nullptr);controller.visible=false;expect(!event::active(),"hidden controller cannot retain a modal item context");controller.visible=true;
 for(int invalid:{-1,2,513}){spoken.clear();event::select_hook(&controller,invalid,0,nullptr);expect(spoken.empty()&&!event::active(),"invalid index never reads or retains a row");}
 List excess(513,&key);controller.list=&excess;spoken.clear();event::select_hook(&controller,0,0,nullptr);
 expect(spoken.empty()&&!event::active(),"oversized native list is rejected before a row lookup");controller.list=&rows;
 const auto original_name=key.name;key.name.assign(129,L'x');event::select_hook(&controller,0,0,nullptr);expect(!event::active(),"oversized identifier is rejected before message resolution");key.name=original_name;
 event::select_hook(&controller,0,0,nullptr);for(const auto& fn:handlers[events::Kind::MapChanging])fn();spoken.clear();details();
 expect(spoken.empty()&&!event::active(),"map transition clears both adapters before native destruction");
 manager_exists=false;event::select_hook(&controller,0,0,nullptr);expect(!event::active(),"missing native manager never certifies ready ownership");
 std::printf("FFIV event-item owner: %d cases/%d failures; preserved FFIII check included\n",cases,failures);return failures?1:0;
}
