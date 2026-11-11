// Original bounded control-state search for the dream train's movable walls.
// No game script is interpreted or executed here: the established map-state
// projector supplies each of eight reviewed native initializer layouts.
#include "dream_train_maze.h"
#include <algorithm>
#include <array>
#include <climits>
#include <tuple>
namespace ffpr::ff6::story {
MazeAdvice dream_train_maze(const Snapshot& snapshot,const json::Value& records,
                           const std::function<bool()>& cancelled) {
    using namespace field;using Cell=baked_route::Cell;
    MazeAdvice out;
    if(snapshot.map!=525)return out;
    const auto* room=baked::submap_by_map_id(525);
    if(!room||room->world||room->width>128||room->height>128){out.unknown=true;return out;}
    const auto& grid=baked::grids(*room);
    constexpr std::array<int,3> controls{41,32,26}; // right, middle, left
    std::array<const baked::Object*,3> actors{};
    const baked::Object* finish=nullptr;int initial=0;
    const auto complete=snapshot.flags.scenario[448];
    if(!complete||(*complete!=0&&*complete!=1)){out.unknown=true;return out;}
    for(int i=0;i<3;++i) {
        const auto value=snapshot.flags.scenario[445+i];
        if(!value||(*value!=0&&*value!=1)){out.unknown=true;return out;}
        initial|=*value<<i;
        for(const auto& o:grid.objects)if(o.type==baked::ObjType::Entity&&o.entity_id()==controls[i]) {
            if(actors[i]){out.unknown=true;return out;}actors[i]=&o;
        }
    }
    for(const auto& o:grid.objects)
        if((*complete==0&&o.type==baked::ObjType::Entity&&o.entity_id()==10)||
           (*complete==1&&o.type==baked::ObjType::GotoMap&&o.a==522&&o.b==7)) {
            // The two adjacent exit tiles are one native doorway.
            if(!finish||std::tie(o.ty,o.tx)<std::tie(finish->ty,finish->tx))finish=&o;
        }
    if(!finish||std::any_of(actors.begin(),actors.end(),[](auto* o){return !o;})){out.unknown=true;return out;}
    std::array<baked_route::Request,8> layouts;
    for(int bits=0;bits<8;++bits) {
        if(cancelled&&cancelled()){out.cancelled=true;return out;}
        auto flags=snapshot.flags;flags.current_map_id=525;
        for(int i=0;i<3;++i)flags.scenario[445+i]=(bits>>i)&1;
        map_state::Resolver state(std::move(flags),records);
        auto& request=layouts[bits];request.submap=room;request.transportation_id=1;request.cancelled=cancelled;
        map_state::add_obstacles(state,*room,request,true,false,false);
    }
    struct Node {int bits=0,actions=0,steps=0,first=-1;Cell at,first_at;bool done=false;};
    std::vector<Node> nodes{{initial,0,0,-1,snapshot.player,{},false}};
    int best_actions=INT_MAX,best_steps=INT_MAX,best_first=-1;
    Cell best_at;
    // A state is one of eight layouts at the original position or a legal
    // cardinal approach to one of three controls: at most104 unique nodes.
    constexpr size_t max_nodes=104;
    for(size_t iteration=0;iteration<max_nodes;++iteration) {
        if(cancelled&&cancelled()){out.cancelled=true;return out;}
        int best=-1;
        for(size_t i=0;i<nodes.size();++i)if(!nodes[i].done&&
            (best<0||std::tie(nodes[i].actions,nodes[i].steps)<std::tie(nodes[best].actions,nodes[best].steps)))best=int(i);
        if(best<0)break;
        nodes[best].done=true;const auto current=nodes[best];
        if(current.actions>best_actions)break;
        auto request=layouts[current.bits];request.start=current.at;
        const auto end=baked_route::goals_for(request,{finish->layer,finish->tx,finish->ty},
            *complete?baked_route::GoalKind::Transition:baked_route::GoalKind::Interact,finish);
        // A native doorway may be closed in the ordinary floor mask. Its
        // selected transition footprint is enterable only as the actual goal,
        // just as in the production route consumer; no other door is opened.
        request.goals=end.cells;request.allow_trap_goal=*complete==1;
        const auto reach=baked_route::flood(request);
        for(const auto& cell:end.cells) {
            const int distance=reach.distance(cell);if(distance<0)continue;
            const int steps=current.steps+distance;
            if(std::tie(current.actions,steps)<std::tie(best_actions,best_steps)) {
                best_actions=current.actions;best_steps=steps;best_first=current.first;
                best_at=current.first<0?cell:current.first_at;
            }
        }
        if(current.actions>=best_actions)continue;
        for(int i=0;i<3;++i) {
            const auto* actor=actors[i];
            const auto goals=baked_route::goals_for(request,{actor->layer,actor->tx,actor->ty},baked_route::GoalKind::Interact,actor);
            for(const auto& cell:goals.cells) {
                const int distance=reach.distance(cell);if(distance<0)continue;
                const int bits=current.bits^(1<<i),steps=current.steps+distance;
                // Native lever operation does not move the player. Reject a
                // hypothetical layout that would strand its exact approach.
                const auto& after=layouts[bits];
                if(!baked_route::open(after,grid,cell)||
                    std::any_of(after.blocked.begin(),after.blocked.end(),[&](const auto& blocked){return blocked.x==cell.x&&blocked.y==cell.y&&(blocked.layer<0||blocked.layer==cell.layer);})||
                    baked_route::transport_rule_at(after,grid,cell,baked_route::TransportRules::Gate)||
                    baked_route::transport_rule_at(after,grid,cell,baked_route::TransportRules::Solid))continue;
                auto old=std::find_if(nodes.begin(),nodes.end(),[&](const auto& node){return node.bits==bits&&node.at==cell;});
                const int first=current.first<0?i:current.first;
                const auto first_at=current.first<0?cell:current.first_at;
                if(old==nodes.end()) {
                    if(nodes.size()>=max_nodes){out.unknown=true;return out;}
                    nodes.push_back({bits,current.actions+1,steps,first,cell,first_at,false});
                } else if(!old->done&&std::make_tuple(current.actions+1,steps)<std::tie(old->actions,old->steps)) {
                    old->actions=current.actions+1;old->steps=steps;old->first=first;old->first_at=first_at;
                }
            }
        }
    }
    if(best_actions!=INT_MAX) {
        out.entity=best_first<0?finish->entity_id():controls[best_first];
        out.actions=best_actions;out.exit=best_first<0&&*complete==1;out.approach=best_at;
    }
    return out;
}
}
