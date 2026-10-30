// Ports FF1 Patches/VehicleLandingPatches.cs: both of its classes,
//  * VehicleLandingPatches - MapUIManager.SwitchLandable postfix ("Can land");
//  * MovementSpeechPatches - FieldPlayer.GetOn / GetOff postfixes ("On ship", "On foot").
//
// FF1 has no landing-ping sound: its "landing" feedback is the spoken "Can land" below
// (sound::play_landing_ping is FF5's later addition and is not called here).
//
// FieldPlayer.GetOn differs per game: GetOn(int typeId, bool isBackground) in FF1, FF2, FF4;
// GetOn(int typeId, float uniqueSpeedScale, bool isBackground) in FF3, FF5, FF6. Both detours
// only read typeId. No hook here is folded (one RVA hit each in every dump.cs).
//
// FF6 (no landing or boarding speech in its own mod) gets FF1's: its TransportationType 5 is an
// internal marker there (Content, skipped by FF6's vehicle list), not a canoe, so it has no name.
#include <cstdint>
#include <cmath>
#include <string>
#include "move_state.h"
#include "vehicle_landing.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../frame.h"
#include "../hooks.h"
#include "../log.h"
#include "../modtext.h"
#include "../refs.h"
#include "../say.h"
#include "../field/field_api.h"
#include "../field/object_cache.h"
#include "../system/system.h"

