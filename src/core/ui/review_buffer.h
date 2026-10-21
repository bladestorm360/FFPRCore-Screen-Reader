#pragma once
#include <string>
#include <optional>
#include <vector>
#include <cstdint>
#include "result_notifications.h"

// Current displayed page/phase only. Consumers supply renderer-backed lines.
namespace ui::review {
enum class Domain { Results, Status, Information };
// Diagnostic identity only: these integer tokens must never be dereferenced on
// a worker. The native owner copies at most 128 raw Text.m_Text UTF-16 units
// from a visible name component at capture time; markup remains diagnostic.
struct NameSource {
    int row = 0;
    uintptr_t content = 0, view = 0, name_text = 0;
    std::wstring name;
};
struct Options {
    Domain domain = Domain::Results;
    bool announce = true;
    bool defer_details = false;
    // Essential displayed totals are announced in either Autodetail state and
    // retained as individual review lines. Detailed rows are separate.
    std::vector<std::wstring> summary;
    // Essential rendered result notifications, separate from totals/stat tables.
    // A session opts into per-battle filtering; absent preserves every repeat.
    std::vector<std::wstring> notifications;
    std::shared_ptr<ResultNotificationSession> notification_session;
    bool include_headline = false;
    // Current-phase changes for both automatic summaries and manual I/West.
    // Separate from the full displayed table kept for arrows/Autodetail.
    std::optional<std::vector<std::wstring>> brief;
    bool announce_brief = false;
    bool announce_brief_with_details = false;
    bool brief_details = false; // Brief mode uses gains for Autodetail too.
    int restore_index = 0;
    std::vector<int> groups;
    // Native information panels can have their own independent page toggle.
    // Manual I/West reads that current panel instead of the comparison buffer.
    std::string details_action;
    int producer_phase = -1;
    unsigned producer_generation = 0;
    std::vector<NameSource> source_names;
};
/// Claim input while an event-owned renderer snapshot is being captured.
void begin(void* owner, const std::wstring& headline, Options options = {});
void show(void* owner, const std::wstring& headline, std::vector<std::wstring> lines, Options options = {});
void clear();
void replace_lines(std::vector<std::wstring> lines, int restore_index,
                   std::optional<std::vector<std::wstring>> brief = std::nullopt);
bool active();
bool owns(void* owner);
Domain domain();
int index();
void details();
void next();
void previous();
void first();
void last();
void current();
void next_group();
void previous_group();
}
