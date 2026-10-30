// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual type11 capture and real-source bake/state consumers; no game loaded.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 auto read=[](const std::string& path){std::ifstream file(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(file),{});};
 json::Value cases;if(!json::parse(read("../tools/map_event_audit_20261028/effect_inventory/collision_cases.json"),cases))return 2;
 expect(cases.arr.size()==101,"all-six exact source inventory contains101 collision-enabled effects");
 int route_witnesses=0,detours=0,world_modes=0;
 auto check_capture=[&](const baked::Submap& sm,const baked::Grids& grid,const baked::Object& effect,const std::wstring& name){
   const Cell origin{int(effect.layer),effect.tx,effect.ty};
   auto flags=std::make_unique<map_state::Flags>();auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
   snapshot->submap=&sm;snapshot->map_id=sm.first_map_id;snapshot->layer=1;snapshot->vehicle={true,1,1,L""};
   fixture::capture_ready=fixture::capture_transport=true;fixture::transport_property.type=11;fixture::transport_property.id=effect.entity_id();
   fixture::current_position=world_of_cell(sm,origin);fixture::capture_raw_name=name;
   auto raw=capture_obstacle(&fixture::transport_actor,*snapshot);fixture::capture_ready=fixture::capture_transport=false;
   expect(raw.type==11&&raw.id==effect.entity_id()&&!raw.npc&&!raw.trigger,"actual capture preserves native type11 without an Entity/NPC/scene stand-in");
   // This host has no Unity layer object. Downstream checks use the independently
   // verified source layer; they do not claim to test native layer acquisition.
   raw.cell={sm.world?-1:origin.layer,origin.x,origin.y};snapshot->obstacles={raw};
   std::thread worker([&]{SnapshotScope scope(*snapshot);
    auto live=live_blockers(sm,grid,{});
    // capture_obstacle normalizes world layers to -1 independently of Unity's
    // native layer API. Keep that production behavior in this native-free host.
    expect(std::find(live.gates.begin(),live.gates.end(),raw.cell)!=live.gates.end(),"current type11 effect blocks its real native footprint");
    if(sm.world)for(const auto& mode:baked::transports())if(mode.id==13||mode.id==11){
     bool witnessed=false;
     for(const auto delta:{Cell{0,1,0},Cell{0,0,1}}){
      baked_route::Request bare;bare.submap=&sm;bare.vehicle_group=mode.group;bare.vehicle_type=mode.type;bare.transportation_id=mode.id;
      bare.live_hazards=bare.live_traps=true;bare.ignore_hazards=true;
      bare.start={1,origin.x-delta.x,origin.y-delta.y};bare.goals={{1,origin.x+delta.x,origin.y+delta.y}};
      const auto direct=baked_route::find(bare);if(!direct.found||std::find(direct.cells.begin(),direct.cells.end(),origin)==direct.cells.end())continue;
      auto blocked=bare;blocked.blocked=live.hard();live.apply_transport(blocked);const auto safe=baked_route::find(blocked);witnessed=true;
      expect(!safe.found||std::find(safe.cells.begin(),safe.cells.end(),origin)==safe.cells.end(),"real world solid effect retains collision in actual ship and Invincible movement modes");
     }if(witnessed)++world_modes;
    }
    if(!sm.world&&baked::name_of(sm)==L"Map_20060"){
     // Use the real native layer/cells and actual find() consumer, rather than
     // just checking that an obstacle was appended. Other scene state is not
     // asserted by this isolated geometry witness.
     for(const auto delta:{Cell{0,1,0},Cell{0,0,1}}){
      baked_route::Request bare;bare.submap=&sm;bare.live_hazards=bare.live_traps=true;bare.ignore_hazards=true;
      bare.start={origin.layer,origin.x-delta.x,origin.y-delta.y};bare.goals={{origin.layer,origin.x+delta.x,origin.y+delta.y}};
      const auto direct=baked_route::find(bare);
      if(!direct.found||std::find(direct.cells.begin(),direct.cells.end(),origin)==direct.cells.end())continue;
      auto blocked=bare;blocked.blocked=live.hard();live.apply_transport(blocked);const auto safe=baked_route::find(blocked);
      ++route_witnesses;if(safe.found)++detours;
      expect(!safe.found||std::find(safe.cells.begin(),safe.cells.end(),origin)==safe.cells.end(),"real Karnak route cannot cross its observed solid flame");
     }
    }
    snapshot->obstacles.clear();live=live_blockers(sm,grid,{});expect(live.gates.empty(),"hidden/absent current effects cannot resurrect baked walls");
   });worker.join();
   // The existing anticipated-release API supplies future static geometry.
   // Its old live effect must not be added again after that projection hides it.
   json::Value projected,record;projected.type=json::Value::Type::Object;
   const auto program="{\"entry\":0,\"code\":[[1,\""+text::to_utf8(baked::str(effect.group))+"\"],[10,0,"+
       std::to_string(effect.entity_id())+",0,-2147483648,-2147483648],[11,0],[6]]}";
   expect(json::parse(program,record),"owned anticipated effect fixture parses");projected.obj[baked::name_of(sm)]=std::move(record);
   auto future=std::make_shared<map_state::Flags>();
   // Resolver owns a reference to its immutable program; construct the future
   // snapshot with that owner instead of assigning over an existing resolver.
   auto release=std::make_unique<RouteSnapshot>(map_state::Resolver(*future,projected));
   release->submap=&sm;release->map_id=sm.first_map_id;release->layer=1;release->vehicle={true,1,1,L""};
   release->anticipated_release=future;release->obstacles={raw};
   std::thread anticipated([&]{SnapshotScope scope(*release);const auto observed=live_blockers(sm,grid,{});release->obstacles.clear();const auto baseline=live_blockers(sm,grid,{});
    expect(observed.gates==baseline.gates,"anticipated release uses projected effect geometry without resurrecting its old live wall");
   });anticipated.join();
 };
 for(int n:{3,4,5}){
  auto bytes=read("data/ff"+std::to_string(n)+"/maps.bin");if(!baked::load(bytes.data(),bytes.size()))return 2;
  if(!json::parse(read("data/ff"+std::to_string(n)+"/mapstate.json"),fixture::records))return 2;
  fixture::game_id=static_cast<game::Id>(n);g_loaded=g_prepared=true;
  int found=0,expected=0;
  for(const auto& row:cases.arr){
   auto num=[&](const wchar_t* key){return int(row.get(key)->n);};
   if(num(L"game")!=n)continue;++expected;
   const auto* sm=baked::submap_by_name(row.get(L"submap")->s);expect(sm!=nullptr,"real effect submap exists");if(!sm)continue;
   const auto& grid=baked::grids(*sm);const baked::Object* effect=nullptr;
   for(const auto& object:grid.objects)if(int(object.type)==11&&object.entity_id()==num(L"id")&&
       baked::object_identifier(object)==row.get(L"raw")->s&&baked::str(object.group)==row.get(L"group")->s&&
       object.tx==num(L"x")&&object.ty==num(L"y")&&object.layer==num(L"layer"))effect=&object;
   if(!effect){
    if(n==5&&row.get(L"submap")->s==L"Map_20060"&&num(L"id")==90){
     // Before the bake correction, retain one original owned prototype with the
     // real source type/id/cell/size to isolate the live classification failure.
     auto copy=grid;baked::Object prototype{};prototype.type=static_cast<baked::ObjType>(11);
     prototype.c=90;prototype.d=1;prototype.layer=uint8_t(num(L"layer"));prototype.tx=int16_t(num(L"x"));prototype.ty=int16_t(num(L"y"));
     prototype.tw=uint16_t(num(L"width"));prototype.th=uint16_t(num(L"height"));copy.objects.push_back(prototype);
     check_capture(*sm,copy,prototype,L"");
    }
    continue;
   }++found;
   expect((effect->d&1)&&effect->tw==num(L"width")&&effect->th==num(L"height"),"exact effect collision footprint survives the bake");
   const auto origin=Cell{int(effect->layer),effect->tx,effect->ty};
   // Exercise existing initializer projection with one original fixture program,
   // retaining the real native group/id. No extracted game script is embedded.
   std::vector<Cell> hidden_obstacles;
   for(int state:{0,1,2}){
    auto flags=std::make_unique<map_state::Flags>();json::Value records;json::Value record;
    std::string program="{\"entry\":0,\"code\":[[1,\""+text::to_utf8(row.get(L"group")->s)+"\"],[10,0,"+
       std::to_string(effect->entity_id())+","+std::to_string(state==0?0:1)+","+
       std::to_string(state==2?origin.x+2:INT_MIN)+","+std::to_string(state==2?origin.y+1:INT_MIN)+"],[11,0],[6]]}";
    expect(json::parse(program,record),"owned visibility/position fixture parses");
    records.type=json::Value::Type::Object;records.obj[baked::name_of(*sm)]=std::move(record);
    map_state::Resolver resolver(*flags,records);baked_route::Request movement;movement.submap=sm;
    map_state::add_obstacles(resolver,*sm,movement,true,true);
    const auto effective=resolver.effective(*sm,*effect);const Cell at{sm->world?-1:int(effect->layer),effective.tx,effective.ty};
    expect(resolver.active(*sm,*effect,true)==(state!=0),"future projection follows the effect's own actual visibility");
    // Other colliders/effects may share this real cell. Hiding this one must
    // remove its contribution, not promise that every other obstacle vanishes.
    if(state==0)hidden_obstacles=movement.blocked;
    else expect(std::count(movement.blocked.begin(),movement.blocked.end(),at)>
                std::count(hidden_obstacles.begin(),hidden_obstacles.end(),at),"visible/moved effect contributes its own collision alongside existing geometry");
    if(state==2)expect(effective.tx==origin.x+2&&effective.ty==origin.y+1,"future geometry uses the moved effect rather than its baked home");
   }
   if(!((n==5&&row.get(L"submap")->s==L"Map_20060")||(n==3&&sm->world)))continue;
   check_capture(*sm,grid,*effect,row.get(L"raw")->s);
  }
  expect(found==expected,"every real collision-enabled effect is retained under native type11");
  std::printf("FF%d effect inventory: %d/%d actual native footprints\n",n,found,expected);
 }
 expect(route_witnesses>0,"real Karnak flame cells supply at least one bare-route collision counterexample");
 expect(world_modes==2,"real world whirlpool route is checked in Enterprise ship13 and floating Invincible11 modes");
 std::printf("Karnak real-cell route witnesses: %d; reachable detours: %d\n",route_witnesses,detours);
 std::printf("World solid-effect modes checked: %d\n",world_modes);
 expect(fixture::wrong_owner==0,"effect geometry never queries native objects from its worker");
 std::printf("Field effects: %d failures\n",failures);return failures?1:0;
}
