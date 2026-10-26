# Warehouse/Fat Chocobo storage, October26

Status: implementation complete, offline verified and installed/hash-verified in
all six games' shared adapters. FFIII gameplay/listening remains pending. All six
builds contain this Warehouse UI; this does not establish that every game's story
opens a storage service. No game launch/control/runtime probe or inventory change.

## Confirmed gap and requested behavior

The user's own FFIII log and screenshots show command focus on Store/Retrieve/Exit
while the generic cursor reader speaks preview inventory names. Counts are absent.
Empty Retrieve visibly says No stored items; Shift+I says No controls displayed
despite Enter:Confirm and Backspace:Back. Popup.Warehouse already reads its initial
flat dialog, including How many are you giving me?, but changing the selected
amount is silent. The screenshot has Hi-Potion stock11 and selected Stored10.

The full screen had no dedicated reader. Earlier popup coverage did not constitute
a complete storage-screen audit. The new reader makes command labels own bar focus,
reads item/count without Store/Retrieve prefixes, speaks visible empty text,
supports current description through I/Autodetail, reads the exact footer and
speaks initial/changed transfer quantity. Existing native controls remain intact.

## Native implementation and ownership

src/core/menus/warehouse.cpp registers Warehouse at119. It owns ten native hooks:
WarehouseController.Open/Close, SelectCommand, SelectContent, ContentUpdate,
SelectingCommandInit, SelectingEntrustTargetInit, SelectingWithdrawTargetInit;
WarehousePopupControllerBase.AddSelectedCount/TakeSelectedCount. All six names,
arities, fields and shared state values were checked against matching dumps.

Native WarehouseControlleBase spelling is preserved. States1/2 own commands,
3/4 Store items,5/6 Retrieve items,7/8 Store amount,9/10 Retrieve amount.
Open claims identity before nested native cursor creation. Visibility requires a
live typed owner, active serialized rootObject and valid native state. Close,
scene/title retire pending work. Hidden preload cannot claim another screen.

SelectCommand reads the current selectCommandCursor and serialized
WarehouseView.commandContentList/CommonCommandContentView.text. Entry adds the
native title and greeting once; later focus reads only the actual command and
any visible noContensMessageText (native spelling). The inventory preview cannot
steal command speech.

List focus validates contentDataList/current cursor index, calls the ready
InfiniteScrollView.GetContentObject for that one index, then reads the actual
ItemListContentController.view IconTextView.nameText and countText. It never
substitutes hidden model counts. A hidden/unready view yields no row. ContentUpdate
reschedules only when both the native data identity and rendered GameObject match
the selected entry. Unrelated pooled-row updates cannot repeat a focus read.
Native readiness callbacks retry population; no polling or frame deadline is added.
Autodetail adds only the displayed description; I reads it afresh.

Focus callbacks share one generation-cancelled next-frame task. A new native
focus retires obsolete work, including nested preview updates. This is event
ownership, not text-content deduplication. Repeated user focus/details requests
remain audible. Generic cursor suppression is registered through cursor_event;
menu_text_discovery rechecks Warehouse.IsActive when deferred generic work runs.

The existing dialogue/popup.cpp Warehouse Show remains the sole initial prompt
producer. It requests Warehouse.QuantityPopup for typed displayed fields:
comment, item name, native Stock label/value and native Stored/Retrieved
label/selected value. The former flat reader remains fallback if typed output is
unavailable. No second Show hook or duplicate prompt reader is introduced.

Native AddSelectedCount clamps to maxCount; TakeSelectedCount clamps to1 and both
finish by updating the target Text. Hooks call the original exactly once and
observe numeric selectedCount before/after solely as a quantity-change event
guard. For an actual change on the current visible Warehouse popup, speech reads
the completed targetCountText and targetCountNameText. Clamp attempts create no
false change. A closed/foreign/hidden popup cannot speak.

## Controls, localization and lifecycle

