# FFIV Mysidian Legend line fades

The actual FFIV game log at 73457.390 contains one queued utterance for the
entire legend after Cecil's preceding line. Both native event scripts
`Map_40002/sc_e_0030_1` and `Map_40002/sc_e_0068_1` call the same legend
operation. The installed English story bundle stores the legend as seven
consecutive message entries under `E0030_00_353_a_01` through `_07`.

The matched `TraditionMessage` native body at RVA `55C1C0` passes its
localized lines to `LineFadeMessageClient.Play`. The manager's coroutine
activates the controller, sets its data and starts playback. The controller's
`Play` binds each message to a distinct canvas group. Its `PlayInit` at
`474120` begins the fade for `targetIndex`. Native fade progress advances that
index. Its state transition at `101A080` invokes the next state's initializer
even when reentering the same state. The original mod flattened the list during `SetData`,
before any of those phases.

FFIV's multiline sequences now register their controller at `SetData` and
queue the selected native line once when its fade starts. Speech remains in
native order, including identical text at different indices. Empty lines
leave the native pause intact. The selected line is copied on the UI owner.
There are no timers, worker reads of game objects, or changes to the fade.
Map changes, scene load and title entry retire the owner. The existing
single-line notices and other titles keep their speech path.

The actual-source speech fixture covers renderer ownership, one submission
per native index, two identical lines, blank lines, replay, scene retirement,
and the single-line defeat notice. The normal speech worker reads each native
page as a separate utterance and normalizes whitespace within it. The raw
repeat state retains the selected page's original breaks. Offline evidence
and bounded runs are in `build_check/mysidian_legend_20261106/`. Native exports
remain outside Git in `../tools/native_evidence/mysidian_legend_20261106/`.

For gameplay confirmation, trigger the legend during the Mysidia sequence.
Let it advance. Expect a new short line at each fade and natural pauses
between them.

Installed locally in FFIV on November 6. All 281 targets, the complete offline
suite, isolated smoke test and exact payload scan passed. All 22 installed
files match the scanned hashes and 24 protected files were preserved.
Gameplay confirmation is pending.
