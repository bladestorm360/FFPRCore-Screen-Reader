// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real FF6 movement/collision and owned defense advice, no native game execution.
#define main existing_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include "../src/core/fieldnav/autowalk_route.h"
int main() {
    using namespace field;using namespace baked_nav;
    int failures=0;const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff6/maps.bin");expect(baked::load(bytes.data(),bytes.size()),"FF6 actual bake loads");
    expect(json::parse(read("data/ff6/mapstate.json"),fixture::records),"matching actual state records load");
    fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
    for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));
    double worst=0;
    for(int map:{318,19}) {
        const auto* sm=baked::submap_by_map_id(map);const auto group=map==318?L"ev_e_0018":L"ev_e_0257";
        const int defender=map==318?32:64,commander=map==318?9:25,threat=map==318?18:38;
        const auto goal=[&](int id){MapGoal out;out.map_id=map;out.entity_id=id;for(const auto& object:baked::grids(*sm).objects)
            if(object.type==baked::ObjType::NPC&&object.entity_id()==id&&baked::str(object.group)==group){out.cell={object.layer,object.tx,object.ty};out.raw_name=baked::object_identifier(object);break;}return out;};
        auto flags=map_state::Flags{};for(auto& v:flags.scenario)v=0;
        flags.scenario[map==318?13:137]=1;
        RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.submap=sm;snapshot.map_id=map;snapshot.layer=1;snapshot.locale=L"en";
        DefenseContext context;context.protected_actor=goal(defender);context.enemies={goal(threat)};context.current_party=0;
        context.warning=map==318?L"Terra in danger.":L"Banon in danger.";
        const auto add=[&](MapGoal target){NativeObstacle actor;actor.type=6;actor.id=target.entity_id;actor.npc=true;actor.cell=target.cell;actor.raw_name=target.raw_name;snapshot.obstacles.push_back(actor);};
        add(context.protected_actor);add(goal(commander));add(goal(threat));
        expect(!context.protected_actor.raw_name.empty()&&!goal(commander).raw_name.empty()&&!goal(threat).raw_name.empty(),"all defense fixtures identify real authored actors");
        auto& moving=snapshot.obstacles.back();
        // Find an enterable cell close to the defended actor and three actual
        // distinct walking starts, then compare advice against independent routes.
        baked_route::Request movement;movement.submap=sm;
        std::vector<Cell> starts;
        const auto anchor=context.protected_actor.cell;
        // Parties intercept from the battlefield below the defended character,
        // not the isolated cells behind Banon which his own NPC blocks.
        for(int dy=1;dy<=3&&starts.size()<4;++dy)for(int dx=-3;dx<=3&&starts.size()<4;++dx) {
            Cell cell{1,anchor.x+dx,anchor.y+dy};
            if(cell==anchor||!baked_route::open(movement,baked::grids(*sm),cell))continue;
            starts.push_back(cell);
        }
        expect(starts.size()==4,"defense has physical nearby walking cells");if(starts.size()<4)continue;
        moving.cell=starts[0];for(int i=0;i<3;++i)context.parties[static_cast<size_t>(i)]=starts[static_cast<size_t>(i+1)];
        snapshot.position=world_of_cell(*sm,*context.parties[0]);
        DefenseAdvice advice;int expected=-1,best=INT_MAX;
        std::thread worker([&]{
            const auto began=std::chrono::steady_clock::now();
            {SnapshotScope scope(snapshot);advice=defense_advice_snapshot(snapshot,context);}
            for(int i=0;i<3;++i) {
                auto route=snapshot;route.position=world_of_cell(*sm,*context.parties[static_cast<size_t>(i)]);route.goal=context.enemies[0];route.kind=TargetKind::Interact;
                const auto result=plan_single_snapshot(route);
                if(result.path.success&&result.path.step_count<best){best=result.path.step_count;expected=i;}
            }
            worst=std::max(worst,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
        });worker.join();
        expect(advice.intercept&&advice.intercept->entity_id==threat&&advice.closest_party==expected&&expected>=0,
            "three-tile threat names the party with the shortest real legal interception route");
        expect(advice.warning.find(context.warning)!=std::wstring::npos,"danger warning names the actual protected character");
        snapshot.candidates={{goal(commander),TargetKind::Interact,L"Fight commander."},{goal(threat),TargetKind::Interact,L"Fight enemy."}};
        snapshot.defense=context;Planned route;
        std::thread urgent([&]{route=plan_snapshot(snapshot);});urgent.join();
        expect(route.path.success&&route.goal.entity_id==threat&&!route.path.defense_warning.empty(),"urgent live threat takes precedence over the event-completion commander");
        auto unknown=context;unknown.parties[2].reset();
        std::thread missing([&]{SnapshotScope scope(snapshot);advice=defense_advice_snapshot(snapshot,unknown);});missing.join();
        expect(advice.intercept&&advice.closest_party<0&&advice.warning==context.warning,"unknown parked party cannot become a guessed closest-party claim");
        snapshot.obstacles.pop_back();
        std::thread finish([&]{route=plan_snapshot(snapshot);});finish.join();
        if(!route.path.success||route.goal.entity_id!=commander)std::printf("Map%d finish control: success%d goal%d reason=%ls\n",map,route.path.success,route.goal.entity_id,route.path.reason.c_str());
        expect(route.path.success&&route.goal.entity_id==commander&&route.path.defense_warning.empty(),"after the threat disappears the same party is guided toward the actual event-ending commander");
        if(map==318) {
            auto blocked=snapshot;blocked.defense.reset();blocked.candidates.clear();
            blocked.position=world_of_cell(*sm,{1,14,33});blocked.goal=goal(commander);blocked.kind=TargetKind::Interact;
            NativeObstacle party;party.party=true;party.party_slot=0;party.npc=true;party.id=commander;party.raw_name=blocked.goal.raw_name;party.cell={1,14,34};
            blocked.obstacles.push_back(party);
            Planned manual,guidance;BrowsePlanned browse;
            std::thread verify([&]{manual=plan_snapshot(blocked);browse=plan_browse_snapshot(blocked);});verify.join();
            bool crosses=false;for(const auto& phase:manual.path.walk_phases)for(const auto& cell:phase.cells)crosses|=cell==party.cell;
            expect(manual.path.success&&manual.path.step_count>6&&!crosses,
                "logged Guard Leader corridor uses the real legal detour instead of passing through parked Party1");
            expect(browse.route.reason.find(L"Party 1 blocks")==std::wstring::npos,
                "browse keeps a usable detour's native event/transition explanation rather than falsely reporting a full party block");
            const auto detour_phases=manual.path.walk_phases;
            // A second parked corps can also occupy that alternate approach.
            // Find its physical choke cell from the actual route, without
            // inventing walls or changing baked geometry to force a failure.
            std::optional<Cell> cut;
            std::thread cutoff([&]{for(const auto& phase:manual.path.walk_phases)for(const auto& cell:phase.cells) {
                if(cell==Cell{1,14,33})continue;
                auto second=party;second.party_slot=1;second.cell=cell;blocked.obstacles.push_back(second);
                const auto result=plan_single_snapshot(blocked);blocked.obstacles.pop_back();
                if(result.path.blocking_party>=0){cut=cell;return;}
            }});cutoff.join();
            expect(cut.has_value(),"two parked corps can physically close both real approaches");
            if(cut){auto second=party;second.party_slot=1;second.cell=*cut;blocked.obstacles.push_back(second);}
            blocked.position=world_of_cell(*sm,{1,14,31}); // distant request, not the already-adjacent logged stop
            std::thread occupied([&]{manual=plan_snapshot(blocked);browse=plan_browse_snapshot(blocked);});occupied.join();
            expect(manual.path.success&&manual.path.blocking_party==0&&manual.path.step_count>0&&
                manual.path.description.ends_with(L"Blocked by Party 1.")&&!browse.route.direct,
                "distant occupied approaches retain usable directions with a terminal party suffix");
            const auto boundary=manual.path.walk_phases.back().cells.back();
            expect(std::abs(boundary.x-party.cell.x)+std::abs(boundary.y-party.cell.y)==1,
                "partial route stops exactly one legal walking step before the parked party");
            ffpr::fieldnav::autowalk::Route walk;walk.set(manual.path.walk_phases,sm->width,sm->height,false);
            ffpr::fieldnav::autowalk::Step terminal;
            for(const auto& phase:manual.path.walk_phases)for(const auto& cell:phase.cells)terminal=walk.next(cell);
            expect(terminal.result==ffpr::fieldnav::autowalk::Result::Action&&terminal.instruction==L"Blocked by Party 1.",
                "autowalk stops at the actual blocker without entering it or declaring destination arrival");
            blocked.candidates=snapshot.candidates;blocked.defense=context;blocked.defense->enemies.clear();
            std::thread guide([&]{guidance=plan_snapshot(blocked);});guide.join();
            expect(guidance.path.success&&guidance.path.description.ends_with(L"Blocked by Party 1.")&&guidance.path.guidance.empty(),
                "defense appends the obstruction after directions and does not claim the commander is ready to fight");
            auto distant=blocked;distant.position=world_of_cell(*sm,{1,13,15});Planned long_prefix;
            std::thread longer([&]{long_prefix=plan_snapshot(distant);});longer.join();
            expect(long_prefix.path.success&&long_prefix.path.blocking_party<0,
                "parties isolating a different corridor do not label a valid northern approach as blocked");
            std::thread capped([&]{
                SnapshotScope scope(blocked);auto phases=detour_phases;
                phases.back().action_after=L"Blocked by Party 1.";phases.back().manual_after=true;
                PathInfo formatted;formatted.blocking_party=0;fill_result(*sm,phases,formatted);
                expect(formatted.description.ends_with(L"Blocked by Party 1.")&&formatted.full_description.ends_with(L"Blocked by Party 1."),
                    "brief and full formatting retain a terminal advisory on the actual long detour cells");
            });capped.join();
            blocked.position=world_of_cell(*sm,boundary);blocked.previous_active=manual.active;
            std::thread adjacent([&]{guidance=plan_snapshot(blocked);});adjacent.join();
            expect(guidance.path.success&&guidance.path.step_count==0&&guidance.path.description==L"Blocked by Party 1.",
                "standalone party-block speech occurs only at the adjacent obstructed step");
            blocked.previous_active.reset();blocked.position=world_of_cell(*sm,{1,14,33});
            std::thread unknown_slot([&]{
                SnapshotScope scope(blocked);Blockers live;
                live.parties={{{1,14,32},-1},{{1,14,34},0}};
                baked_route::Request request;request.submap=sm;request.start={1,14,31};request.goals={{1,14,39}};
                int slot=-1;const auto prefix=party_approach(request,live,slot);
                expect(!prefix||std::none_of(prefix->cells.begin(),prefix->cells.end(),[](const auto& cell){return cell==Cell{1,14,32};}),
                    "unknown native party ownership remains solid before any named later boundary");
            });unknown_slot.join();
            if(cut)blocked.obstacles.pop_back();
            blocked.obstacles.back().cell={2,14,34};
            std::thread floor([&]{manual=plan_snapshot(blocked);});floor.join();
            expect(manual.path.success,"a party on another physical layer does not block Ground");
            blocked.obstacles.back().cell={1,14,31};
            std::thread moved([&]{manual=plan_snapshot(blocked);});moved.join();
            expect(manual.path.success,"moving the parked party clear restores commander access on the same map");
            blocked.position=world_of_cell(*sm,{1,14,34});blocked.obstacles.back().party_slot=1;blocked.obstacles.back().cell={1,14,33};
            std::thread switched([&]{manual=plan_snapshot(blocked);});switched.join();
            expect(manual.path.success,"the newly active party can fight while the previous party remains parked behind it");
        }
        std::printf("Defense map%d urgent/intercept/party/commander checks completed\n",map);
    }
    expect(fixture::wrong_owner==0,"defense computation owns no game or localization pointer");
    std::printf("Defense advice worst fixture %.3fms; failures %d\n",worst,failures);return failures?1:0;
}
