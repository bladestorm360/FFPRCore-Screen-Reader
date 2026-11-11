// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Crescent's active-role observation reuses the same bounded native owner pass.
#define main retained_gau_native_main
#include "mapstate_native_check.cpp"
#undef main
int main(){using namespace fixture;using field::map_state::snapshot;int cases=0;
 const auto check=[&](bool ok,const char* why){++cases;expect(ok,why);};running_game=game::Id::FF6;
 MiniMember locke{41},sabin{42},strago{43},gau{44};locke.status=2;sabin.status=6;strago.status=8;gau.status=12;
 MiniMember a{41},b{42},c{43},d{44};List corps{&a,&b,&c},owned{&gau,&sabin,&strago,&locke};user.corps=&corps;user.owned=&owned;
 auto state=snapshot();check(state.party_gau==false&&state.party_locke==true&&state.party_sabin==true&&state.party_strago==true,"reserve Gau does not open the helmet-search events");
 corps.push_back(&d);const int reads=locke_status_reads;state=snapshot();check(state.party_gau==true,"active canonical Gau12 is recognized");
 check(locke_status_reads-reads==4,"adding the Gau bit adds no getter call per active member");
 gau.enabled=false;check(snapshot().party_gau==true,"native membership adds no invented HP/enabled requirement");
 gau.id=12;d.id=12;gau.status=11;check(snapshot().party_gau==false,"saved roster ID12 is not canonical status12");gau.status=12;
 gau.save=nullptr;check(!snapshot().party_gau,"unfinished character data cannot certify Gau");gau.save=reinterpret_cast<void*>(uintptr_t(1));
 MiniMember missing{99};corps.push_back(&missing);check(!snapshot().party_gau,"unresolved active slot stays unknown");corps.pop_back();
 owned.resize(65,&gau);const int before=locke_status_reads;check(!snapshot().party_gau&&locke_status_reads==before,"overbound roster rejects before status reads");owned={&gau,&sabin,&strago,&locke};
 for(int game=1;game<=5;++game){running_game=static_cast<game::Id>(game);const int n=locke_status_reads;check(!snapshot().party_gau&&locke_status_reads==n,"other games add no native party-role calls");}
 running_game=game::Id::FF6;state=snapshot();std::thread worker([&]{check(state.party_gau==true,"worker receives an owned Gau scalar");});worker.join();
 check(wrong_owner==0,"no worker-side native pointer or API");std::printf("FFVI Gau reader:%d assertions/%d failures/owner%d\n",cases,failures,wrong_owner.load());return failures||wrong_owner?1:0;
}
