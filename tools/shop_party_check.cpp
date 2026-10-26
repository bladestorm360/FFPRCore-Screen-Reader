// check-sources: text.cpp ui/navigation_buffer.cpp
// Actual shop reader with rendered sprite/text/animation owners, no game launch.
#define main retained_party_main
#include "party_esper_check.cpp"
#undef main
#include "../src/core/shop/shop_party.cpp"
namespace shop_fixture {void* shop=nullptr;int type=2;void* name(void*p,const void*){fixture::native();return p;}}
namespace ffpr::shop {bool is_shop_menu_active(){return shop_fixture::shop!=nullptr;}void*cached_or_find_controller(){return shop_fixture::shop;}int last_item_content_type(){return shop_fixture::type;}}
namespace ref {
std::wstring str(void*p){fixture::native();return p?static_cast<fixture::Node*>(p)->value:std::wstring();}
std::wstring rd_str_bounded(void*p,Field&f,size_t n){return str(rd<void*>(p,f)).substr(0,n);}
}
int main(){
 using namespace fixture;int failures=0;auto expect=[&](bool b,const char*s){if(!b){++failures;std::printf("FAIL %s\n",s);}};
 extra_methods["get_Name"]=reinterpret_cast<void*>(&shop_fixture::name);
 Node shop,list,main,content,view,portrait,portrait_view,body=image(),name=make_text(L"Corrad"),animation=make_text(L"win"),up=image(),down=image(),equipped=image(),impossible=make_text(L"-");
 List members{&content};set(shop,"listController",&list);set(list,"listController",&main);set(main,"characterContentList",&members);
 set(content,"view",&view);set(content,"characterController",&portrait);set(content,"<Data>k__BackingField",&name);
 set(portrait,"view",&portrait_view);set(portrait_view,"mainImage",&body);set(portrait,"currentState",&animation);
 set(view,"stateUpImage",&up);set(view,"stateDownImage",&down);set(view,"stateEquippedImage",&equipped);set(view,"stateImpossibleText",&impossible);
 shop_fixture::shop=&shop;
 for(int n=1;n<=6;++n){
  fixture::game=static_cast<game::Id>(n);shop_fixture::type=3;impossible.active=false;equipped.active=false;down.active=false;
  for(const auto* state:{L"win",L"Default_00"})for(int arrow=0;arrow<3;++arrow){
   animation.value=state;up.active=arrow==1;down.active=arrow==2;spoken.clear();ffpr::shop::announce_slot(1);
   std::wstring expected=L"Corrad, ";if(animation.value==L"Default_00")expected+=L"Can not equip";
   else expected+=std::wstring(L"Can equip, ")+(arrow==1?L"Stronger":arrow==2?L"Weaker":L"No change");
   expect(spoken==std::vector<std::wstring>{expected},"native idle incompatibility differs from compatible stronger/weaker/unchanged in all six games");
  }
  animation.value=L"win";equipped.active=true;up.active=down.active=false;spoken.clear();ffpr::shop::announce_slot(1);
  expect(spoken==std::vector<std::wstring>{L"Corrad, Can equip, Equipped"},"equipped sprite retains name and eligibility");
  body.active=false;spoken.clear();ffpr::shop::announce_slot(1);
  expect(!contains(L"Corrad")&&!contains(L"Can equip")&&!contains(L"Can not equip"),"hidden portrait cannot disclose identity or cached animation");body.active=true;
  shop_fixture::type=-1;animation.value=L"Default_00";equipped.active=false;spoken.clear();ffpr::shop::announce_slot(1);
  expect(spoken==std::vector<std::wstring>{L"Corrad"},"consumable/magic idle portraits are not equipment-ineligible");
  shop_fixture::type=2;impossible.active=true;spoken.clear();ffpr::shop::announce_slot(1);
  expect(contains(L"Can not equip"),"native impossible symbol has text equivalency");impossible.active=false;
 }
 // Native arrows are already the game's comparison result. Equal item text
 // must not override a visible up/down arrow with an invented stat calculation.
 shop_fixture::type=2;animation.value=L"win";up.active=true;down.active=false;spoken.clear();ffpr::shop::announce_slot(1);
 expect(contains(L"Stronger"),"Robyn Ice Staff screenshot preserves the native up arrow independently of item text");
 up.active=false;down.active=true;spoken.clear();ffpr::shop::announce_slot(1);
 expect(contains(L"Weaker"),"Ryko staff screenshots preserve the native down arrow");
 spoken.clear();shop_fixture::shop=nullptr;ffpr::shop::announce_slot(1);expect(spoken.empty(),"closed shop retires comparison control");
 expect(wrong==0,"native reads stay on their owner");std::printf("All-six actual shop portrait comparisons: %d failures\n",failures);return failures?1:0;
}
