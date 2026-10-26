// The basic map and the Tab location menu are distinct native screens. Never
// read GPS treasure data on the basic map or reconstruct it from save flags.
#include "map_panels_data.h"
#include "../core.h"
#include "../hooks.h"
#include "../displayed_ui.h"
#include "../worker_job.h"
#include "../modtext.h"
#include "../say.h"
#include "../text.h"
#include "../unity.h"
#include "../system/system.h"
#include "../field/capture_budget.h"
#include "../field/baked_map.h"
#include "../field/field_api.h"
#include "../ui/review_buffer.h"
#include "../ui/preferences.h"
#include "../log.h"
#include <memory>

namespace ffpr::maps {
namespace {
ref::Class kBasic{"Last.UI.KeyInput","ExpandMinimapController"};
ref::Method mBasicShow{kBasic,"Show",4},mBasicHide{kBasic,"Hide",0};
ref::Class kGps{"Last.UI.KeyInput","GpsMenuController"};
ref::Method mGpsShow{kGps,"Show",3},mGpsClose{kGps,"Close",0};
ref::Class kInfo{"Last.UI.KeyInput","GpsMenuInfoController"};
ref::Method mInfoUpdate{kInfo,"UpdateView",1};
ref::Class kMap{"Last.UI.Common","ExpandMinimapMapController"};
ref::Method mScroll{kMap,"UpdateScroll",1},mScale{kMap,"Scaling",1};
ref::Class kProperty{"Last.Map","PropertyEntity"},kIcon{"Last.Map","PropertyMinimapIcon"};
ref::Field fPosition{kProperty,"<Pos>k__BackingField"},fIconType{kIcon,"<IconType>k__BackingField"};
ref::Class kService{"Last.UI","GpsIconContentView"};
ref::Field fServiceType{kService,"gpsIconType"};
enum class Screen {None,Basic,Gps};Screen g_screen=Screen::None;
void* g_owner=nullptr;void* g_review_owner=nullptr;
unsigned g_generation=0;worker_job::Owner g_work;
Reading g_reading;
bool g_capture_pending=false,g_details_requested=false;
bool active(){return g_screen!=Screen::None&&displayed::container_visible(g_owner);}
void retire() {
    ++g_generation;g_work.cancel();frame::cancel_label("MapPanels.Capture");
    if(ui::review::owns(g_review_owner))ui::review::clear();
    g_review_owner=nullptr;g_reading={};g_capture_pending=false;g_details_requested=false;
}
void close(void* owner) {
    if(owner!=g_owner)return;
    retire();g_owner=nullptr;g_screen=Screen::None;menu_state::set_active("MapPanels",false);
}
void publish(Snapshot snapshot,void* review_owner,unsigned generation) {
    const bool announce=snapshot.announce,basic=snapshot.basic,autodetail=snapshot.autodetail;
    const double requested=snapshot.requested;const int restore=ui::review::index();
    worker_job::start(g_work,std::move(snapshot),[](Snapshot& input) {
        auto result=build(input);
        LOGF("[MapPanels] copied mode=%d markers=%zu services=%zu counters=%zu slices=%u native=%.3f ms capture-age=%.3f ms",
            input.basic,input.markers.size(),input.services.size(),input.counters.size(),input.slices,input.native_ms,(frame::now()-input.requested)*1000);
        return result;
    },[generation,review_owner,announce,basic,autodetail,restore,requested](Reading reading) {
        if(generation!=g_generation||!active()||!displayed::container_visible(review_owner))return;
        g_reading=std::move(reading);g_review_owner=review_owner;g_capture_pending=false;
        const bool manual=g_details_requested;g_details_requested=false;
        const auto count=g_reading.details.size();
        if(g_reading.buffer) {
            ui::review::Options options;options.domain=ui::review::Domain::Information;
            options.announce=announce;options.restore_index=announce?0:restore;
            // Transfer the worker-built rows; copying a large basic-map buffer
            // here would move string allocation back onto the native owner.
            ui::review::show(review_owner,g_reading.name,std::move(g_reading.details),std::move(options));
            if(manual)ui::review::details(); // review owns replay until its worker is ready
        } else if(announce) {
            std::vector<std::wstring> parts{g_reading.name};
            if(autodetail)parts.insert(parts.end(),g_reading.details.begin(),g_reading.details.end());
            ffpr::say_parts(std::move(parts),true);
        }
        if(manual&&!g_reading.buffer) {
            auto parts=g_reading.details;parts.insert(parts.begin(),g_reading.name);
            ffpr::say_parts(std::move(parts),true);
        }
        LOGF("[MapPanels] published mode=%d details=%zu request-to-submit=%.3f ms",basic,count,(frame::now()-requested)*1000);
    });
}
Snapshot begin_snapshot(bool basic,bool announce) {
    Snapshot out;out.basic=basic;out.announce=announce;out.autodetail=ui::prefs::auto_detail_enabled();
    out.locale=modtext::lang();out.requested=frame::now();return out;
}
// Only known serialized view/list/renderer owners are traversed; native pointers
// remain in this frame closure. A new Show/selection/scroll retires stale work.
void basic_capture(bool announce) {
    if(g_screen!=Screen::Basic)return;retire();const unsigned generation=g_generation;
    g_capture_pending=true;auto input=std::make_shared<Snapshot>(begin_snapshot(true,announce));
    auto* owner=g_owner;
    frame::every_frame([owner,generation,input,next=0,initialized=false]() mutable {
        if(generation!=g_generation||!active()||g_owner!=owner)return false;
        const double began=frame::now();field::CaptureBudget budget;
        auto* map=displayed::field(owner,"mapController");
        if(!displayed::container_visible(map))return false;
        auto* properties=displayed::field(map,"minimapIconPropertyList");
        auto* points=displayed::field(map,"pointItemList");
        const int count=unity::list_count(properties);
        if(count<0||count>8192||unity::list_count(points)!=count)return false;
        if(!initialized) {
            initialized=true;input->name=T(L"Map");
            // Show's native center/player coordinates and property positions
            // share the same units. Read the owner field, not transformed UI.
            auto* cls=il2cpp::object_class(static_cast<il2cpp::Object*>(map));const int offset=cls?il2cpp::field_offset(cls,"playerPos"):-1;
            if(offset<0)return false;
            const auto pos=il2cpp::read_field<unity::Vec2>(map,offset);input->x=pos.x;input->y=pos.y;
            if(const auto* sm=field::baked::submap_by_map_id(field::current_map_id());sm&&sm->world) {
                input->wrap_width=sm->width*16.f;input->wrap_height=sm->height*16.f;
            }
        }
        int copied=0;
        while(next<count&&copied++<512&&budget.available()) {
            auto* property=unity::list_at(properties,next);auto* item=unity::list_at(points,next++);
            auto* point=displayed::field(item,"<PointView>k__BackingField");
            if(!property||!displayed::image_visible(displayed::field(point,"image")))continue;
            const auto pos=ref::rd<unity::Vec3>(property,fPosition);
            input->markers.push_back({ref::rd<int32_t>(property,fIconType),pos.x,pos.y});
        }
        ++input->slices;input->native_ms+=(frame::now()-began)*1000;
        if(next<count)return true;
        publish(std::move(*input),owner,generation);return false;
    },"MapPanels.Capture",INT_MAX);
}
void info_capture(void* controller,void* content) {
    if(g_screen!=Screen::Gps)return;retire();const unsigned generation=g_generation;
    g_capture_pending=true;auto input=std::make_shared<Snapshot>(begin_snapshot(false,true));auto* owner=g_owner;
    frame::every_frame([owner,controller,content,input,generation,next=0,initialized=false]() mutable {
        if(generation!=g_generation||!active()||owner!=g_owner||!unity::alive(controller)||!unity::alive(content))return false;
        const double began=frame::now();field::CaptureBudget budget;
        auto* view=displayed::field(controller,"view");
        if(!initialized) {
            initialized=true;
            input->name=displayed::text_field(displayed::field(content,"view"),"placeText");
            // Current Location/vehicles hide contentParent while cached Text
            // objects can still contain the previous location's numbers.
            if(displayed::container_visible(displayed::field(view,"contentParent"))) {
                input->counters.push_back({displayed::text_field(view,"fixedChestText"),displayed::text_field(view,"numChestText"),displayed::text_field(view,"maxChestText")});
                input->counters.push_back({displayed::text_field(view,"fixedHideItemText"),displayed::text_field(view,"numHideItemText"),displayed::text_field(view,"maxHideItemText")});
            } else {publish(std::move(*input),owner,generation);return false;}
        }
        auto* icons=displayed::field(view,"shopIconList");const int count=unity::list_count(icons);
        if(count<0||count>64)return false;
        int copied=0;while(next<count&&copied++<64&&budget.available()) {
            auto* icon=unity::list_at(icons,next++);
            if(displayed::image_visible(displayed::field(icon,"image")))input->services.push_back(ref::rd<int32_t>(icon,fServiceType));
        }
        ++input->slices;input->native_ms+=(frame::now()-began)*1000;
        if(next<count)return true;
        publish(std::move(*input),owner,generation);return false;
    },"MapPanels.Capture",INT_MAX);
}
using BasicShow=void(*)(void*,void*,unity::Vec2,unity::Vec2,void*,const void*);
BasicShow o_basic;
void d_basic(void* self,void* texture,unity::Vec2 pos,unity::Vec2 dir,void* properties,const void* mi) {
    o_basic(self,texture,pos,dir,properties,mi);
    hooks::guarded("MapPanels.Basic",[=]{retire();g_owner=self;g_screen=Screen::Basic;menu_state::set_active("MapPanels",true);basic_capture(true);});
}
using Empty=void(*)(void*,const void*);Empty o_hide,o_close;
void d_hide(void* self,const void* mi){o_hide(self,mi);hooks::guarded("MapPanels.Hide",[=]{close(self);});}
void d_close(void* self,const void* mi){o_close(self,mi);hooks::guarded("MapPanels.Close",[=]{close(self);});}
using GpsShow=void(*)(void*,void*,void*,bool,const void*);GpsShow o_gps;
void d_gps(void* self,void* sprite,void* data,bool from_basic,const void* mi) {
    // Native Show invokes its first selection synchronously; establish only
    // owner identity before forwarding. Renderer reads defer until it returns.
    retire();g_owner=self;g_screen=Screen::Gps;menu_state::set_active("MapPanels",true);
    o_gps(self,sprite,data,from_basic,mi);
}
using Update=void(*)(void*,void*,const void*);Update o_info;
void d_info(void* self,void* content,const void* mi){o_info(self,content,mi);hooks::guarded("MapPanels.Info",[=]{info_capture(self,content);});}
using Vector=void(*)(void*,unity::Vec2,const void*);Vector o_scroll,o_scale;
void d_scroll(void* self,unity::Vec2 value,const void* mi){o_scroll(self,value,mi);if(value.x||value.y)hooks::guarded("MapPanels.Scroll",[=]{if(displayed::field(g_owner,"mapController")==self)basic_capture(false);});}
void d_scale(void* self,unity::Vec2 value,const void* mi){o_scale(self,value,mi);hooks::guarded("MapPanels.Scale",[=]{if(displayed::field(g_owner,"mapController")==self)basic_capture(false);});}
int install() {
    queries::define("MapPanels.IsActive",[]{return active();});
    actions::define("MapPanels.ReadDetails",[]{
        if(!active())return;
        if(g_capture_pending){g_details_requested=true;return;}
        if(g_reading.buffer){ui::review::details();return;}
        auto lines=g_reading.details;lines.insert(lines.begin(),g_reading.name);ffpr::say_parts(std::move(lines),true);
    });
    actions::define("MapPanels.ReadControls",[]{
        auto* help=active()?displayed::field(g_owner,"keyHelpController"):nullptr;
        ffpr::system::announce_controls_for(help);
    });
    events::on(events::Kind::SceneLoaded,[]{close(g_owner);});
    events::on(events::Kind::BattleStart,[]{close(g_owner);});
    int n=0;
    n+=hooks::attach(mBasicShow,&d_basic,&o_basic);n+=hooks::attach(mBasicHide,&d_hide,&o_hide);
    n+=hooks::attach(mGpsShow,&d_gps,&o_gps);n+=hooks::attach(mGpsClose,&d_close,&o_close);
    n+=hooks::attach(mInfoUpdate,&d_info,&o_info);n+=hooks::attach(mScroll,&d_scroll,&o_scroll);n+=hooks::attach(mScale,&d_scale,&o_scale);
    return n;
}
core::Install s_install{"Menus.MapPanels",&install,125};
}
}
