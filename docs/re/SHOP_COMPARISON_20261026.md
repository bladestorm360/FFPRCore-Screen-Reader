# Shop equipment compatibility and hand comparisons, October26

Status: implementation complete and offline verified. The exact scanned payload
is installed/hash-verified in I, II, IV, V and VI. FFIII's running-process guard
deferred its installation; it retains the prior build until closed. Gameplay
listening of the new fix remains user-side. Stronger/Weaker are user-confirmed;
this report
specifically concerns equipment-ineligible portraits and Robyn's equal-stat staves.

## Purpose, inputs and output

In an open shop, keyboard1..9 requests the corresponding displayed party slot.
The assigned visible portrait supplies its name when the name text is absent.
Weapon/armor incompatibility must say Can not equip after the name. Compatible
portraits retain Can equip and the actual Stronger/Weaker/No change indicator;
Equipped remains independent. No new setting, remap or game action is introduced.
The new shop wording copies the established Cannot equip translations for all
12 supported languages; English uses the user's requested Can not equip. Auto
and the persistent manual language override retain central modtext behavior.

## Root cause and production flow

shop_patches.cpp resolve_shop_item_type previously probed Weapon and Armor using
the shop product ContentId. These tables have independent keys: Content.TypeId
identifies weapon2/armor3, and Content.TypeValue identifies the corresponding
equipment record. Probing the wrong primary key can return unrelated equipment
or nothing. Most equipment therefore lost its type gate, leaving no eligibility
speech when the impossible text and arrows were absent.

The existing focused-item native SetDescription path now resolves that exact
Content record and reads get_TypeId, then set_last_item publishes its type to
shop_tracker.cpp. ShopPartyReader's existing visible-equipment gate can then read
the portrait's bounded32-character currentState. Native Default_00 means the
shop's ineligible pose; win means compatible. Visible impossible text also owns
its existing accessibility equivalent. Empty/missing/non-equipment records remain
unknown; magic/consumable idle poses do not become Cannot equip. Arrows and
Equipped still come directly from the rendered view, without new stat arithmetic.

Ownership, closed/hidden slot guards, event identity and central say worker remain
unchanged. No native object leaves its owner thread; no worker wait, scene search,
new callback, polling or text deduplication is added. This fixes an existing
bounded product lookup and does not claim measured native/audible performance.

## Comparison question and source evidence

The supplied FFIII screenshots show identical item summary text: Attack+20,
Accuracy+50, Intellect+2, Spirit+2. Robyn has an Equipped marker on Fire Staff and
a native up arrow on Ice Staff. Ryko has a down arrow on both; the user corrected
the earlier suggestion that his other hand improved. Preserve those native arrows.

Existing build-matched all-six native exports are outside Git under
../tools/npc_floors_20261025/ffN/native.txt and provenance.json. FFIII KeyInput
ShopCharaStatusContentController.Comparison(int, Weapon) is RVA0x7ECC50;
CharacterImageController.SetDefaultAnime0x3DAC10 stores Default_00 and
SetCanEquipAnime0x3DA9C0 stores win. The weapon path asks GetCanEquipSlots0x3F3510 for permitted slots, using the
character's dominant arm and Ambidextrous flag. IsEquipSlot0x3F39A0 maps Slot1/2
through PartsGroup.IsEquip1/2; the staff group1 permits both hand slots. Each
comparison uses a cloned character and temporary native SetEquip, then compares
PlayerCharacterParameter.ConfirmedAttack (virtual slot13, RVA0xA800B0). The mod
never invokes those simulation/mutation functions; it reads their finished icon.

The native loop exits at the first attack improvement. Equal/lower results can
continue to another slot; one aggregate icon does not identify which hand caused
it. An up arrow means an improvement was found among the considered slots, not
that the staff improves on every equipped weapon. The game's equipped marker
checks the item identity separately. The shop comparison takes no selected-hand
argument and the list exposes no hand selector; Q toggles item description/stats.
Per-hand replacement belongs to Equip. Robyn's precise other-hand equipment is
not established by the supplied shop screenshots, so the up arrow's exact
inventory cause remains unconfirmed. No invented equal-stat override is added.

Content/Weapon/Armor audits find old misclassifications of84/85,99/107,117/153,
176/176,140/191 and223/223 equipment products in I..VI respectively. These are
master-data coverage counts, not gameplay observations. The all-six getter and
native portrait-state facts/hashes are retained in native_data_facts.json.
No native implementation is copied here.

## Validation and safe extension

Actual production sources are exercised by equipment_party_check (Content getter,
focused-item publication, all-six real product IDs, consumables/magic/missing
records, prices/details) and shop_party_check (all-six visible poses, up/down,
no-change/equipped, hidden portraits and closed shops). Canonical build.bat runs
both; smoketest.bat verifies the isolated DLL without launching a game.

Keep ContentId separate from TypeValue in future shop adapters. Never infer
ineligibility merely because an arrow is absent, or replace native arrows with a
comparison of the item's own summary text. No game launch/control/probe is
performed. Exact reports/recovery and subsequent scan/delivery receipts belong in
build_check/shop_comparison_20261026/. Shared architecture, native source locators
and per-feature documentation remain indexed from FEATURE_INDEX.md.

## Final offline evidence and delivery

Canonical build and its complete offline suite pass:7994 native names/0 missing,
137 used/344 defined/0 undefined wiring. Actual shop/Content/party checks report
zero failures. The isolated smoke host resolves/forwards all47 exports without a
game process. All49 data and18 sound resources match current source, including
the new12-locale key. The131 literal source registrations are indexed; the
previously documented StartupDiagnostics registration gained its missing table
link. Checked memory/feature/menu topic links resolve. Existing subsystem topics
are reused rather than copied into a new broad audit.

The restricted first build stalled four minutes at Checking File Globs and was
stopped; the unchanged canonical build succeeded with the installed toolchain
outside the restriction. first_stalled_build_log.txt is retained. The first
post-deploy receipt used the old task's scan folder name and failed before
verification; correcting that evidence-only path produced deploy_partial.json.
Neither issue caused an intermediate payload to be installed.

DLL10950144 bytes, SHA256
1b5f454bc8d4f3e0c7e9f482c5a634f9ee855c8336fb2e8d1d349299dad637b7.
Enabled/current Defender definitions1.461.211.0, scan exit0/no threats,
no matching detection/remediation events, all22 staged hashes unchanged. Existing
unsigned/no signing certificate, protected exclusion-query and outer-package/
reputation limits remain; no protection change or external upload occurred.

The process guard installed I/II/IV/V/VI and deferred running FFIII. All110 payload
hashes in the five installed games match the scanned stage; FFIII's DLL still
matches its recovery copy. All144 protected files match deploy_finalbefore.json.
Prism prerequisites were checked read-only, dependency versions unchanged. No
release/publication, commit/push, delegation, game launch/control/probe or gameplay
confirmation is part of this change. See verification.json, native_data_facts.json,
embedded_resources.json, documentation_coverage.json, security_predeploy/,
deploy_output.txt and deploy_partial.json in the task evidence folder.

## Combined final delivery

The historical five-game delivery above is superseded by the
[October26 status/world-event payload](ALLY_STATUS_20261026.md), now installed
and hash-verified in all six after the user closed FFIII. The Content-type and
12-language Can not equip correction are included. Current build/scan/hash and
recovery receipts live in build_check/ally_status_20261026/; earlier receipts are
preserved. Gameplay listening remains pending.
