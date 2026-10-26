// Screen-owned hint bars and standalone hint widgets. Native callbacks copy
// only serialized KeyIconView texts; no descendant scan or content deduplication.
#include "system.h"
#include "../displayed_ui.h"
#include "../unity.h"
#include "../worker_job.h"
#include "../field/capture_budget.h"
#include "../modtext.h"
#include "../text.h"
#include "../say.h"
#include "../refs.h"
#include <algorithm>
#include <array>

namespace ffpr::system {
namespace {
worker_job::Owner g_controls_work;unsigned g_controls_generation=0;
struct Owner {void* controller=nullptr;uint64_t publication=0;};
std::array<Owner,256> owners;
std::array<Owner,1024> icon_owners;
uint64_t publication=0;
void* selected_owner=nullptr;
std::vector<void*> selected_icons;
struct Candidate {void* controller=nullptr;bool single=false;};
ref::Class kControlsManager{"Last.Management","KeyHelpIconManager"};
ref::Field fManagerIcons{kControlsManager,"iconControllers"},fManagerBars{kControlsManager,"helpControllers"};
struct CapturedControls {std::wstring locale;std::vector<std::vector<std::wstring>> rows;};
bool current_bar(void* controller) {
    if(!controller)return true; // explicit missing bar reports No controls displayed
    return unity::alive(controller)&&displayed::container_visible(displayed::field(controller,"view"));
}
}
void cancel_controls_reading() {
    ++g_controls_generation;selected_owner=nullptr;selected_icons.clear();g_controls_work.cancel();frame::cancel_label("KeyHelp.OwnedCapture");frame::cancel_label("KeyHelp.RegistryCapture");
}
namespace {
template<size_t N> void remember(std::array<Owner,N>& registry,void* controller,bool published) {
    if(!controller)return;
    if(published&&(selected_owner==controller||std::find(selected_icons.begin(),selected_icons.end(),controller)!=selected_icons.end()))
        {++g_controls_generation;g_controls_work.interrupt();}
    Owner* place=nullptr;
    for(auto& owner:registry)if(owner.controller==controller){place=&owner;break;}
    if(place&&!published)return;
    if(!place)for(auto& owner:registry)if(!owner.controller){place=&owner;break;}
    // Fixed retention of recently published bars, rather than an unbounded
    // initialization history. Native OnDestroy retires owners too. No UI scan.
    if(!place) {
        if(!published)return; // hidden preload cannot evict a real published bar
        place=&*std::min_element(registry.begin(),registry.end(),[](const Owner& a,const Owner& b){return a.publication<b.publication;});
    }
    *place={controller,published?++publication:0};
}
template<size_t N> void retire(std::array<Owner,N>& registry,void* controller) {
    for(auto& owner:registry)if(owner.controller==controller)owner={};
    // Also invalidate pointers obtained from the native manager before the
    // mod observed Initialize. OnDestroy runs before the object is released.
    ++g_controls_generation;g_controls_work.interrupt();
    if(selected_owner==controller)selected_owner=nullptr;
}
// Each widget is formatted once by native identity. A widget also registered
// as a footer child is the same displayed control, not a repeated text event.
void copy_widget(void* content,CapturedControls& copied) {
    if(std::find(selected_icons.begin(),selected_icons.end(),content)!=selected_icons.end())return;
    auto* hint=displayed::field(content,"view");
    if(!displayed::container_visible(hint))return;
    selected_icons.push_back(content);
    std::vector<std::wstring> parts;
    auto add=[&](void* value){auto line=displayed::text(value);if(!line.empty()){line.resize((std::min)(line.size(),size_t{256}));parts.push_back(std::move(line));}};
    auto* icons=displayed::field(hint,"iconTextList");const int keys=unity::list_count(icons);
    if(keys<0||keys>16)return;
    for(int i=0;i<keys;++i)add(unity::list_at(icons,i));
    for(const char* name:{"keyText","keyText2","separateText"})add(displayed::field(hint,name));
    if(!parts.empty())copied.rows.push_back(std::move(parts));
}
void capture_controls(std::vector<Candidate> candidates,bool explicit_owner) {
    cancel_controls_reading();const auto generation=g_controls_generation;
    if(explicit_owner&&!candidates.empty()&&!current_bar(candidates.front().controller)){ffpr::say(T(L"No controls displayed"),true);return;}
    auto copied=std::make_shared<CapturedControls>();copied->locale=modtext::lang();
    frame::every_frame([candidates=std::move(candidates),explicit_owner,generation,copied,next=0,owner_index=size_t{0}]() mutable {
        if(generation!=g_controls_generation)return false;
        field::CaptureBudget budget;int rows=0,checked=0;
        while(owner_index<candidates.size()&&checked++<32&&budget.available()) {
          const auto candidate=candidates[owner_index];void* controller=candidate.controller;
          // Only one current footer, followed by every separately observed
          // visible widget. Explicit map/Bestiary reads retain their own bar.
          if(selected_owner&&!candidate.single&&selected_owner!=controller){++owner_index;next=0;continue;}
          if(!controller||!current_bar(controller)) {
            if(controller&&(explicit_owner||selected_owner==controller))return false; // closed after the request
            ++owner_index;next=0;continue;
          }
          if(candidate.single) {
              const auto before=copied->rows.size();copy_widget(controller,*copied);
              if(!selected_owner&&copied->rows.size()!=before)selected_owner=controller;
              ++owner_index;continue;
          }
          auto* contents=displayed::field(controller,"contentList");
          const int count=unity::list_count(contents);
          if(count<0||count>64)return false;
          while(next<count&&rows++<8&&budget.available()) {
            // Native KeyHelpController already owns the exact pooled controls.
            // Its serialized view may be rooted separately from its script.
            copy_widget(unity::list_at(contents,next++),*copied);
          }
          if(next<count){if(!copied->rows.empty())selected_owner=controller;return true;}
          if(!copied->rows.empty())selected_owner=controller;
          // A pooled active parent can contain no visible hints. Try the next
          // real published owner instead of treating that empty pool as the UI.
          ++owner_index;next=0;
        }
        if(owner_index<candidates.size())return true;
        void* chosen=selected_owner;
        worker_job::start(g_controls_work,std::move(*copied),[](CapturedControls& input){
            modtext::LocaleScope locale(input.locale);std::vector<std::wstring> entries;
            for(const auto& parts:input.rows)entries.push_back(text::join(parts,L": "));
            return entries.empty()?T(L"No controls displayed"):text::join(entries,L", ");
        },[chosen,generation](std::wstring result){
            if(generation==g_controls_generation&&current_bar(chosen))ffpr::say(result,true);
        });
        return false;
    },"KeyHelp.OwnedCapture",INT_MAX);
}
}
void register_controls_owner(void* controller,bool published){remember(owners,controller,published);}
void retire_controls_owner(void* controller){retire(owners,controller);}
void register_controls_icon(void* controller,bool published){remember(icon_owners,controller,published);}
void retire_controls_icon(void* controller){retire(icon_owners,controller);}
void announce_controls_for(void* controller){capture_controls({{controller,false}},true);}
void announce_current_controls() {
    cancel_controls_reading();const auto generation=g_controls_generation;
    std::vector<Owner> ordered;
    for(const auto& owner:owners)if(owner.controller)ordered.push_back(owner);
    std::sort(ordered.begin(),ordered.end(),[](const Owner& a,const Owner& b){return a.publication>b.publication;});
    struct Registry {std::vector<Candidate> bars,widgets;int stage=0,next=0;};
    auto copied=std::make_shared<Registry>();
    for(const auto& owner:ordered)copied->bars.push_back({owner.controller,false});
    for(const auto& icon:icon_owners)if(icon.controller)copied->widgets.push_back({icon.controller,true});
    // The game's exact registered hint lists cover controls initialized before
    // our hooks attached. This is a bounded on-request read of their owner,
    // never a scene search. All pointers stay on this native-owner coroutine.
    frame::every_frame([copied,generation] {
        if(generation!=g_controls_generation)return false;
        void* manager=ref::instance(kControlsManager);
        if(manager&&!unity::alive(manager))manager=nullptr;
        field::CaptureBudget budget;int checked=0;
        while(manager&&copied->stage<2&&checked<32&&budget.available()) {
            const bool single=copied->stage==1;
            void* list=ref::rd<void*>(manager,single?fManagerIcons:fManagerBars);
            const int count=unity::list_count(list);
            if(count<0||count>(single?4096:1024)){++copied->stage;copied->next=0;continue;}
            auto& target=single?copied->widgets:copied->bars;
            while(copied->next<count&&checked++<32&&budget.available()) {
                void* controller=unity::list_at(list,copied->next++);
                if(!controller||!current_bar(controller))continue;
                if(std::none_of(target.begin(),target.end(),[=](const Candidate& p){return p.controller==controller;}))
                    target.push_back({controller,single});
            }
            if(copied->next<count)return true;
            ++copied->stage;copied->next=0;
        }
        if(manager&&copied->stage<2)return true;
        copied->bars.insert(copied->bars.end(),copied->widgets.begin(),copied->widgets.end());
        capture_controls(std::move(copied->bars),false);return false;
    },"KeyHelp.RegistryCapture",INT_MAX);
}
}
