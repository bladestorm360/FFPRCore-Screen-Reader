# Character identity at the top of status review

Owner requested every status buffer start with the current character name and
displayed level, for example Cecil. Lv.12; games without a displayed level
start with the name. The FFIV screenshot shows that header above favored hand.
This finite change follows the completed Levistone correction under the single
[ledger](MAP_AUDIT_FOLLOWUPS_20261028.md).

All six status pages already build their rendered title from statusController's
visible nameText and fixedLevelText/currentLevelText. The review owner announced
that title separately but omitted it from arrow navigation, leaving job/hand
as row0. status_displayed.cpp now opts into the existing include_headline
behavior. It shifts subsequent section indices by one so group jumps retain
their existing boundaries. Manual full review contains the title once; native
ready speech and Autodetail retain their current phases. No text deduplication,
new field, hidden level, localization string, hook or state mutation is added.

Implementation/current consumer/native compilation and integration are complete;
FFIV received the qualified fix November2 at18:16UTC in payload011f0dc2. Exact
174-report canonical/smoke/scan/install receipt: sibling Class-Delivery/
build_check/ff4_party_keys_20261102/completion.json. This small reversible option
change does not add a mirror test or redesign the shared review owner. Existing
rendered title/lifetime and review-buffer contracts remain authoritative. Actual
first/next-group/character-switch behavior will be a natural confirmation after
installation; other game installations wait for the final all-six delivery.
