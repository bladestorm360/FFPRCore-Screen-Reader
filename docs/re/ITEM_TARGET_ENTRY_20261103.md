# Current item target on initial entry

The owner's FF1 game-directory logs reproduce Giant's Gloves correctly selecting
Robyn, followed by Strength Tonic incorrectly announcing Robyn at initial entry
on Seren's next turn. Cursor movement then announces the current targets. Actual
entries881819.531 and881830.765, further examples and shared-handle tail hashes
are retained in build_check/ff1_item_target_entry_20261103/. No tester logs,
game launch, control or runtime probe were used.

The initial ally reader preferred private playerDataList whenever nonempty.
Exact native PlayerInit and SelectContent show that this is the previous
selection/click cache; the current command's filtered eligible targets are in
TargetPlayerList. A self-only item leaves a valid but obsolete one-member cache.
This is an incorrect source owner, not a missing cursor-movement announcement.

All six matched GameAssembly/dump pairs establish distinct fields. Private
playerDataList is0x30; the current backing field is0x88 in I/II,0x98 in III and
0xA0 in IV/V/VI. PlayerInit reads the current field to filter and position the
cursor. VI's SideAttackPlayerInit and EnemySideAttackPlayerInit use the same
field. Full native exports/provenance remain outside Git under
../tools/ally_targets_20261024/ffN/ and ../tools/item_target_entry_20261103/ff6/.
Normalized hashes, field layouts and function/access locators are in the local
checkpoint's native_facts.json. No recovered implementation belongs here.

The minimal correction in src/core/battle/battle_command.cpp,
try_read_initial_target, reads the current eligible list directly. No fallback
to the older click cache is allowed. Cursor movement continues using its native
supplied list. Existing bounded64-target validation, index checks, rendered party
row identity, event-scoped ten-frame readiness retry, lifecycle cancellation and
FF6 focus-event guard remain unchanged. No new hook, polling, deduplication,
worker pointer, setting or user-facing string is introduced.

The old battle_focus_check fixture accidentally mapped both fields to the same
member, hiding their different lifetimes. They now have separate storage.
Thirty new assertions failed against the old production reader, and pass with
the correction. All-six cases cover self-only to full-party and the reverse,
initial index, unchanged cursor movement/backtracking, unready/empty current
lists without stale fallback, later readiness and a native nested SelectContent
that must remain authoritative. VI side entries are included. The actual
party_esper_check also exercises displayed party-row ownership. Existing target,
command-ready/ATB, menu, field magic and lifecycle coverage is retained.

The changed path replaces two conditional list lookups with one. Native reads
remain on the existing UI owner; no wait, extra native traversal or unbounded
work is added. Fixture output/performance is not actual audible/frame evidence.
Source/focused qualification and combined build/smoke/security/delivery are
separate statuses. Natural confirmation remains pending: the next freely
targeted item after a self-targeting item should announce its actual first ally.
The owner's FF1 completion confirmation does not erase this individual bug.

## November3 scoped FF1 delivery

Installed only FF1:9c2ba647d1e5408a3548141348e1de78c482999dc0760dd3f735da1143da46a1,
11625984 bytes. The isolated Stage base is the owner-confirmed-beatable b62;
only four source/test inputs differ: this reader/fixture and the raw hidden
enemy condition producer/fixture. All map data and the base's independent
dialogue-choice ownership remain unchanged. Canonical169 targets/178 reports,
8320 native names/zero missing,67 exact embedded resources/12 languages,47 smoke
exports and exact22-file Defender scan pass. Fresh game-closed guards passed;
22 installed hashes and144 protected files are verified. Other five payloads
are unchanged. No game execution/probe/dependency installation/release/push.

Canonical qualification642.078s, complete build/smoke/scan tree654.272s,
797401088B peak aggregate, hard2GB/8.33%CPU/one worker/below-normal, exit0/drained.
The isolated base retains its older conservative CPU cap; no active job was
altered. These are build figures, not game/audible responsiveness measurements.
Recovery, frozen724 inputs, exact deltas and delivery are in
../FFPRCore-Class-Delivery/build_check/ff1_item_target_entry_20261103/; Main's
delivery.json links these receipts. Source/offline/FF1 delivery Closed; natural
gameplay confirmation of this targeted fix remains pending. Broad audit open.
