// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual FFV scene/shore graph and owned planner; no game execution. Source
// bodies and native movement proofs remain outside Git, not fixture snippets.
#define main existing_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include <tuple>

int main(){
    using namespace field;using namespace baked_nav;
    std::setvbuf(stdout,nullptr,_IONBF,0);int failures=0;
    const auto expect=[&](bool ok,const char* text){if(!ok){++failures;std::printf("FAIL %s\n",text);}};
    const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
    const auto* sm=baked::submap_by_map_id(375);if(!sm)return 2;
    for(int variant:{0,1})for(int point:{1,2}){
        map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);
        flags.current_map_id=375;flags.current_area_id=51;flags.scenario[122]=variant;
        RouteSnapshot snapshot{map_state::Resolver(flags,fixture::records)};snapshot.submap=sm;snapshot.map_id=375;snapshot.locale=L"en";snapshot.layer=1;
        const baked::Object* target=nullptr;const baked::Object* entry=nullptr;
        std::set<std::tuple<int,int,int,int,std::wstring>> native_seen;
        const int npc_id=variant?16:2;
        for(const auto& original:baked::grids(*sm).objects){
            if(original.type==baked::ObjType::PointIn&&original.a==point)entry=&original;
            if(!snapshot.state.active(*sm,original))continue;const auto o=snapshot.state.effective(*sm,original);
            if((o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC)&&o.entity_id()==npc_id)target=&original;
            const bool npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;
            const bool chest=o.type==baked::ObjType::TreasureBox;
            const bool event=o.type==baked::ObjType::Event||(o.type==baked::ObjType::Warp&&(o.d&255)==1);
            const bool wall=(o.type==baked::ObjType::Entity||o.type==baked::ObjType::AnimEntity||o.type==baked::ObjType::EffectEntity)&&(o.d&1);
            if(!npc&&!chest&&!event&&!wall)continue;
            // A graph companion and its original Event describe one native
            // actor, not two colliders in the captured scene.
            if(!native_seen.emplace(o.entity_id(),o.layer,o.tx,o.ty,baked::object_identifier(o)).second)continue;
            NativeObstacle actor;actor.cell={o.layer,o.tx,o.ty};actor.type=event?2:int(o.type);actor.id=o.entity_id();actor.raw_name=baked::object_identifier(o);
            actor.npc=npc;actor.chest=chest;actor.trigger=event;actor.movement_known=npc;actor.wanders=(o.d&16)!=0;
            snapshot.obstacles.push_back(actor);
        }
        if(!target||!entry)return 2;
        snapshot.position=world_of_cell(*sm,{entry->layer,entry->tx,entry->ty});
        snapshot.goal.map_id=375;snapshot.goal.entity_id=npc_id;snapshot.goal.raw_name=baked::object_identifier(*target);
        snapshot.goal.cell={target->layer,target->tx,target->ty};snapshot.kind=TargetKind::Interact;
        Planned planned;std::thread worker([&]{planned=plan_snapshot(snapshot);});worker.join();
        expect(planned.path.success,"real NPC route uses the actual scripted shore crossing");
        if(planned.path.success){
            expect(planned.pending.valid&&planned.pending.goal.entity_id==npc_id,"shore approach preserves the final NPC identity");
            expect(!planned.path.walk_phases.empty()&&!planned.path.walk_phases.back().cells.empty()&&
                planned.path.walk_phases.back().cells.back()==Cell{1,12,35},"first leg ends on the actual native shore trigger");
        }
        // A completed native crossing leaves the controlled player at12,32.
        // Replanning from that real pose must finish at the original NPC.
        snapshot.position=world_of_cell(*sm,{1,12,32});snapshot.pending={};snapshot.active={};snapshot.previous_active.reset();
        Planned landed;std::thread next([&]{landed=plan_snapshot(snapshot);});next.join();
        expect(landed.path.success&&landed.goal.entity_id==npc_id,"actual shore landing reaches the same NPC without a fabricated walking edge");
        std::printf("Ghido variant%d entry%d: shore-route%d, after-native-landing%d, reason=%ls\n",variant,point,planned.path.success,landed.path.success,planned.path.reason.c_str());
    }
    expect(fixture::wrong_owner==0,"scene entry planner uses owned snapshots with no native worker calls");
    std::printf("NPC scripted-entry check: %d failures\n",failures);return failures?1:0;
}
