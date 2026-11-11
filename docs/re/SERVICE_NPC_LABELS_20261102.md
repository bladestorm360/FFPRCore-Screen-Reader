# Service NPC names across six games

Source/focused checkpoint **Closed**; no new payload has been installed. Human
reports: Gaia Inn and Church remote actors say NPC; Onrac's innkeeper says
FieldNpc. The finite review covers all382 native ShopNPC17 records plus103
scripted service actors:485 physical records,54/98/75/55/96/107 by game.
This is a service-name audit, separate from the unfinished map/puzzle audit.

## Evidence and decisions

The real pre-change translator reports18 generic service labels:11 FF1,3 FF3,
4 FF4. Broader semantic review also finds valid but unhelpful class/appearance
labels on actual service actors, and FFVI developer labels which name a different
shop from the native operation. The review retains275 usable role/personal
labels and corrects210 exact actor/raw identities:16/14/11/23/77/69 by game.
Existing useful descriptive shop labels and the personal identifier ツース are
preserved. No language-wide NPC/FieldNpc alias is introduced.

Gaia Inn61/entity3/sc_sty_20151_2, Gaia Church65/entity3/sc_chu_0006,
Onrac Inn53/entity4/sc_sty_20131_3 and Onrac Church55/entity4/sc_chu_0005
are ordinary NPC6/action2 actors, all raw NPC. Native inn scripts own payment,
recovery and return; the church scripts invoke the native member-selection and
revival operations. ShopNPC17 actors instead use exact product-group captions.
Other scripted shops use the exact native shop-opening call, not a name guess.
FFVI240/entity0 has separate raw names for its weapon and armor services; the
raw accessory label on the latter is misleading and its native call establishes
Armor Shop. FFV35 has the same raw name for entity2's item shop and entity25's
innkeeper. These examples require actor identity as well as map and raw name.

Primary inputs stay outside Git: ../tools/mapdump/ffN_objects.jsonl,
ffN_scripts.jsonl and each game's map/script/product-group/system-English
masters. The factual review manifest service_npc_labels_20261102.json records
all485 identities, native script body hashes and six primary input hashes per
game; it contains no recovered implementations. The author/check tool rejects
changed primary inputs, unreviewed typed shops, non-NPC identity collisions,
conflicting labels and missing translations.

## Runtime owner and extension

entity_translator owns the new exact @actor/map/entity/raw lookup in each
game's existing immutable translation resource. It falls back to existing
@map corrections and ordinary translation. All210 corrections have12 languages,
including Japanese. The data author holds only generic mod-owned service words;
actual character names and existing useful labels remain game-specific.

entity_detection reads the already-qualified PropertyEntity.EntityId alongside
the existing raw name on the native owner. This fixes the property name before
the live NPC scanner's NPC-to-GameObject fallback can replace it with FieldNpc.
RemoteBuilder passes its baked actor ID to the same resolver. Worker obstruction
instructions pass the copied actor ID through the snapshot resolver with native
name tracking disabled. Raw identity, availability/story projection, positions,
inclusion, categories, route goals and duplicate numbering are unchanged.

No hooks, polls, asynchronous native reads or role inference were added. Each
name adds one immutable dictionary probe and bounded key construction; the live
property reader adds one ready integer read. Existing capture slices, locale
ownership and worker cancellation remain authoritative. No game-frame/audio
responsiveness claim follows from the resource-limited fixtures.

To extend this table, review exact native service semantics and identity first,
update the factual manifest/current primary hashes, then author/check all12
translations. Do not use a global generic-name alias or an unqualified map-only
override when actors on that map share the raw name.

## Verification and delivery limits

build_check/shop_npc_audit_20261102 retains the actual pre-change labels,
semantic decisions, initial failed helper attempts and resource receipts.
The real translator/numbering fixture now observes exact map/actor identity
across the entire NPC census. It also checks all485 service records in12
languages, live/snapshot agreement, unchanged usable names, and rejected
wrong-map/unknown-actor/unknown-raw overrides. Focused native scanner, route
worker and retained FF1 Ship checks pass, including zero worker-native violations.
All6962 NPC/shop records pass12-language normalization/numbering:83544 labels.
The485 service records additionally pass live/snapshot, exact-identity rejection
and preserved-name checks. FF1's nine Ship profiles remain valid. Final focused
group234.217s/498970624B/exit0/children drained, aggregate2GB/25%CPU/one worker.
Canonical build.bat now also checks the primary identities/role data before the
six actual naming fixtures. Source/report/resource identities are frozen in
build_check/shop_npc_audit_20261102/completion.json.
FF1 canonical/smoke/security/guarded installation Closed21:57UTC in
DLLb62ed342de0f6896; [exact delivery](SUNKEN_SHRINE_LOOP_20261102.md).
Other-game installation and natural confirmation remain separate.
