// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Canonical Mog11, not roster identity or reserve ownership; same bounded pass.
#define main retained_mog_native_main
#include "mapstate_native_check.cpp"
#undef main
int main(){using namespace fixture;using field::map_state::snapshot;int cases=0;const auto check=[&](bool ok,const char* why){++cases;expect(ok,why);};running_game=game::Id::FF6;
 MiniMember mog{71},cyan{72},a{71},b{72};mog.status=11;cyan.status=3;List corps{&b},owned{&mog,&cyan};user.corps=&corps;user.owned=&owned;auto f=snapshot();check(f.party_mog==false&&f.party_cyan==true,"reserve Mog does not qualify recruitment and active Cyan remains correct");
 corps={&a,&b};int before=locke_status_reads;f=snapshot();check(f.party_mog==true&&f.party_cyan==true&&f.active_party_members==2,"both canonical roles share current-corps observation");check(locke_status_reads-before==2,"Mog adds no getter or traversal to the existing pass");
 const auto original=f;mog.status=10;f=snapshot();check(f.party_mog==false,"canonical Mog11 is not confused with Setzer10");check(field::map_state::Resolver(original,json::Value{}).signature()!=field::map_state::Resolver(f,json::Value{}).signature(),"Mog membership changes invalidate the owned route signature");mog.status=11;
 mog.save=nullptr;f=snapshot();check(!f.party_mog&&!f.party_cyan,"unfinished owned data cannot publish a partial role mask");mog.save=reinterpret_cast<void*>(uintptr_t(1));
 for(int game=1;game<=5;++game){running_game=static_cast<game::Id>(game);before=locke_status_reads;f=snapshot();check(!f.party_mog&&locke_status_reads==before,"other games add no native Mog reads");}
 running_game=game::Id::FF6;f=snapshot();std::thread worker([&]{check(f.party_mog==true&&f.party_cyan==true,"worker gets only owned role scalars");});worker.join();check(wrong_owner==0,"no worker pointer or native API");std::printf("Mog reader:%d assertions/%d failures/owner%d\n",cases,failures,wrong_owner.load());return failures||wrong_owner?1:0;
}
