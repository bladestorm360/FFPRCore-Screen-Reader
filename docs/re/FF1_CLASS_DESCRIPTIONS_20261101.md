# FF1 class descriptions — source, offline checks and FF1 delivery Closed

Installed November1 at15:19 UTC in the isolated FF1 follow-up candidate,
SHA2b7c5f50f0a1c30ba0367d36d67c0fcdeedc3eb33a553b4c8f6eb6d17e929284.
Full build,165 reports,8308/0 native names,47-export smoke,67 resources/12 locales
and22-file Defender scan pass; installed hashes and144 protected files verified.
No game launch/probe. Receipt/source/recovery and honest natural-confirmation
limits: FF1_GATE_FOLLOWUPS_20261101.md. Historical preparation failures below
are retained, not the current installed status.

Owner requested the current FF1 new-game class chooser to read its description
with I and Autodetail before continuing the map audit. Scope is this exact menu,
shared existing details-key dispatch and its native lifetime. Sol only; no game
launch/control/probe. The installed FF1 log confirms native SelectContent hook
and class labels, and the owner confirms descriptions are missing.

The old reader searched children, matched an English class whitelist and guessed
the longest other text with at least ten characters. The actual controller has
serialized contentList0x38, discriptionParent0x40 and discriptionText0x48. The
description panel need not be a child of the list. Native SetDiscriptionText
831450 writes that exact text component; SelectContent830FA0 binds the cursor
to contentList's selected row. Row SetData830580 writes its JobContentView name.

The adapter now reads the exact selected row's displayed jobNameText and the
current displayed discriptionText, honoring its native panel visibility. The
same reader supplies I and the existing preference-controlled Autodetail.
No English whitelist, hierarchy scan, guessed text, dictionary fallback or new
deduplication remains. Each native selection retains the established one-frame
handoff; generation/controller/index/readiness checks cancel superseded, reset,
hidden and destroyed owners before description access. At most twelve native
rows are admitted; no full-row or child scan is performed. Data stays on the
native owner; speech uses the existing central service. No new mod text needed.

Original consumer tools/ff1_class_description_check.cpp now exercises29 cases,
including the separate panel, short descriptions, repeated explicit details,
Autodetail off/on, rapid focus, hidden/blank/closed/destroyed state, reset,
invalid/oversized rows and all twelve language paths through rendered labels.
The exact installed original adapter reproduces22 failures and41 hierarchy
scans in the original26-case baseline; corrected adapter passes29/29 with zero
hierarchy scans. The three added cases cancel owned pending speech/cache on
MapChanging, TitleScreen and SceneLoaded, without dereferencing teardown owners.
Before fixture
source/outputs remain under build_check/ff1_class_description_20261101/;
consumer_retry_budget.json:28.787s, verified serial1 GB, children drained.
Native contract runner: ff1_class_native_retry_budget.json, exit0/drained.
The earlier fixture include/namespace failures and first wrong-namespace
contract assertion remain preserved; they are not behavioral before evidence.

Primary contract: tools/re/ff1_class_description_contract.py.
GameAssembly SHA25633f001b4420bea06b44740c07c4afcf92547e7196bdfd3ec27ae5feaf0229108;
dump8313bacc58a606108db7dc0786b4fc5bcd57dc07e171cb8b051c2a68cd817740.
SelectContent304B SHA b29fb226cb080cdcd7aaff37476b015acfa6c61849867da6f73efe609a02fbd5;
SetDiscriptionText336B SHA688e33f082804a6ed0e6d13e88830d7e945d6b5ff52fa70bd4ea6c9089838995;
SetData560B SHA49b9fa010f128c7ce9cf9007fdb3fb03b0a8977bb38b19f1169e578042f5a81f.
All three bodies unique; raw native bodies stay outside Git in
../tools/ff1_class_description_20261101/. No native snippet is copied here.

Before delivery, FF1 was a6d9b4ce19dfb6adb33fb0051851814a62b4fcd0848c041c0ef23625b466998f
(October30 build). That baseline is retained in recovery. Canonical/smoke/security,
exact scanned-payload/recovery evidence and guarded closed-game installation
were completed for the final candidate above. The isolated FF1 delivery was prepared from all688 exact
installed-baseline inputs. Its first canonical configure failed on a stale
copied CMake-cache path before compilation; case-insensitive relocation repaired
that path. It is not yet a checked candidate, and its copied old hid.dll must
not be installed. Merge the expanded all-map door and Melmond findings before
retrying. Preserve unfinished audit and the sibling Eureka delivery.

Natural confirmation after installation: in FF1's new-game class list, move to
a class and press I. Its displayed description should speak on every request.
With Autodetail enabled, class and current description should speak when focus
changes. Rapid movement must speak only the current class; closing the list
must prevent delayed class speech. No native audio/frame-latency claim is made.
