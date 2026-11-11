// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual FFVI light-puzzle regions and production route output. Original mod
// fixture only; game scripts/disassembly stay in the external evidence archive.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#undef FFPR_ROUTE_FIXTURE_ONLY
int main() {
    using namespace field;using namespace baked_nav;
    const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff6/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
    const auto* room=baked::submap_by_map_id(320);if(!room)return 2;
    const auto& grid=baked::grids(*room);
    int failures=0;const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
    const auto physical=[&](int id)->const baked::Object* {
        for(const auto& object:grid.objects)if(object.entity_id()==id&&
            (object.type==baked::ObjType::Event||(id==4&&object.type==baked::ObjType::GotoMap)))return &object;
        return nullptr;
    };
    // Native scripts assign these candidate values, accepting exactly the
    // previous value. Four other regions assign99 and always fail the route.
    const std::map<int,int> checkpoints{{17,1},{20,2},{25,2},{33,3},{29,4},{28,5},{30,6},{24,7},
        {23,8},{18,9},{19,10},{22,11},{35,12},{31,13},{32,14},{21,99},{26,99},{27,99},{34,99}};
    const int ordered[]{17,20,33,29,28,30,24,23,18,19,22,35,31,32};
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);
    flags.current_map_id=320;flags.scenario[67]=1;flags.scenario[68]=1;flags.scenario[558]=1;
    map_state::Resolver arrival_state(flags,fixture::records);
    const auto* entrance=point_in(*room,1,arrival_state);
    if(!entrance)return 2;
    Cell at{entrance->layer,entrance->tx,entrance->ty};int progress=0,resumes=0;
    std::printf("Light native south arrival%d,%d L%d\n",at.x,at.y,at.layer);
    std::thread worker([&] {
        const auto began=std::chrono::steady_clock::now();
        const auto route_from=[&](Cell start,int stage,int id) {
            const auto* object=physical(id);expect(object!=nullptr,"native checkpoint has one physical baked Event");if(!object)return PathInfo{};
            auto current=flags;current.local[1]=stage;
            RouteSnapshot snapshot(map_state::Resolver(current,fixture::records));snapshot.map_id=320;snapshot.submap=room;
            snapshot.layer=1;snapshot.position=world_of_cell(*room,start);snapshot.locale=L"en";snapshot.vehicle={true,1,1,{}};
            snapshot.goal.map_id=320;snapshot.goal.entity_id=id;snapshot.goal.raw_name=baked::object_identifier(*object);
            snapshot.goal.avoid_other_events=true;
            snapshot.goal.cell={object->layer,object->tx,object->ty};snapshot.kind=id==4?TargetKind::Transition:TargetKind::Spot;
            for(const auto& source:grid.objects)if(source.type==baked::ObjType::Event) {
                NativeObstacle observed;observed.id=source.entity_id();observed.type=2;
                observed.raw_name=baked::object_identifier(source);observed.cell={source.layer,source.tx,source.ty};
                observed.trigger=!source.contains(start.x,start.y);snapshot.obstacles.push_back(std::move(observed));
            }
            auto planned=plan_snapshot(snapshot).path;
            if(stage==10&&id==22&&start.x==16&&start.y==20) {
                SnapshotScope inspect(snapshot);const auto live=live_blockers(*room,grid,{});
                const auto* other=physical(23);
                std::printf("Escape witness: native%zu hazards%zu bad-hazard%d consumed%d helper%d\n",snapshot.obstacles.size(),live.triggers.size(),
                    std::any_of(live.triggers.begin(),live.triggers.end(),[&](const Cell& c){return c.x==other->tx&&c.y==other->ty;}),
                    snapshot.state.consumed(*room,*other),baked::transition_helper_at(grid,23,baked::object_identifier(*other),other->tx,other->ty));
                auto request=walk_request(*room,start);request.goals={snapshot.goal.cell};request.blocked=live.hard();live.apply_transport(request);
                request.directional=live.directional;request.live_hazards=request.live_traps=true;request.hazards=live.triggers;request.traps=live.traps;request.allow_trap_goal=true;
                const auto direct=baked_route::find(request);
                std::printf("Escape direct hazard-respecting proof: found%d cells%zu\n",direct.found,direct.cells.size());
            }
            return planned;
        };
        const auto validate=[&](const PathInfo& path,int stage,int goal) {
            expect(path.success,"native light objective has a legal production route");
            int witnessed=stage;std::vector<int> touched;
            for(const auto& phase:path.walk_phases)for(size_t i=1;i<phase.cells.size();++i) {
                const auto& cell=phase.cells[i];
                for(const auto& [entity,value]:checkpoints)if(const auto* marker=physical(entity);marker&&marker->contains(cell.x,cell.y)) {
                    if(i>1&&marker->contains(phase.cells[i-1].x,phase.cells[i-1].y))continue;
                    touched.push_back(entity);if(value!=witnessed+1)expect(false,"route crosses an out-of-order/failure region");witnessed=value;
                }
            }
            expect(witnessed==stage+(goal==4?0:1),"actual route reaches exactly its expected native stage");
            return touched;
        };
        for(const int id:ordered) {
            const auto* object=physical(id);if(!object)continue;
            const auto path=route_from(at,progress,id);const auto touched=validate(path,progress,id);
            // Each natural intermediate position must resume against the same
            // native progress, not silently skip/repeat a region after an input.
            for(const auto& phase:path.walk_phases)for(size_t i=1;i+1<phase.cells.size();++i) {
                if(object->contains(phase.cells[i].x,phase.cells[i].y))continue; // native progress already advanced here
                const auto resumed=route_from(phase.cells[i],progress,id);validate(resumed,progress,id);++resumes;
            }
            std::printf("Light checkpoint%d entity%d from%d,%d to%d,%d route%d cells%d entered",progress+1,id,at.x,at.y,object->tx,object->ty,path.success,path.step_count);
            for(const int entity:touched)std::printf("/%d",entity);std::printf("\n");
            bool entered=false;
            for(const auto& phase:path.walk_phases)for(size_t i=1;i<phase.cells.size()&&!entered;++i)
                if(object->contains(phase.cells[i].x,phase.cells[i].y)){at=phase.cells[i];entered=true;}
            expect(entered,"native progress advances at the actual first region entry");++progress;
        }
        const auto exit=route_from(at,14,4);validate(exit,14,4);
        expect(exit.success&&exit.step_count>0,"completed path still guides the actual northern GotoMap exit");
        // The native ring escape can advance either adjacent region9/10 to
        // stage10. Its actual current pose need not be the ordinary checkpoint.
        for(int entity:{18,19})if(const auto* escaped=physical(entity)) {
            const auto resumed=route_from({escaped->layer,escaped->tx,escaped->ty},10,22);
            std::vector<int> touched;
            if(entity==18)expect(!resumed.success&&resumed.reason==L"No safe route found"&&resumed.walk_phases.empty(),
                "an isolated escaped pose cannot fall back through a wrong checkpoint");
            else touched=validate(resumed,10,22);
            ++resumes;
            std::printf("Light escaped region%d: next-map%d required-trap%d entered",entity,resumed.next_map_id,resumed.required_trap);
            for(int id:touched)std::printf("/%d",id);std::printf("\n");
        }
        std::printf("Light complete owned route/retry episode %.3fms\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
    });worker.join();
    expect(fixture::wrong_owner==0,"light-puzzle computation uses no native API");
    std::printf("Narshe light route: %d intermediate resumes, %d failures; owner%d\n",resumes,failures,fixture::wrong_owner.load());return failures?1:0;
}
