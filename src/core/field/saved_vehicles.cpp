// FFIII off-map craft, FFIV's Lunar Whale and FFVI's Blackjack.
// Read ready value fields; never clone the collection, instantiate a singleton,
// transform an unloaded object, serialize data or move a game pointer to work.
#include "field_internal.h"
#include "baked_map.h"
#include "../frame.h"
#include "../system/system.h"
#include <cmath>
namespace field {
namespace {
constexpr auto kSavedGames=game::kFF3|game::kFF4|game::kFF6;
ref::Class kSavedUser{"Last.Management","UserDataManager",kSavedGames};
ref::Class kSavedStorage{"Last.Interpreter","DataStorage",kSavedGames};
ref::Class kSavedOwned{"Last.Data.User","OwnedTransportationData",kSavedGames};
ref::Class kSavedTransport{"Last.Data.User","SaveTransportationData",kSavedGames};
ref::Class kSavedEntity{"Last.Map","SaveEntityData",kSavedGames};
ref::Field fSavedList{kSavedUser,"<OwnedTransportationList>k__BackingField"};
ref::Field fSavedData{kSavedOwned,"saveData"};
ref::Field fSavedId{kSavedTransport,"id"};
ref::Field fSavedMap{kSavedTransport,"mapId"};
ref::Field fSavedCell{kSavedEntity,"position"};
ref::Method mSavedEnabled{kSavedOwned,"get_Enable",0};
}
std::vector<SavedWorldVehicle> saved_world_vehicles() {
    std::vector<SavedWorldVehicle> result;
    const bool lunar_whale=game::is(game::Id::FF4);
    const bool blackjack=game::is(game::Id::FF6);
    if((!game::is(game::Id::FF3)&&!lunar_whale&&!blackjack)||!frame::on_main_thread()||!ffpr::system::is_field_navigation_ready())return result;
    // get_Enable reads the existing interpreter flag bank. The static pointer
    // checks prevent its native singleton getter from creating missing state.
    void* user=il2cpp::read_static_ptr(kSavedUser.get(),"instance");
    if(!user||!il2cpp::read_static_ptr(kSavedStorage.get(),"instance")||
       fSavedList.get()<0||fSavedData.get()<0||fSavedId.get()<0||fSavedMap.get()<0||fSavedCell.get()<0||!mSavedEnabled)return result;
    void* list=ref::rd<void*>(user,fSavedList);
    const int count=unity::list_count(list);
    if(count<0||count>32)return result; // actual six masters contain at most19 rows
    for(int index=0;index<count;++index) {
        void* owned=unity::list_at(list,index);if(!owned||!kSavedOwned.is(owned))continue;
        void* saved=ref::rd<void*>(owned,fSavedData);if(!saved||!kSavedTransport.is(saved))continue;
        const int id=ref::rd<int32_t>(saved,fSavedId),map=ref::rd<int32_t>(saved,fSavedMap);
        // This extension is deliberately one reviewed craft/world, not a
        // general promise about every FFVI saved vehicle or cutscene.
        if(blackjack&&(id!=4||map!=1))continue;
        if(lunar_whale&&!((id==5&&map==1)||(id==11&&map==3)))continue;
        const auto* row=baked::transport(id);
        if(!row||row->type<=1||row->type==4||row->type==5||id<=0||id>=32||map<1||map>4||!ref::call<bool>(mSavedEnabled,owned))continue;
        const auto cell=ref::rd<unity::Vec3>(saved,fSavedCell);
        if(!std::isfinite(cell.x)||!std::isfinite(cell.y)||cell.x<0||cell.y<0||cell.x>=256||cell.y>=256)continue;
        result.push_back({id,map,cell});
    }
    return result;
}
}
