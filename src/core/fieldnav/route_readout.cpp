#include "route_readout.h"
#include "../core.h"
#include "../field/baked_nav.h"
#include "../frame.h"
#include "../system/system.h"
#include "../ui/preferences.h"
#include <optional>
#include <utility>

namespace ffpr::fieldnav::route_readout {
namespace {
using ::field::baked_route::Cell;
struct IssuedChunk {
    int map;
    std::uint64_t token;
    std::vector<Cell> cells;
    size_t cursor = 0;
    std::function<void()> next;
};
std::optional<IssuedChunk> issued;
std::uint64_t generation = 0;
bool sampling = false;
constexpr int settle_frames = 24;
}

void cancel()
{
    ++generation;
    issued.reset();
    sampling = false;
}

std::uint64_t begin()
{
    cancel();
    return generation;
}

bool current(std::uint64_t token) { return token == generation; }

int leg_limit()
{
    return ui::prefs::full_route_readout() ? 0 : ui::prefs::route_legs_per_readout();
}

void follow(::field::PathInfo& path, std::uint64_t token, std::function<void()> next)
{
    if (token != generation || !ui::prefs::auto_route_readout() ||
        path.readout_cells.size() < 2 || !next) return;
    issued.emplace(IssuedChunk{::field::current_map_id(), token, std::move(path.readout_cells), 0, std::move(next)});
}

void motion_input()
{
    if (!issued || issued->token != generation || sampling) return;
    sampling = true;
    // Native movement input can return before the destination cell is settled.
    // Watch only a bounded tail of this input burst, while a chunk is armed.
    frame::every_frame([token = generation, budget = settle_frames]() mutable {
        if (token != generation) return false;
        if (!issued || !ffpr::system::is_field_navigation_ready() ||
            menu_state::any_active() || queries::is("ModMenu.IsOpen") ||
            issued->map != ::field::current_map_id()) {
            cancel();
            return false;
        }
        auto& chunk = *issued;
        Cell at;
        if (::field::baked_nav::player_cell(::field::map_handle(), ::field::player(), at)) {
            using ::field::route_progress::ChunkProgress;
            const auto progress = ::field::route_progress::advance_chunk(chunk.cells, chunk.cursor, at);
            if (progress == ChunkProgress::Deviated) {
                cancel();   // deviation requires a new explicit request
                return false;
            }
            if (progress == ChunkProgress::Completed) {
                auto next = std::move(chunk.next);
                cancel();
                next();
                return false;
            }
        }
        if (--budget > 0) return true;
        sampling = false;
        return false;
    }, "RouteReadout.Movement", settle_frames);
}

namespace {
int install()
{
    for (auto event : {events::Kind::MapChanging, events::Kind::BattleEntering,
                       events::Kind::PartyChanging}) {
        // Native transition prefixes invalidate the token only. Release the
        // owned cells after the gate has closed, off the native prefix.
        events::on(event, [] { ++generation; sampling = false; });
    }
    for (auto event : {events::Kind::MapChanged, events::Kind::BattleStart,
                       events::Kind::MenuOpened, events::Kind::TitleScreen})
        events::on(event, [] { cancel(); });
    for (const char* event : {"Settings.Changed.FullRouteReadout", "Settings.Changed.RouteLegsPerReadout",
                              "Settings.Changed.AutoRouteReadout"})
        actions::define(event, [] { cancel(); });
    return 0;
}
core::Install install_readout{"RouteReadout", &install, 135};
}
}
