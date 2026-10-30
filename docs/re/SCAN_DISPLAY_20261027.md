# Libra and Study displayed information

## Scope ledger

User-initiated FFIII audit: source checkpoint complete, Closed. Verified native display behavior and corrected one reader timing defect. Final combined delivery and gameplay confirmation remain pending. Temple of Water source is complete, Closed. The next active source scope is multiple-NPC route oscillation; EXP-counter Enter feedback, Amur Weapons remote-return preview and Boost visible-display audit remain separately queued. No game launch/control/runtime probe was authorized or performed.

No hidden HP reveal or optional enemy-HP setting change was included. This scope concerns reading actual native message events; Kraken’s native HP suppression remains unchanged.

## Native display facts and original reader issue

Libra ability15/function-group62 and Study ability89/group141 both invoke function50 with identical zero parameters. Their native FFIII `ScanBattleFunction` therefore displays the same HP/weakness information when permitted. Kraken is monster204 (`MSG_MON_NAME_204`), flag-group2, which contains flag14, `ResistLibraHideHp`. Native `IsInvalidScan`/`DefaultAddMessage` deliberately omit Kraken HP. The user's own log at253451.109/253459.187 records Libra/Study on Kraken and only “Vulnerable to lightning.” That boss-specific omission is native behavior, not a reader failure; the mod does not query parameters or synthesize HP.

A separate issue was in speech timing. Native `IsFunctionEnd` at `0x676840` calls `BattleUIManager.SetCommadnMessage` each frame for the current `messageDisplayCount` until its timer advances. KeyInput `BattleMenuController` at `0x6A19B0` forwards to `BattleCommandMessageController.SetMessage` at `0x3BC2B0`; neither caches by content. The former two-frame deferred message task was repeatedly replaced, so a line could disappear before its speech was submitted, including when the native line index changed. The fix preserves one actual nonempty native message per producer/index event; it does not suppress identical text or read enemy parameters.

Exact native source/functions/provenance remain outside Git under parent `tools/scan_display_20261027/ff3/`. Original extractor mode is `--scan-display`; ability/function/monster flags come from parent `tools/mapdump/ff3_master`, and official names were resolved read-only using `tools/official_text.py`. No game-source excerpts or game text tables belong in this repository.

## Implementation and ownership

FFIII-only `src/core/battle/scan_feedback.cpp/.h` registers `Battle.ScanFeedback` for FFIII. It hooks `ScanBattleFunction.IsFunctionEnd` with the exact native `bool(void*, const void*)` ABI. In the prefix, on the native owner thread, it safely copies only `isFirstProcess` and `messageDisplayCount`. Invalid/missing metadata falls back to ordinary speech. The original is called exactly once and its bool result is returned unchanged.

A thread-local RAII source scope associates the current owner address and integer index with one event. The opaque address is only compared as an identity token, never dereferenced after the native invocation. A first-process marker, changed owner or changed index starts a new event; no text comparison is used. `src/core/battle/battle_message.cpp` calls `accept_message()` only after formatting/cleanup and blank rejection, so only the first nonempty actual message for that event is accepted. Other battle messages, existing pairing and FF6's established guard are unchanged. The prior source scope is restored on nested calls and exception unwinding. Event identity is cleared on native completion, BattleEntering, BattleEnd and SceneLoaded.

No worker receives native pointers; no native writes, polling or hidden-HP query was added. Speech remains on the existing message-reader path. Root reviewed and accepted Terra's bounded code change after Sol's critical source review. Remaining source facts and lifecycle contracts above are unchanged.

## Offline verification and limits

The meaningful fixture uses actual `battle_message.cpp`, `frame.cpp` and the Scan producer path. It verifies the old unscoped repeated-frame replacement, then confirms HP submits within two frames and a later weakness line speaks once. Coverage includes 22-frame ordinary repeat, index change, identical text at a new index, new cast at the same address, weak-only output without invented HP, true/false native return preservation and exact original-call count, blank and negative-index cases, missing metadata, null/other-game owner, nested invocation, exception restoration and battle/scene lifecycle reset. The displayed lines remain native-owned.

Final receipts are `build_check/scan_display_20261027/final_build.txt` and `final_smoke.txt`. The actual-source regression/check reports are built from `tools/scan_feedback_check.cpp` and `tools/battle_feedback_check.cpp`; build attempts `first_build.txt` and `second_build.txt` are retained separately. The first fixture build failed because of a fake-signature mismatch; it was corrected, then the second and final canonical builds passed. Final canonical evidence:8,291 name checks/0 missing,142 used/349 defined/0 undefined; battle-feedback, formations, route-worker and new Scan checks all pass with0 failures. Smoke forwards all47 exports and exits normally. DLL is11,061,760 bytes, SHA256 `e3355604469fec30904f4c839164d3599595c903ac8fdeaa403b89cf99b6cd40`.

Ten thousand detour-plus-actual-message fixture calls measured5.572ms aggregate with one pending speech task. This is fake-native/offline fixture cost, not native callback/frame or audible response. Native frame cost, queue backlog/tail and user-perceived timing remain unmeasured; no responsiveness claim is made. No game was launched, probed or deployed. The source checkpoint is Closed; combined delivery and gameplay confirmation remain pending.

## Closed-system register

Scan display reader source is Closed. Do not reopen it as an incidental consequence of the separate native Kraken HP restriction or new findings. The current authorized source task is multiple-NPC blocker-route oscillation, followed by EXP-counter Enter feedback, Amur Weapons remote-return preview and Boost visible-display audit as separate scopes.