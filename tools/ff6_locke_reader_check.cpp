// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Actual owner snapshot: active owned-ID match and canonical Locke status.
#define main retained_narshe_native_main
#include "mapstate_native_check.cpp"
#undef main
int main(){
 using namespace fixture;using field::map_state::snapshot;
 int cases=0;const auto check=[&](bool ok,const char* why){++cases;expect(ok,why);};
 running_game=game::Id::FF6;MiniMember a{41},b{42},reserve{43};a.status=1;b.status=2;reserve.status=2;
 MiniMember slot_a{41},slot_b{42};List corps{&slot_a},owned{&reserve,&b,&a};user.corps=&corps;user.owned=&owned;
 check(snapshot().party_locke==false,"reserve Locke does not qualify");
 corps.push_back(&slot_b);check(snapshot().party_locke==true,"current corps ID42 resolves canonical status2 despite roster ordering");
 b.status=7;b.id=2;check(!snapshot().party_locke,"unresolved active owned ID stays unknown");
 slot_b.id=2;check(snapshot().party_locke==false,"owned ID2 is not a substitute for canonical Locke status");slot_b.id=42;b.id=42;
 b.status=2;b.enabled=false;check(snapshot().party_locke==true,"native membership does not invent an enabled/HP prerequisite");
 corps={&slot_b};b.save=nullptr;const int getters=locke_status_reads;check(!snapshot().party_locke&&locke_status_reads==getters,"unfinished active save cannot call its canonical-status getter");b.save=reinterpret_cast<void*>(uintptr_t(1));corps={&slot_a,&slot_b};
 MiniMember absent{99};corps.push_back(&absent);check(!snapshot().party_locke,"a missing active member keeps the complete snapshot unknown");corps.pop_back();
 owned.resize(65,&a);const int before=locke_status_reads;check(!snapshot().party_locke&&locke_status_reads==before,"oversized roster rejected before status observation");owned={&reserve,&b,&a};
 corps.resize(17,&slot_a);check(!snapshot().party_locke,"active-corps work is bounded at16");corps={&slot_a,&slot_b};
 missing_locke_method=true;check(!snapshot().party_locke,"missing getter is unknown");missing_locke_method=false;
 user.corps=nullptr;check(!snapshot().party_locke,"missing current corps is unknown");user.corps=&corps;
 for(int number=1;number<=5;++number){running_game=static_cast<game::Id>(number);const int count=locke_status_reads;check(!snapshot().party_locke&&locke_status_reads==count,"other games perform no Locke status reads");}
 running_game=game::Id::FF6;const auto state=snapshot();std::thread worker([&]{check(state.party_locke==true,"worker consumes only the owned capability scalar");});worker.join();
 check(wrong_owner==0,"no worker native API or pointer access");
 std::printf("FFVI actual Locke reader: %d cases/%d failures; no clone/script/write, owner%d\n",cases,failures,wrong_owner.load());return failures||wrong_owner?1:0;
}
