// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Observation Post copies Terra and native member count in the existing pass.
#define main retained_terra_native_main
#include "mapstate_native_check.cpp"
#undef main
int main(){using namespace fixture;using field::map_state::snapshot;int cases=0;
 const auto check=[&](bool ok,const char* why){++cases;expect(ok,why);};running_game=game::Id::FF6;
 MiniMember terra{91},other{92},third{93};terra.status=1;other.status=9;third.status=8;
 MiniMember a{91},b{92},c{93};List corps{&b,&c},owned{&terra,&third,&other};user.corps=&corps;user.owned=&owned;
 int before=locke_status_reads;auto state=snapshot();check(state.party_terra==false&&state.active_party_members==2,"reserve Terra cannot qualify the first camp visit");check(locke_status_reads-before==2,"two new scalars add no getter to the existing two-member pass");
 corps={&a,&b};before=locke_status_reads;state=snapshot();check(state.party_terra==true&&state.active_party_members==2,"active canonical Terra plus one other member qualifies");check(locke_status_reads-before==2,"active Terra reuses exactly one canonical status read per member");
 const auto both=state;corps={&a};state=snapshot();check(state.party_terra==true&&state.active_party_members==1,"solo Terra remains a distinct one-member state");
 check(field::map_state::Resolver(both,json::Value{}).signature()!=field::map_state::Resolver(state,json::Value{}).signature(),"same Terra membership with different active count invalidates an old route");
 terra.enabled=false;check(snapshot().active_party_members==1,"native count adds no invented HP/enabled requirement");terra.enabled=true;
 terra.id=1;a.id=1;terra.status=9;check(snapshot().party_terra==false,"saved roster ID1 is not canonical status1");terra.status=1;
 terra.save=nullptr;state=snapshot();check(!state.party_terra&&!state.active_party_members,"unfinished owned data leaves both fields unknown");terra.save=reinterpret_cast<void*>(uintptr_t(1));
 MiniMember missing{99};corps.push_back(&missing);state=snapshot();check(!state.party_terra&&!state.active_party_members,"unresolved active slot cannot produce a partial count");corps.pop_back();
 owned.resize(65,&terra);before=locke_status_reads;state=snapshot();check(!state.party_terra&&!state.active_party_members&&locke_status_reads==before,"overbound roster is rejected before canonical reads");owned={&terra,&third,&other};
 corps.clear();state=snapshot();check(!state.party_terra&&!state.active_party_members,"unready empty corps remains unknown");corps={&a,&b};
 for(int game=1;game<=5;++game){running_game=static_cast<game::Id>(game);before=locke_status_reads;state=snapshot();check(!state.party_terra&&!state.active_party_members&&locke_status_reads==before,"other games add no native party-role calls");}
 running_game=game::Id::FF6;state=snapshot();std::thread worker([&]{check(state.party_terra==true&&state.active_party_members==2,"worker receives owned membership/count values only");});worker.join();
 check(wrong_owner==0,"no worker native API or pointer");std::printf("FFVI Terra/member reader:%d assertions/%d failures/owner%d\n",cases,failures,wrong_owner.load());return failures||wrong_owner?1:0;
}
