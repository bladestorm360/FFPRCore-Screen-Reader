// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Cyan is another scalar in the accepted bounded active-corps pass.
#define main retained_cyan_native_main
#include "mapstate_native_check.cpp"
#undef main
int main(){using namespace fixture;using field::map_state::snapshot;int count=0;
 const auto check=[&](bool ok,const char* why){++count;expect(ok,why);};running_game=game::Id::FF6;
 MiniMember cyan{81},one{82},two{83},three{84},a{81},b{82},c{83},d{84};cyan.status=3;one.status=1;two.status=9;three.status=8;
 List corps{&b,&c,&d},owned{&one,&cyan,&two,&three};user.corps=&corps;user.owned=&owned;auto state=snapshot();
 check(state.party_cyan==false&&state.active_party_members==3,"reserve Cyan is distinct from active canonical Cyan");
 corps={&a,&b,&c,&d};const int before=locke_status_reads;state=snapshot();check(state.party_cyan==true&&state.active_party_members==4,"active Cyan and four resolved members share one observation");
 check(locke_status_reads-before==4,"Cyan adds no native getter to the existing role pass");
 const auto qualified=state;cyan.status=5;state=snapshot();check(state.party_cyan==false,"canonical Cyan3 is not confused with native status5");
 check(field::map_state::Resolver(qualified,json::Value{}).signature()!=field::map_state::Resolver(state,json::Value{}).signature(),"same-size party changing Cyan invalidates cached routing");cyan.status=3;
 cyan.save=nullptr;state=snapshot();check(!state.party_cyan&&!state.active_party_members,"unfinished owned character leaves both fields unknown");cyan.save=reinterpret_cast<void*>(uintptr_t(1));
 for(int game=1;game<=5;++game){running_game=static_cast<game::Id>(game);const int reads=locke_status_reads;state=snapshot();check(!state.party_cyan&&locke_status_reads==reads,"other games add no native Cyan reads");}
 running_game=game::Id::FF6;state=snapshot();std::thread worker([&]{check(state.party_cyan==true&&state.active_party_members==4,"worker gets only owned Cyan/count scalars");});worker.join();
 check(wrong_owner==0,"no native pointer/read crosses onto a worker");std::printf("Cyan reader:%d assertions/%d failures/owner%d\n",count,failures,wrong_owner.load());return failures||wrong_owner?1:0;
}
