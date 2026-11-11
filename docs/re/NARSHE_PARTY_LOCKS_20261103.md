# Narshe active-party doors, November 3

Source/focused/documentation closure is recorded in
`build_check/narshe_gates_20261103/completion.json`. Combined Main DLL, security,
delivery and natural gameplay checks remain pending. No game was launched or
probed; Sol performed this work without delegation.

## Behavior and exact scope

Six World of Ruin Narshe doors115–119/114, group ev_e_0703, require Locke in the
current corps. Merely recruiting or owning Locke in reserve is insufficient.
Native flag353 enables the locks after the Falcon; receipts411–416 persist each
opening. The door itself remains a valid navigation target. Destinations behind
it are gated without the ability; an active Locke produces a route stopping at
the closed door with “Interact to unlock the door.” The player operates the
native door. An already-open door stays passable after Locke leaves the party.
The sixth entrance uses Event197, unlike the five ordinary map exits.

The shared manual-door prefix retains FFIII's Magic Key quantity, Thief,
prerequisite, opened-door and no-key behavior. Every closed affordable door is
solid during approach search. No path traverses a closed door merely because the
player can unlock it. No native event, save flag, inventory or collision write
is added. This is a route capability, not a new puzzle recipe.

## Native evidence and ownership

The matching FFVI assembly/dump and six script hashes, current-corps/member-ID
contracts, bounds and renderer locators are in
[the factual manifest](narshe_party_gates_native_20261103.json). Raw native
analysis is outside every Git tree under the parent tools/puzzle_audit_20261030
Narshe archives. Native BranchByCharacter B5B680 checks canonical character2
from active corps. GetMemberData57D610 allocates a closure, predicate and cloned
character; the mod never calls it. The existing owner snapshot instead reads
the name-resolved corps/owned lists, matches the current saved CharacterId to
the owned Id and reads that member's canonical characterStatusId. Limits are
16 corps entries and64 owned characters. Invalid/missing/type-incorrect or
overbound input stays unknown. Native checks neither HP nor enabled-corps, so
the mod does not invent those restrictions. Only the copied optional boolean
reaches workers. Its unknown/false/true signatures are distinct; absence keeps
the earlier games' signature path unchanged. No hook, polling or worker-side
native API is introduced.

The first actual route fixture exposed another concrete cause: initializer
row21 was an opaque foreground-display operation, leaving all six doors only
possibly active and preventing valid goal projection. Exact source-hash-qualified
ShowCloseView1 sends message82 into FieldController/MainViewMapRenderer and
MapMultipleScroll.Show. That chain changes visual activation/particle/color/fade
state, without routing flags/grid state. Only this row becomes a private
projection no-op; the game's visual operation still runs unchanged. Unsupported
Shadow-membership branch86 stays opaque. All other initializer rows, rooms,
metadata and terrain are preserved. No broad character-branch approximation is
added. An early-world fixture must clear both297 and353; Falcon353 with an
uncollapsed world is an inconsistent native profile, not a production bug.

## Extension and verification

Ownership is map_state's bounded owner capture, map_state_program's exact
six-record capability resolver and the existing manual_key_prefix route
consumer. author_narshe_party_gates/state pin source and metadata;
review_narshe_party_gates records only six original dispositions. The prompt is
localized in all12 languages by modtext_narshe_gates. Canonical build.bat checks
those authors and actual reader/route fixtures. Do not generalize this predicate
to owned-party branches or add another native helper without matching evidence.

Actual reader18 cases and six-door route/browse93 assertions pass with zero
wrong-owner calls. Coverage includes reserve rejection, noncanonical owned IDs,
unknown capture, all fronts/behind-door goals, early-world absence, manual
interaction, opened receipts, signature invalidation and the scripted sixth
entrance. Current FFIII key routes pass246859 assertions,203 journeys and61
manual unlocks/490 hops; its reader37 cases still passes. FFI all-map key doors
pass143 routes/118 chest cases and Cornelia277 cases. Preserved failed fixtures
explain the projection and inconsistent native-profile corrections. Resource
receipts enforce2GB aggregate/25% CPU/one worker, including child compilers;
fixture wall time is not a game-response measurement.

After combined delivery, natural progression should confirm that an active
Locke permits manual unlocking, reserve-only Locke does not, and opened doors
remain usable. No gameplay test is requested before delivery. The Closed Narshe
light guide retains all15 stages/recovery/ready317. Other43 remaining Narshe
interactions, including Lone Wolf/Mog, stay separate required audit work.
