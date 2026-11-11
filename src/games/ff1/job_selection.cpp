// Ports FF1 Patches/JobSelectionPatches.cs.
//
// Class choice on FF1's new-game screen (Last.UI.KeyInput.JobContentListController): the job
// name, then one frame later its description (when Auto Detail is on) and the position.
// The on-demand detail key reads the description through "JobSelectionPatches.GetCurrentDescription".
//
// Native contentList rows own their localized jobNameText; discriptionText
// (the game's spelling) owns the separate current description. Never search
// unrelated child texts or guess a description from its length. Exact producer,
// field and lifetime evidence: docs/re/FF1_CLASS_DESCRIPTIONS_20261101.md.
//
// JobContentListController is a shared Last.UI.KeyInput class (all five dumps); masked to FF1
// because only FF1's module hooks it here.
#include "../../core/ui/preferences.h"
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include "../../core/core.h"
#include "../../core/displayed_ui.h"
#include "../../core/frame.h"
#include "../../core/hooks.h"
#include "../../core/log.h"
#include "../../core/refs.h"
#include "../../core/say.h"
#include "../../core/settings.h"
#include "../../core/text.h"
#include "../../core/ui_util.h"
#include "../../core/unity.h"
#include "../../core/field/scene_events.h"

namespace ffpr::ff1 {
namespace {

ref::Class kJobList{"Last.UI.KeyInput", "JobContentListController", game::kFF1};
ref::Method mJobSelectContent{kJobList, "SelectContent", 1};
ref::Field fContents{kJobList,"contentList"};
ref::Field fDescriptionParent{kJobList,"discriptionParent"};
ref::Class kJobContent{"Last.UI.KeyInput","JobContentController",game::kFF1};
ref::Field fContentView{kJobContent,"view"};

// IsHandlingCursor: set by SelectContent, cleared by the cursor router or ResetState.
bool g_handling_cursor = false;
// _lastController: the live job list, for the on-demand detail key. Re-checked on every use.
void* g_controller = nullptr;
uint64_t g_generation=0;
int g_index=-1;

bool controller_alive(void* ctrl)
{
    return ctrl && kJobList.is(ctrl) && unity::alive(ctrl);
}

/// Exactly one selected native row, in any game-supported UI language.
std::wstring selected_name(void* list_ctrl, int index, int& job_count)
{
    job_count = -1;
    if(!controller_alive(list_ctrl)||!unity::active_in_hierarchy(list_ctrl))return {};
    void* list=ref::rd<void*>(list_ctrl,fContents);const int count=unity::list_count(list);
    if(count<1||count>12||index<0||index>=count)return {};
    void* row=unity::list_at(list,index);
    if(!kJobContent.is(row)||!unity::alive(row))return {};
    void* view=ref::rd<void*>(row,fContentView);if(!view)return {};
    const auto name=displayed::text_field(view,"jobNameText");
    if(!name.empty())job_count=count;return name;
}

/// SetDiscriptionText writes this exact native renderer. Its parent can be
/// outside the row hierarchy; hidden/disabled description panels stay absent.
std::wstring current_panel_description(void* list_ctrl)
{
    if(!controller_alive(list_ctrl)||!unity::active_in_hierarchy(list_ctrl))return {};
    void* panel=ref::rd<void*>(list_ctrl,fDescriptionParent);
    if(!displayed::container_visible(panel))return {};
    return displayed::text_field(list_ctrl,"discriptionText");
}

std::wstring safe_description(void* list_ctrl)
{
    return hooks::guarded_r("JobSelection.Description", std::wstring(), [&] { return current_panel_description(list_ctrl); });
}

/// AnnounceJobWithDelayedDescription, one frame after SelectContent.
void announce_job(void* ctrl, const std::wstring& job_name, int index, int count)
{
    std::wstring announcement = job_name;
    // The description only with Auto Detail on; otherwise the detail key reads it on demand.
    if (ui::prefs::auto_detail_enabled()) {
        std::wstring description = safe_description(ctrl);
        if (!description.empty()) announcement = job_name + L". " + description;
    }
    // Position last, after any description.
    ffpr::say(ui::menu_position(announcement, index, count));
}

void on_select_content(void* self, void* cursor)
{
    const auto generation=++g_generation;
    g_controller=nullptr;g_index=-1;g_handling_cursor=false;
    if (!cursor || !self) return;
    int index = ui::cursor_index(cursor);
    int job_count = -1;
    std::wstring job_name = selected_name(self, index, job_count);
    if (job_name.empty()) return;
    g_controller=self;g_index=index;g_handling_cursor=true;
    frame::next_frame([self, job_name, index, job_count,generation] {
        if(generation!=g_generation||g_controller!=self||g_index!=index||
           !controller_alive(self)||!unity::active_in_hierarchy(self))return;
        announce_job(self,job_name,index,job_count);
    },
                      "JobSelection.Announce");
}

bool is_active()
{
    void* ctrl = g_controller;
    return controller_alive(ctrl) && unity::active_in_hierarchy(ctrl);
}

std::wstring current_description()
{
    void* ctrl = g_controller;
    return is_active() ? safe_description(ctrl) : std::wstring();
}

void reset_state()
{
    g_handling_cursor = false;
    g_controller = nullptr;
    g_index=-1;++g_generation;
}

using VoidPtr_t = void (*)(void* self, void* arg, const void* mi);
VoidPtr_t o_JobSelectContent;

void d_JobSelectContent(void* self, void* cursor, const void* mi)
{
    o_JobSelectContent(self, cursor, mi);
    hooks::guarded("JobSelection.SelectContent", [&] { on_select_content(self, cursor); });
}

int install()
{
    int n = hooks::attach(mJobSelectContent, &d_JobSelectContent, &o_JobSelectContent) ? 1 : 0;
    if (!n) LOGF("FF1 JobSelection: JobContentListController.SelectContent not hooked - job selection disabled");

    queries::define("JobSelectionPatches.IsActive", [] { return is_active(); });
    queries::define("JobSelectionPatches.IsHandlingCursor", [] { return g_handling_cursor; });
    queries::define_text("JobSelectionPatches.GetCurrentDescription", [] { return current_description(); });
    actions::define("JobSelectionPatches.ClearHandlingFlag", [] { g_handling_cursor = false; });
    actions::define("JobSelectionPatches.ResetState", [] { reset_state(); });
    // Title-owned controllers must retire before later field/scene input can
    // consult them; teardown touches owned state only, never the old pointer.
    for(auto event:{events::Kind::MapChanging,events::Kind::TitleScreen,events::Kind::SceneLoaded})
        events::on(event,[]{reset_state();});
    return n;
}

core::Install s_install{"FF1.JobSelection", &install, 501, game::kFF1};

} // namespace
} // namespace ffpr::ff1
