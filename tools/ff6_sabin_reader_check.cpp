// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Exact Duncan role added to the existing bounded owner pass, without extra calls.
#define main retained_sabin_native_main
#include "mapstate_native_check.cpp"
#undef main
int main(){using namespace fixture;using field::map_state::snapshot;int cases=0;
 const auto check=[&](bool ok,const char* why){++cases;expect(ok,why);};running_game=game::Id::FF6;
 MiniMember locke{41},sabin{42},strago{43};locke.status=2;sabin.status=6;strago.status=8;
 MiniMember a{41},b{42},c{43};List corps{&a},owned{&sabin,&strago,&locke};user.corps=&corps;user.owned=&owned;
 auto state=snapshot();check(state.party_sabin==false&&state.party_locke==true,"reserve-only Sabin cannot unlock training");
 corps.push_back(&b);corps.push_back(&c);const int reads=locke_status_reads;state=snapshot();
 check(state.party_sabin==true&&state.party_locke==true&&state.party_strago==true,"all three actual active canonical identities are copied together");
 check(locke_status_reads-reads==3,"Sabin adds no status getter call per active member");
 sabin.enabled=false;check(snapshot().party_sabin==true,"native character membership does not invent an enabled/HP requirement");
 sabin.id=6;b.id=6;sabin.status=7;check(snapshot().party_sabin==false,"saved roster ID6 is not canonical character status6");sabin.status=6;
 sabin.save=nullptr;check(!snapshot().party_sabin,"unfinished member keeps the observation unknown");sabin.save=reinterpret_cast<void*>(uintptr_t(1));
 MiniMember missing{99};corps.push_back(&missing);check(!snapshot().party_sabin,"unresolved active membership cannot certify a party role");corps.pop_back();
 owned.resize(65,&sabin);const int before=locke_status_reads;check(!snapshot().party_sabin&&locke_status_reads==before,"overbound ownership rejects before canonical reads");owned={&sabin,&strago,&locke};
 for(int game=1;game<=5;++game){running_game=static_cast<game::Id>(game);const int count=locke_status_reads;check(!snapshot().party_sabin&&locke_status_reads==count,"other games add no native party-role calls");}
 running_game=game::Id::FF6;state=snapshot();std::thread worker([&]{check(state.party_sabin==true,"worker receives only an owned Sabin scalar");});worker.join();
 check(wrong_owner==0,"no worker-side pointer or native read");std::printf("FFVI Sabin reader:%d assertions/%d failures/owner%d\n",cases,failures,wrong_owner.load());return failures||wrong_owner?1:0;
}