Global I routes to Warehouse.ReadDetails before other screen descriptions.
Shift+I routes to Warehouse.ReadControls, which uses the exact serialized
WarehouseView.keyHelpController with the existing bounded controls service.
This fixes the unregistered/unselected footer path without scanning the scene.
The warehouse positively owns Global input context while visible, including its
native event-driven UI; field navigation cannot claim those inputs. Controller
details/controls use the same existing central actions. No new key or mode.

Command, greeting, empty text, descriptions, Stock/Stored labels and controls come
from actual localized UI fields. Item counts reuse the existing quantity {0}
catalog key in all12 supported languages. Auto/manual language preferences remain;
no translated game text or newly invented Store/Retrieve prefix is injected.

On close, focus and control reading are cancelled before native return callbacks.
The current popup identity gates quantity events independently. No worker receives
a controller, cursor, row or native UI pointer. Speech receives owned strings via
the existing central service. Original calls and native selection/transfer rules
are unchanged.

## Bounds, verification and limits

Each focus reads one selected rendered row or at most8 command entries for index
validation; list bounds are4096, with direct single-index access, not enumeration.
Normal bar/list/quantity output copies only fixed serialized fields. The existing
controls service retains its sliced capture/worker limits. There is one pending
focus task and one owner, cancelled by generation/lifecycle. No timer, scene
search, native mutation or game-thread wait. Closed-state queries return before
native lookups. Actual callback/frame/audio and queue-tail costs remain unmeasured;
bounded ready reads do not by themselves certify runtime responsiveness.

tools/warehouse_check.cpp executes the actual reader plus displayed-field/frame
helpers with an owner-thread native-boundary fixture in all six game modes:
command focus versus nested preview, Store/Retrieve/Exit, native empty message,
both lists with Autodetail off/on, manual descriptions, refreshed counts, pooled
row identity, hidden views/invalid indices, initial typed popup, increase/decrease,
native min/max clamp, Retrieve labels, hidden/foreign popup, footer identity,
close cancellation and hidden preload. All pass with192 original callbacks and
zero worker native access. The actual key_help_check retains rendered-footer,
hidden-caption, cancellation and lifecycle coverage. Full canonical suite passes.

The first storage test found a missing explicit row-view visibility check; it was
added. Its newline assertion was corrected to use the existing text whitespace
cleanup. Reports are retained in build_check/warehouse_20261026/. No failed build
was installed. All-six native names8275/0 missing, wiring141/348/0 undefined,
47-export smoke and67 embedded-resource checks pass.

Native/schema evidence is outside every Git tree under parent
tools/warehouse_20261026/ff1..ff6/{schema.txt,native.txt,provenance.json}, exported
read-only by tools/re/map_location_native.py --warehouse. Each includes DLL/dump
hashes. FFIII anchors: Open0x8D1F30, SelectCommand0x8D2AB0, SelectContent0x8D3370,
popup Show0x8D6D80, AddSelectedCount0x2F09E0 and TakeSelectedCount0x2F0AD0.
No game source/snippets enter this repository.

Study was briefly reported silent, then explicitly confirmed vocalizing by the
user; that request was withdrawn and Study/other abilities were left unchanged.

## Combined delivery

Includes the [remote return-exit listing](REMOTE_RETURNS_20261026.md).
DLL10983424 bytes, SHA256
bc93b87173047349cf8d6e74ca3a7b1eb654445c72d5812f7fbafa740db3b8e4.
After the user's close confirmation, all six deployments succeeded.132 installed
payload hashes match the exact22-file Defender stage and144 protected files match
the fresh predeployment baseline. Defender reported no threats with unchanged
stage hashes and zero matching detections. Existing unsigned/no signing certificate,
protected exclusion-query and outer-package/reputation limits remain. No security/
dependency change, external upload, publication, release, commit/push or delegation.

Final shared evidence/recovery, complete reports, smoke, scan and receipts:
build_check/remote_returns_20261026/. Storage-specific initial log/recovery/failures:
build_check/warehouse_20261026/. New gameplay/listening remains unconfirmed.
[Prioritized simple checks](../TESTING.md).
