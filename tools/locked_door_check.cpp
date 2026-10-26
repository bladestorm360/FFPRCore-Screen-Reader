// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual selected-route and story-filter semantics on all-six baked doors.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include "../src/core/field/target_kind.h"

namespace {
using namespace field;using namespace field::baked_nav;using namespace field::baked_route;
int failures=0;int argus_reproduced=0;
void expect(bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}}
std::string read_file(const std::string& path){std::ifstream file(path,std::ios::binary);return {std::istreambuf_iterator<char>(file),{}};}
NativeObstacle obstacle(const baked::Object& object){
    NativeObstacle out;out.cell={object.layer,object.tx,object.ty};out.type=static_cast<int>(object.type);out.id=object.entity_id();out.raw_name=baked::object_identifier(object);return out;
}
void check_door(const baked::Submap& map,const baked::Object& original,const map_state::Flags& flags,int& checked,int& unavailable){
    map_state::Resolver state(flags,fixture::records);
    if(!state.active(map,original)){++unavailable;return;}
    const auto object=state.effective(map,original);const Cell target{object.layer,object.tx,object.ty};
    Request movement;movement.submap=&map;movement.blocked={target};
    const auto approaches=baked_route::goals_for(movement,target,GoalKind::Interact,&object);
    const auto approach=std::find_if(approaches.cells.begin(),approaches.cells.end(),[&](const Cell& c){return c!=target&&std::abs(c.x-target.x)+std::abs(c.y-target.y)==1;});
    if(approach==approaches.cells.end()){++unavailable;return;}
    RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.submap=&map;snapshot.map_id=map.first_map_id;snapshot.layer=object.layer;snapshot.locale=L"en";
    snapshot.position=world_of_cell(map,*approach);snapshot.obstacles.push_back(obstacle(object));
    snapshot.goal.map_id=map.first_map_id;snapshot.goal.cell=target;snapshot.goal.entity_id=object.entity_id();snapshot.goal.raw_name=baked::object_identifier(object);
    snapshot.kind=navigation_target_kind(Kind::Event,Category::Objects);
    Planned planned;
    std::thread routing([&]{planned=plan_snapshot(snapshot);});routing.join();
    expect(planned.path.success&&planned.path.step_count==0&&planned.path.reason.empty(),"reachable locked/manual door has a zero-step interaction approach without any key/ability inventory input");
    expect(!planned.path.walk_phases.empty()&&planned.path.walk_phases.back().arrival_direction>=0,"door approach preserves native facing instead of walking through its collider");
    // Exercise the actual filter pipeline: Objects metadata must reach the same
    // interaction-goal policy as the selected navigation caller.
    filter_worker::FilterInput input;input.current_map=map.first_map_id;input.category=Category::Objects;input.story_gates=true;
    input.navigation=std::make_shared<ExitSnapshot>(false,map_state::Resolver(flags,fixture::records));
    auto& route=input.navigation->route;route.submap=&map;route.map_id=map.first_map_id;route.layer=object.layer;route.locale=L"en";
    route.position=snapshot.position;route.obstacles=snapshot.obstacles;
    filter_worker::FilterRow row;row.index=0;row.kind=Kind::Event;row.category=Category::Objects;row.event_type=L"Interactive Object";
    row.entity_id=object.entity_id();row.raw_name=baked::object_identifier(object);row.position=world_of_cell(map,target);input.rows={row};
    filter_worker::Categories categories;std::thread filtering([&]{categories=filter_worker::build_categories(input);});filtering.join();
    expect(!input.rows[0].story_gated&&input.rows[0].available,"Hide story-gated destinations retains the reachable locked-door object");
    if(game::is(game::Id::FF3)&&map.first_map_id==67&&(object.entity_id()==29||object.entity_id()==30)){
        auto inferred=RouteSnapshot(map_state::Resolver(flags,fixture::records));inferred.submap=&map;inferred.map_id=map.first_map_id;inferred.layer=object.layer;inferred.locale=L"en";
        inferred.position=snapshot.position;inferred.obstacles=snapshot.obstacles;inferred.goal=snapshot.goal;inferred.kind=TargetKind::Infer;
        Planned old;std::thread reproduce([&]{old=plan_snapshot(inferred);});reproduce.join();
        argus_reproduced+=!old.path.success;
        std::printf("Argus door%d at(%d,%d): inferred success%d reason=%ls; explicit interaction success%d facing%d\n",object.entity_id(),target.x,target.y,old.path.success,old.path.reason.c_str(),planned.path.success,planned.path.walk_phases.empty()?-1:planned.path.walk_phases.back().arrival_direction);
        // Block every possible approach with separate physical gate colliders.
        // The object must not receive a blanket exemption from real obstructions.
        std::vector<NativeObstacle> blocked=snapshot.obstacles;
        for(int y=target.y-1;y<=target.y+1;++y)for(int x=target.x-1;x<=target.x+1;++x){
            if(x==target.x&&y==target.y)continue;
            NativeObstacle gate;gate.type=19;gate.collider=true;gate.collision=1;gate.id=10000+(y-target.y+1)*3+x-target.x+1;gate.cell={object.layer,x,y};blocked.push_back(gate);
        }
        Cell outside{object.layer,target.x,target.y+2};
        if(baked_route::open(movement,baked::grids(map),outside)){
            auto capture=std::make_unique<ExitSnapshot>(false,map_state::Resolver(flags,fixture::records));
            capture->route.submap=&map;capture->route.map_id=map.first_map_id;capture->route.layer=object.layer;capture->route.locale=L"en";capture->route.position=world_of_cell(map,outside);capture->route.obstacles=blocked;
            MapGoal goal;goal.map_id=map.first_map_id;goal.cell=target;goal.entity_id=object.entity_id();goal.raw_name=baked::object_identifier(object);
            std::vector<bool> hidden;std::thread barrier([&]{hidden=story_gate_mask(*capture,{{goal,TargetKind::Interact}},{});});barrier.join();
            expect(hidden==std::vector<bool>{true},"a separate obstruction preventing access to the door still qualifies as story gated");
        }
    }
    ++checked;
}
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    expect(navigation_target_kind(Kind::Event,Category::Objects)==TargetKind::Interact,"manual Objects explicitly select interaction goals");
    expect(navigation_target_kind(Kind::MapExit,Category::MapExits)==TargetKind::Transition&&navigation_target_kind(Kind::Event,Category::Events)==TargetKind::Infer&&navigation_target_kind(Kind::SavePoint,Category::SavePoints)==TargetKind::Infer,"ordinary exits/automatic events/save points retain their established semantics");
    std::vector<std::string> storage;storage.reserve(6);
    for(int number=1;number<=6;++number){
        fixture::game_id=static_cast<game::Id>(number);const auto root="data/ff"+std::to_string(number)+"/";
        storage.push_back(read_file(root+"maps.bin"));expect(baked::load(storage.back().data(),storage.back().size()),"real all-six bake loads");
        expect(json::parse(read_file(root+"mapstate.json"),fixture::records),"real all-six state program loads");json::Value labels;expect(json::parse(read_file(root+"eventlabels.json"),labels),"reviewed physical door labels load");
        for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));g_loaded=g_prepared=true;
        auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
        int doors=0,checked=0,unavailable=0;
        for(int i=0;i<baked::submap_count();++i){
            const auto* sm=baked::submap_at(i);if(sm->first_map_id<=0||sm->world)continue;
            for(const auto& object:baked::grids(*sm).objects){
                if(object.type!=baked::ObjType::Entity&&object.type!=baked::ObjType::AnimEntity)continue;
                const auto* label=labels.get(baked::object_identifier(object));
                if(!label||(label->s!=L"Door"&&label->s!=L"Iron gate")||!(object.d&12))continue;
                ++doors;check_door(*sm,object,*flags,checked,unavailable);
            }
        }
        std::printf("FF%d: %d actionable baked Door identities, %d active physical approaches verified, %d inactive/no-cardinal-approach cases retained for native-state confirmation\n",number,doors,checked,unavailable);
        if(number==3)expect(doors>=21&&checked>=16,"FFIII audit includes Argus/key doors plus ordinary story-locked door objects");
    }
    expect(argus_reproduced==2,"both Castle Argus locked doors reproduce the former inferred-transition failure");
    expect(!fixture::wrong_owner,"door goals/routes/filtering use owned state without native worker access");
    std::printf("Locked-door interaction: %d failures\n",failures);return failures?1:0;
}
