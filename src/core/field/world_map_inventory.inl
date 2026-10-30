// Literal inventory of connected world bakes only. Entrances stay Map Exits;
// entering a dungeon does not cause its floors/contents to be enumerated here.
// Native continent border copies remain owned by the existing nearest-trigger
// provider, rather than adding all 1535 of them to remote discovery.
std::vector<RemoteObject> world_objects_snapshot(int map_id,map_state::Resolver& state,std::function<bool()> cancelled) {
    std::vector<RemoteObject> result;
    if(!prepared())return result;
    const auto* here=baked::submap_by_map_id(map_id);if(!here)return result;
    std::vector<const baked::Submap*> worlds;
    const auto add=[&](const baked::Submap* map) {
        if(map&&map->world&&worlds.size()<32&&std::find(worlds.begin(),worlds.end(),map)==worlds.end())worlds.push_back(map);
    };
    add(here);
    if(!here->world) {
        auto floors=connected_floors(here,map_id,state,cancelled);
        floors.push_back({here,map_id});
        for(const auto& floor:floors)for(const auto& edge:edges_of(baked::grids(*floor.submap),floor.map_id)) {
            if(cancelled&&cancelled())return {};
            if(state.active(*floor.submap,*edge.object))add(baked::submap_by_map_id(edge.dest_map_id));
        }
    }
    for(size_t index=0;index<worlds.size();++index) {
        if(cancelled&&cancelled())return {};
        const auto* world=worlds[index];const auto& grid=baked::grids(*world);
        for(const auto& edge:edges_of(grid,world->first_map_id))
            if(state.active(*world,*edge.object))add(baked::submap_by_map_id(edge.dest_map_id));
        if(game::is(game::Id::FF3)) {
            if(world->first_map_id==3)add(baked::submap_by_map_id(4));
            if(world->first_map_id==4)add(baked::submap_by_map_id(3));
            for(const auto& object:grid.objects)if(state.active(*world,object))
                if(const auto* rule=air_transitions::match(3,world->first_map_id,object)) {
                    int destination=rule->destination;
                    if(!destination) {
                        const auto restored=state.scenario_value(66);if(!restored)continue;
                        destination=*restored?4:2;
                    }
                    add(baked::submap_by_map_id(destination));
                }
        }
    }
    for(const auto* world:worlds) {
        if(world==here)continue; // the live current-floor registry owns these
        const auto& grid=baked::grids(*world);
        for(const auto& original:grid.objects) {
            if(cancelled&&cancelled())return {};
            if(!state.active(*world,original)||state.consumed(*world,original))continue;
            const auto object=state.effective(*world,original);
            if(!baked::in_bounds(grid,object.tx,object.ty)||
               air_transitions::match(static_cast<int>(game::current()),world->first_map_id,object))continue;
            RemoteObject row;row.map_id=world->first_map_id;row.cell={1,object.tx,object.ty};
            row.type=object.type;row.entity_id=object.entity_id();row.raw_name=baked::object_identifier(object);row.group=baked::str(object.group);
            switch(object.type) {
            case baked::ObjType::GotoMap:if(object.a<=0)continue;row.dest_map_id=object.a;break;
            case baked::ObjType::Event:case baked::ObjType::RandomEvent:case baked::ObjType::SwitchEvent:
                if(!object.a&&!object.b)continue;
                if(const auto* exit=baked::world_exit_at(grid,row.entity_id,row.raw_name,object.tx,object.ty)) {
                    row.world_exit=true;row.dest_map_id=exit->a;
                } else row.event_type=L"Event";
                break;
            case baked::ObjType::NPC:case baked::ObjType::ShopNPC:
                if(!(object.d&(4|8)))continue;row.scripted=(object.d&4)!=0;row.shop=object.type==baked::ObjType::ShopNPC;break;
            case baked::ObjType::Entity:case baked::ObjType::AnimEntity:
                if(!(object.d&(4|8)))continue;row.event_type=L"Interactive Object";break;
            case baked::ObjType::TreasureBox:
                row.opened=state.treasure_opened(*world,object);row.pickup=baked::pickup_treasure(object);
                if(row.pickup&&row.opened.value_or(false))continue;row.content_id=object.chest_content_id();break;
            case baked::ObjType::SavePoint:break;
            case baked::ObjType::TransportationEventAction:
                if(!object.a&&!object.b)continue;row.event_type=L"Event";break;
            case baked::ObjType::Warp:
                if(std::any_of(grid.objects.begin(),grid.objects.end(),[&](const auto& actor) {
                    return actor.type!=baked::ObjType::Warp&&actor.entity_id()==object.entity_id()&&actor.group==object.group&&actor.tx==object.tx&&actor.ty==object.ty;
                }))continue;
                row.event_type=(object.d&255)==1?L"Event":L"Interactive Object";break;
            default:continue;
            }
            result.push_back(std::move(row));
        }
    }
    return result;
}
