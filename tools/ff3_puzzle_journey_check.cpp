// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Complete owned FFIII puzzle frontiers through the actual route worker.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff3/puzzle_model.cpp"
namespace {
int failures=0,episodes=0,legs=0,forms=0;
json::Value guide_data;
void episode(const char* name,ff3::puzzles::Family family,int map,std::initializer_list<int> completed,int initial_form,int voice_pose=0) {
    using namespace field;using namespace baked_nav;using namespace ff3::puzzles;
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);
    for(const int bit:completed)flags.scenario[bit]=1;
    const bool frog_family=family==Family::Owen||family==Family::Lake;
    int mini=!frog_family?initial_form:0,frog=frog_family?initial_form:0;
    const auto* start=baked::submap_by_map_id(map);map_state::Resolver initial(flags,fixture::records);
    const auto* entry=point_in(*start,1,initial);if(!entry){++failures;return;}
    Cell at{entry->layer,entry->tx,entry->ty};bool finished=false;
    const auto wanted=family==Family::Nepto?L"nepto_forms":family==Family::Owen?L"owen_forms":
        family==Family::Tozus?L"tozus_forms":family==Family::Hein?L"hein_forms":family==Family::Lake?L"lake_forms":L"magic_circle_forms";
    const auto& definitions=guide_data.get(L"form_guides")->arr;
    const auto definition=std::find_if(definitions.begin(),definitions.end(),[&](const auto& d){return d.get(L"id")->s==wanted;});
    if(definition==definitions.end()){++failures;return;}
    for(int operation=0;operation<32;++operation) {
        Snapshot observed;observed.map=map;std::copy_n(flags.scenario.begin(),256,observed.flags.begin());observed.mini=mini;observed.frog=frog;
        const auto decision=select(family,observed);
        if(decision.action==Action::Finished){finished=true;break;}
        if(decision.action==Action::CastMini){mini=1;++forms;continue;}
        if(decision.action==Action::CastFrog){frog=1;++forms;continue;}
        if(decision.action==Action::RemoveFrog){frog=0;++forms;continue;}
        if(decision.action==Action::RemoveMini){mini=0;++forms;continue;}
        const auto& actions=definition->get(L"actions")->arr;
        const auto step=std::find_if(actions.begin(),actions.end(),[&](const auto& s){return s.get(L"id")->s==action_id(decision.action);});
        if(step==actions.end()){std::printf("FAIL %s missing action%d\n",name,int(decision.action));break;}
        MapGoal goal;goal.map_id=int(step->get(L"map")->n);goal.entity_id=int(step->get(L"entity")->n);goal.raw_name=step->get(L"raw")->s;
        goal.cell={int(step->get(L"layer")->n),int(step->get(L"x")->n),int(step->get(L"y")->n)};goal.examine=step->get(L"examine")->b;
        Pending pending;bool reached=false;
        for(int hop=0;hop<64;++hop) {
            flags.current_map_id=map;flags.party_mini=mini==1?1:0;
            RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=map;snapshot.submap=baked::submap_by_map_id(map);
            snapshot.layer=at.layer;snapshot.position=world_of_cell(*snapshot.submap,at);snapshot.vehicle={true,1,1,{}};
            snapshot.locale=L"en";snapshot.goal=goal;snapshot.kind=goal.examine?TargetKind::Interact:TargetKind::Spot;snapshot.pending=pending;
            for(const auto& original:baked::grids(*snapshot.submap).objects) {
                if(original.type==baked::ObjType::PointIn||original.type==baked::ObjType::GotoMap||original.type==baked::ObjType::Warp)continue;
                if(!snapshot.state.active(*snapshot.submap,original))continue;
                auto actor=snapshot.state.effective(*snapshot.submap,original);NativeObstacle seen;
                seen.id=actor.entity_id();seen.type=int(actor.type);seen.raw_name=baked::object_identifier(original);seen.cell={actor.layer,actor.tx,actor.ty};
                seen.enabled=seen.applies=true;seen.trigger=baked_route::step_event(actor);seen.chest=actor.type==baked::ObjType::TreasureBox;
                seen.npc=actor.type==baked::ObjType::NPC||actor.type==baked::ObjType::ShopNPC;seen.collider=actor.type==baked::ObjType::CollisionEntity;seen.collision=actor.a;
                seen.transportation_ids=snapshot.state.transportation_mask(*snapshot.submap,original);seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);
                snapshot.obstacles.push_back(std::move(seen));
            }
            Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();++legs;
            if(!route.path.success||route.path.walk_phases.empty()||route.path.npc_approach) {
                std::printf("FAIL %s action%d from%d L%d %d,%d to%d/%d NPC%d: %ls / %ls\n",name,int(decision.action),map,at.layer,at.x,at.y,goal.map_id,goal.entity_id,route.path.npc_approach,route.path.reason.c_str(),route.path.error.c_str());break;
            }
            if(map==goal.map_id){at=route.path.walk_phases.back().cells.back();reached=true;break;}
            const auto arrival=route.pending.incoming_transition?route.pending.incoming_transition:route.pending.incoming_trap;
            if(!arrival){std::printf("FAIL %s missing actual arrival map%d next%d\n",name,map,route.path.next_map_id);break;}
            pending=route.pending;
            {SnapshotScope scope(snapshot);snapshot.pending=pending;note_trap_start(arrival->map_id,arrival->entry);record_map_arrival(arrival->destination,arrival->arrival);pending=snapshot.pending;}
            map=arrival->destination;at=arrival->arrival;flags.local.fill(0);
        }
        if(!reached)break;
        if(decision.action==Action::LakeApproach)continue; // approach alone never records a waterway operation
        const int bit=int(step->get(L"done_scenario")->n);flags.scenario[bit]=1;
        if(decision.action==Action::MiniOpening||decision.action==Action::FrogWaterway||decision.action==Action::HeinHole||decision.action==Action::LakeWaterway||decision.action==Action::MagicHole) {
            map=decision.action==Action::MiniOpening?170:decision.action==Action::FrogWaterway?175:
                decision.action==Action::HeinHole?195:decision.action==Action::LakeWaterway?185:238;flags.current_map_id=map;
            const auto* room=baked::submap_by_map_id(map);map_state::Resolver state(flags,fixture::records);const auto* arrival=point_in(*room,1,state);
            if(!arrival)break;at={arrival->layer,arrival->tx,arrival->ty};flags.local.fill(0);
        }
        if(decision.action==Action::NeptoGuardian){flags.scenario[32]=1;at={0,13,7};}
        if(decision.action==Action::VoiceFifth)at=voice_pose==0?Cell{1,18,7}:voice_pose==1?Cell{1,17,7}:Cell{1,16,6};
        if(decision.action==Action::OwenEngine)map=1; // native44 then automatic1/22 return
        if(decision.action==Action::LakeGuardian){flags.scenario[47]=1;flags.treasure[419]=1;}
        if(decision.action==Action::DwarfTalk)at={1,19,11}; //0050 root route19,13, one step north
        if(decision.action==Action::MagicUpgrade)map=4; // native78 then Surface4/15
    }
    std::printf("FFIII puzzle %s: complete%d final%d\n",name,finished,map);
    if(finished)++episodes;else ++failures;
}
}
int main() {
    using namespace field;using namespace baked_nav;using ff3::puzzles::Family;
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff3/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records)||!json::parse(read("data/ff3/puzzles.json"),guide_data))return 2;
    fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
    episode("Nepto fresh",Family::Nepto,169,{},0);
    episode("Nepto mixed",Family::Nepto,169,{},2);
    episode("Nepto entered",Family::Nepto,170,{31},1);
    episode("Nepto guardian floor",Family::Nepto,173,{31},1);
    episode("Nepto defeated before pickup",Family::Nepto,173,{31,32},1);
    episode("Nepto Eye recovered",Family::Nepto,173,{31,32,33},0);
    episode("Nepto returning normal",Family::Nepto,169,{31,32,33},0);
    episode("Nepto finished",Family::Nepto,169,{31,32,33,34},0);
    episode("Owen fresh",Family::Owen,174,{},0);
    episode("Owen mixed",Family::Owen,174,{},2);
    episode("Owen entered",Family::Owen,175,{39},1);
    episode("Owen first clue",Family::Owen,175,{39,40},0,1);
    episode("Owen fourth clue",Family::Owen,178,{39,40,41},0,2);
    episode("Owen fifth clue",Family::Owen,179,{39,40,41,42},0);
    episode("Owen wall open",Family::Owen,179,{39,225},0);
    episode("Owen guardian down",Family::Owen,184,{39,42,225,43},0);
    episode("Owen basement revisit",Family::Owen,174,{39,40,41,42,225},0);
    episode("Owen finished",Family::Owen,184,{44},0);
    episode("Tozus fresh",Family::Tozus,45,{},1);
    episode("Tozus mixed",Family::Tozus,45,{},2);
    episode("Tozus patient room",Family::Tozus,46,{},1);
    episode("Tozus finished",Family::Tozus,45,{29},0);
    episode("Hein prison",Family::Hein,194,{},0);
    episode("Hein soldier helped",Family::Hein,194,{55},0);
    episode("Hein already small",Family::Hein,194,{},1);
    episode("Hein escaped mixed",Family::Hein,195,{55,56},2);
    episode("Hein escaped normal",Family::Hein,195,{55,56},0);
    episode("Hein prison return",Family::Hein,194,{55,56},0);
    episode("Lake from Hollow",Family::Lake,84,{},0);
    episode("Lake waterway",Family::Lake,90,{},0);
    episode("Lake mixed",Family::Lake,185,{46},2);
    episode("Lake guardian floor",Family::Lake,188,{46},0);
    episode("Lake horn recovered",Family::Lake,188,{46,47,48},0);
    episode("Lake dwarf clue",Family::Lake,85,{46,47,48,49},0);
    episode("Lake altar finished",Family::Lake,85,{50},0);
    episode("Magic Manor fresh",Family::MagicCircle,131,{76},0);
    episode("Magic Doga joined",Family::MagicCircle,133,{76,77},0);
    episode("Magic wall open",Family::MagicCircle,133,{76,77,198},2);
    episode("Magic cave entered",Family::MagicCircle,238,{76,77,195},1);
    episode("Magic cave recovery",Family::MagicCircle,239,{76,77,195},0);
    episode("Magic upgrade floor",Family::MagicCircle,241,{76,77,195},1);
    episode("Magic finished",Family::MagicCircle,131,{78},0);
    std::printf("FFIII puzzle journeys: %d complete episodes/%d production legs, %d form operations, failures%d owner%d\n",episodes,legs,forms,failures,fixture::wrong_owner.load());
    return failures||fixture::wrong_owner?1:0;
}
