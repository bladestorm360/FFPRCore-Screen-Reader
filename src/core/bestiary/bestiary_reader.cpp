// Ports FF1 Menus/BestiaryReader.cs: bestiary (Picture Book) data formatted for speech. Only
// what the game renders, including the native reveal-gated item rows. Shared master
// slots must never bypass the bestiary's secret placeholders or displayed item volume.
#include "bestiary.h"
#include "../game.h"
#include "../log.h"
#include "../modtext.h"
#include "../refs.h"
#include "../text.h"
#include "../unity.h"
#include "../status/status.h"
#include "../displayed_ui.h"
#include <algorithm>

namespace ffpr::bestiary {
namespace {

ref::Class  kPictureBookData{"Last.Data.PictureBooks", "PictureBookData"};
ref::Method mPbNo{kPictureBookData, "get_No", 0};
ref::Method mPbIsRelease{kPictureBookData, "get_IsRelease", 0};
ref::Method mPbMonsterName{kPictureBookData, "get_MonsterName", 0};

ref::Class  kMonsterData{"Last.OutGame.Library", "MonsterData"};
ref::Method mMonsterHabitatNames{kMonsterData, "get_HabitatNameList", 0};
ref::Method mMonsterMaster{kMonsterData, "get_MonsterMaster", 0};

ref::Class  kInfoContent{"Last.UI.Common.Library", "LibraryInfoContent"};
ref::Field  fMonsterDataTable{kInfoContent, "monsterDataTable"};
ref::Field  fStatusTable{kInfoContent, "statusTable"};
ref::Field  fOptionTable{kInfoContent, "optionTable"};
ref::Field  fHierarchyTable{kInfoContent, "hierarchyTable"};
ref::Field  fDropTable{kInfoContent, "dropTable"};

ref::Class  kParamValue{"Last.UI.Common.Library", "LibraryInfoParamValue"};
ref::Field  fParamText{kParamValue, "paramText"};
ref::Field  fValueText{kParamValue, "valueText"};
ref::Field  fMultipliedValueText{kParamValue, "multipliedValueText"};
ref::Field  fParameterValueText{kParamValue, "parameterValueText"};
ref::Field  fPercentText{kParamValue, "percentText"};
ref::Field  fPersentValueText{kParamValue, "persentValueText"};
ref::Field  fDefaultValueText{kParamValue, "defaultValueText"};
ref::Field  fMultipliedText{kParamValue, "multipliedText"};

ref::Class  kParamListView{"Last.UI.Common.Library", "LibraryInfoContentParamListView"};
ref::Field  fListTitleText{kParamListView, "titleText"};
ref::Field  fListTextTable{kParamListView, "textTable"};
ref::Class  kParamTextView{"Last.UI.Common.Library", "LibraryInfoParamText"};
ref::Field  fDetailText{kParamTextView, "detailText"};

ref::Class  kMonsterParty{"Last.Data.Master", "MonsterParty"};
ref::Method mParty1{kMonsterParty, "get_Monster1", 0};
ref::Method mParty2{kMonsterParty, "get_Monster2", 0};
ref::Method mParty3{kMonsterParty, "get_Monster3", 0};
ref::Method mParty4{kMonsterParty, "get_Monster4", 0};
ref::Method mParty5{kMonsterParty, "get_Monster5", 0};
ref::Method mParty6{kMonsterParty, "get_Monster6", 0};
ref::Method mParty7{kMonsterParty, "get_Monster7", 0};
ref::Method mParty8{kMonsterParty, "get_Monster8", 0};
ref::Method mParty9{kMonsterParty, "get_Monster9", 0};

ref::Class  kMonster{"Last.Data.Master", "Monster"};
ref::Method mMonsterMesIdName{kMonster, "get_MesIdName", 0};
// C# Text.text?.Trim() of a Text field, empty when missing.
std::wstring trimmed_text(void* owner, ref::Field& f)
{
    void* t = owner ? ref::rd<void*>(owner, f) : nullptr;
    return displayed::text(t);
}

// GetParamValueLabel: paramText.text, trimmed.
std::wstring param_value_label(void* param)
{
    void* t = ref::rd<void*>(param, fParamText);
    if (!t) return {};
    std::wstring s = displayed::text(t);
    return s.empty() ? std::wstring() : text::trim(s);
}

// GetParamValueText: the first non-empty of the value text fields, trimmed.
std::wstring param_value_text(void* param)
{
    ref::Field* fields[] = {&fValueText, &fMultipliedValueText, &fParameterValueText, &fPercentText,
                            &fPersentValueText, &fDefaultValueText, &fMultipliedText};
    for (ref::Field* f : fields) {
        void* t = ref::rd<void*>(param, *f);
        if (!t) continue;
        std::wstring s = displayed::text(t);
        if (!s.empty()) return text::trim(s);
    }
    return {};
}

// GetGameMessage: MessageManager.GetMessage(id) (isReplace true), empty when missing.
std::wstring game_message(const std::wstring& mes_id)
{
    if (mes_id.empty()) return {};
    return ffpr::status::game_message(mes_id, true);
}

std::wstring monster_fallback(int monster_id) { return TF(L"Monster {0}", {text::num(monster_id)}); }

// ResolveMonsterName: the monster master's name through the game's localization.
std::wstring resolve_monster_name(int monster_id)
{
    void* monster = ffpr::status::master_entry(kMonster, monster_id);
    if (!monster) return monster_fallback(monster_id);
    std::wstring mes_id = ref::call_str(mMonsterMesIdName, monster);
    if (mes_id.empty()) return monster_fallback(monster_id);
    std::wstring name = game_message(mes_id);
    return name.empty() ? monster_fallback(monster_id) : name;
}

} // namespace

std::wstring read_list_entry(void* data)
{
    if (!data) return {};
    // No.ToString("D3")
    int no = ref::call<int32_t>(mPbNo, data);
    std::wstring digits = text::num(no < 0 ? -static_cast<long long>(no) : no);
    while (digits.size() < 3) digits.insert(digits.begin(), L'0');
    std::wstring number = (no < 0 ? L"-" : L"") + digits;
    std::wstring name = ref::call<bool>(mPbIsRelease, data) ? ref::call_str(mPbMonsterName, data) : std::wstring(L"???");
    return number + L": " + name;
}

std::wstring read_encounter_summary(void* list)
{
    if (!list) return {};
    int total = unity::list_count(list);
    int encountered = 0;
    for (int i = 0; i < total; ++i) {
        void* item = unity::list_at(list, i);
        if (item && ref::call<bool>(mPbIsRelease, item)) ++encountered;
    }
    return TF(L"Bestiary. Encountered: {0} of {1}", {text::num(encountered), text::num(total)});
}

bool capture_stat_step(StatCapture& capture)
{
    // A library has five short serialized tables. Reject corrupt native counts
    // before indexing; copy at most one row/leaf field on each owner-thread step.
    constexpr int kMaxRows = 64, kMaxDetails = 64;
    constexpr size_t kMaxCharacters=65536, kMaxField=4096;
    if (!unity::alive(capture.content) || capture.table >= 5 || capture.characters>=kMaxCharacters) return true;
    const auto retain=[&](std::wstring value) {
        value.resize(std::min({value.size(),kMaxField,kMaxCharacters-capture.characters}));
        capture.characters+=value.size();return value;
    };
    ref::Field* tables[] = {&fMonsterDataTable, &fStatusTable, &fOptionTable, &fDropTable, &fHierarchyTable};
    constexpr StatGroup groups[] = {StatGroup::MonsterData, StatGroup::Status, StatGroup::Options, StatGroup::Items, StatGroup::Properties};
    void* table = ref::rd<void*>(capture.content, *tables[capture.table]);
    const int count = unity::array_length(table);
    if (count < 0 || count > kMaxRows || capture.row >= count) {
        ++capture.table; capture.row = 0; capture.detail = -1;
        return capture.table >= 5;
    }
    void* row = unity::array_at(table, capture.row);
    if (!row || !unity::active_in_hierarchy(row)) {
        ++capture.row; capture.detail = -1; return false;
    }
    if (capture.table < 3) {
        auto label = retain(param_value_label(row));
        if (!label.empty()) capture.entries.push_back({std::move(label), retain(param_value_text(row)), groups[capture.table]});
        ++capture.row; return false;
    }
    if (capture.detail < 0) {
        capture.current = {retain(trimmed_text(row, fListTitleText)), {}, groups[capture.table]};
        capture.detail = 0;
    }
    void* details = ref::rd<void*>(row, fListTextTable);
    const int detail_count = unity::array_length(details);
    if (detail_count < 0 || detail_count > kMaxDetails || capture.detail >= detail_count || capture.current.label.empty()) {
        if (!capture.current.label.empty()) capture.entries.push_back(std::move(capture.current));
        ++capture.row; capture.detail = -1; return false;
    }
    void* leaf = unity::array_at(details, capture.detail++);
    if (leaf && unity::active_in_hierarchy(leaf)) {
        // Item drops render their name in IconTextView, while Gil and some
        // property rows render detailText. Read each active native renderer;
        // neither item IDs nor unrevealed master drop slots are a fallback.
        void* icon=displayed::field(leaf,"iconView");
        auto name=retain(displayed::text_field(icon,"nameText"));
        auto value = retain(trimmed_text(leaf, fDetailText));
        if(!name.empty())capture.current.parts.push_back(std::move(name));
        if (!value.empty()) capture.current.parts.push_back(std::move(value));
    }
    return false;
}

std::wstring read_formation(int formation_index, void* party)
{
    if (!party) return {};
    try {
        ref::Method* slots[] = {&mParty1, &mParty2, &mParty3, &mParty4, &mParty5, &mParty6, &mParty7, &mParty8, &mParty9};
        std::wstring names;
        for (ref::Method* m : slots) {
            int id = ref::call<int32_t>(*m, party);
            if (id <= 0) continue;
            std::wstring name = resolve_monster_name(id);
            if (name.empty()) continue;
            if (!names.empty()) names += L", ";
            names += name;
        }
        if (names.empty()) return TF(L"Formation {0}: {1}", {text::num(formation_index + 1), T(L"Empty")});
        return TF(L"Formation {0}: {1}", {text::num(formation_index + 1), names});
    } catch (...) {
        LOGF("[Bestiary] Error reading formation");
        return TF(L"Formation {0}: {1}", {text::num(formation_index + 1), T(L"Error")});
    }
}

std::wstring read_map_name(void* data, int map_index)
{
    if (!data) return {};
    try {
        void* habitat_names = ref::call<void*>(mMonsterHabitatNames, data);
        int count = unity::list_count(habitat_names);
        if (!habitat_names || count == 0) return T(L"No habitat data");
        if (map_index < 0 || map_index >= count) map_index = 0;
        std::wstring name = ref::str(unity::list_at(habitat_names, map_index));
        return !name.empty() ? name : T(L"Unknown location");
    } catch (...) {
        LOGF("[Bestiary] Error reading map name");
        return T(L"Error reading habitat");
    }
}

} // namespace ffpr::bestiary
