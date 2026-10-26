// The native field-magic range state is distinct from a character cursor.
// AllInit owns group speech; SingleInit restores the actual focused row only
// when returning from All. No key interception, polling or text deduplication.
#include "../core.h"
#include "../displayed_ui.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../system/system.h"
#include "../system/cursor_event.h"
#include "../ui/character_row.h"
#include "../ui_util.h"
#include "../unity.h"

namespace ffpr::menus::field_targets {
namespace {
ref::Class k1{"Serial.FF1.UI.KeyInput","AbilityUseContentListController",game::kFF1};
ref::Class k2{"Serial.FF2.UI.KeyInput","AbilityUseContentListController",game::kFF2};
ref::Class k3{"Serial.FF3.UI.KeyInput","AbilityUseContentListController",game::kFF3};
ref::Class k4{"Serial.FF4.UI.KeyInput","AbilityUseContentListController",game::kFF4};
ref::Class k5{"Serial.FF5.UI.KeyInput","AbilityUseContentListController",game::kFF5};
ref::Class k6{"Serial.FF6.UI.KeyInput","AbilityUseContentListController",game::kFF6};
ref::Field fList1{k1,"contentList"};
ref::Field fCursor1{k1,"selectCursor"};
ref::Field fList2{k2,"contentList"};
ref::Field fCursor2{k2,"selectCursor"};
ref::Field fList3{k3,"contentList"};
ref::Field fCursor3{k3,"selectCursor"};
ref::Field fList4{k4,"contentList"};
ref::Field fCursor4{k4,"selectCursor"};
ref::Field fList5{k5,"contentList"};
ref::Field fCursor5{k5,"selectCursor"};
ref::Field fList6{k6,"contentList"};
ref::Field fCursor6{k6,"selectCursor"};
ref::Method mAll1{k1,"AllInit",0};
ref::Method mSingle1{k1,"SingleInit",0};
ref::Method mClose1{k1,"Close",0};
ref::Method mAll2{k2,"AllInit",0};
ref::Method mSingle2{k2,"SingleInit",0};
ref::Method mClose2{k2,"Close",0};
ref::Method mAll3{k3,"AllInit",0};
ref::Method mSingle3{k3,"SingleInit",0};
ref::Method mClose3{k3,"Close",0};
ref::Method mAll4{k4,"AllInit",0};
ref::Method mSingle4{k4,"SingleInit",0};
ref::Method mClose4{k4,"Close",0};
ref::Method mAll5{k5,"AllInit",0};
ref::Method mSingle5{k5,"SingleInit",0};
ref::Method mClose5{k5,"Close",0};
ref::Method mAll6{k6,"AllInit",0};
ref::Method mSingle6{k6,"SingleInit",0};
ref::Method mClose6{k6,"Close",0};
struct Adapter {ref::Class* type;ref::Field* list;ref::Field* cursor;ref::Method* all;ref::Method* single;ref::Method* close;};
Adapter adapters[]={{&k1,&fList1,&fCursor1,&mAll1,&mSingle1,&mClose1},{&k2,&fList2,&fCursor2,&mAll2,&mSingle2,&mClose2},
 {&k3,&fList3,&fCursor3,&mAll3,&mSingle3,&mClose3},{&k4,&fList4,&fCursor4,&mAll4,&mSingle4,&mClose4},
 {&k5,&fList5,&fCursor5,&mAll5,&mSingle5,&mClose5},{&k6,&fList6,&fCursor6,&mAll6,&mSingle6,&mClose6}};
enum class Phase {None,All,ReturningSingle};
void* owner=nullptr;
Phase phase=Phase::None;
void reset(){owner=nullptr;phase=Phase::None;}
bool ready(Adapter& a,void* self) {
 return self&&a.type->is(self)&&unity::active_in_hierarchy(self)&&system::is_main_menu_state();
}
bool owns_range(){return owner&&phase!=Phase::None&&system::is_main_menu_state()&&unity::alive(owner)&&unity::active_in_hierarchy(owner);}
using Void=void(*)(void*,const void*);
Void original_all[6]{},original_single[6]{},original_close[6]{};
void announce_single(Adapter& a,void* self) {
 void* list=ref::rd<void*>(self,*a.list);void* cursor=ref::rd<void*>(self,*a.cursor);
 const int slots=unity::list_count(list),index=ui::cursor_index(cursor);
 if(slots<=0||slots>9||index<0||index>=slots)return;
 void* row=displayed::cursor_row(cursor,list);
 if(!row)row=unity::list_at(list,index);
 const auto line=displayed::target(row);const int count=displayed::target_count(list);
 if(!line.empty()&&index<count)ui::character_row::announce(row,cursor,ui::menu_position(line,index,count));
}
template<int N> void all_init(void* self,const void* mi) {
 auto& a=adapters[N];const bool claim=ready(a,self);
 // Retire individual-target callbacks before native cursor activation/reparenting.
 if(claim){owner=self;phase=Phase::All;ui::character_row::clear();}
 original_all[N](self,mi);
 hooks::guarded("FieldMagicTargets.All",[&]{if(claim&&owner==self&&phase==Phase::All&&ready(a,self))ffpr::say(T(L"All allies"),true);});
}
template<int N> void single_init(void* self,const void* mi) {
 auto& a=adapters[N];const bool returning=owner==self&&phase==Phase::All&&a.type->is(self);
 if(returning)phase=Phase::ReturningSingle;
 original_single[N](self,mi);
 hooks::guarded("FieldMagicTargets.Single",[&]{
  if(!returning||owner!=self||phase!=Phase::ReturningSingle)return;
  reset();if(ready(a,self))announce_single(a,self);
 });
}
template<int N> void close(void* self,const void* mi) {
 if(owner==self&&adapters[N].type->is(self))reset();
 original_close[N](self,mi);
}
template<int N> int connect() {
 auto& a=adapters[N];int count=0;
 count+=hooks::attach(*a.all,&all_init<N>,&original_all[N])?1:0;
 // FFV already owns a bounded first-ready SingleInit reader. It clears the
 // range prefix before its native call; keep that producer instead of a second.
 if constexpr(N!=4)count+=hooks::attach(*a.single,&single_init<N>,&original_single[N])?1:0;
 count+=hooks::attach(*a.close,&close<N>,&original_close[N])?1:0;
 return count;
}
int install() {
 queries::define("FieldMagicTargets.OwnsRange",&owns_range);
 actions::define("FieldMagicTargets.SingleFF5",&reset);
 cursor::add_suppressor("FieldMagicTargets.OwnsRange");
 for(auto event:{events::Kind::MapChanging,events::Kind::MenuClosed,events::Kind::BattleEntering,events::Kind::TitleScreen})events::on(event,&reset);
 switch(game::current()) {
 case game::Id::FF1:return connect<0>();case game::Id::FF2:return connect<1>();case game::Id::FF3:return connect<2>();
 case game::Id::FF4:return connect<3>();case game::Id::FF5:return connect<4>();case game::Id::FF6:return connect<5>();
 default:return 0;
 }
}
core::Install registration{"Menus.FieldMagicTargets",&install,510};
}
}
