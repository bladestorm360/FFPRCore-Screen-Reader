# Citadel of Trials reward objective

Owner-triggered November2 correction under the single
[audit ledger](MAP_AUDIT_FOLLOWUPS_20261028.md). The owner confirms the existing
maze directions work but the guide stops before its actual Rat Tail objective.
Preserve that accepted route; this is a finite missing-final-step correction.

The current recipe finishes at native3F PointIn1, Upper35,32. Exact external
mapdump identities identify the reward on map95/Map_30071_3: TreasureBox9,
Ground27,29, treasure bank512, content57. Script sc_e_0047 awards the chest and
sets persistent scenario45. Its factual normalized body hash is
7fddbe923b11e8c5d94a745fd54f3f67fdc752168e7b0f77503338563343b0dd.
Primary game data remains outside Git at ../tools/mapdump/; no implementation
is copied into this repository.

Append the exact chest as an Interact target after the unchanged maze waypoint.
Use scenario45 for both this step and the whole recipe's completion so a later
visit stays finished even after the reward is handed to Bahamut. Existing
localized chest instructions cover all12 languages. No new hook, polling,
native read service, item-name disclosure or routing architecture is required.

Acceptance covers fresh entrance, intermediate maze resumes,3F resume, chest
approach, missing receipt, battle return, actual reward, stale route rejection
and completed resumes on every floor. Actual controller and production planner
fixtures must pass before the source/focused checkpoint closes. Build/security/
installation and natural gameplay confirmation remain separate. Evidence:
build_check/ff1_trial_reward_20261102/. No game is launched or probed.

Source/focused Closed: same actual controller fails10 assertions with the old
recipe and passes with the new one. All87 native controls in52 recipes pass;
12 full maze resumes continue through48 reward approaches across all four
guardian progress profiles, with24 actual guardian handoffs and no return-pad
contact. The native operation and localization authors pass unchanged12-language
entries. Focused47.938s/451661824B/exit0/drained, hard2GB/8.33%CPU/oneworker.
Exact input/report/receipt hashes are bound in completion.json.

The installed delivery branch predates the maintained native-receipt guide.
Its isolated713-input candidate ports only the required identity/scenario/
floor-entry semantics and native-ready event listeners, with actual controller
regressions and a new canonical reward-route check. Other games retain their
existing recipes. Main's pending Rod/Lute and other all-six guide work is not
silently imported. The candidate also includes the independently qualified B3
exit identity fix; no bake changes, native hook, polling, worker/native pointer
crossing or resource-budget increase. Its build/smoke/security/install evidence
must finish before claiming delivery. Prior partial build artifacts are retained.

Delivery Closed November2 at15:33UTC: installed DLL42fe55ac0e6d793332eb7918bbc38d924378e332b9cf72cd96707cc50ea237c7,
11538432B,713 frozeninputs/165targets/171freshreports/8317native names0missing/
47exports/67resources/12locales. Full canonical1553.620s; build/smoke/scan job
1573.499s/800321536B/exit0/drained, hard2GB/8.33%CPU/oneworker. The long first
header-change rebuild filled the preserved check cache; no routine warm-time
claim is inferred. Guarded installation7.432s/115027968B/exit0/drained. Defender
22exactfiles/no threats/stablebytes; exclusions unverified/signing identities0.
All22 installedhashes match;144protected/other five games unchanged. Fresh guard
found FF1 closed, no launch/probe. Owner told may reopen. Evidence sibling
Class-Delivery/build_check/ff1_trial_reward_20261102/completion.json. Natural
Rat Tail/B3 confirmation remains separate; preserve user-confirmed Levistone.
