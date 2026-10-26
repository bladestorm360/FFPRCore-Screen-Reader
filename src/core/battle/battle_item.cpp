// Native battle item panels. Read the rendered row, independent of command order.
// Retains FF6 Items/Tools behavior ported from BlindGuyNW/FF6ScreenReader
// (ZKline), with the inclusion permission recorded in the project.
#include "battle_internal.h"
#include "battle_util.h"
#include "../core.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../text.h"
#include "../ui_util.h"
#include "../unity.h"
#include "../displayed_ui.h"
#include "../ui/preferences.h"

namespace ffpr::battle {
namespace {
ref::Class kItemCtl{"Last.UI.KeyInput", "BattleItemInfomationController"};
ref::Class kItemContent{"Last.UI.KeyInput", "BattleItemInfomationContentController"};
ref::Method mSelectContent{kItemCtl, "SelectContent", 2};
ref::Method mSelectEquip{kItemCtl, "SelectEquipContent", 3};
ref::Method mUseEquipInit{kItemCtl, "UseEquipSelectInit", 0};
ref::Method mUseInit{kItemCtl, "UseSelectInit", 0}, mClose{kItemCtl, "Close", 0};
// Tab's hand-slot panel is a different native owner from the equipped strip
// above Items. Its spelling is native and shared by all six games.
ref::Class kGearCtl{"Last.UI.KeyInput", "BattleEquipInfomationConroller"};
ref::Method mGearSelect{kGearCtl,"SelectContent",2},mGearInit{kGearCtl,"UseSelectInit",0},mGearClose{kGearCtl,"Close",0};
ref::Class kItem6{"Last.UI.KeyInput", "BattleItemInfomationController", game::kFF6};
ref::Field fMachine{kItem6, "isMachineState"};
void* g_item_owner = nullptr;
uint64_t g_focus_sequence = 0;

bool panel_ready(void* self) {
    return unity::alive(self) && displayed::container_visible(
        displayed::field(displayed::field(self, "view"), "rootObject"));
}
std::wstring item_announcement(void* self, void* cursor, int index, int& count) {
    count = -1;
    if (!panel_ready(self)) return {};
    const bool tools = game::is(game::Id::FF6) && ref::rd<bool>(self, fMachine);
    void* list = displayed::field(self, tools ? "machineContentList" : "contentList");
    const int slots = unity::list_count(list);
    if (slots < 0 || slots > 128) return {};
    void* content = unity::get_component_in_parent(cursor, kItemContent);
    if (!content && index >= 0 && index < slots) content = unity::list_at(list, index);
    void* view = displayed::field(content, "view");
    auto name = displayed::text_field(displayed::field(view, "IconTextView"), "nameText");
    if (name.empty()) name = displayed::text_field(displayed::field(view, "nonItemTextView"), "nameText");
    if (name.empty()) name = displayed::text_field(displayed::field(self, "view"), "noItemText");
    if (name.empty()) return {};
    const auto quantity = displayed::text_field(view, "countText");
    if (!quantity.empty()) name += L", " + quantity;
    count = tools ? slots : unity::list_count(displayed::field(self, "displayDataList"));
    return name;
}
// The equipped strip is a separate native selection phase above Items. Its
// cursor can still be parented to an ordinary item, so use the explicit native
// equipment index and the rendered equipment pool, never that stale parent.
std::wstring equipment_announcement(void* self, int index, int& count) {
    count = -1;
    if (!panel_ready(self) || !displayed::container_visible(
        displayed::field(displayed::field(self, "view"), "equipParent"))) return {};
    count = unity::list_count(displayed::field(self, "equipList"));
    void* rows = displayed::field(self, "equipContnetList"); // Native spelling.
    const int slots = unity::list_count(rows);
    if (count < 1 || count > 16 || slots < count || slots > 16 || index < 0 || index >= count) return {};
    void* view = displayed::field(unity::list_at(rows, index), "view");
    auto name = displayed::text_field(displayed::field(view, "iconTextView"), "nameText");
    if (name.empty()) return {};
    const auto quantity = displayed::text_field(view, "countText");
    if (!quantity.empty()) name += L", " + quantity;
    if (displayed::image_visible(displayed::field(view, "equipedIcon"))) name += L", " + T(L"Equipped");
    return name;
}
std::wstring gear_announcement(void* self,int index,int& count) {
    count=-1;
    if(!panel_ready(self))return {};
    void* list=displayed::field(self,"contentList");
    count=unity::list_count(list);
    if(count<1||count>16||index<0||index>=count)return {};
    void* view=displayed::field(unity::list_at(list,index),"view");
    if(!displayed::container_visible(displayed::field(view,"rootObject")))return {};
    // This stripped battle view has no quantity or stat comparison fields.
    // Read the exact visible hand/favored-hand labels and equipped name.
    auto label=text::join({displayed::text_field(view,"partsText"),
                           displayed::text_field(view,"dominantText")},L" ");
    auto name=displayed::text_field(displayed::field(view,"iconTextView"),"nameText");
    if(name.empty())return label;
    return label.empty()?name:label+L": "+name;
}
void announce_line(void* self, std::wstring line, int index, int count) {
    if (line.empty()) return;
    ++g_focus_sequence;
    g_item_owner = self;
    clear_other_menu_states(menu_state::BATTLE_ITEM);
    state_set(menu_state::BATTLE_ITEM, true);
    set_target_selection_active(false);
    notify_command_submenu_active();
    const auto description = displayed::text_field(displayed::field(self, "view"), "descriptionText");
    if (ui::prefs::auto_detail_enabled() && !description.empty()) line += L": " + description;
    ffpr::say(ui::menu_position(line, index, count), true);
}
void announce(void* self, void* cursor) {
    if (!cursor) return;
    const int index = ui::cursor_index(cursor);
    int count = -1;
    auto line = item_announcement(self, cursor, index, count);
    announce_line(self, std::move(line), index, count);
}
void announce_equipment(void* self, int index) {
    int count = -1;
    auto line = equipment_announcement(self, index, count);
    announce_line(self, std::move(line), index, count);
}
void announce_current_description() {
    const auto description = panel_ready(g_item_owner) ?
        displayed::text_field(displayed::field(g_item_owner, "view"), "descriptionText") : std::wstring();
    ffpr::say(description.empty() ? T(L"No description") : description, true);
}
using Select = void (*)(void*, void*, int32_t, const void*);
Select o_SelectContent;
void d_SelectContent(void* self, void* cursor, int32_t type, const void* mi) {
    o_SelectContent(self, cursor, type, mi);
    hooks::guarded("BattleItem.SelectContent", [&] { announce(self, cursor); });
}
using EquipSelect = void (*)(void*, void*, int32_t, int32_t, const void*);
EquipSelect o_select_equip;
void d_select_equip(void* self, void* cursor, int32_t index, int32_t type, const void* mi) {
    o_select_equip(self, cursor, index, type, mi);
    hooks::guarded("BattleItem.SelectEquipContent", [&] { announce_equipment(self, index); });
}
using Phase = void (*)(void*, const void*);
Phase o_use_init, o_use_equip_init, o_close;
using GearSelect=void(*)(void*,void*,int32_t,const void*);
GearSelect o_gear_select;
Phase o_gear_init,o_gear_close;
void d_gear_select(void* self,void* cursor,int32_t index,const void* mi) {
    o_gear_select(self,cursor,index,mi);
    hooks::guarded("BattleEquipment.SelectSlot",[&]{
        int count=-1;auto line=gear_announcement(self,index,count);
        announce_line(self,std::move(line),index,count);
    });
}
void d_gear_init(void* self,const void* mi) {
    const auto before=g_focus_sequence;
    o_gear_init(self,mi);
    hooks::guarded("BattleEquipment.Entry",[=]{
        if(before==g_focus_sequence) {
            const int index=ui::cursor_index(displayed::field(self,"selectCursor"));
            int count=-1;auto line=gear_announcement(self,index,count);
            announce_line(self,std::move(line),index,count);
        }
    });
}
void d_gear_close(void* self,const void* mi) {
    o_gear_close(self,mi);
    hooks::guarded("BattleEquipment.Close",[=]{
        if(g_item_owner==self){g_item_owner=nullptr;state_set(menu_state::BATTLE_ITEM,false);}
    });
}
void d_use_equip_init(void* self, const void* mi) {
    const auto before = g_focus_sequence;
    o_use_equip_init(self, mi);
    hooks::guarded("BattleItem.UseEquipSelectInit", [self, before] {
        // Some native entries select before enabling the rendered strip. The
        // operation sequence guards this entry callback only, never text.
        if (before == g_focus_sequence)
            announce_equipment(self, ui::cursor_index(displayed::field(self, "selectCursor")));
    });
}
void d_use_init(void* self, const void* mi) {
    const auto before = g_focus_sequence;
    o_use_init(self, mi);
    hooks::guarded("BattleItem.UseSelectInit", [self, before] {
        // Even an empty Items panel owns the cancel-return event.
        if (panel_ready(self)) {
            g_item_owner = self; notify_command_submenu_active();
            // Native entry selects before enabling its root in some games.
            // Read that completed entry once if its nested focus was unready.
            if (before == g_focus_sequence) announce(self, displayed::field(self, "selectCursor"));
        }
    });
}
void d_close(void* self, const void* mi) {
    o_close(self, mi);
    hooks::guarded("BattleItem.Close", [self] {
        if (g_item_owner == self) { g_item_owner = nullptr; state_set(menu_state::BATTLE_ITEM, false); }
    });
}
int install() {
    actions::define("BattleItemPatches.AnnounceCurrentDescription", [] { announce_current_description(); });
    return (hooks::attach(mSelectContent, &d_SelectContent, &o_SelectContent) ? 1 : 0) +
        (hooks::attach(mSelectEquip, &d_select_equip, &o_select_equip) ? 1 : 0) +
        (hooks::attach(mUseEquipInit, &d_use_equip_init, &o_use_equip_init) ? 1 : 0) +
        (hooks::attach(mUseInit, &d_use_init, &o_use_init) ? 1 : 0) +
        (hooks::attach(mClose, &d_close, &o_close) ? 1 : 0) +
        (hooks::attach(mGearSelect,&d_gear_select,&o_gear_select)?1:0) +
        (hooks::attach(mGearInit,&d_gear_init,&o_gear_init)?1:0) +
        (hooks::attach(mGearClose,&d_gear_close,&o_gear_close)?1:0);
}
core::Install s_install{"Battle.Item", &install, 110};
}
}
