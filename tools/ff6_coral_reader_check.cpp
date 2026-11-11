// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Actual bounded owner snapshot: both canonical roles in one existing pass.
#define main retained_coral_native_main
#include "mapstate_native_check.cpp"
#undef main
int main(){
 using namespace fixture;using field::map_state::snapshot;int cases=0;
 const auto check=[&](bool okay,const char* message){++cases;expect(okay,message);};
 running_game=game::Id::FF6;MiniMember locke{41},strago{42},reserve{43};locke.status=2;strago.status=8;reserve.status=8;
 MiniMember a{41},b{42};List corps{&a},owned{&reserve,&strago,&locke};user.corps=&corps;user.owned=&owned;
 auto state=snapshot();check(state.party_locke==true&&state.party_strago==false,"reserve-only Strago does not qualify for first feeding");
 corps.push_back(&b);const int before=locke_status_reads;state=snapshot();check(state.party_locke==true&&state.party_strago==true,"both active saved IDs resolve their canonical roles");check(locke_status_reads-before==2,"both roles reuse one status getter per active member");
 strago.enabled=false;check(snapshot().party_strago==true,"no invented HP or enabled-member requirement");
 strago.id=8;b.id=8;strago.status=7;check(snapshot().party_strago==false,"owned ID8 is not canonical status8");strago.status=8;
 strago.save=nullptr;state=snapshot();check(!state.party_locke&&!state.party_strago,"unfinished active member makes both observations unknown");strago.save=reinterpret_cast<void*>(uintptr_t(1));
 MiniMember missing{99};corps.push_back(&missing);state=snapshot();check(!state.party_locke&&!state.party_strago,"unresolved member cannot certify either role");corps.pop_back();
 owned.resize(65,&locke);const int reads=locke_status_reads;check(!snapshot().party_strago&&locke_status_reads==reads,"overbound owned list rejects before observation");owned={&reserve,&strago,&locke};
 corps.resize(17,&a);check(!snapshot().party_strago,"current corps is bounded at16");corps={&a,&b};
 missing_locke_method=true;check(!snapshot().party_strago,"missing canonical getter remains unknown");missing_locke_method=false;
 for(int game=1;game<=5;++game){running_game=static_cast<game::Id>(game);const int count=locke_status_reads;state=snapshot();check(!state.party_strago&&!state.party_locke&&locke_status_reads==count,"other games add no canonical-role native work");}
 running_game=game::Id::FF6;state=snapshot();std::thread worker([&]{check(state.party_strago==true&&state.party_locke==true,"worker reads only owned role scalars");});worker.join();
 check(wrong_owner==0,"no worker-side native APIs or pointers");
 std::printf("FFVI coral party reader:%d cases/%d failures/owner%d\n",cases,failures,wrong_owner.load());return failures||wrong_owner?1:0;
}
