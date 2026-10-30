// check-sources: field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp field/field_refs.cpp field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp json.cpp text.cpp
// Real reviewed records through the production native detector and remote
// publication builder. Native route stubs do not certify physical reachability.
#define main retained_contextual_scanner_main
#include "entity_scanner_check.cpp"
#undef main
int main(int argc,char** argv){
 using namespace field;
 const bool before=argc>1&&std::strcmp(argv[1],"--before")==0;
 const auto read=[](const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 json::Value reviews;if(!json::parse(read("tools/world_event_reviews.json"),reviews))return 2;
 int failures=0,checked=0,transfers=0,mechanisms=0;
 const auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 fixture::real_event_labels=fixture::capture_cells=true;
 fixture::captured_object_type=2;fixture::captured_class="EventTriggerEntity";fixture::captured_go_name=L"Event";
 for(int n=1;n<=6;++n){
  const auto prefix="data/ff"+std::to_string(n)+"/";
  const auto archived="build_check/silent_context_20261029/before/ff"+std::to_string(n)+"_";
  const auto bytes=read((before?archived:prefix)+"maps.bin");if(!baked::load(bytes.data(),bytes.size()))return 2;
  if(!json::parse(read((before?archived:prefix)+"eventlabels.json"),fixture::event_metadata))return 2;
  fixture::selected_game=static_cast<game::Id>(n);
  for(const auto& row:reviews.arr){
   const auto* grammar=row.get(L"review_grammar");if(int(row.get(L"game")->n)!=n||!grammar||!grammar->s.starts_with(L"contextual_"))continue;
   const auto* map=baked::submap_by_name(row.get(L"submap")->s);if(!map)return 2;
   const auto& grid=baked::grids(*map);const baked::Object* source=nullptr;
   for(const auto& object:grid.objects)if(object.type==baked::ObjType::Event&&object.entity_id()==int(row.get(L"entity_id")->n)&&
       baked::object_identifier(object)==row.get(L"raw")->s&&baked::str(object.group)==row.get(L"group")->s)source=&object;
   if(!source)return 2;
   fixture::map=map->first_map_id;fixture::talk.id=source->entity_id();fixture::talk.name=row.get(L"raw")->s;
   fixture::talk.script=int(row.get(L"script_id")->n);fixture::talk.action=int(row.get(L"action_id")->n);
   fixture::pose={float(source->tx),float(source->ty),0};
   const auto* expected_exit=baked::world_exit_at(grid,source->entity_id(),fixture::talk.name,source->tx,source->ty);
   auto native=convert(&fixture::object);const bool transfer=grammar->s==L"contextual_transfer"||(grammar->s==L"contextual_conditional"&&expected_exit);
   expect(native!=nullptr,"real reviewed event survives the actual native detector chain");
   if(native){
    expect(native->kind==(transfer?Kind::MapExit:Kind::Event),"native detector publishes the correct event/exit type");
    expect(native->category()==(transfer?Category::MapExits:Category::Events),"native event is in exactly its proper destination category");
    if(transfer){const auto* final=row.get(L"final_transfer");const auto* primary=row.get(L"primary_transfer");
     expect(native->dest_map_id==(final?int(final->arr[0].n):primary?int(primary->arr[0].n):expected_exit->a),"native exit retains its exact destination");}
    else expect(native->name==row.get(L"label")->s,"native event speaks the source-reviewed functional label");
   }
   baked_nav::RemoteObject remote;remote.map_id=map->first_map_id;remote.type=source->type;
   remote.entity_id=source->entity_id();remote.raw_name=fixture::talk.name;remote.cell={source->layer,source->tx,source->ty};remote.event_type=L"Event";
   const auto* exit=baked::world_exit_at(grid,remote.entity_id,remote.raw_name,source->tx,source->ty);
   remote.world_exit=exit!=nullptr;remote.dest_map_id=exit?exit->a:0;
   RemoteBuilder builder({remote});builder.step();auto published=build_remote(builder.input);
   expect(published.size()==1,"actual remote publication retains the reviewed destination");
   if(published.size()==1){
    expect(published[0]->category()==(transfer?Category::MapExits:Category::Events),"remote category agrees with the native detector");
    if(!transfer)expect(published[0]->name==row.get(L"label")->s,"remote functional label agrees with native output");
   }
   ++checked;if(transfer)++transfers;else ++mechanisms;
  }
 }
 expect(checked>=368,"all current contextual reviews exercise both production producers");
 expect(fixture::wrong_owner==0,"all native observation stays on its owner");
 std::printf("Contextual scanner %s: %d records/%d transfers/%d mechanisms, %d failures, owner%d; physical routing separately verified\n",
  before?"before":"after",checked,transfers,mechanisms,failures,fixture::wrong_owner);
 return failures?1:0;
}
