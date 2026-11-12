# Boss animation speech in FFIV, FFV and FFVI

The FFIV log at 14413–15138 shows Barbariccia's attacks and the
wind warnings after attacks hit the barrier. Her encounter script
changes the sprite on abilities 318 and 317 without a matching
message. The later mode reset on 319 carries the normal sprite.
This is the primary replay case.

## Script evidence and coverage

The installed asset audit examined the 257 FFIV, 383 FFV and 430
FFVI monster AI assets, and the 13, 42 and 52 encounter scripts.
The audit matches master monster, ability and function records
to the encounter branch, native text and presentation producer.
The extracted scripts, messages and native traces are workspace
research outside Git under `tools/boss_visuals_20261112`.

| Game | Confirmed silent visual changes covered in source | Native context retained |
| --- | --- | --- |
| IV | Barbariccia's wind, Mom Bomb's swelling, Octomammoth's silent tentacle losses, Cagnazzo's water receding after the wave, Rubicante's cloak, Odin's sword, Asura's faces and the elemental lords' silent replacements. | Mist Dragon has a native message for each form. Cagnazzo's buildup, lightning break and shell defense have native messages. The near-final Octomammoth change and the scripted Rubicante encounter use their native messages. |
| V | Wing Raptor opens or folds its wings. Liquid Flame changes among three distinct images. Sandworm's resurfacing is tied to a verified position change and the native emergence script. | Wing Raptor's counter warning follows an attack. Gilgamesh's morph and Twintania's charge have native lines. Archeoaevis' same-image phase changes carry no visual cue. |
| VI | Ymir and Angler Whelk withdraw and reappear. Chadarnook's recurring switches expose the demon or goddess sprite. Ifrit and Shiva switch places. Opera Ultros moves between positions. | Ymir's short cry stays in the native message reader. Chadarnook's opening exchange and Ultros's first spoken entrance stay with the game. |

## Runtime ownership

On IV and V the deformation callback reads the one self-targeted
native request. The existing body sprite detour verifies that it
applies the expected different sprite to the same body. Each battle
can retain eight copied identity tokens. The party-change completion
checks the old body has left and the replacement body is visible.
Mode changes with the same image and branches with native context
are excluded by their exact native IDs.

VI observes the transition to a verified no-select emergence or
retreat pose for the two head sprites, the two espers and the
dedicated Opera Ultros effect. The Chadarnook script's
native alpha write identifies the revealed body. Its shadow write
has no cue. The observer resolves that body in the native enemy
array only on the matching script's reveal and limits the read to
nine slots. Sandworm verifies the same enemy body's displacement
and waits for the emergence operation to progress.

All observers run on the owner thread after BattleStart. Battle entry,
battle end and title close the gate. There are no update-loop hooks,
new workers, retained engine objects on workers or game-thread waits.
Cues enter the normal queued speech producer. Every cue has all 12
game locales and follows the existing Auto/manual language setting.

## Verification

The focused policy fixture passes exact forms, mode-only and same-art
rejection, native-message precedence, and all 12 localized entries for
each of 29 cues. The linked build, entry and feedback fixtures,
8,427 native name checks, isolated smoke and exact-payload scan
passed. The installed FFIV, FFV and FFVI payloads match the scan.
[Delivery evidence](../SECURITY_AND_DISTRIBUTION.md#november-12-ffivvi-boss-animation-cues).

Gameplay confirmation is pending. On the next relevant encounter,
listen for Barbariccia's wind rise and fall, Wing Raptor's wings
opening and closing, or Ymir's head withdrawing and emerging.
Each visible change should speak once when it happens. For Mist
Dragon, listen for the game's form messages without an added cue.
The offline checks cover form selection and the native battle-entry
sequence. Audible timing and native-frame responsiveness require gameplay.
