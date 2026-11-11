# Esper Cave navigation and Thamasa story conditions

The twenty-seventh FFVI story guide follows the initial cave journey after
native276. It covers the two initial Relm approach scenes, the statues, Ultros,
both pit-branch treasure chests and the final Esper meeting. The sightings are
navigation cues, not requirements imposed on the later game scripts; after
actual statue279, a missed sighting never sends the player backwards.

Actual receipts are277/278 for the sightings,279 for the statue information,
280 for the completed battle and281 for the full return to Thamasa. Battle
entry or280 cannot finish the whole guide. The two treasure objectives are
427/4 and428/4, with native receipts148/149; contents are not revealed. The last
physical objective is430/24. Its native scene changes to180 and the full return
callback supplies281. The established ready-state completion owner handles
this receipt without a new hook, native read, timer or worker protocol.

The Ultros step gives the solution to the portrait mechanic: when Relm joins,
use Sketch on the enemy. The exact native battle asset sc_b_0547 owns the
Relm-entry scene, portrait dialogue and subsequent mode-dependent battle exit.
The instruction is supplied before entering that battle, using the existing
story-guide pattern for native special-command encounters. It is classified
as non-navigation guidance; the other cave operations use navigation guidance.
Disabling a type suppresses its current unsolved operation rather than skipping
its native receipt. Ordinary battle commands/targeting are unchanged.

Three bridge controls0543/0544/0545 were missing their manually operated
same-floor connections. Each actual player script performs an absolute Lerp
then two steps South. New source-qualified metadata retains its real control,
native interaction-side mask, current layer and final landing. No collision
grid or physical object is rewritten; the normal worker supplies the route
and player-operated crossing action. Native interaction remains manual.

The three pits0548/0549/0550 return through native PointIn7/8/9, followed by
eight South steps in the actual continuation. The maintained trap data already
had all three correct final landings. They are verified unchanged. An initial
new author duplicated those existing records; the focused one-arc-per-pit
assertions caught this before installation. Only those new duplicates were
removed, preserving the original data and its accepted design. Necessary pit
falls retain the existing minimal trap warning; no trap-free route is claimed.

Thamasa's five remaining discoveries are ordinary native story/information:
boy269 and woman270 are optional scenes, introduction271 includes ordinary
Strago/Relm naming and returns through180/8, and Strago273 joins/recoveries
before the actual412/1 start of the existing burning-house guide. Gung-Ho's
native283 conversation requires active Strago8; the alternate dialogue does
not set283. This changes the conversation receipt, not the validity of routing
to the NPC. No NPC path gate, duplicated rescue guide or Ebot redesign is added.

Authoring and safe extension use tools/author_esper_guide.py,
tools/author_esper_passages.py and the existing owned story model/controller.
Factual esper_native_20261104.json pins119 episode scripts,318 physical facts
and one exact battle asset/bundle. Source-bearing scripts and battle data stay
outside Git. The one new instruction has all12 translations with canonical
localized Relm and the actual rendered Sketch command name. Explicit puzzle
type metadata now covers90 recipes/native policies across all six games.

The completion receipt under build_check/broader_audit_20261104 records exact
current-source checks, preservation, failed attempts and finite resource limits.
Fixtures use representative owned positions and native receipts, not actual
game movement or audible/frame measurements. Combined DLL, smoke/security,
guarded installation and natural gameplay confirmation remain separate. No
game execution, runtime probe, new native hook or input automation is used.

The current full journey passes31 assertions/29 actual-bake legs, including
both treasure branches, all three individually routed bridge approaches and
the full final meeting. Twelve legs describe routes with necessary later pit
falls; five manual phases include the bridge tests. The current model passes
649 physical identities and the current controller passes its retained actual
producer/cancellation/completion checks. Performance numbers from throttled
fixtures do not certify actual game responsiveness. Preservation checks also
retain all26 previous story recipes and the entire prior localized catalog.