namespace ffpr::fieldnav::vehicle_landing {
namespace {

// --- VehicleLandingPatches ------------------------------------------------------------------

ref::Class  kMapUIManager{"Last.Map", "MapUIManager"};
ref::Method mSwitchLandable{kMapUIManager, "SwitchLandable", 1};
ref::Method mShowLandingGuide{kMapUIManager, "ShowLandingGuide", 1};
ref::Method mHideLandingGuide{kMapUIManager, "HideLandingGuide", 0};
ref::Field fLandingGuide{kMapUIManager, "landingGuide"};

bool g_last_landable_state = false;
// The native boolean stays true across land<->dive terrain. Remember the
// available action, rather than treating those distinct native choices as one.
enum class LandingAction { Unknown, Land, Dive, Surface };
LandingAction g_landing_action=LandingAction::Unknown;
ref::Class kLandingFieldMap{"","FieldMap",game::kFF3};
ref::Class kLandingFieldController{"Last.Map","FieldController",game::kFF3};
ref::Field fLandingFieldController{kLandingFieldMap,"fieldController"};
ref::Method mLandingCell{kLandingFieldController,"ConvertWorldPositionToCellPosition",1};
ref::Method mLandingAttribute{kLandingFieldController,"GetCellAttribute",1};
void* g_landing_guide = nullptr;
int g_landing_vehicle_id = 0, g_landing_vehicle_type = 0;
double g_last_announcement_time = -1.0e9;

// These native TransportationType values represent mounted manual vehicles in
// the six matching dumps. Content(5) covers automatic walk-on transport; Symbol
// is a map marker. Neither a boardable preview nor a canoe is a landing context.
bool manual_landing_vehicle(const ::field::VehicleState& v)
{
    if(!v.valid || v.id<=0)return false;
    switch(v.type) {
    case 2: case 3: case 6: case 7: case 8: // ship, planes, submarine
    case 9: case 10: case 11: case 12:     // chocobos/Boko, mounted armor
        return true;
    default:return false;
    }
}

LandingAction available_action(const ::field::VehicleState& vehicle) {
    // Native Submarine(6) is FFIII's underwater Nautilus14 or FFV's two
    // submarine models8/13. A ready guide in this mode offers surfacing.
    // Its own native availability/edge remains authoritative; no extra query.
    if(vehicle.type==6)return LandingAction::Surface;
    // FFIII transportation9 alone has confirm_landing_foot_id16 and the
    // reviewed end-landing script358: surface world4 -> underwater world3.
    // Native guide availability still owns collision/overlap/landing validity.
    if(!game::is(game::Id::FF3)||vehicle.id!=9||vehicle.type!=3||::field::current_map_id()!=4)
        return LandingAction::Land;
    void* map=::field::cache::get(kLandingFieldMap);
    void* controller=map?ref::rd<void*>(map,fLandingFieldController):nullptr;
    void* actor=::field::player();void* tr=actor?unity::transform(actor):nullptr;
    if(!controller||!tr)return LandingAction::Unknown;
    const auto pos=unity::position(tr);
    if(!std::isfinite(pos.x)||!std::isfinite(pos.y)||!std::isfinite(pos.z))return LandingAction::Unknown;
    auto* convert=mLandingCell.ptr();auto* attribute=mLandingAttribute.ptr();
    if(!convert||!attribute)return LandingAction::Unknown;
    try {
        using Convert=unity::Vec3(*)(void*,unity::Vec3,const void*);
        using Attribute=int32_t(*)(void*,unity::Vec2,const void*);
        auto cell=reinterpret_cast<Convert>(convert)(controller,pos,mLandingCell.get());
        if(!std::isfinite(cell.x)||!std::isfinite(cell.y))return LandingAction::Unknown;
        // Native GetCellAttribute wraps its negative coordinates with loops.
        // Surface world4 is exactly256x256: normalize first to bound that work.
        cell.x=std::fmod(cell.x,256.f);if(cell.x<0)cell.x+=256;
        cell.y=std::fmod(cell.y,256.f);if(cell.y<0)cell.y+=256;
        const int foot=reinterpret_cast<Attribute>(attribute)(controller,{cell.x,cell.y},mLandingAttribute.get());
        if(foot<1||foot>32)return LandingAction::Unknown;
        return foot==16?LandingAction::Dive:LandingAction::Land;
    } catch(...) {return LandingAction::Unknown;}
}

void observe_landing_guide(void* self, bool landable)
{
    if(!kMapUIManager.is(self))return;
    // The native current transportation is authoritative. Movement speech can
    // still say on foot during boarding; nearby parked models aren't mounted.
    const auto vehicle=::field::current_vehicle();
    if(!manual_landing_vehicle(vehicle)) {reset_landing_state();return;}
    if(vehicle.id!=g_landing_vehicle_id || vehicle.type!=g_landing_vehicle_type) {
        reset_landing_state();
        g_landing_vehicle_id=vehicle.id;g_landing_vehicle_type=vehicle.type;
    }
    void* guide=ref::rd<void*>(self,fLandingGuide);
    if(!displayed::container_visible(guide)) {
        if(guide==g_landing_guide) {g_landing_guide=nullptr;g_last_landable_state=false;}
        return;
    }
    if(guide!=g_landing_guide) {g_landing_guide=guide;g_last_landable_state=false;}
    // Native UI initialization can activate this guide while loading even on
    // foot. Observe its state, but announce an edge only when field/vehicle
    // control is ready and no transition/fade is active. Always seed the last
    // state below, so readiness returning cannot replay a loading-time edge.
    // Cached movement speech is not authoritative during boarding/loaded saves.
    const bool ready=landable&&system::is_field_navigation_ready()&&!system::input_transition_blocked();
    const auto action=landable&&ready?available_action(vehicle):LandingAction::Unknown;
    if(landable&&ready&&action!=LandingAction::Unknown&&
       (!g_last_landable_state||(g_landing_action!=LandingAction::Unknown&&action!=g_landing_action))) {
        g_last_announcement_time=frame::now();
        const auto* line=action==LandingAction::Surface?L"Can surface":
                         action==LandingAction::Dive?L"Can dive":L"Can land";
        ffpr::say(T(line),false);
    }
    g_last_landable_state=landable;
    g_landing_action=action;
}

using SwitchLandable_t = void (*)(void* self, bool landable, const void* mi);
SwitchLandable_t o_SwitchLandable = nullptr;

void d_SwitchLandable(void* self, bool landable, const void* mi)
{
    o_SwitchLandable(self, landable, mi);
    hooks::guarded("VehicleLanding.SwitchLandable", [&] {
        observe_landing_guide(self,landable);
    });
}
SwitchLandable_t o_ShowLandingGuide=nullptr;
void d_ShowLandingGuide(void* self,bool landable,const void* mi)
{
    o_ShowLandingGuide(self,landable,mi);
    // FieldController.UpdateStateSwitchLandable calls ShowLandingGuide, whose
    // native body bypasses MapUIManager.SwitchLandable. Observe both producers
    // through one visibility/edge guard, without hooking folded UI leaf bodies.
    hooks::guarded("VehicleLanding.ShowLandingGuide",[&]{observe_landing_guide(self,landable);});
}
using HideLandingGuide_t=void(*)(void*,const void*);
HideLandingGuide_t o_HideLandingGuide=nullptr;
void d_HideLandingGuide(void* self,const void* mi)
{
    o_HideLandingGuide(self,mi);
    hooks::guarded("VehicleLanding.HideLandingGuide",[&]{
        if(kMapUIManager.is(self)&&ref::rd<void*>(self,fLandingGuide)==g_landing_guide)reset_landing_state();
    });
}

// --- MovementSpeechPatches ------------------------------------------------------------------

// MapConstants.TransportationType values (identical in all five games).
constexpr int kTransportPlayer = 1;
constexpr int kTransportShip = 2;
constexpr int kTransportPlane = 3;          // airship
constexpr int kTransportContent = 5;        // canoe in FF1
constexpr int kTransportSubmarine = 6;
constexpr int kTransportLowFlying = 7;
constexpr int kTransportSpecialPlane = 8;
constexpr int kTransportYellowChocobo = 9;
constexpr int kTransportBlackChocobo = 10;

ref::Class  kFieldPlayer2{"Last.Entity.Field", "FieldPlayer", game::kFF1 | game::kFF2 | game::kFF4};
ref::Class  kFieldPlayer3{"Last.Entity.Field", "FieldPlayer", game::kFF3 | game::kFF5 | game::kFF6};
ref::Class  kFieldPlayer{"Last.Entity.Field", "FieldPlayer"};
ref::Method mGetOn2{kFieldPlayer2, "GetOn", 2};     // (int typeId, bool isBackground)
ref::Method mGetOn3{kFieldPlayer3, "GetOn", 3};     // (int typeId, float uniqueSpeedScale, bool isBackground)
ref::Method mGetOff{kFieldPlayer, "GetOff", 2};     // (int typeId, int layer)

/// Human-readable name for a TransportationType, or empty (FF1 mapping).
std::wstring transportation_name(int type_id)
{
    switch (type_id) {
    case kTransportShip: return T(L"ship");
    case kTransportPlane: return T(L"airship");
    case kTransportContent:                              // FF1: canoe uses the Content slot
        return game::is(game::Id::FF6) ? std::wstring() : T(L"canoe");
    case kTransportSubmarine: return T(L"submarine");
    case kTransportLowFlying: return T(L"airship");      // low-flying mode of the airship
    case kTransportSpecialPlane: return T(L"airship");
    case kTransportYellowChocobo: return T(L"chocobo");
    case kTransportBlackChocobo: return T(L"chocobo");
    default: return std::wstring();
    }
}

/// The boarded vehicle's spoken name: the game's own (field::vehicle_display_name, from the
/// TransportationController's current vehicle, which the game switches before GetOn) when it
/// names the vehicle of this type; else FF1's per-type words. "On canoe" for FF2's snowcraft
/// (both are type 5) ended here on 2026-10-10.
std::wstring boarded_vehicle_name(int type_id)
{
    ::field::VehicleState v = ::field::current_vehicle();
    if (v.valid && v.type == type_id) {
        std::wstring name = ::field::vehicle_game_name(v.id, v.message_id);
        if (!name.empty()) return name;
    }
    return transportation_name(type_id);
}

/// GetOn postfix body: boarding AND disembarking. FF1 uses GetOn(vehicle) to board and
/// GetOn(1 = Player) to return to walking.
void on_get_on(int type_id)
{
    if (type_id == kTransportPlayer) {
        // Only announce if we were previously in a vehicle
        if (!move_state::is_on_foot()) {
            move_state::set_on_foot();
            ffpr::say(T(L"On foot"), true);
        }
        return;
    }

    // A script can restore remembered transport context while the constructed
    // local player still uses the walking asset. Do not announce that context
    // as a real canoe/ship boarding (for example after Shrine dialogue).
    const auto state=::field::player_movement_state();
    if(state&&(*state==move_state::kWalk||*state==move_state::kDush)) {
        const auto applied=::field::player_uses_vehicle_asset();
        if(applied&&!*applied) {
            const bool riding=!move_state::is_on_foot();move_state::set_on_foot();
            if(riding)ffpr::say(T(L"On foot"),true);
            return;
        }
    }

    std::wstring vehicle_name = boarded_vehicle_name(type_id);
    if (!vehicle_name.empty()) {
        std::wstring announcement = TF(L"On {0}", {vehicle_name});
        move_state::set_vehicle_state(type_id, vehicle_name);
        ffpr::say(announcement, true);
    }
}

using GetOn2_t = void (*)(void* self, int32_t type_id, bool is_background, const void* mi);
using GetOn3_t = void (*)(void* self, int32_t type_id, float unique_speed_scale, bool is_background, const void* mi);
using GetOff_t = void (*)(void* self, int32_t type_id, int32_t layer, const void* mi);
GetOn2_t o_GetOn2 = nullptr;
GetOn3_t o_GetOn3 = nullptr;
GetOff_t o_GetOff = nullptr;

void d_GetOn2(void* self, int32_t type_id, bool is_background, const void* mi)
{
    o_GetOn2(self, type_id, is_background, mi);
    hooks::guarded("MovementSpeech.GetOn", [&] { on_get_on(type_id); });
}

void d_GetOn3(void* self, int32_t type_id, float unique_speed_scale, bool is_background, const void* mi)
{
    o_GetOn3(self, type_id, unique_speed_scale, is_background, mi);
    hooks::guarded("MovementSpeech.GetOn", [&] { on_get_on(type_id); });
}

void d_GetOff(void* self, int32_t type_id, int32_t layer, const void* mi)
{
    o_GetOff(self, type_id, layer, mi);
    hooks::guarded("MovementSpeech.GetOff", [&] {
        std::wstring vehicle_name = transportation_name(type_id);
        // A vehicle the game named (boarding said "On <name>") counts as known too.
        const bool was_on_vehicle = !move_state::is_on_foot();
        move_state::set_on_foot();

        // Only announce "On foot" if we were on a known vehicle
        if (!vehicle_name.empty() || was_on_vehicle)
            ffpr::say(T(L"On foot"), true);
    });
}

int install()
{
    int n = 0;
    if (hooks::attach(mSwitchLandable, &d_SwitchLandable, &o_SwitchLandable)) ++n;
    else LOGF("[Landing] Could not hook MapUIManager.SwitchLandable");
    if(hooks::attach(mShowLandingGuide,&d_ShowLandingGuide,&o_ShowLandingGuide))++n;
    if(hooks::attach(mHideLandingGuide,&d_HideLandingGuide,&o_HideLandingGuide))++n;
    events::on(events::Kind::MapChanging,[]{reset_landing_state();});
    events::on(events::Kind::BattleEntering,[]{reset_landing_state();});

    bool get_on = false;
    if (kFieldPlayer2.applies()) get_on = hooks::attach(mGetOn2, &d_GetOn2, &o_GetOn2);
    else if (kFieldPlayer3.applies()) get_on = hooks::attach(mGetOn3, &d_GetOn3, &o_GetOn3);
    if (get_on) ++n;
    else LOGF("[MoveState] Could not hook FieldPlayer.GetOn");

    if (hooks::attach(mGetOff, &d_GetOff, &o_GetOff)) ++n;
    else LOGF("[MoveState] Could not hook FieldPlayer.GetOff");
    return n;
}
core::Install s_install{"VehicleLanding", &install, 120};

} // namespace

void reset_landing_state() {
    g_last_landable_state=false;g_landing_guide=nullptr;
    g_landing_action=LandingAction::Unknown;
    g_landing_vehicle_id=g_landing_vehicle_type=0;
}
double last_announcement_time() { return g_last_announcement_time; }

void reset_movement_state() { move_state::reset(); }

} // namespace ffpr::fieldnav::vehicle_landing
