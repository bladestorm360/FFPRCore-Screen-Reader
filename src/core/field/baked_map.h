#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

// The baked maps: every sub-map's collision, hidden-passage, terrain-attribute and
// per-vehicle-group grids, its routing objects, and the transportation masters, extracted
// offline from the game's Addressables (tools/dump_map_objects.py) and packed by
// tools/bake_maps.py into data/<game>/maps.bin (embedded as MAPS_FFN). The format is
// documented in bake_maps.py; this reader and that writer change together.
//
// Pure data: no game pointers, no IL2CPP. Usable in offline checks (tools/route_check.cpp).
// Prepared once on the exclusive data worker, then published as immutable data to
// compute workers; offline checks may load from a file without any engine access.
namespace field::baked {

// A source-pinned looping NPC visits this manual crossing pose repeatedly.
// It is a waiting interaction, never an automatic move or an all-layer actor.
inline constexpr int kRecurringNpcSource = 1 << 20;
// Source-qualified Trench passages keep their native timed UI player-operated.
inline constexpr int kSerpentRightJunction = 1 << 21;
// FFVI's exact Blackjack flight control resumes at the saved craft position.
inline constexpr int kBlackjackSavedFlight = 1 << 23;
// Exact permanent currents and post-Sandworm automatic script crossings.
inline constexpr int kAutomaticCurrent = 1 << 24;
// A no-return story replacement temporarily prevents this exact floor exit.
inline constexpr int kConditionalSceneExit = 1 << 25;
// The native step-on rope script supplies the same-floor player landing.
inline constexpr int kRopeCrossing = 1 << 27;
inline constexpr int kSerpentDiveChoice = 1 << 22;

/// MapConstants.ObjectType values that are baked (bake_maps.py BAKED_TYPES).
enum class ObjType : uint8_t {
    PointIn = 0, Event = 2, GotoMap = 3, ToLayer = 4, Entity = 5, NPC = 6, TreasureBox = 7,
    OpenTrigger = 8, SavePoint = 9, AnimEntity = 10,
    EffectEntity = 11, // native collision-only effect; never a destination
    MoveArea = 13, RelayInteraction = 16,
    ShopNPC = 17, CollisionEntity = 19, TelepoPoint = 20, TransportationEventAction = 21,
    IgnoreRoute = 22, NonEncountArea = 23, RandomEvent = 28, SwitchEvent = 30,
    /// Baked only: an event whose script changes map (ChangeMap in its Main segment): a warp
    /// pad, a trapdoor. a = destination map id, b = destination PointIn point id, d = 1 when
    /// stepping on it fires (an Event), 0 when it is examined (an Entity). Low byte 2 is
    /// reviewed manual travel; bits 8..11 optionally restrict native approach sides
    /// for manual travel or the final edge into a reviewed automatic passage;
    /// bit16 identifies ordinary scripted world travel separately from scenes.
    /// bit19 identifies a reviewed random outcome; routing reaches the pad and
    /// requests a fresh observation after the native RNG chooses its arrival.
    /// A cutscene whose
    /// destination script changes map straight back (FF1's bridge credits) is not one
    /// (bake_maps.py round_trip, 2026-10-11).
    Warp = 100,
    // Reviewed companion of a collocated native GotoMap. Identity is retained
    // for live suppression, but the real GotoMap alone owns route/exit semantics.
    TransitionHelper = 101,
    // Original mod geometry, never a native actor or destination. Four bounded
    // points describe a reviewed passage between pit centers. a/b are twice
    // tile-center coordinates, c passage identity, d ordered point index.
    PrecisionPoint = 102,
    // Owned rectangle metadata,not a native actor or destination. a contains
    // native Up/Right/Down/Left bits;d retains the source area ID for evidence.
    CurrentArea = 103,
};

/// One baked routing object (34 bytes in the file). Fields a-d per type: see bake_maps.py.
struct Object {
    ObjType type;
    uint8_t layer;      // 0 Bottom, 1 Ground, 2 Upper
    int16_t tx, ty;     // tile position (top-left for rects)
    uint16_t tw, th;    // size in tiles (1x1 for points)
    int32_t a, b, c, d;
    uint32_t name;      // string index
    uint32_t group;     // string index: "entity_default" or the conditional "ev_e_NNNN"

