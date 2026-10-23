// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual owned filter and planner on all-six bakes; native calls are fake owners.
#define main existing_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include "../src/core/field/save_points.h"

int main() {
    using namespace field;using namespace baked_nav;using namespace filter_worker;
    int failures=0,save_points=0,tutorials=0,decorations=0;
    double worst=0;
    const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    const auto read=[](const std::string& path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    for(int n=1;n<=6;++n) {
        fixture::game_id=static_cast<game::Id>(n);
        const auto bytes=read("data/ff"+std::to_string(n)+"/maps.bin");
        expect(baked::load(bytes.data(),bytes.size()),"actual game bake loads");
        expect(json::parse(read("data/ff"+std::to_string(n)+"/mapstate.json"),fixture::records),"actual game state loads");
        for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));
        g_loaded=g_prepared=true;
        bool moved_checked=false;
        for(int i=0;i<baked::submap_count()&&!moved_checked;++i) {
            const auto* sm=baked::submap_at(i);if(sm->world||sm->first_map_id<=0)continue;
            for(const auto& object:baked::grids(*sm).objects) {
                if(object.type!=baked::ObjType::Event||object.tw!=1||object.th!=1||!baked_route::step_event(object))continue;
                RouteSnapshot moved(map_state::Resolver(map_state::Flags{},fixture::records));
                moved.submap=sm;moved.map_id=sm->first_map_id;
                NativeObstacle trigger;trigger.cell={object.layer,-100,-100};trigger.type=2;trigger.id=object.entity_id();
                trigger.raw_name=baked::object_identifier(object);trigger.trigger=true;moved.obstacles.push_back(trigger);
                std::thread worker([&]{SnapshotScope scope(moved);const auto live=live_blockers(*sm,baked::grids(*sm),{});
                    expect(live.triggers==std::vector<Cell>({trigger.cell}),"all-six moved scene triggers retain the native current cell, never their baked home");});worker.join();
                moved_checked=true;break;
            }
        }
        expect(moved_checked,"each game supplies a native moved-trigger regression fixture");
        if(n==6) {
            const auto* sm=baked::submap_by_map_id(323);
            RouteSnapshot ymir(map_state::Resolver(map_state::Flags{},fixture::records));
            ymir.submap=sm;ymir.map_id=323;ymir.layer=1;ymir.position=world_of_cell(*sm,{1,20,10});
            ymir.goal.map_id=323;ymir.goal.cell={1,20,7};ymir.kind=TargetKind::Transition;ymir.locale=L"en";
            for(const auto& object:baked::grids(*sm).objects)if(object.type==baked::ObjType::Event&&object.entity_id()==11&&baked::str(object.group)==L"ev_e_0011") {
                NativeObstacle trigger;trigger.cell={object.layer,object.tx,object.ty};trigger.type=2;trigger.id=11;
                trigger.raw_name=baked::object_identifier(object);trigger.trigger=true;ymir.obstacles.push_back(trigger);
            }
            expect(ymir.obstacles.size()==1,"logged Ymir scene has one exact native trigger");
            Planned before,after;BrowsePlanned browse;
            std::thread active([&]{before=plan_snapshot(ymir);});active.join();
            expect(before.path.success&&(before.path.event_on_route||before.path.step_count>3),
                "active Ymir scene is still respected by a legal detour or an intervening-event warning");
            ymir.obstacles[0].cell={1,-100,-100};
            std::thread moved([&]{after=plan_snapshot(ymir);browse=plan_browse_snapshot(ymir);});moved.join();
            expect(after.path.success&&after.path.step_count==3&&!after.path.event_on_route&&browse.route.direct,
                "exact logged three-step Narshe exit route clears after native trigger movement without re-entering the map");
        }
        // Find an ordinary open area; a ring of captured blockers then gives
        // decisive gate versus transient-NPC evidence on actual movement grids.
        const baked::Submap* room=nullptr;Cell start;
        for(int i=0;i<baked::submap_count()&&!room;++i) {
            const auto* sm=baked::submap_at(i);if(sm->world||sm->first_map_id<=0)continue;
            const auto& grid=baked::grids(*sm);
            baked_route::Request request;request.submap=sm;
            for(int y=4;y<sm->height-4&&!room;++y)for(int x=4;x<sm->width-4&&!room;++x) {
                bool open=true;
                for(int dy=-2;dy<=2;++dy)for(int dx=-2;dx<=2;++dx)
                    open &= baked_route::open(request,grid,{1,x+dx,y+dy});
                for(const auto& object:grid.objects)
                    if(std::abs(object.tx-x)<4&&std::abs(object.ty-y)<4)open=false;
                if(open){room=sm;start={1,x,y};}
            }
        }
        expect(room!=nullptr,"each game supplies a decisive open movement fixture");if(!room)continue;
        auto snapshot=std::make_shared<ExitSnapshot>(true,map_state::Resolver(map_state::Flags{},fixture::records));
        snapshot->route.submap=room;snapshot->route.map_id=room->first_map_id;snapshot->route.layer=1;
        snapshot->route.position=world_of_cell(*room,start);
        for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx)if(dx||dy) {
            NativeObstacle gate;gate.type=19;gate.cell={-1,start.x+dx,start.y+dy};
            gate.collider=true;gate.collision=1;snapshot->route.obstacles.push_back(gate);
        }
        StoryTarget outside;outside.goal.map_id=room->first_map_id;outside.goal.cell={1,start.x+2,start.y};outside.kind=TargetKind::Spot;
        auto here=outside;here.goal.cell=start;
        auto lever=outside;lever.goal.cell={1,start.x+1,start.y};lever.kind=TargetKind::Interact;
        auto unknown=outside;unknown.goal.map_id=999999;
        std::vector<StoryTarget> targets{outside,here,lever,unknown};
        std::vector<bool> hidden;
        std::thread worker([&]{
            const auto began=std::chrono::steady_clock::now();
            hidden=story_gate_mask(*snapshot,targets,{});
            worst=std::max(worst,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
        });worker.join();
        expect(hidden==std::vector<bool>({true,false,false,false}),"hide proven gated target, retain reachable gate action and unknown map");
        FilterInput input;input.navigation=snapshot;input.story_gates=true;input.current_map=room->first_map_id;
        for(size_t i=0;i<2;++i) {
            FilterRow row;row.index=i;row.kind=Kind::NPC;row.category=Category::NPCs;row.cell_layer=1;
            row.position=world_of_cell(*room,targets[i].goal.cell);row.numbering_position=row.position;
            row.name=L"Soldier";row.label=npc_names::parse(row.name);input.rows.push_back(row);
        }
        // NPC approaches adjacent to the ring can be reached from its centre;
        // use actual Spot targets for this list-pruning assertion instead.
        for(auto& row:input.rows){row.kind=Kind::SavePoint;row.category=Category::SavePoints;}
        Categories filtered;
        std::thread list([&]{filtered=build_categories(input);});list.join();
        expect(filtered.views[0].order==std::vector<size_t>({1})&&filtered.views[static_cast<size_t>(Category::SavePoints)].order==std::vector<size_t>({1}),
            "story mask prunes All and the dedicated category together");
        input.story_gates=false;
        std::thread restored([&]{filtered=build_categories(input);});restored.join();
        expect(filtered.views[0].order.size()==2,"disabling filter restores original rows");
        for(auto& obstacle:snapshot->route.obstacles){obstacle.type=0;obstacle.npc=true;}
        std::thread npcs([&]{hidden=story_gate_mask(*snapshot,targets,{});});npcs.join();
        expect(!hidden[0],"NPC-only obstruction is never pruned as a story gate");
        for(auto& obstacle:snapshot->route.obstacles){obstacle.type=19;obstacle.npc=false;obstacle.collision=obstacle.cell.x==start.x?6:7;}
        std::thread axes([&]{hidden=story_gate_mask(*snapshot,targets,{});});axes.join();
        expect(hidden[0]&&!hidden[1],"native one-axis story colliders participate in gate proof");
        int polls=0;
        std::thread cancel([&]{hidden=story_gate_mask(*snapshot,targets,[&]{return ++polls>3;});});cancel.join();
        expect(std::none_of(hidden.begin(),hidden.end(),[](bool v){return v;}),"cancelled evidence cannot hide a target");
        snapshot->route.obstacles.clear();
        std::thread opened([&]{hidden=story_gate_mask(*snapshot,targets,{});});opened.join();
        expect(!hidden[0],"opened story gates restore their destinations");
        if(n==6)for(int i=0;i<baked::submap_count();++i) {
            const auto* sm=baked::submap_at(i);if(sm->first_map_id<=0)continue;
            for(const auto& point:baked::grids(*sm).objects)if(point.type==baked::ObjType::SavePoint) {
                ++save_points;
                FilterInput input;input.current_map=sm->first_map_id;
                const auto add=[&](const baked::Object& object,Kind kind) {
                    FilterRow row;row.index=input.rows.size();row.kind=kind;
                    row.category=kind==Kind::SavePoint?Category::SavePoints:Category::Events;
                    row.remote_map=sm->first_map_id;row.cell_layer=object.layer;row.cell_x=object.tx;row.cell_y=object.ty;
                    row.entity_id=object.entity_id();row.raw_name=baked::object_identifier(object);row.name=L"Save Point";
                    input.rows.push_back(std::move(row));
                };
                add(point,Kind::SavePoint);add(point,Kind::SavePoint);
                for(const auto& companion:baked::grids(*sm).objects)if(companion.group==point.group) {
                    if(save_points::tutorial(baked::object_identifier(companion))) {
                        expect(companion.a==20&&companion.b==2&&point.contains(companion.tx,companion.ty),"all save tutorials have the verified script/action and anchor");
                        ++tutorials;add(companion,Kind::Event);
                    }
                    if(companion.type==baked::ObjType::AnimEntity&&save_points::animation(baked::object_identifier(companion)))++decorations;
                }
                Categories views;std::thread worker([&]{views=build_categories(input);});worker.join();
                expect(views.views[0].order.size()==1&&views.views[static_cast<size_t>(Category::SavePoints)].order.size()==1&&
                    views.views[static_cast<size_t>(Category::Events)].order.empty(),"each native FF6 save point has one canonical save-category target");
            }
        }
        std::printf("FF%d story-filter and live-trigger fixtures completed\n",n);
    }
    expect(save_points==42&&tutorials==42&&decorations==42,"all 42 FF6 save-point records and companions are covered");
    expect(fixture::wrong_owner==0,"filter worker never calls a native or game-localization API");
    std::printf("Save points %d, tutorials %d, animations %d; worst gate batch %.3f ms; failures %d\n",save_points,tutorials,decorations,worst,failures);
    return failures?1:0;
}
