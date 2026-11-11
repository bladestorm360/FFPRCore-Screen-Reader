// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Actual Catapult initializer + four native operations versus the independently
// verified six-engine Ready->Show contract. Source-bearing programs are external.
#include "field/map_state.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
namespace data {bool game_data_ready(){return false;}const json::Value& game_json(const char*){static json::Value empty;return empty;}}
std::string read(const char* p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(){
 json::Value programs;if(!json::parse(read("../tools/ff5_catapult_20261101/ready_programs.json"),programs))return 2;
 int failures=0;auto check=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL Ready %s\n",why);}};
 for(const auto* version:{L"before",L"after"}){
  field::map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);flags.treasure.fill(0);flags.current_map_id=293;
  std::map<int,field::map_state::EntityState> entities;
  int stage=0;
  for(const auto* name:{L"sc_map_20231_4",L"sc_e_0224",L"sc_e_0224_1",L"sc_e_0224_2",L"sc_e_0224_3"}){
   const auto row=std::find_if(programs.arr.begin(),programs.arr.end(),[&](const auto& r){return r.get(L"name")->s==name;});if(row==programs.arr.end())return 2;
   const auto actual=field::map_state::project(*row->get(version),flags);check(actual.known,"real complete native operation projects");
   for(const auto& [id,e]:actual.entities){auto& into=entities[id];if(e.visible)into.visible=e.visible;if(e.x)into.x=e.x;if(e.y)into.y=e.y;}
   for(int id:actual.uncertain_entities)entities[id].visible.reset();
   if(stage>=2)flags.scenario[307+stage]=1;++stage;
  }
  const bool shown=entities[23].visible==true;
  std::printf("Catapult real four-operation book23 %ls: visible%d position%d,%d\n",version,shown,entities[23].x.value_or(-99),entities[23].y.value_or(-99));
  if(std::wstring_view(version)==L"before")check(!shown,"preserved compiler reproduces the omitted activation");
  else check(shown&&entities[23].x==10&&entities[23].y==14,"native Ready reveals the actual relocated book after the urn");
 }
 // Unknown final bodies supersede an earlier Hide, and a later known Hide
 // restores certainty. These are lifecycle receipts, not native execution.
 json::Value sample;field::map_state::Flags flags;
 json::parse(R"({"code":[[10,0,23,0,-2147483648,-2147483648],[11,0],[10,0,23,-2,-2147483648,-2147483648],[11,0],[6]]})",sample);
 auto unknown=field::map_state::project(sample,flags);
 check(unknown.entities[23].visible==std::nullopt&&unknown.uncertain_entities.count(23),"opaque restarted body cannot certify the old hidden state");
 const auto first=sample.obj[L"code"].arr;
 sample.obj[L"code"].arr.insert(sample.obj[L"code"].arr.end()-1,first.begin(),first.begin()+2);
 auto hidden=field::map_state::project(sample,flags);check(hidden.entities[23].visible==false&&!hidden.uncertain_entities.count(23),"later exact Hide restores a current receipt");
 std::printf("Puppet Ready source/consumer: %d failures; no game execution\n",failures);return failures?1:0;
}
