// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Actual bounded native-owner snapshot; bucket lookup and active enabled corps.
#define main retained_key_reader_native_main
#include "mapstate_native_check.cpp"
#undef main
int main(){
 using namespace fixture;using namespace field;
 int cases=0;const auto check=[&](bool okay,const char* why){++cases;expect(okay,why);};
 running_game=game::Id::FF3;MiniMember a{1},b{2},reserve{3};List corps{&a,&b},owned{&reserve,&b,&a};user.corps=&corps;user.owned=&owned;
 KeyItem item;KeyDictionary items;user.normal_items=&items;local.values[0]=37;
 const auto read=[&]{return map_state::snapshot();};
 auto state=read();check(state.magic_keys==0&&state.party_thief==false,"known empty bucket and ordinary active corps");
 check(key_lookup_id==15,"Magic Key uses content15, not item14 as dictionary key");
 items.values[14]=&item;check(read().magic_keys==0,"unrelated item14 bucket does not qualify");items.values.clear();items.values[15]=&item;
 for(int count:{0,1,2,99,INT_MAX}){item.quantity=count;const int before=key_lookups;state=read();check(state.magic_keys==count,"copied exact nonnegative owned key count");check(key_lookups==before+1,"one bucket lookup per complete snapshot");}
 item.quantity=-1;check(!read().magic_keys,"negative quantity remains unknown");item.quantity=1;
 item.item=13;check(!read().magic_keys,"mismatched item identity stays unknown");item.item=14;
 item.content=14;check(!read().magic_keys,"mismatched content identity stays unknown");item.content=15;
 item.save=nullptr;const int before_getters=key_getters;check(!read().magic_keys&&key_getters==before_getters,"unfinished item save never calls a getter");item.save=reinterpret_cast<void*>(uintptr_t(1));
 bad_key_type=true;check(!read().magic_keys,"wrong native dictionary value type");bad_key_type=false;
 items.size_override=4097;const int before_lookup=key_lookups;check(!read().magic_keys&&key_lookups==before_lookup,"oversized inventory rejected without lookup/scan");items.size_override=-1;
 missing_key_field=true;check(!read().magic_keys,"missing normal-items field stays unknown");missing_key_field=false;
 missing_key_method=true;check(!read().magic_keys,"missing native count getter stays unknown");missing_key_method=false;
 user.normal_items=nullptr;check(!read().magic_keys,"missing inventory stays unknown");user.normal_items=&items;
 reserve.job=3;check(read().party_thief==false,"reserve Thief does not unlock a party door");b.job=3;check(read().party_thief==true,"active enabled Thief qualifies");
 b.enabled=false;check(!read().party_thief,"disabled active member cannot manufacture a Thief receipt");b.enabled=true;
 b.save=nullptr;check(!read().party_thief,"uninitialized character save stays unknown");b.save=reinterpret_cast<void*>(uintptr_t(1));
 MiniMember absent{9};corps.push_back(&absent);check(!read().party_thief,"unresolved corps ID stays unknown");corps.pop_back();
 owned.resize(17,&a);check(!read().party_thief,"owned-character work bounded at16");owned={&reserve,&b,&a};
 corps.resize(10,&a);check(!read().party_thief,"corps work bounded at9");corps={&a,&b};
 for(int game_number:{1,2,4,5,6}){running_game=static_cast<game::Id>(game_number);const int lookups=key_lookups,jobs=job_getters;state=read();check(!state.magic_keys&&!state.party_thief&&key_lookups==lookups&&job_getters==jobs,"other games perform no key or job observation");}
 running_game=game::Id::FF3;state=read();check(local.values[0]==37&&item.quantity==1,"native scratch and consumable quantity remain unchanged");
 std::thread worker([&]{check(state.magic_keys==1&&state.party_thief==true,"worker consumes owned scalar capabilities only");});worker.join();
 check(wrong_owner==0,"zero worker native reads");
 std::printf("FFIII actual key/Thief reader: %d cases/%d failures; one content15 lookup, no inventory scan, zero script calls/writes; owner%d\n",cases,failures,wrong_owner.load());
 return failures||wrong_owner?1:0;
}
