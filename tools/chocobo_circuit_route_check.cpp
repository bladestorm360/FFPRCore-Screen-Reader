// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#include "../src/games/ff3/chocobo_circuit.h"
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
using namespace field; using namespace baked_nav;
int failures=0;
void expect(bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}}
std::string bytes(const std::string& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
std::unique_ptr<RouteSnapshot> make(int map,Cell start,Cell goal,TargetKind kind){
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=map;
    auto out=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
    out->map_id=map;out->submap=baked::submap_by_map_id(map);out->layer=start.layer;out->locale=L"en";
    out->position=world_of_cell(*out->submap,start);out->goal.map_id=map;out->goal.cell=goal;out->kind=kind;return out;
}
Planned plan(RouteSnapshot& snapshot){Planned out;std::thread worker([&]{out=plan_snapshot(snapshot);});worker.join();return out;}
int main() {
    const auto data=bytes("data/ff3/maps.bin");
    expect(baked::load(data.data(),data.size()),"FFIII real bake loads");
    expect(json::parse(bytes("data/ff3/mapstate.json"),fixture::records),"FFIII initializers load");
    fixture::game_id=game::Id::FF3; g_loaded=g_prepared=true;
    const auto& points=ff3::chocobo_circuit::points;
    int total=0,checkpoint=0;double peak=0;auto begin=std::chrono::steady_clock::now();
    for(int i=1;i<14;++i) {
        auto snapshot=make(1,points[i-1],points[i],TargetKind::Spot);
        snapshot->vehicle={true,12,9,{}}; auto leg_start=std::chrono::steady_clock::now(); auto result=plan(*snapshot);
        peak=std::max(peak,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-leg_start).count());
        expect(result.path.success && result.path.walk_phases.size()==1,"mounted leg has one successful phase");
        if(!result.path.success || result.path.walk_phases.size()!=1)continue;
        const auto& phase=result.path.walk_phases.front();
        expect(phase.action_before.empty() && phase.action_after.empty() && phase.vehicle_before==route_progress::VehicleAction::None && !phase.manual_after,"no mount or transition action");
        expect(!phase.cells.empty() && phase.cells.front()==points[i-1] && phase.cells.back()==points[i],"leg endpoints match");
        baked_route::Request request;request.submap=snapshot->submap;request.vehicle_group=2;request.vehicle_type=9;
        const auto& grid=baked::grids(*snapshot->submap);
        for(size_t j=0;j<phase.cells.size();++j){
            const auto c=phase.cells[j];expect(baked_route::open(request,grid,c),"all circuit cells permit chocobo");
            if(j){const auto p=phase.cells[j-1];expect(abs(c.x-p.x)+abs(c.y-p.y)==1 && baked_route::edge_allowed(request,grid,p,c),"each cardinal edge obeys native riding rules");++total;}
            if(c==points[5] && (j || i==1))++checkpoint;
        }
    }
    const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
    expect(checkpoint==1,"checkpoint crossed once");expect(!fixture::wrong_owner,"worker performs no native reads");
    std::printf("Circuit: %d steps, %d checkpoint visits, %.3f ms whole loop, %.3f ms peak leg, %d failures\n",total,checkpoint,ms,peak,failures);
    return failures?1:0;
}
