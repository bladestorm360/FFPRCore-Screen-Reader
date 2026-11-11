// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Weighted world distances must equal the minimum of independent unit floods.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main() {
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    int failures=0,profiles=0,worlds=0,searches=0;size_t compared=0;double weighted_ms=0,old_ms=0,new_ms=0;
    const auto expect=[&](bool value,const char* why){if(!value){++failures;if(failures<=16)std::printf("FAIL %s\n",why);}};
    for(int game_id=1;game_id<=6;++game_id) {
        const auto path="data/ff"+std::to_string(game_id)+"/maps.bin";std::ifstream stream(path,std::ios::binary);
        const std::string bytes(std::istreambuf_iterator<char>(stream),{});
        if(!baked::load(bytes.data(),bytes.size()))return 2;
        fixture::game_id=static_cast<game::Id>(game_id);
        if(game_id==3)for(int id:{10,11}) {
            const auto* map=baked::submap_by_map_id(id==10?4:1);const auto& grid=baked::grids(*map);
            const auto* row=baked::transport(id);
            baked_route::Request request;request.submap=map;request.transportation_id=id;
            request.vehicle_group=row->group;request.vehicle_type=row->type;request.landing_group=row->landing_group;
            request.start=id==10?Cell{1,32,216}:Cell{1,64,64};
            for(const auto goal:id==10?std::vector<Cell>{{1,152,74},{1,125,184},{1,1,150}}:
                                      std::vector<Cell>{{1,127,43},{1,60,8},{1,46,57}}) {
                request.goals={goal};
                baked_route::adaptive_world_potential(false);auto began=std::chrono::steady_clock::now();const auto reference=baked_route::find(request);
                old_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                baked_route::adaptive_world_potential(true);began=std::chrono::steady_clock::now();const auto optimized=baked_route::find(request);
                new_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                const auto cost=[&](const baked_route::Route& route) {
                    int result=0,last=-1;
                    for(size_t i=1;i<route.cells.size();++i) {
                        const auto& a=route.cells[i-1];const auto& b=route.cells[i];
                        const int dx=(b.x-a.x+grid.width)%grid.width,dy=(b.y-a.y+grid.height)%grid.height;
                        const int heading=dx?(dx<=grid.width/2?1:3):(dy<=grid.height/2?2:0);
                        result+=10*(std::min(dx,grid.width-dx)+std::min(dy,grid.height-dy))+(last>=0&&last!=heading?15:0);last=heading;
                    }return result;
                };
                expect(reference.found==optimized.found&&(!reference.found||
                    (reference.mountain_actions==optimized.mountain_actions&&cost(reference)==cost(optimized))),
                    "Invincible retains exact mountain-action/physical-distance/turn objective");++searches;
            }
        }
        std::vector<std::pair<int,int>> modes{{0,0}};
        for(int kind:{2,3})for(const auto& row:baked::transports())
            if((kind==2?(row.type==2||row.type==6):(row.type==3||row.type==8))&&row.group>0) {
                modes.push_back({row.group,row.type});break;
            }
        for(size_t index=0;index<baked::submap_count();++index) {
            const auto* map=baked::submap_at(index);if(!map||!map->world)continue;++worlds;
            const auto& grid=baked::grids(*map);
            for(const auto& [group,type]:modes) {
                // Ordinary movement policy: specialized Enterprise conversions
                // and Invincible mountain actions retain their separate queue.
                baked_route::Request request;request.submap=map;request.vehicle_group=group;request.vehicle_type=type;
                std::optional<Cell> seed;
                for(int y=0;y<grid.height&&!seed;++y)for(int x=0;x<grid.width&&!seed;++x) {
                    Cell at{1,x,y};if(!baked_route::open(request,grid,at)||baked_route::terminal_transition(request,grid,at))continue;
                    std::array<Cell,8> next;if(baked_route::successors(request,at,next))seed=at;
                }
                if(!seed)continue;
                auto base=baked_route::flood(request,{*seed});std::vector<Cell> points{*seed};
                for(int y=0;y<grid.height&&points.size()<3;++y)for(int x=0;x<grid.width&&points.size()<3;++x) {
                    Cell at{1,x,y};if(base.distance(at)>0&&!baked_route::terminal_transition(request,grid,at))points.push_back(at);
                }
                if(points.size()<2)continue;
                // Compare the adaptive search with the unchanged Manhattan
                // heading-state A* on a distant real reachable endpoint. Equal
                // weighted cost preserves both distance and turn objectives.
                std::optional<Cell> distant;int farthest=-1;
                for(int y=0;y<grid.height;++y)for(int x=0;x<grid.width;++x) {
                    Cell at{1,x,y};const int distance=base.distance(at);
                    if(distance>farthest&&!baked_route::terminal_transition(request,grid,at)){distant=at;farthest=distance;}
                }
                if(distant)for(bool blocked:{false,true}) {
                    auto query=request;query.start=*seed;query.goals={*distant};
                    if(blocked)query.blocked.push_back(*distant);
                    const auto cost=[&](const baked_route::Route& route) {
                        int result=0,last=-1;
                        for(size_t i=1;i<route.cells.size();++i) {
                            const int heading=baked_route::step_direction(route.cells[i-1],route.cells[i],grid.width,grid.height);
                            result+=10+(last>=0&&last!=heading?15:0);last=heading;
                        }
                        return result;
                    };
                    baked_route::adaptive_world_potential(false);
                    auto began=std::chrono::steady_clock::now();const auto reference=baked_route::find(query);
                    old_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                    baked_route::adaptive_world_potential(true);
                    began=std::chrono::steady_clock::now();const auto optimized=baked_route::find(query);
                    new_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
                    expect(reference.found==optimized.found&&(!reference.found||cost(reference)==cost(optimized)),"adaptive world search preserves reference reachability and exact turn/distance cost");
                    if(blocked)expect(!optimized.found,"both search modes reject the native-layer blocked endpoint");
                    ++searches;
                }
                if(group==0) {
                    auto blocked=request;blocked.blocked={points[1]};
                    const auto closed=baked_route::flood(blocked,{points.front()});
                    expect(closed.distance(points[1])<0,"world flood honors a native logical-layer1 sparse blocker");
                }
                std::vector<Cell> starts=points;std::vector<int> costs;
                for(size_t i=0;i<points.size();++i)costs.push_back(static_cast<int>(i*7+3));
                starts.push_back(points[0]);costs.push_back(0);
                starts.push_back(points[1]);costs.push_back(-1);
                starts.push_back({1,-1,-1});costs.push_back(5);
                const auto begin=std::chrono::steady_clock::now();
                const auto actual=baked_route::flood(request,starts,costs);
                weighted_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
                std::vector<uint16_t> expected(static_cast<size_t>(grid.width)*grid.height,0xFFFF);
                for(size_t i=0;i<points.size()+1;++i) {
                    const auto single=baked_route::flood(request,{starts[i]});
                    for(size_t cell=0;cell<expected.size();++cell)if(single.dist[0][cell]!=0xFFFF)
                        expected[cell]=std::min(expected[cell],static_cast<uint16_t>(std::min(0xFFFE,int(single.dist[0][cell])+costs[i])));
                }
                expect(actual.dist[0]==expected,"weighted world distances equal independent unit-cost shortest paths at every cell");
                compared+=expected.size();++profiles;
            }
        }
    }
    std::printf("World weighted flood: %d worlds, %d profiles, %zu cells, %d failures, %.3fms weighted compute; not game latency\n",
        worlds,profiles,compared,failures,weighted_ms);
    std::printf("World adaptive search: %d reference pairs, %.3fms original / %.3fms adaptive, %d failures; not game latency\n",searches,old_ms,new_ms,failures);
    return failures?1:0;
}
