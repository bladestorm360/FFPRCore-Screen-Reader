# Keyboard remapping across all six games

Open mod settings with the configured menu key (F8 by default), then Controls >
Keyboard bindings. Every registered mod command has a named entry, including
global reports, field navigation and waypoints, battle and five party checks,
shop/status/bestiary/results/information navigation, game-specific timers,
mod-menu controls, confirmation/text-editing controls, and binding cancellation.
Controller mappings remain independent. No keyboard mod mode is added.

Choose an action and use Capture keyboard input, or choose its Key and Modifiers
with the setting arrows. Capture waits for released keys before accepting a
fresh press, includes Shift/Control/Alt, and speaks the current cancel chord.
The cancel chord has its own editable entry and always retains an assigned key.
Reset binding restores the original chord; Reset all bindings restores the
catalog defaults. Other actions can be unbound. Binding IDs and per-game
settings remain unchanged; existing custom mappings are retained.

Letters and Brackets and backslash have separate navigation entries. Their
custom mappings persist independently. Switching layouts enables only that
layout’s rows. Shared commands are checked against both profiles when edited,
including the inactive one, so a later layout switch cannot silently introduce
an accepted conflicting edit. Mutually exclusive profiles may reuse a key.

New assignments are checked against other mod commands in overlapping contexts
and against the game’s current keyboard configuration. A modifier does not make
a native game key free: the game checks that key independently of modifiers.
Assignments also check configured native modifier keys. A rejected assignment
keeps the previous binding. If the game configuration is unavailable, the picker
stops and explains the failure; it does not silently cycle to Unbound.

Existing approved default chords deliberately share some native controls, such
as game-toggle reports and information-buffer navigation. Restoring these exact
original chords preserves that established behavior. The native check applies
to new gameplay-context assignments; exclusive mod-dialog controls can use
native gameplay keys because those dialogs already own all input. Native game
configuration changes are read on the next settings entry; the mod never edits
the game’s bindings. This feature does not police edits made inside the game’s
own configuration screen.

Battle party checks retain their five battle-only slots and their existing
behavior in item/magic submenus. The same physical punctuation/M/N keys keep
their field functions outside battle. Native2/3 shoulder equivalents remain
available. Default context overrides are retained; new overlapping assignments
are rejected. Character typing remains text input, rather than a remappable
alphabet.

The interception toggle uses its effective saved binding, F9 by default. The
former F9 key stops toggling after reassignment and can be captured for another
action. Recovery remains available while ordinary game-context mod shortcuts
are disabled. Capture and text input own their keys first. The settings help
speaks the current recovery chord. A remapped key remains consumed through
modifier release and context changes until its primary key is released, so it
cannot become an unintended native action halfway through a chord.

## Source and ownership

`input/bindings.cpp` remains the catalog, persistence, capture and conflict owner.
`input/input_manager.cpp` dispatches registered commands using effective chords;
`ui/mod_menu.cpp` owns the existing keyboard/controller editor. The early
recovery toggle now uses the catalog. Binding-capture cancellation is also
registered and editable. Standalone modifier actions cannot also require
themselves as modifiers or overlap a chord using that modifier in the same
context. Dispatch and native ownership use the same normalized modifier state.

`input/native_keyboard.cpp` observes the current InputListener/KeyConfig once
when settings opens. It reads the existing singleton without creating one,
selects keyboard device0, and copies the current dictionary/list key values.
It never calls a native setter or transfers a pointer to a worker. Missing,
oversized, unsupported or malformed state remains unknown. Observation is
restricted to the native owner. Off-owner calls neither read nor mutate its cache.

Bounds are8 device slots,64 action slots,16 variants per action and128 copied
bindings; names are limited to64 characters. Cache admission does no native
reading during picker iteration or capture. Native input hooks read the prepared
keyboard-ownership bit rather than scan the binding catalog for each callback.
Ownership is prepared by the existing input tick/context changes, retaining a
bounded109-key release ledger. No hook, timer, worker or background native read
is added.

Exact native source evidence for all six games is external at
`tools/keyboard_bindings_20261104/ffN/`. Source hashes and factual key numbers
are recorded in [keyboard_keycodes_20261104.json](keyboard_keycodes_20261104.json)
and `build_check/broader_audit_20261104/keyboard_native_provenance.json`.
All six InputKeyCheck overloads directly read Unity key state independently of
modifiers. Their InputListener getter and KeyConfig.SetKey establish the copied
dictionary ownership. `native_keycodes.h` is generated factual metadata, not
recovered game implementation. All109 mod key mappings agree across all six
declarations; F16–F24 have no legacy Unity equivalent and remain valid mod keys.

Eight new capture/feedback/help texts have all12 supported translations. Binding
action and scope labels use the existing translation catalog; physical key
names retain the established keyboard legends. Both navigation-profile labels
now use their localized names in the editor.

## Verification and delivery

Actual binding-store, keyboard/controller dispatcher, settings editor and all ten
native input detours are exercised offline. The native-reader fixture uses
bounded fake collections and the production reader/checker. It covers current
game changes, unknown/malformed/oversized input, modifiers, exclusive dialogs,
owner abstention, release/context handoff, cached admission and remappable
capture cancellation. Actual input tests cover effective recovery while
interception is disabled, old-key release, F9 capture and text-entry priority.
Both custom profiles, inactive-profile conflicts, settings persistence, capture,
focus seeding and existing controller/party contexts are retained.

The initial native fixture had a constructor-brace/name ambiguity compilation
failure, corrected before runtime checks. A settings test initially expected the
error to be the final spoken line; actual behavior correctly explains the error
then repeats the retained value. The test now checks the complete feedback episode
and unchanged binding. Failed receipts remain preserved, not overwritten.

Exact final source/report/executable/resource identities belong to
`build_check/broader_audit_20261104/keyboard_completion.json` when closed. Capped
offline timing does not establish natural frame or audible latency. Combined
canonical build, smoke/security and guarded installation remain required.
No game was launched or controlled for this feature.

After delivery, when convenient: remap one navigation action in each profile,
try a conflicting game key, and confirm the old binding remains. Change the
recovery key and confirm its old key stops toggling. Restore defaults if desired.
There is no requirement for the user to inspect implementation details.
