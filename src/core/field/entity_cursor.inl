// Native-owner collection cursor, included by navigation.cpp and its offline
// producer check. No engine pointer may leave this owning callback.
bool EntityCursor::next(void*& entity)
{
    entity = nullptr;
    void* fc=nullptr;void* info_data=nullptr;
    if(!cache_frame_roots||roots_frame!=frame::count()) {
        void* map=field_map();fc=map?ref::rd<void*>(map,r::fFieldController):nullptr;
        void* transportation=fc?ref::rd<void*>(fc,r::fTransportation):nullptr;
        info_data=transportation?ref::rd<void*>(transportation,r::fInfoData):nullptr;
        if(cache_frame_roots){roots_frame=frame::count();frame_controller=fc;frame_info=info_data;collection_stage=-1;}
    } else {fc=frame_controller;info_data=frame_info;}
    if(!fc)return false;
    const auto collection=[&](void* owner,ref::Field& field,bool dictionary) {
        if(!cache_frame_roots||collection_stage!=stage) {
            frame_collection=owner?ref::rd<void*>(owner,field):nullptr;
            collection_count=dictionary?unity::dict_count(frame_collection):unity::list_count(frame_collection);
            collection_stage=stage;
        }
        return frame_collection;
    };
    if (stage < 2) {
        void* list=collection(fc,stage==0?r::fEntityList:r::fColliderEntityList,false);
        if (index < collection_count) entity = unity::list_at(list, index++);
        else { ++stage; index = 0; }
    } else if (stage == 2) {
        void* dict=collection(info_data,r::fModelList,true);
        if (index < collection_count) {
            uint64_t key = 0, value = 0;
            if (unity::dict_entry(dict, index++, key, value, 4, 8)) {
                void* info = reinterpret_cast<void*>(value);
                if (info) {
                    void* object = ref::call<void*>(r::mInfoGetMapObject, info);
                    // NeedInteractiveList is only this model dictionary filtered by
                    // CheckNeedInteractive(id,true). Avoid allocating/walking its list.
                    if (object && ref::call<bool>(r::mCheckNeedInteractive, info_data, static_cast<int32_t>(key), true)
                        && r::kFieldEntity.is(object)) entity = object;
                    const int type = ref::call<int32_t>(r::mInfoGetType, info);
                    if (object && !skip_transport_type(type) && ref::call<bool>(r::mInfoGetEnable, info)) {
                        entity = object;
                        g_vehicle_types[object] = VehicleInfo{type, ref::call<int32_t>(r::mInfoGetId, info),
                            ref::call_str(r::mInfoGetMessageId, info), current_map_id()};
                    } else if (object) g_vehicle_types.erase(object);
                    if (type == 5 && ref::call<bool>(r::mInfoGetEnable, info)) {
                        if (const auto* row = baked::transport(ref::call<int32_t>(r::mInfoGetId, info)))
                            if (row->group > 0 && std::find(auto_groups.begin(), auto_groups.end(), row->group) == auto_groups.end())
                                auto_groups.push_back(row->group);
                    }
                }
            }
        } else { g_auto_map = current_map_id(); g_auto_groups = auto_groups; ++stage; index = 0; }
    } else if (stage == 3) {
        void* foot=cache_frame_roots&&collection_stage==stage?nullptr:ref::rd<void*>(fc,r::fFootEvent);
        void* dict=collection(foot,r::fStepOnTriggerList,true);
        if (index < collection_count) {
            uint64_t key = 0, value = 0;
            if (unity::dict_entry(dict, index++, key, value, 4, 8)) {
                void* item = reinterpret_cast<void*>(value);
                if (item && r::kFieldEntity.is(item)) entity = item;
            }
        } else { ++stage; index = 0; }
    } else if (stage == 4) {
        // Parked corps are not in entityList. Yield one bounded native slot;
        // the existing owner-side active/lifetime checks decide its presence.
        if(!party_entities::next_native(fc,index,entity)){++stage;index=0;}
    } else {
        for (auto it = g_vehicle_types.begin(); it != g_vehicle_types.end();)
            if (!seen.count(it->first)) it = g_vehicle_types.erase(it); else ++it;
        return false;
    }
    // Retain the established cross-list inclusion guard, not speech deduplication.
    if (entity && !seen.insert(entity).second) entity = nullptr;
    return true;
}
