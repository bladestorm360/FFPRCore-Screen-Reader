// check-sources: menus/map_panels_data.cpp bestiary/bestiary_reader.cpp ui/navigation_buffer.cpp text.cpp dedup.cpp
// Actual screen-owned controls capture with the same native-owner/worker fixture.
#define FFPR_MAP_FIXTURE_ONLY
#define announce_controls_for retained_controls_for
#include "map_panels_check.cpp"
#undef announce_controls_for
#include "../src/core/system/key_help_owned.cpp"
namespace controls_fixture {void* manager=nullptr;}
namespace ref {void* instance(Class&){fixture::native_read();return controls_fixture::manager;}}
namespace unity {
void* transform(void* p){fixture::native_read();return p;}
int child_count(void* p){return list_count(displayed::field(p,"children"));}
void* child(void* p,int index){return list_at(displayed::field(p,"children"),index);}
void* get_component(void* p,ref::Class&){fixture::native_read();return p;}
}
int main() {
    using namespace fixture;int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    for(int n=1;n<=6;++n) {
        ffpr::system::owners={};
        ffpr::system::icon_owners={};
        fixture::game=static_cast<game::Id>(n);Node controller,view,parent,entry,hint,key,caption,other,other_hint,other_caption;
        set(controller,"view",&view);set(view,"keyHelpContentsParent",&parent);List children{&entry,&other};set(controller,"contentList",&children);
        set(entry,"view",&hint);set(other,"view",&other_hint);List icons{&key};set(hint,"iconTextList",&icons);set(hint,"keyText",&caption);set(other_hint,"keyText",&other_caption);
        key.words=L"W, S";caption.words=L"Scroll";other_caption.words=L"Hidden previous monster";other_hint.visible=false;
        spoken.clear();ffpr::system::announce_controls_for(&controller);map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"W, S: Scroll"}),"all-six owned bars read rendered shortcut plus action and exclude hidden sibling entries");
        spoken.clear();ffpr::system::announce_controls_for(nullptr);map_fixture::drain();expect(spoken==std::vector<std::wstring>({L"No controls displayed"}),"missing owned bar reports no controls instead of using another cached bar");
        controller.visible=false;spoken.clear();ffpr::system::announce_controls_for(&controller);map_fixture::drain();expect(spoken==std::vector<std::wstring>({L"W, S: Scroll"}),"a separately rooted rendered bar remains readable with an inactive controller");controller.visible=true;
        spoken.clear();ffpr::system::announce_controls_for(&controller);view.visible=false;map_fixture::drain();expect(spoken.empty(),"closing the owning rendered bar cancels pending controls speech");view.visible=true;
        view.visible=false;spoken.clear();ffpr::system::announce_controls_for(&controller);map_fixture::drain();expect(spoken==std::vector<std::wstring>({L"No controls displayed"}),"an explicit controls request on an already hidden bar reports no controls");view.visible=true;
        spoken.clear();ffpr::system::announce_controls_for(&controller);ffpr::system::cancel_controls_reading();map_fixture::drain();expect(spoken.empty(),"scene retirement cancels capture before worker publication");
        Node pooled,pooled_view;set(pooled,"view",&pooled_view);List empty_controls;set(pooled,"contentList",&empty_controls);
        controller.visible=false;ffpr::system::register_controls_owner(&controller);ffpr::system::register_controls_owner(&pooled);
        spoken.clear();ffpr::system::announce_current_controls();map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"W, S: Scroll"}),"generic Shift+I skips a newer empty pool and reads the actual visible bar despite its inactive script");
        pooled_view.visible=false;spoken.clear();ffpr::system::announce_current_controls();map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"W, S: Scroll"}),"hidden preloaded bars cannot replace a rendered published footer");
        ffpr::system::retire_controls_owner(&controller);spoken.clear();ffpr::system::announce_current_controls();map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"No controls displayed"}),"a retired bar cannot leak controls from the prior screen");
        controller.visible=true;view.visible=true;ffpr::system::register_controls_owner(&controller);
        spoken.clear();ffpr::system::announce_current_controls();ffpr::system::retire_controls_owner(&controller);map_fixture::drain();
        expect(spoken.empty(),"destroying a snapshotted owner cancels the request before dereferencing it");
        ffpr::system::retire_controls_owner(&pooled);
        Node standalone,standalone_view,tab,caption_widget;
        set(standalone,"view",&standalone_view);List widget_keys{&tab};set(standalone_view,"iconTextList",&widget_keys);
        set(standalone_view,"keyText",&caption_widget);tab.words=L"Tab";caption_widget.words=L"Change Equipment";
        ffpr::system::register_controls_icon(&standalone);
        spoken.clear();ffpr::system::announce_current_controls();map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"Tab: Change Equipment"}),"standalone battle hint is readable with no footer controller");
        ffpr::system::register_controls_owner(&controller);ffpr::system::register_controls_icon(&entry);
        spoken.clear();ffpr::system::announce_current_controls();map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"W, S: Scroll, Tab: Change Equipment"}),"footer plus standalone controls are combined, same physical footer child occurs once");
        spoken.clear();ffpr::system::announce_controls_for(&controller);map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"W, S: Scroll"}),"explicit map/Bestiary footer cannot inherit standalone controls from another screen");
        standalone_view.visible=false;spoken.clear();ffpr::system::announce_current_controls();map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"W, S: Scroll"}),"hidden standalone captions never leak");standalone_view.visible=true;
        spoken.clear();ffpr::system::announce_current_controls();ffpr::system::retire_controls_icon(&standalone);map_fixture::drain();
        expect(spoken.empty(),"standalone destruction cancels snapshotted pointer before read");
        // Exact game-owned registry, including a visible prefab initialized
        // before hooks attached. Hidden preload records must not become speech.
        Node manager,hidden,hidden_view;set(hidden,"view",&hidden_view);hidden_view.visible=false;
        List native_icons(80,&hidden),native_bars;native_icons.push_back(&standalone);
        set(manager,"iconControllers",&native_icons);set(manager,"helpControllers",&native_bars);
        controls_fixture::manager=&manager;ffpr::system::owners={};ffpr::system::icon_owners={};
        const auto start_tick=fixture::ticks;spoken.clear();ffpr::system::announce_current_controls();map_fixture::drain();
        expect(spoken==std::vector<std::wstring>({L"Tab: Change Equipment"})&&fixture::ticks>start_tick+2,
               "native manager recovers an unobserved standalone hint through bounded slices despite many hidden preloads");
        if(n==2)for(int scenario=0;scenario<3;++scenario) {
            std::vector<double> times;uint64_t max_ticks=0;
            ffpr::system::owners={};ffpr::system::icon_owners={};
            controls_fixture::manager=scenario==2?&manager:nullptr;
            if(scenario<2)ffpr::system::register_controls_owner(&controller);
            if(scenario==1)ffpr::system::register_controls_icon(&standalone);
            for(int sample=0;sample<50;++sample) {
                spoken.clear();const auto before_ticks=fixture::ticks;
                const auto at=std::chrono::steady_clock::now();
                ffpr::system::announce_current_controls();map_fixture::drain();
                times.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-at).count());
                max_ticks=(std::max)(max_ticks,fixture::ticks-before_ticks);
                expect(spoken.size()==1,"full controls request publishes one complete current result");
            }
            std::sort(times.begin(),times.end());
            std::printf("Controls request %d: median %.3f us, p95 %.3f us, max %.3f us; max %llu fixture ticks (native calls/glyphs/audio fake; worker strings real)\n",
                        scenario,times[25],times[47],times.back(),static_cast<unsigned long long>(max_ticks));
        }
        controls_fixture::manager=nullptr;
    }
    expect(native_thread_violations==0,"controls workers only join copied strings, with every native view/list read on its owner");
    std::printf("Screen-owned controls: actual bounded native capture/worker formatting: %d failures\n",failures);return failures?1:0;
}
