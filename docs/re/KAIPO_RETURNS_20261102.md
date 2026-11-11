# Kaipo off-floor return listings

FFIV delivery Closed November2 at18:16UTC in payload011f0dc2, after full canonical,
smoke/scan and guarded22-file hash verification;144protected/otherfive unchanged.
Exact receipt: sibling Class-Delivery/build_check/ff4_party_keys_20261102/
completion.json. Natural off-floor/on-floor confirmation remains separate.

Owner-triggered correction under the single
[audit ledger](MAP_AUDIT_FOLLOWUPS_20261028.md). Actual IV game-directory log
774592.703/774595.375/774597.453 lists Weapon Shop→Kaipo and Inn/Item Shop→Kaipo
returns while the player is still outside in Kaipo32. User requirements preserve
these exits on their own floor, ordinary forward dungeon exits and one-way
transitions. Native game source remains outside Git; no runtime probe is used.

Exact baked/native portal identities show all four Kaipo interiors33/34/36/37
place their initial PointIn three cells inside, facing inward. Their doors are
at Ground5,11;30,13;5,11;13,21, with arrivals three cells North. Town native
return points remain adjacent to the corresponding town doors. Both floors'
state is known; the missing listing proof was the old two-cell inset limit.

The existing listing-only policy now recognizes a straight three-cell inward
arrival in addition to two. Both explicit native arrival IDs, matching layers,
known state, depth toward the current floor and one unique reciprocal physical
door are still required. No general radius is added. Diagonal/outward/unfaced
arrivals, larger insets, uncertain/ambiguous pairs, loops and one-way routes stay
visible. Routing graphs, bakes and native current-floor exit detection are
unchanged. The policy remains in remote_exit_policy.h at its existing caller.
No hook, native pointer, string or localization change is introduced.

Actual production listing/route fixture reproduces all four bad off-floor
returns before the change. Its first report has eight failures: four valid
listing failures plus four mistaken test expectations that a local exit route
sets next_map_id to its destination. Local goals instead walk onto their native
trigger, so the fixture now verifies successful arrival at the actual exit cell.
The original failed report is retained rather than attributed to game behavior.

Corrected fixture passes all four off-floor suppressions and all four local
return routes, retained FFIII Dwarven/shop/animated-object cases, all-six largest
floor graphs and negative policy cases including the new three-cell boundaries.
Exact source/report/resource evidence is in build_check/kaipo_returns_20261102/.
Source/focused Closed:61.722s/445018112B/exit0/drained,2GB/8.33%CPU/oneworker;
exact input hashes are bound in completion.json. No new IV payload is installed;
canonical/security/deployment and natural gameplay remain separate.
