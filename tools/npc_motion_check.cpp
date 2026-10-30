// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact route assets remain outside Git. Exercise the actual owned live-goal
// overlay at their native positions, without certifying sampled story states.
#define main existing_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include <set>

int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    using namespace field;using namespace baked_nav;namespace r=field::baked_route;
    const auto read=[](const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    json::Value contexts;
    if(!json::parse(read("../tools/npc_geometry_context_20261029/contexts.json"),contexts))return 2;
    int failures=0,routes=0,positions=0,reachable=0;
    const auto expect=[&](bool ok,const char* message){if(!ok){++failures;std::printf("FAIL %s\n",message);}};
    for(int game_number:{5,6}) {
      json::Value assets;
      if(!json::parse(read("../tools/npc_geometry_context_20261029/ff"+std::to_string(game_number)+"_route_assets.json"),assets))return 2;
      const auto bytes=read("data/ff"+std::to_string(game_number)+"/maps.bin");if(!baked::load(bytes.data(),bytes.size()))return 2;
      fixture::game_id=static_cast<game::Id>(game_number);
      for(const auto& c:contexts.get(L"cases")->arr) {
        if(int(c.get(L"game")->n)!=game_number)continue;
        const auto* sm=baked::submap_by_name(c.get(L"submap")->s);if(!sm)return 2;
        const auto& grid=baked::grids(*sm);const baked::Object* authored=nullptr;
        for(const auto& o:grid.objects)if((o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC)&&
            o.entity_id()==int(c.get(L"id")->n)&&baked::str(o.name)==c.get(L"raw")->s&&baked::str(o.group)==c.get(L"group")->s)authored=&o;
        if(!authored)return 2;
        std::set<int> route_ids;
        for(const auto& script:c.get(L"local_motion")->arr)for(const auto& m:script.get(L"instructions")->arr)
            if(m.get(L"mnemonic")->s==L"AssignMoveRoute"){
                const auto& v=m.get(L"operands")->get(L"iValues")->arr;
                if(v.size()>=2&&int(v[0].n)==authored->entity_id())route_ids.insert(int(v[1].n));
            }
        for(int id:route_ids) {
            const std::wstring tail=L"/"+baked::name_of(*sm)+L"/route_instruction_"+std::to_wstring(id)+L".json";
            const json::Value* asset=nullptr;
            for(const auto& entry:assets.get(L"assets")->obj)if(entry.first.size()>=tail.size()&&entry.first.compare(entry.first.size()-tail.size(),tail.size(),tail)==0)asset=&entry.second;
            if(!asset)return 2;++routes;int local_positions=0,local_reachable=0;
            const auto& bounds=c.get(L"bounds")->arr;
            r::Request request;request.submap=sm;request.passages=true;request.ignore_hazards=true;request.live_relays=true;
            std::vector<r::Cell> seeds;
            for(const auto& o:grid.objects){
                if(o.type==baked::ObjType::PointIn)seeds.push_back({o.layer,o.tx,o.ty});
                if(o.type==baked::ObjType::RelayInteraction){const auto g=baked::str(o.group);if(g.empty()||g==L"entity_default"||g==L"default"||o.group==authored->group)request.relays.push_back(o);}
            }
            const auto flood=r::flood(request,seeds);
            for(const auto& instruction:asset->get(L"body")->get(L"instruction_list")->arr){
                const int type=int(instruction.get(L"instruction_type")->n);if(type!=1&&type!=7)continue;
                const Cell live{int(instruction.get(L"layer")->n),int(instruction.get(L"cellx")->n)-int(bounds[0].n),int(instruction.get(L"celly")->n)-int(bounds[1].n)};
                RouteSnapshot snapshot{map_state::Resolver()};snapshot.submap=sm;snapshot.kind=TargetKind::Interact;
                snapshot.map_id=std::stoi(c.get(L"map_aliases")->arr.front().get(L"id")->s);
                snapshot.goal.map_id=snapshot.map_id;snapshot.goal.entity_id=authored->entity_id();snapshot.goal.raw_name=baked::object_identifier(*authored);
                snapshot.goal.cell={authored->layer,authored->tx,authored->ty};
                NativeObstacle actor;actor.npc=true;actor.id=authored->entity_id();actor.raw_name=snapshot.goal.raw_name;actor.cell=live;
                snapshot.obstacles.push_back(actor);baked::Object projected;
                g_route_snapshot=&snapshot;const bool copied=live_interaction(*sm,live,TargetKind::Interact,projected);g_route_snapshot=nullptr;
                expect(copied&&projected.tx==live.x&&projected.ty==live.y&&projected.layer==live.layer,"actual native position/layer overrides authored home");
                expect(projected.entity_id()==authored->entity_id()&&projected.b==authored->b,"native motion preserves source identity and talk-direction mask");
                const auto* identity=baked_by_entity_id(grid,actor.id,authored->type,live,actor.raw_name);
                expect(identity==authored,"moved actor resolves immutable source identity across native layers");
                const auto goals=r::goals_for(request,live,r::GoalKind::Interact,&projected);
                bool accessible=false;for(const auto& goal:goals.cells)accessible|=flood.distance(goal)>=0;
                ++local_positions;local_reachable+=accessible;
            }
            positions+=local_positions;reachable+=local_reachable;
            expect(local_reachable>0,"each exact assigned route has a real entry-connected talk position");
            std::printf("FF%d %ls/%d route%d: %d native waypoints, %d have entry-connected talk approaches\n",game_number,baked::name_of(*sm).c_str(),authored->entity_id(),id,local_positions,local_reachable);
        }
      }
    }
    // Two survey suspects have entry-connected terrain once the real
    // initializer replaces mutually exclusive transition masks. Reproduce their
    // projected NPC congestion through the full planner, preserving blockers.
    // Production bakes retain their immutable input bytes for process lifetime.
    // The per-game loop's buffer has ended; own a fresh FFVI buffer through all
    // following planner threads instead of using that released input memory.
    const auto ff6_bytes=read("data/ff6/maps.bin");if(!baked::load(ff6_bytes.data(),ff6_bytes.size()))return 2;
    if(!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
    g_loaded=g_prepared=true;int congested=0;
    for(const auto& c:contexts.get(L"cases")->arr){
        if(int(c.get(L"game")->n)!=6||!(c.get(L"submap")->s==L"Map_20051_4"||c.get(L"submap")->s==L"Map_40005"))continue;
        const auto* sm=baked::submap_by_name(c.get(L"submap")->s);if(!sm)return 2;
        const auto& alias=c.get(L"map_aliases")->arr.front();map_state::Flags flags;
        flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);
        flags.current_map_id=std::stoi(alias.get(L"id")->s);flags.current_area_id=std::stoi(alias.get(L"area_id")->s);
        RouteSnapshot snapshot{map_state::Resolver(flags,fixture::records)};snapshot.submap=sm;snapshot.map_id=flags.current_map_id;snapshot.locale=L"en";
        const baked::Object* target=nullptr;Cell entry;bool have_entry=false;
        for(const auto& original:baked::grids(*sm).objects){
            if(original.type==baked::ObjType::PointIn&&!have_entry){entry={original.layer,original.tx,original.ty};have_entry=true;}
            if(!snapshot.state.active(*sm,original,true))continue;const auto o=snapshot.state.effective(*sm,original);
            const bool npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;
            if(npc&&o.entity_id()==int(c.get(L"id")->n)&&baked::str(o.name)==c.get(L"raw")->s)target=&original;
            if(!npc&&o.type!=baked::ObjType::TreasureBox)continue;
            NativeObstacle actor;actor.cell={o.layer,o.tx,o.ty};actor.type=int(o.type);actor.id=o.entity_id();actor.npc=npc;actor.chest=!npc;actor.raw_name=baked::object_identifier(o);
            actor.movement_known=npc;actor.wanders=(o.d&16)!=0;snapshot.obstacles.push_back(actor);
        }
        if(!have_entry||!target)return 2;snapshot.position=world_of_cell(*sm,entry);snapshot.layer=entry.layer;
        snapshot.goal.map_id=snapshot.map_id;snapshot.goal.cell={target->layer,target->tx,target->ty};snapshot.goal.entity_id=target->entity_id();snapshot.goal.raw_name=baked::object_identifier(*target);snapshot.kind=TargetKind::Interact;
        Planned result;std::thread worker([&]{result=plan_snapshot(snapshot);});worker.join();
        expect(result.path.success,"real projected NPC congestion keeps a route or a named blocker approach");
        std::printf("FF6 %ls/%d projected source actors: success%d blocker%d steps%d\n",baked::name_of(*sm).c_str(),target->entity_id(),result.path.success,result.path.npc_approach,result.path.step_count);
        ++congested;
    }
    expect(congested==2,"both state/NPC congestion contexts use actual production planning");
    // The remaining FFII actor is on the scene's Ground stage; the native
    // player approaches three Bottom triggers instead. Use the actual mixed
    // story state, which neither all-zero nor all-one flag surveys represent.
    const auto ff2_bytes=read("data/ff2/maps.bin");if(!baked::load(ff2_bytes.data(),ff2_bytes.size()))return 2;
    if(!json::parse(read("data/ff2/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF2;const auto* scene=baked::submap_by_name(L"Map_30131_7");if(!scene)return 2;
    auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
    flags->scenario[74]=1;flags->scenario[75]=0;flags->current_map_id=273;flags->current_area_id=28;
    const baked::Object* entry=nullptr;const baked::Object* staged=nullptr;std::vector<const baked::Object*> triggers;
    for(const auto& object:baked::grids(*scene).objects){
        if(object.type==baked::ObjType::PointIn&&!entry)entry=&object;
        if(object.type==baked::ObjType::NPC&&object.entity_id()==4)staged=&object;
        if(object.type==baked::ObjType::Event&&object.entity_id()>=5&&object.entity_id()<=7)triggers.push_back(&object);
    }
    if(!entry||!staged)return 2;expect(triggers.size()==3,"three real FFII scene-control identities are retained");
    int scenes=0;
    std::thread scene_worker([&]{
      map_state::Resolver state(*flags,fixture::records);
      expect(state.active(*scene,*staged)&&staged->layer==1&&entry->layer==0,"native mixed-state stage and player layers are preserved");
      for(const auto* trigger:triggers){
        auto input=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));input->submap=scene;input->map_id=273;input->locale=L"en";
        input->position=world_of_cell(*scene,{entry->layer,entry->tx,entry->ty});input->layer=entry->layer;
        input->goal.map_id=273;input->goal.cell={trigger->layer,trigger->tx,trigger->ty};input->goal.entity_id=trigger->entity_id();
        input->goal.raw_name=baked::object_identifier(*trigger);input->kind=TargetKind::Infer;
        for(const auto* visible:triggers){NativeObstacle event;event.trigger=true;event.type=2;event.id=visible->entity_id();
            event.cell={visible->layer,visible->tx,visible->ty};event.raw_name=baked::object_identifier(*visible);input->obstacles.push_back(event);}
        const auto output=plan_snapshot(*input);
        expect(state.active(*scene,*trigger)&&output.path.success,"actual FFII scene-control route is usable in the native active story state");
        scenes+=output.path.success;
      }
      auto finished=*flags;finished.scenario[75]=1;map_state::Resolver complete(finished,fixture::records);
      expect(!complete.active(*scene,*staged),"completed native scene does not retain its staged actor");
    });scene_worker.join();
    std::printf("FF2 staged actor: %d/3 real mixed-state scene controls have production routes; no cross-layer NPC shortcut\n",scenes);
    expect(scenes==3,"every real scene approach is checked");
    expect(routes==11,"all eleven exact assigned routes are covered");expect(fixture::wrong_owner==0,"owned goal projection makes no native worker query");
    std::printf("Native-motion source/owned-goal check: %d routes, %d positions, %d entry-connected; %d failures; story prerequisites remain separate\n",routes,positions,reachable,failures);
    return failures?1:0;
}
