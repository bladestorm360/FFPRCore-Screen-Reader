// check-sources: field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp field/field_refs.cpp field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp json.cpp text.cpp
// Fifteen actual native actor identities from the seven source/closure cases
// and their exact sibling. Source-bank and finite control proofs are separate.
#define main retained_source_case_scanner_main
#include "entity_scanner_check.cpp"
#undef main
int main(){
 using namespace field;
 const auto read=[](const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 json::Value cases;if(!json::parse(read("build_check/final_script_cases_20261029/cases.json"),cases))return 2;
 int failures=0,checked=0;auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 fixture::real_event_labels=fixture::capture_cells=true;fixture::captured_object_type=2;
 fixture::captured_class="EventTriggerEntity";fixture::captured_go_name=L"Event";
 for(int n:{2,4,5,6}){
  const auto bytes=read("data/ff"+std::to_string(n)+"/maps.bin");if(!baked::load(bytes.data(),bytes.size()))return 2;
  if(!json::parse(read("data/ff"+std::to_string(n)+"/eventlabels.json"),fixture::event_metadata))return 2;
  fixture::selected_game=static_cast<game::Id>(n);
  for(const auto& row:cases.arr){
   if(int(row.get(L"game")->n)!=n)continue;++checked;
   fixture::map=int(row.get(L"map_id")->n);fixture::talk.id=int(row.get(L"entity_id")->n);
   fixture::talk.script=int(row.get(L"script")->n);fixture::talk.action=int(row.get(L"action")->n);fixture::talk.name=row.get(L"raw")->s;
   fixture::pose={float(row.get(L"cell")->arr[0].n),float(row.get(L"cell")->arr[1].n),0};
   const auto native=convert(&fixture::object);
   if(row.get(L"skip")->b){expect(!native,"exact missing-script companion remains suppressed beside its real native exit");continue;}
   expect(native&&native->kind==Kind::Event,"real source-case actor survives the actual event detector without a false map exit");
   if(native)expect(native->name==row.get(L"label")->s,"source-owned or exact physical identity reaches native speech output");
   baked_nav::RemoteObject remote;remote.map_id=fixture::map;remote.entity_id=fixture::talk.id;remote.type=baked::ObjType::Event;
   remote.raw_name=fixture::talk.name;remote.cell={0,int(row.get(L"cell")->arr[0].n),int(row.get(L"cell")->arr[1].n)};remote.event_type=L"Event";
   RemoteBuilder builder({remote});builder.step();const auto rows=build_remote(builder.input);
   expect(rows.size()==1&&rows[0]->name==row.get(L"label")->s,"actual remote publication preserves the same source-case name");
  }
 }
 expect(checked==15,"all source/closure actor cases are covered");expect(fixture::wrong_owner==0,"native observations stay on their owner");
 std::printf("Final script cases: %d native actor cases, %d failures, owner%d; source/bake contracts separately checked\n",checked,failures,fixture::wrong_owner);return failures?1:0;
}
