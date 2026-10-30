#pragma once
#include "baked_router.h"
#include "route_progress.h"
#include <optional>

// Owned geometry only. After every complete detour fails, approach the proved
// first obstruction: wait for an authored patrol, or hand back at a fixed gate.
// Walking retains every solid obstacle; unrelated nearby actors are never used.
namespace field::npc_route {
struct Actor {
    baked_route::Cell cell;
    baked::Object interaction;
    std::wstring raw_name;
    bool wanders = false; // authored travelling controller; Stamp/facing alone is stationary
};
struct Approach {
    route_progress::Phase phase;
    size_t actor = 0;
};
inline std::optional<Approach> approach(baked_route::Request request,
    const std::vector<Actor>& actors, const std::vector<baked_route::Cell>& proof) {
    using namespace baked_route;
    // The caller has already failed every legal full route. Its proof removes
    // current-floor NPCs, preserving parties, gates, terrain and trap rules.
    // Cross-floor proofs may include later interaction stops; this helper sees
    // only the current map's cells and actual captured actors.
    for(size_t step=1;step<proof.size();++step) {
        if(request.cancelled && request.cancelled())return {};
        for(size_t index=0;index<actors.size();++index) {
            const auto& actor=actors[index];const auto& cell=proof[step];
            if(actor.cell.x!=cell.x||actor.cell.y!=cell.y||
               (actor.cell.layer>=0&&actor.cell.layer!=cell.layer))continue;
            const Cell target{actor.cell.layer<0?cell.layer:actor.cell.layer,actor.cell.x,actor.cell.y};
            auto interaction=actor.interaction;interaction.layer=static_cast<uint8_t>(target.layer);
            const auto goals=goals_for(request,target,GoalKind::Interact,&interaction);
            request.goals=goals.cells;request.goals_open=false;request.allow_trap_goal=false;
            auto route=find(request);
            if(!route.found)return {}; // no legal side of this actor is reachable
            Approach out;out.actor=index;out.phase.cells=std::move(route.cells);
            // A doorway does not turn an autonomous patrol into a scripted
            // gatekeeper. Movement metadata owns this distinction, even in a
            // one-tile corridor. The solid cell remains blocked in this route.
            out.phase.wait_for_npc=actor.wanders;
            if(!out.phase.wait_for_npc) {
                out.phase.arrival_direction=facing_direction(out.phase.cells.back(),goals);
                out.phase.manual_after=true;
            }
            return out;
        }
    }
    return {};
}
}
