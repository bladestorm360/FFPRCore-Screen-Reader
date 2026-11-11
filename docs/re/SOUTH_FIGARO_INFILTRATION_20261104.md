# South Figaro infiltration and escape

Shift+G follows Locke's scenario from the first merchant through the complete
Tunnel Armor escape. It uses current game receipts and actual player position;
it does not write story flags, choose dialogue options or operate the game.
Minimal remains the default guide verbosity. Essential combat instructions and
native option positions are retained in either verbosity mode.

The ordered operations are merchant Steal, merchant-boy passage, green-soldier
Steal, the moved guard, cider merchant Steal, the return past the boy, the old
man, second-option password, basement passage, first-option costume removal,
Celes sighting, first-option recruitment, first-option sleeping-guard key,
first-option clock, exit conversation, cave noises582/583, final warning63 and
Tunnel Armor. The bedroom draft region is handled before its secret stair.
Stealing in the cider battle provides the merchant outfit for
the boy on the return. Red soldiers do not provide the green uniform.

A wrong password returns Locke to town and removes his costume, while completed
cider/old-man receipts remain. Guidance resumes at the password instead of
repeating that chain. A defeated merchant without a stolen costume requires
leaving and re-entering his real room before trying again. A saved cider state
without a merchant outfit uses the alternative merchant; a soldier must choose
the second option to challenge him. Current-room locals own defeated/moved
poses. Remote locals are never supplied from another room or invented as zero.

The six native choice controls can repeat their last actually published
instruction with Shift+G while the native choice is open. This uses the existing
bounded choice cache, with no flag read, route request or speech deduplication.
Map/battle/load/context changes retain the existing cancellation and readiness
owners. Battle64 finishes only when the resulting native field is ready.

## Source and map correction

Exact field-script body hashes and four costume/tutorial battle asset identities
are in [south_figaro_native_20261104.json](south_figaro_native_20261104.json).
Raw scripts/disassembly remain outside Git under the matching local
`tools/puzzle_audit_20261030/broader_audit_20261104/ff6_South_Figaro/` archive.
The native battle44 script sets52/reset53 after merchant Steal; battle41 sets53/
reset52 after green-soldier Steal. Native Current.CheckLockDisguise observes
conditions665/666 into script-local2; it does not itself own those story writes.

Native opening51 precedes the town's actual47,43 starting pose and selects cave
interior aliases626..629. The guide proves the active world entrance333/630
through its native edge. Persistent receipts are guard54, cider55, old man56,
password57, sighting58, recruitment59, key60, clock61, exit62, noises582/583,
final warning63 and boss64. The order is582→583→63;63 suppresses the earlier
regions, so a resumed63 goes directly to the boss without demanding an
unavailable earlier noise.
Costume removal clears52/53. The clock's blocking entity14 moves only after61;
its front control remains an ordinary target. The native key is obtained before
the guide requests the clock operation. No door or treasure availability is
granted by guide proximity or by an assumed operation.

The two basement events58/0 and57/0 visit town38/PointIn1 solely for a hidden-player
camera animation, then finish on57/PointIn1 or58/PointIn1 respectively. Their
previous first-transfer bake stranded routing at the camera scene. The exact
source-pinned offline correction uses those final arrivals. Only the two derived
Warp destinations change;638 rooms' terrain, native
objects, masters, state programs and twenty older guides are preserved.

The accepted strict event-safe planner is indoor-only. The guide uses its
existing ordinary walking-world policy only for the actual town-to-cave leg.
All other targets retain strict event avoidance and reviewed scripted crossings.
There is no runtime script parser, new hook, new native getter, new worker lane
or change to the closed routing/search design.

This recipe alone opts into first-matching-rule ownership. Alternatives within
the current operation still share the planner, but a later quest stage cannot
become a fallback when the current operation is unavailable. A recovery check
found the former additive rules also offered the old man's later waypoint
alongside the merchant challenge. The opt-in fixes that actual ambiguity;
the twenty previous recipes keep their existing additive behavior and outputs.

The production Resolver already copies current native SetEntities observations.
The world fixture now supplies that same owned input. A Flags-only projection
cannot infer these groups after an opaque world-initializer operation. The
fixture's default native facade matches the existing constructor; production
readers, hooks and snapshot interfaces are unchanged.

After the boss callback,64 is followed either by scenario selection on584 or
the all-scenarios sequence584→14→588→14→3→18→19. Earlier scene maps are not a
ready completion. The ordinary native readiness observer owns that boundary.
Two later-world cave events belong to the separate Gerad/Figaro review.

## Verification and remaining limits

Actual model, production planner and native-owner controller fixtures qualify
the guide. The initial fixture used a generic town arrival instead of the
native opening's final47,43 pose; later checks exposed the missing cider-Steal
instruction, wrong basement arrival, bedroom draft and separate costume-removal
choice. The cave check qualifies native51, map aliases and the initializer's
sound order. Failed
resource receipts remain evidence, never reported as passes. Recovery, physical
approaches, source preservation and twelve-language checks are required before
this capability is Closed in the single audit ledger. Final evidence covers34
assertions and43 actual native continuations, including31 in the complete normal
sequence;535 physical guide identities, all six native choice controls and both
post-boss scene branches pass. Source/arrival/preservation/all12-language checks
and the all-six route-worker suite pass. A build-script edit during an active
check invalidated that runner result despite passing assertions; the check was
rerun successfully with the script fixed. That failed receipt remains retained.

Exact reports/resources/source identities are recorded in the capability's
completion receipt when Closed. Capped fixture durations establish offline
behavior, not game-frame or audible response. Combined build/security/deployment
and natural gameplay remain separate. When progression permits after delivery,
verify disguise/boy/password recovery, choice repeats, the secret passage, clock
and complete cave escape. No game launch or runtime probe is authorized here.
