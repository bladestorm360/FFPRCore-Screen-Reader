# F9 recovery and screen-owned controls

October24: implemented; canonical build and offline checks passed, gameplay pending. Shared
by all six games, including native menus.

ControlInterception is persistent, default On. F9 toggles it; the same setting is
F8 > Navigation > Controls > Control interception. Off suspends game-context mod
keyboard/controller shortcuts and lets native controls through. F9 restores the
saved bindings. F8 and mod-owned settings/text/confirmation dialogs remain usable
with exclusive input. Turning Off cancels autowalk. No keyboard mod mode is added.

input_manager checks physical F9 before modal dispatch, after unsafe transition
rejection, with focus/no modifiers. Registered F9 can also be remapped; physical
F9 remains recovery. Ordinary dispatch while Off admits only F8/F9. The controller
router returns to Normal outside the mod menu. input_passthrough caches the flag
on its native owner through Settings.Changed.ControlInterception. Hooks perform
no preference I/O. Persistence uses the existing asynchronous writer.

Both native APIs remain installed: four InputSystemManager and six InputListener
entry points. Off bypasses shortcut/review consumption and forwards originals.
SDL still reaches the game; unhooking would lose the existing SDL/Unity bridge.
Mod-owned modal ownership retains precedence. No native-thread wait is added.

## Shift+I

The generic cache could select Bestiary's top monster-page bar and read only2,3.
Map/Bestiary now pass their serialized keyHelpController to
system::announce_controls_for. An absent bar says No controls displayed, without
falling back to another initialized controller.

key_help_owned.cpp copies at most64 KeyIconController entries from the native contentList. Each uses
serialized KeyIconView.iconTextList (at most16), keyText/keyText2/separateText,
requiring rendered text. Each field retains at most256 units. CaptureBudget
bounds slices between entries, with an8-entry defensive ceiling. No recursive
descendant scan occurs. Native pointers remain on their owner; a worker joins
copied text in its captured locale. New request, scene retirement or hidden rendered bar
cancels publication. The config controls popup keeps its existing buffer.

interception_check uses actual dispatch/router and all ten native detours: Off
forwarding, dialog ownership, early recovery and suspended/restored Bestiary
arrows. Existing bindings, transition, modal, autowalk and results checks remain.
key_help_check covers rendered keys/actions, hidden siblings, absent bars,
close/scene cancellation and native ownership. Fakes do not measure device input
latency or live game frames. See [TESTING](../TESTING.md) for player checks.


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


Final control-bar check: the serialized contentList supplies exact native row
owners, so reading does not depend on prefab child hierarchy. A separately rooted
visible bar is readable even if its controller is inactive. An explicit request
on an already hidden bar says No controls displayed; hiding during capture
cancels stale speech. All-six native member and actual capture fixtures pass.


## Shared generic controls owner follow-up, October24

The original delivery record above is historical. Generic Shift+I now shares
[the bounded published-owner reader](KEY_HELP_20261024.md). Native UpdateView
publications outrank initialized preloads, actual view visibility permits
separately rooted bars, and destruction retires a candidate before dereference.
Map/Bestiary retain explicit owner selection. Combined verification/deployment
status is tracked in that follow-up and project memory.
