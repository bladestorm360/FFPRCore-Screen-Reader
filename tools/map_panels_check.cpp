// check-sources: menus/map_panels_data.cpp bestiary/bestiary_reader.cpp ui/navigation_buffer.cpp text.cpp dedup.cpp
// Actual map detours/renderer capture/owned formatting; no game or UI control.
#define main retained_bestiary_main
#include "bestiary_check.cpp"
#undef main
#include <thread>
#include <chrono>
#include "../src/core/menus/map_panels.cpp"
#include "../src/core/ui/review_buffer.cpp"
namespace map_fixture {
std::vector<std::function<void()>> work,completed;
bool autodetail=false;void* review_owner=nullptr;std::wstring headline;
std::vector<std::wstring> lines;bool announce=false;int copies=0;
void drain(){for(int i=0;i<4096;++i){fixture::tick();auto jobs=std::move(work);work.clear();std::thread worker([&]{for(auto& job:jobs)job();});worker.join();auto done=std::move(completed);completed.clear();for(auto& callback:done)callback();if(fixture::tasks.empty()&&work.empty()&&completed.empty())break;}lines=ui::review::g_lines;headline=ui::review::g_headline;}
}
namespace modtext {LocaleScope::LocaleScope(std::wstring){}LocaleScope::~LocaleScope(){}const std::wstring& lang(){static std::wstring en=L"en";return en;}}
namespace background {bool post(std::function<void()> fn,Lane){map_fixture::work.push_back(std::move(fn));return true;}}
namespace frame {void post(Task fn){map_fixture::completed.push_back(std::move(fn));}}
namespace settings {bool get_bool(const char*,bool fallback){return map_fixture::autodetail?true:fallback;}}
namespace displayed {bool image_visible(void* p){return container_visible(p)&&static_cast<fixture::Node*>(p)->rendered;}}
namespace il2cpp {
Class* object_class(Object* p){return reinterpret_cast<Class*>(p);}
int32_t field_offset(Class*,const char* name){return fixture::offset(name);}
}
namespace field {int current_map_id(){return 98;}}
namespace field::baked {const Submap* submap_by_map_id(int){return nullptr;}}
namespace ffpr {void say_parts(std::vector<std::wstring> values,bool){for(auto& value:values)fixture::spoken.push_back(std::move(value));}}
namespace ui {std::wstring menu_position(const std::wstring& s,int,int){return s;}}
namespace input {void add_binding(Key,Mod,Context,const char*,const char*){}}
namespace {
void empty(void*,const void*){}
void basic(void*,void*,unity::Vec2,unity::Vec2,void*,const void*){}
void gps(void*,void*,void*,bool,const void*){}
void info(void*,void*,const void*){}
void vector(void*,unity::Vec2,const void*){}
}
#ifndef FFPR_MAP_FIXTURE_ONLY
int main() {
    using namespace fixture;using namespace ffpr::maps;
    int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    o_basic=&basic;o_gps=&gps;o_info=&info;o_hide=o_close=&empty;o_scroll=o_scale=&vector;install();
    for(int n=1;n<=6;++n) {
        fixture::game=static_cast<game::Id>(n);spoken.clear();ui::review::clear();
        Node owner,info_controller,view,content,content_view,place,panel,chest,current,total,hidden_label,hidden_num,hidden_total;
        Node first_icon,second_icon,first_image,second_image,help,wrong_help;
        set(owner,"keyHelpController",&help);set(info_controller,"view",&view);set(content,"view",&content_view);set(content_view,"placeText",&place);
        set(view,"contentParent",&panel);set(view,"fixedChestText",&chest);set(view,"numChestText",&current);set(view,"maxChestText",&total);
        set(view,"fixedHideItemText",&hidden_label);set(view,"numHideItemText",&hidden_num);set(view,"maxHideItemText",&hidden_total);
        List icons{&first_icon,&second_icon};set(view,"shopIconList",&icons);set(first_icon,"image",&first_image);set(second_icon,"image",&second_image);
        *reinterpret_cast<int*>(reinterpret_cast<char*>(&first_icon)+offset("gpsIconType"))=6;
        *reinterpret_cast<int*>(reinterpret_cast<char*>(&second_icon)+offset("gpsIconType"))=1;
        second_image.visible=false;hidden_label.visible=hidden_num.visible=hidden_total.visible=false;
        place.words=L"Tropical Island";chest.words=L"Chests";current.words=L"5";total.words=L"20";
        map_fixture::autodetail=false;d_gps(&owner,nullptr,nullptr,true,nullptr);d_info(&info_controller,&content,nullptr);map_fixture::drain();
        expect(map_fixture::headline==L"Tropical Island"&&map_fixture::lines==std::vector<std::wstring>({L"Inn",L"Chests 5/20"}),
            "all-six native location selection copies only rendered service icons and counters into its information buffer");
        spoken.clear();ui::review::next();expect(spoken==std::vector<std::wstring>({L"Chests 5/20"}),"real arrow buffer reads the next displayed location detail");
        spoken.clear();ui::review::details();expect(spoken.size()==1&&spoken[0].find(L"Tropical Island")!=spoken[0].npos&&spoken[0].find(L"5/20")!=spoken[0].npos,"real I buffer retains the location and its complete current details");
        controls_owner=&wrong_help;actions::run("MapPanels.ReadControls");expect(controls_owner==&help,"map controls use the screen-owned hint bar rather than another initialized controller");
        panel.visible=false;place.words=L"Current Location";spoken.clear();d_info(&info_controller,&content,nullptr);map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"Current Location"})&&map_fixture::lines.empty(),"blank Current Location panel clears previous location's chest and service details");
        place.words=L"Ship";spoken.clear();d_info(&info_controller,&content,nullptr);map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"Ship"}),"vehicle selection remains name-only with no inherited counters");
        panel.visible=true;first_image.visible=false;place.words=L"Dungeon";spoken.clear();d_info(&info_controller,&content,nullptr);map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"Dungeon"})&&!g_reading.buffer,"Autodetail off announces only the name for a chest-only location");
        spoken.clear();actions::run("MapPanels.ReadDetails");expect(spoken==std::vector<std::wstring>({L"Dungeon",L"Chests 5/20"}),"manual I action reads current displayed count without a chest-only arrow buffer");
        map_fixture::autodetail=true;spoken.clear();d_info(&info_controller,&content,nullptr);map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"Dungeon",L"Chests 5/20"}),"Autodetail on includes current visible count automatically");
        map_fixture::autodetail=false;spoken.clear();d_info(&info_controller,&content,nullptr);actions::run("MapPanels.ReadDetails");map_fixture::drain();
        expect(spoken.size()>=2&&spoken.back()==L"Chests 5/20","manual I requested during native capture survives until current details are ready");
        chest.visible=current.visible=total.visible=false;
        hidden_label.visible=hidden_num.visible=hidden_total.visible=true;
        hidden_label.words=L"Items";hidden_num.words=L"6";hidden_total.words=L"6";
        spoken.clear();d_info(&info_controller,&content,nullptr);map_fixture::drain();actions::run("MapPanels.ReadDetails");
        expect(!g_reading.buffer&&spoken.back()==L"Items 6/6","Items uses its own displayed counter, independent of Chests wording, in all six games");
        chest.visible=current.visible=total.visible=true;
        spoken.clear();d_info(&info_controller,&content,nullptr);map_fixture::drain();ui::review::last();
        expect(g_reading.buffer&&spoken.back()==L"Items 6/6","both native counters assemble into the real arrow buffer");
        spoken.clear();d_info(&info_controller,&content,nullptr);panel.visible=false;place.words=L"Ship";d_info(&info_controller,&content,nullptr);map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"Ship"}),"rapid selection cancels the previous location before its counters can publish");
        d_info(&info_controller,&content,nullptr);d_close(&owner,nullptr);spoken.clear();map_fixture::drain();expect(spoken.empty()&&!active(),"closing retires pending native capture/worker speech");
        Node basic_owner,map,property,item,point,image;List properties{&property},points{&item};
        set(basic_owner,"mapController",&map);set(basic_owner,"keyHelpController",&help);set(map,"minimapIconPropertyList",&properties);set(map,"pointItemList",&points);set(item,"<PointView>k__BackingField",&point);set(point,"image",&image);
        *reinterpret_cast<unity::Vec2*>(reinterpret_cast<char*>(&map)+offset("playerPos"))={0,0};
        *reinterpret_cast<unity::Vec3*>(reinterpret_cast<char*>(&property)+offset("<Pos>k__BackingField"))={0,64,0};
        *reinterpret_cast<int*>(reinterpret_cast<char*>(&property)+offset("<IconType>k__BackingField"))=2;
        d_basic(&basic_owner,nullptr,{0,0},{0,1},&properties,nullptr);map_fixture::drain();
        expect(map_fixture::lines==std::vector<std::wstring>({L"Stairs, about 4 steps North"}),"basic map captures its visible native markers with player-relative bearing and never GPS chest totals");
        d_hide(&basic_owner,nullptr);
    }
    Snapshot wrapped;wrapped.basic=true;wrapped.locale=L"en";wrapped.x=15*16;wrapped.wrap_width=16*16;wrapped.markers={{1,0,0}};
    expect(build(wrapped).details==std::vector<std::wstring>({L"Map marker, about 1 steps East"}),"world marker bearing uses wrapped native coordinates");
    expect(fixture::native_thread_violations==0,"map workers only format owned snapshots and never read native views or lists");
    for(int count:{32,8192}) {
        Node owner,map,property,item,point,image;List properties(count,&property),points(count,&item);
        set(owner,"mapController",&map);set(map,"minimapIconPropertyList",&properties);set(map,"pointItemList",&points);set(item,"<PointView>k__BackingField",&point);set(point,"image",&image);
        *reinterpret_cast<unity::Vec2*>(reinterpret_cast<char*>(&map)+offset("playerPos"))={0,0};
        *reinterpret_cast<unity::Vec3*>(reinterpret_cast<char*>(&property)+offset("<Pos>k__BackingField"))={16,64,0};
        *reinterpret_cast<int*>(reinterpret_cast<char*>(&property)+offset("<IconType>k__BackingField"))=2;
        const auto start=std::chrono::steady_clock::now();d_basic(&owner,nullptr,{0,0},{0,1},&properties,nullptr);map_fixture::drain();
        const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
        expect(ui::review::active()&&!ui::review::g_build_pending&&ui::review::g_lines.size()==size_t(count),"normal/busy map reaches the real ready review buffer without truncating marker rows");
        std::printf("Map%d capture -> owned map worker -> real review worker/usable buffer: %.3f ms, synthetic native/frame scheduler and speech (no game frames/device/audio)\n",count,elapsed);d_hide(&owner,nullptr);
    }
    std::printf("Map panels: actual native capture, selection/screen disclosure, controls, Autodetail/I and owned marker formatting: %d failures\n",failures);return failures?1:0;
}
#endif