    int entity_id() const { return c; }
    bool contains(int x, int y) const { return x >= tx && y >= ty && x < tx + tw && y < ty + th; }
    /// TreasureBox: its content id (bake_maps.py packs it above the flag bits of d), 0 = none.
    int chest_content_id() const { return type == ObjType::TreasureBox ? (d >> 8) : 0; }
};

// High Warp flags describe navigation semantics independently of activation's
// low byte. The legacy WorldExit name also covers reviewed scripted entrances
// and passages: their actual destination is named, and they are not scenes.
constexpr int kWorldExit = 1 << 16;
inline bool world_exit(const Object& object) {return object.type==ObjType::Warp && (object.d&kWorldExit)!=0;}
inline bool pickup_treasure(const Object& object) {return object.type==ObjType::TreasureBox && (object.d&32)!=0;}
inline bool stand_on_facing_north(const Object& object) { return object.type == ObjType::Entity && (object.d & 64) != 0; }
// Reviewed scripted sea scenes disembark after entry, unlike native town doors.
// This flag permits Ship entry only; terrain, route and story state still apply.
constexpr int kShipEntry = 1 << 17;
inline bool ship_entry(const Object& object) {return object.type==ObjType::Warp && (object.d&255)==1 && (object.d&kShipEntry)!=0;}
// Reviewed underwater entrances disembark inside their transfer script. Only
// submarine movement may enter them aboard; surface towns still need landing.
constexpr int kSubmarineEntry=1<<18;
inline bool submarine_entry(const Object& object) {return object.type==ObjType::Warp&&(object.d&255)==1&&(object.d&kSubmarineEntry)!=0;}
// Terminal entry is independent of optional story-event warnings. Bit1 is a
// native door (world vehicles fly/sail over it); bit2 is a step-on script warp.
using TransitionGrid = std::array<std::vector<uint8_t>,3>;
inline int transition_bits(const Object& object) {
    if(object.type==ObjType::GotoMap && object.a>0)return 1;
    return object.type==ObjType::Warp && (object.d&255)==1 ? 2 : 0;
}
struct TransportRow { int id, group, landing_group, type; uint32_t message; };

struct Submap {
    uint32_t name;          // string index ("Map_20011_1")
    int32_t first_map_id;   // the lowest map master id using it (-1 when none)
    uint16_t width, height;
    bool world;             // a world map: no collision, attribute + vehicle group grids
    uint8_t layer_mask;     // collision layers present (bit 0 Bottom, 1 Ground, 2 Upper)
    uint8_t transport_layers;
    uint32_t collision_off[3];
    uint32_t passage_off[3];
    uint32_t attribute_off;
    uint32_t transport_off;
    uint32_t objects_off;
    uint32_t object_count;
    uint8_t loop_axes = 0;  // native LoopType: bit0 horizontal, bit1 vertical
};

// Old world bakes omit axis metadata and retain both native wrap axes. Indoor
// loops keep collision/layers; they never become terrain/vehicle world maps.
inline int wrap_width(const Submap& s) { return (s.loop_axes&1)||(s.world&&!s.loop_axes)?s.width:0; }
inline int wrap_height(const Submap& s) { return (s.loop_axes&2)||(s.world&&!s.loop_axes)?s.height:0; }

/// Decoded grids of one sub-map, built on first use and kept (a few hundred KB at most).
struct Grids {
    int width = 0, height = 0;
    std::vector<uint8_t> collision[3];   // native allowed-direction masks; 0 blocked, empty absent
    std::vector<uint8_t> passage[3];     // 1 = hidden-passage cell
    std::vector<uint8_t> attribute;      // world maps: terrain attribute id per cell
    std::vector<std::vector<uint8_t>> transport;   // [group - 1]: 1 = blocked for the group
    std::vector<Object> objects;
    // Prepared once with the immutable bake on Data; preserve original order
    // for duplicate/zero IDs, nearest-cell ties and conditional companions.
    std::unordered_map<int,std::vector<size_t>> objects_by_entity;
    bool entity_index_prepared=false;
    std::unordered_map<int,std::vector<size_t>> world_exits; // prepared on Data, read-only on native/Navigation owners
    std::unordered_map<int,std::vector<size_t>> transition_helpers;
    std::unordered_map<int,std::vector<size_t>> vehicle_pads; // world cell -> prepared object indices
    std::vector<uint8_t> hazard;         // 1 = a step-on warp / trapdoor cell: never crossed by
                                         // an ordinary route, only entered as its goal
    std::vector<uint8_t> traps[3];       // identified pitfalls, retaining their native layer
    // Allocated only on source-qualified current floors. Low four bits are
    // forced outgoing direction;bit16 marks an automatic scripted crossing.
    std::vector<uint8_t> currents[3];
    TransitionGrid transitions;        // prepared immutable terminal-entry lookup
};
inline int current_bits(const Grids& grid,int layer,int x,int y) {
    if(layer<0||layer>2||x<0||y<0||x>=grid.width||y>=grid.height||grid.currents[layer].empty())return 0;
    return grid.currents[layer][static_cast<size_t>(y)*grid.width+x];
}

/// Bind the baked data (the bytes must outlive the process; a resource does). False when the
/// header is not FFPRMAP1 / the sizes do not fit.
bool load(const void* data, size_t size);
bool loaded();
/// Native object identifiers for an environmental pitfall; no dialogue/outcome inference.
bool is_trap(const Object& object);
std::wstring object_identifier(const Object& object); // original developer name, without baked transport prefix
/// The game byte of the file (1-6), 0 when not loaded.
int game_number();

const Submap* submap_by_map_id(int map_id);
const Submap* submap_by_name(const std::wstring& name);
const Submap* submap_at(int index);
int submap_count();
int submap_index(const Submap* s);
std::wstring name_of(const Submap& s);
std::wstring str(uint32_t index);

/// Decoded grids (cached per sub-map).
const Grids& grids(const Submap& s);

/// Cell tests. A layer absent from the sub-map is closed.
bool in_bounds(const Grids& g, int x, int y);
// Exact native identity/footprint lookup in the prepared per-ID exit index. No
// decoding, script execution, engine access or map-wide scan at a call site.
const Object* world_exit_at(const Grids& grid,int id,const std::wstring& raw,int x,int y);
bool transition_helper_at(const Grids& grid,int id,const std::wstring& raw,int x,int y);
/// Native vehicle-action pad's target model IDs (empty list permits any vehicle).
bool vehicle_pad_accepts(const Object& object,int transportation_id);
/// Has native physical movement directions on this layer. Hidden tile-cover IDs
/// never override collision; the final argument is retained for caller compatibility.
bool walkable(const Grids& g, int layer, int x, int y, bool passages);
bool is_passage(const Grids& g, int layer, int x, int y);
/// World maps: open for a transportation group (its Transportation layer), group 1-based.
bool vehicle_open(const Grids& g, int group, int x, int y);
/// World maps: the terrain attribute id of a cell (0 when none).
int attribute(const Grids& g, int x, int y);

// --- masters ---------------------------------------------------------------------------------
const TransportRow* transport(int transportation_id);
const std::vector<TransportRow>& transports();
/// transportation_group.txt: the group may cross cells of this attribute (1-32).
bool group_crosses(int group, int attribute);
/// landing_group.txt: the group may land on cells of this attribute.
bool group_lands(int landing_group, int attribute);

} // namespace field::baked
