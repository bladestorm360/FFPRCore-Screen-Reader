# FFII Poison and Venom

October24: catalog correction implemented; canonical build and offline checks passed.
The fresh Poison Toad log proves the issue. At1163447.171 speech says Guy: Venom.
Field Status damage follows at1163457.718/.828; the menu says Venom at1163460.234
and its cure says Venom removed at1163462.562. The user identifies persistent Poison.

| Master | Internal type | Name source | English | Lifetime evidence |
|---|---|---|---|---|
| Condition10 | Poison10 | MSG_SYSTEM_042 | Venom | Probability5,30; no field sound |
| Condition13 | Venom204 | no message; fallback | Poison | Continuation3, icon3, field sound5 |

Enum spelling does not establish the displayed label. Antidote's description
names both venom and poison; Japanese distinguishes 毒 and 猛毒. The existing
FFII Poison override correctly maps type10 to Venom. The common Venom fallback
also said Venom, collapsing both. A separate FFII Venom override now says Poison
(猛毒 in Japanese), in all12 languages. Other games' labels and native lifetime/
cure/damage behavior are unchanged.

battle_util::condition_name and menus::condition_name prefer native messages,
otherwise sharing condition_labels::fallback. Infliction/cure tracking uses the
former; rendered icons and party/target/menu screens use the latter. Translation
therefore corrects every producer. Field damage remains generic Status damage
from the visible flash, without hidden numbers or an inferred condition name.

Exports/provenance: ../tools/poison_status_20261024/ff2/, outside Git. Masters and
English/Japanese tables: ../tools/mapdump/ff2_master/. official_text.py reads all12
installed message tables offline; MSG_SYSTEM_042 and MSG_ITEM_INF_02 establish
the distinction. Log copy: ignored build_check/mysidia_objects_20261024/.

condition_labels_check verifies records/lifetimes and12-locale overrides.
battle_feedback_check uses actual infliction and the real FFII catalog entries,
including separate cure tracking. Existing rendered-icon checks retain disclosure
guards. New speech is gameplay-unconfirmed. No game was launched, controlled or
probed, and no native status/save data was changed.


## October24 delivery and limits

Canonical build and every required offline check pass; isolated smoke resolves
and forwards all47 exports. Native-name audit:7,373 checks, zero missing; action
wiring:131 used/339 defined, zero undefined. All49 embedded data and18 sound
resources match current sources. DLL10793984 bytes, SHA256
 3c8562206a2fe22ca9540a4349cff82ec04c68ee645e34db8f7e5f8df73b5b82.
Installed and hash-verified all six after the user closed FFII. All144 protected
files match the fresh applicable deployment baseline. Recovery, reports,
source/resource hashes and receipts: ignored build_check/mysidia_objects_20261024/.
No game launch/control/probe, dependency install, delegation or release/push.
Gameplay/listening confirmation remains separate and pending; see TESTING.


Persistence proof: native BattleEndRecoveryCondition RVA6B77D0 calls
Condition.get_UntilType RVA A18650. Its6B794E branch retains type3/Continuation
and removes the other conditions. Condition13/type204 is therefore retained;
condition10/type10 is cleared at battle end. The live log independently shows
the former's field damage. IsOutBattle is a separate incapacitation predicate,
not the persistence rule, and must not be used to infer this distinction.
