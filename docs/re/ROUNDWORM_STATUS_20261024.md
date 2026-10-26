# Roundworm room identity and battle status summaries

Status: implementation complete and offline verified; installed to FF1/III/IV/V/VI.
FFII deployment is pending because the canonical running-game guard refused it;
the user has been asked to close it. Gameplay/listening confirmation is pending.
No game launch/control or runtime probe. Existing dirty work is preserved.

## Room and NPC identity

The current user session21044/6627125 speaks Big Horn in map230 and labels the
room Leviathan Big Horn's Room. At8471.031 Ricard calls it a roundworm; at8478.609
the native formation is1 Roundworm. The user confirms the boss identity.

Two different FFII NPCs reuse developer identifier ビックホーン: Tropical Island
Map_30091_6/entity9/script90/asset63 is Big Horn; Leviathan Map_30111_3/map230/
entity10/script99/asset64 is Roundworm. The global translation was valid for
the former but incorrectly reused for the latter and the authored room title.
Source metadata stays outside Git in tools/mapdump/ff2_objects.jsonl; no game
script/source body is copied into this record or repository.

Map_titles230 now uses canonical ラウンドウォーム as the room owner. The game's
MSG_MON_NAME_73 supplies all12 localized boss names, read with official_text.py.
Translation metadata also has exact @map/230/ビックホーン: map-scoped correction
before ordinary fallback. Native property names, remote NPC rows and worker
interaction advice share this correction; raw identity, routes and numbering
remain unchanged. Other maps retain the original Big Horn translation. No new
resource file, hook, control or setting is introduced.

entity_translator.cpp exposes native translate_entity_name_for_map and an owned
snapshot overload. The worker overload suppresses native map tracking exactly
like its existing counterpart. Native callers supply the current map on its
owner; remote and route workers use copied map IDs. Lookup uses prepared immutable
JSON/locale data, with one bounded key construction/lookup and ordinary fallback;
no scene search, new queue, scan, native-pointer transfer or wait.

## H and rendered vitals

The supplied screenshot shows Firion HP1900/1900, MP10 and a separate Attack
command column. The current log at8652.578,8654.328 and8656.921 contains one
utterance: Firion, HP1900/1900, MP10, Attack. This identifies battle_party::read_row
as the extra-command producer, rather than an unrelated native focus event.

read_row now includes commandText only for full ally target speech, preserving
the user's prior requirement to read that selection exactly as displayed. H,
party-slot status and party review omit the command while retaining name,
rendered HP/MP and conditions. All six games share this context distinction.
The existing displayed::character/vitals path already reads current/max HP and
only the visible current MP on this battle screen. Hidden/nonexistent maxMpText
does not supply a denominator. Other screens retain their own displayed values.
No parameter fallback, status inference or text deduplication is added.

## Verification and remaining check

party_esper_check drives the actual H/status action in all six games with the
screenshot's values and a hidden999 maximum-MP field. It expects HP1900/1900,
MP10, no Attack, and repeated requests remain audible. Full ally targeting must
still include Attack from the rendered command column. Existing conditions,
target-owner and display-visibility coverage remains.

The actual translator/NPC audit checks scoped native and snapshot names in all
12 languages and unchanged Big Horn elsewhere. Navigation catalog checks bind
room230 to the canonical name. Names are direct official game text; the existing
localized Room template remains authoritative. Native speech/audio latency and
the corrected installed labels remain gameplay-unconfirmed.

## Build and delivery evidence

Canonical build and all required checks pass:7409 native names, zero missing;
131 used/339 defined actions, zero undefined; party_esper has zero failures.
FFII NPC audit covers854 records in12 languages (10248 labels), including the
map230 correction and unchanged map205 Big Horn. The isolated smoke host resolves
and forwards all47 HID exports and exits normally. All49 embedded data and18
sound resources match the current files. No native game was run or probed.

The10805760-byte DLL has SHA256
0c5721b60b27935db3a5912ac68c54302f9c1c421b00a6196f54b83db822fe9f.
Recovery, log/native-text provenance, source/resource hashes, successful reports
and the two repaired compile/fixture failures are retained outside Git in
build_check/roundworm_room_20261024/. The first compile lacked field_api.h;
the first fixture reused changed screenshot values in an older assertion.
Neither failure is omitted from the evidence archive.

security_predeploy/result.json records the exact22-file staged payload scan:
enabled Defender, definitions1.461.181.0, scan exit0, no matching detections,
unchanged hashes. Exclusion inspection remains administrator-protected;
four DLLs remain unsigned with no available signing certificate. No protection
settings, dependencies, external uploads or publication changed. Deployment uses
the existing process guard; installed-payload and protected-file receipts bind
the final installation separately from this predeployment scan.

deploy_partial.json verifies the complete22-file scanned manifest in each of the
five installed games, with120 protected files unchanged. FFII's DLL remains the
prior verified build, as required by the running-game refusal. A fresh protected
baseline is required after closing FFII before its final copy/receipt.
