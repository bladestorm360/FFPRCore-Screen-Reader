# Istory Falls routing — verified

The main route remains ordinary navigation. The lower spike room has three
independent controls. The third alone opens the two drops to the final cavern.
The replay from all three actual entry points reaches that control in every
layout. The other two affect directional treasure traps. The upper timed
switch stops one waterfall contact for an optional chest. These controls keep
their existing Switch labels and interaction routes. They do not add a puzzle
guide. The timed optional interaction is outside this routing correction.

The original planner treated six waterfall contacts as ordinary walkable
events. A route starting at Ground 16,2 counted eleven south steps through
the current. The game starts the current after nine steps and moves the player
two more. The bake now records six directed same-floor transfers and the real
arrival of each. Normal directions end their walking count at the current and
use the existing localized ride instruction. Routes resume from the actual
arrival. The native event remains responsible for movement.

The four Bottom contacts each carry the controlled player one tile south.
Ground contacts 37 and 38 carry two tiles south. During those moves, native
layer contacts 17 and 45 switch to Bottom. Source qualification follows
Puppet.Move through the native simple movement list, FieldEntity.MoveTo and
the per-cell SquareMoveFinished callback. The layer trigger registers its
perfect-position callback while the actor is moving. Each completed cell
preserves the current vertical position and dispatches that callback. This
qualifies the intermediate layer contact for 37 and the final contact for 38.
The exact native methods and both complete field programs are pinned in
ff5_istory_falls_native_20261105.json. Recovered bodies stay in the sibling
non-Git source capture.

The timed switch hides contact 38 while native scenario358 is set. Its complete
expiry callback clears the receipt, restores the actors and checks the trigger
under the player. The existing guarded-arc mechanism projects only this exact
current. The optional chest behind it reports Event required when a route on
that floor fails with the current running. The switch itself stays reachable.
The route uses the copied native state, with no added timing read or automatic
switch operation. Current duration and actual gameplay remain native behavior.

The existing two upper-floor traps reset the three spike states and land at
Ground 19,13 or 6,18 on the lower floor. The lower final drops reach the existing
Ground 12,36 point after the complete destination callback. Those arrivals and
all earlier gate, trap and guide records are preserved. The bake adds only six
derived movement arcs and their graph points. Source authority, physical replay
and gameplay confirmation are tracked separately in the audit ledger.

The final checks passed 56 assertions over 68 routes, the retained Exdeath and
Desert current checks, and all-six state and route replay. The source checks
passed 110 canonical producer guards, 1362 functional occurrences, 245 contextual
closures and 1754 repository content checks with zero rejections. Preservation
keeps all 655 native room geometries, actors, masters, earlier arrivals and
guides. Four production files change, with 477 others preserved. No new hook
or language key is required. All resource trees drained within the 2 GB and
25 percent CPU limits. Checkpoint files are ff5_IstoryFalls/inputs.json and
istory_falls_completion.json in the audit workspace. The initial probe's
non-native switch approach was replaced by the real approach in final checks.

The V1.1 release replay caught a later regression in the pushback mask. It
marked all six redirecting contacts as impassable even though the bake had
their verified onward landings. The mask now keeps a contact blocked only
when it lacks that exact automatic-current edge. The native timer guard
still controls contact 38. The replay checks the normal exit, optional chest,
and the transition receipt needed to resume from the native landing.
