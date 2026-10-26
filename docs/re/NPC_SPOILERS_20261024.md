# Navigation NPC story-name masking

Status: complete; full build/offline checks, smoke and payload scan pass. The user reports a future-story NPC identity exposed from another floor
of Castle Palamecia and requests ??? for such identities. This narrows the standing
pathfinding-name approval for this reviewed case, without replacing proper names
for every remote NPC or changing route validity.

The source is the FFII translation key for the developer's resurrected variant,
not rendered game dialogue: Map_30141_11/map278, group ev_e_0085, editor11,
native entity9/type6. Its message key is shared with the ordinary named variant.
The variant is projectable before the Leon conversation reveals the story fact.
The requested label will be ??? through the shared live/remote/snapshot translator
and nearby interaction cue. Ordinary Emperor aliases later in the story and all
native dialogue remain unchanged. The masked variant disappears with its native
scene; no guessed reveal flag, new visibility polling or persistent knowledge
state is needed. It remains ??? while that developer variant exists.

All-six catalog and raw NPC/ShopNPC label audits found this one explicit revived
identity. Another developer distinction already has a named character's ordinary
identity; it is not a reason to hide every recognizable NPC. No broad keyword
blacklist or automatic spoiler inference is introduced. Reviewed exact aliases
are kept in a small original mod-owned override recipe. Original assets/scripts
remain outside Git under ../tools/mapdump/. This record contains factual locators,
not game source or dialogue bodies.

Evidence/recovery: build_check/emperor_spoiler_20261024/. No game launch/control,
runtime probe, dependency change, delegation, commit/push or publication.

Implementation changes FFII's two exact catalog aliases (full-width native
parentheses plus ASCII normalization), with ??? in all12 supported languages,
including the previously absent Japanese entry. The maintained recipe/check is
tools/npc_story_label_overrides.py. Existing live property lookup, remote builder,
route/snapshot label and native nearby-cue paths all consume the shared translator.
The original raw name, map/entity IDs, category, coordinates, story flags, routes,
selection and numbering identities are unchanged; no hooks or runtime work added.
Native dialogue/battle monster names use their existing game-owned readers.
The translator also recognizes explicit @npc-mask/ policy entries before its
Japanese bypass. Ordinary Japanese names still bypass translation unchanged.
No general keyword or string-content inference is used. This adds one prepared
exact dictionary lookup to the existing bounded name resolution, with no new
native reads, scans, state/pointers, workers, polling or waits.

Actual translator tests run all12 locales for both parenthesis forms and numeric
prefixes, plus snapshot presentation and the ordinary named variant. Existing
all-six raw placement/label numbering audits remain. The all-six source census
binds the reviewed mask to the exact one actual placement, preventing an unnoticed
future alias reuse. This is an explicit case review, not a distance rule.

The initial all-language fixture correctly found Japanese bypassing the ordinary
translation table. That language now applies the exact reviewed policy first;
its ordinary-name assertion was also corrected to preserve native Japanese text,
rather than incorrectly expecting the English fallback. The failed report is
retained. Existing prefix normalization exposes the same mask to prefixed NPCs.

## Combined verification and delivery

Canonical build exits0 with7671 native-name checks/0 missing,131 used/339 defined
wiring names/0 undefined; every required checker passes. All12-language actual
NPC translator/numbering and shared live/remote/snapshot label checks pass.
Logged cave comparison:258 steps walking,153 with parked Airship,149 already
aboard; full+browse combined43.759ms offline compute. Legal boarding/landing/facing,
newly available/missing cached models, short walking route and filter/legacy mode
cases pass. These are actual production planners/real bakes with simulated native
owners; live audible/frame response and gameplay activation remain unconfirmed.

The isolated DLL smoke host resolves/forwards all47 exports and completes boot
without launching a game. All49 embedded data+18 sound resources match sources.
The exact22-file stage passed enabled/current Defender with no matching detection
and unchanged hashes. Existing unsigned-DLL/no-certificate and administrator-
protected exclusion-query limits remain; no protection change or external upload.

DLL10874880 bytes, SHA256
d75864ec918d7eaeb0300d5a342ff3e9a520983b3aadd27f8554d7117535d627.

Installed/hash-verified in all six closed games. Every installed22-file payload
matches the scanned stage;144 protected files remain unchanged against the fresh
pre-deployment baseline. Recovery, prior failures, source/resource hashes, build/
check/smoke/security and final installed manifests are retained in ignored
build_check/emperor_spoiler_20261024/. No game probe, dependency upgrade, release,
commit/push or delegation. Gameplay listening/landing confirmation remains pending.
