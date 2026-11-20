# Visual description audit
The November 19 gameplay fixes are installed. This audit records the exact art
and scenes that need descriptions, with separate verification for the speech
controls and for localization. The reviewed wording below is working audit
content. The first appearance readers and localized samples are still
outside the installed mod.

## Battle appearance reader

On manual I, after the existing menu details checks, the current enemy
selector reacquires the single target from its rooted native list. It
matches the current cursor index and the last admitted focus identity.
That identity never supplies a live pointer on its own. The battle-ready
gate keeps the reader out of battle entry. No new hook, background work
or per-frame scanning is involved.

The reader uses the current unit's `BattleSpriteEntity`, body mesh, renderer
and `Sprite.name`. The installed enemy Sprite names match the art master
keys for every asset present across the six games. This lets the reader
follow a changed sprite while the cursor stays on the same unit. Use the
current art key, including its palette, to resolve the localized entry.
The FF1 Goblin and Goblin Guard and FFV Wing Raptor's two forms are the
first four enemy entries, each with 12 localizations.

The existing battle focus fixture covers the actual reader with fake
native objects. The build verifies entry gating, both Wing Raptor sprites,
hidden body, cursor change, row replacement, missing wording and all-target
focus. The effects of material-only hiding, including the FFVI painted
form, need separate visibility proof before that content is enabled.

Ctrl+I uses the command actor while a command or targeting panel owns
input, and the actual party actor during execution. Both resolve through
the rooted native party row. A stale focus or an enemy action supplies no
party actor. The body renderer carries the current character status and
job. Its pose Sprite name is shared across outfits, so the initialized
native character asset table resolves the body key. The six installed
`GetAssetName` implementations have the same 70-instruction shape.
The first localized FF1 variants are Warrior and Knight. Focus and
party fixtures passed with both job keys, hidden bodies, actor handoffs
and after-action cancellation. The DLL build verified 67 embedded
resources.

## Direct dialogue appearance reader

Manual I can use the rendered field dialogue and the current message ID
from the native `MessageWindowClient`. Across the six installed binaries,
`MessageWindowClient.Play` is the direct caller of `SetContent`. An active
direct field action and its interaction target identify the speaker. When
the indicator clears, the fallback checks at most 64 rooted field entities
for a unique visible NPC with the exact active talk key. Scripted actions,
non-field windows, duplicate keys without a target and hidden sprites are
excluded from direct attribution. The scripted path has its own exact map,
script, message and actor binding for Garland's first two lines. Both paths
resolve the costume from the rendered sprite's packed atlas texture and use
no new hooks or frame polling. FF1's Cornelia soldier and Garland have
entries in all 12 languages. Remaining cutscene speaker bindings, automatic
scene cues and localized entries are open.

The focused offline build passes the field dialogue, battle focus and
party checks. The dialogue fixture exercises explicit target resolution,
cleared indicators, ambiguous speakers, hidden sprites and windows,
transition and thread gates, and a matching scripted speaker whose map and
script can change. The six-game native name check and byte
verification of 67 embedded resources passed. Field gameplay confirmation
is pending.

## Six-game source inventory

The census reads the installed Addressables bundle names, the game masters and
the local field script and object extracts. The art tables record exact
installed bundles and isolate unresolved or transparent entries. Enemy rows
distinguish alternate palettes and forms. Character outfit
and job variants use their own art records. Equipment records are candidates
for visual review of distinct sprites. Motion scripts include ordinary patrols,
movement and visual actions that native dialogue may already explain. Each
candidate still needs a scene-level review.

| Game | Enemy forms | Battle character art | Field character art | Placed NPCs and shopkeepers | Equipment art | Candidate scripts with motion |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| I | 124 | 12 | 15 | 503 | 39 | 220 |
| II | 129 | 18 | 15 | 854 | 55 | 259 |
| III | 225 | 25 | 43 | 820 | 72 | 435 |
| IV | 248 | 30 | 26 | 1,258 | 87 | 503 |
| V | 301 | 109 | 27 | 1,313 | 71 | 2,001 |
| VI | 365 | 39 | 34 | 2,214 | 90 | 1,737 |
| Total | 1,392 | 233 | 160 | 6,962 | 414 | 5,155 |

The external workspace folder `../tools/visual_audit_20261119` contains the
inventory command, per-game asset and script index, decoded art sheets and
resource receipts. The indexes preserve the asset IDs and the matching local
bundle names. Review NPC placements separately from those character masters.
The same base outfit can be used by different speakers, and a scene can change
an actor's costume. Match each description to the art visible in the current
context. Review openings first within the scene pass, and reuse the existing
coverage of silent boss form changes. [Boss animation coverage](BOSS_VISUALS_20261112.md).

Enemy sprites are now rendered into local sheets for all six games. FFIV
art 242 names palette
C02 in bundle `mn_ff4_148`, whose installed Sprite and Texture2D records
have C00, C01, C03 and C04. The base monster master and the examined battle
form producers do not select 242. The [FFIV enemy art pass](VISUAL_DESCRIPTIONS_FF4_20261119.md)
describes the 247 sprites present in the installed assets.

For FFV, `sc_e_0418` and `sc_e_0419` have full bodies on `Map_20170`.
The same names in `Map_20171_1` declare no segments, flags or instructions.
Keep those two declarations separate from the three executable variants on
that submap. Review the playable original scenes and variants individually.

The [FFII enemy art pass](VISUAL_DESCRIPTIONS_FF2_20261119.md) covers all 129
art records including the separate Toad form. The [FFIII enemy art pass](VISUAL_DESCRIPTIONS_FF3_20261119.md)
covers all 225 rows including the alternate Sea Dragon art.
The [FFV enemy art pass](VISUAL_DESCRIPTIONS_FF5_20261119.md) covers 300
visible sprites and confirms that the remaining art row is transparent.
The [FFVI enemy art pass](VISUAL_DESCRIPTIONS_FF6_20261119.md) covers 355
visible sprites, including the complete Statue of the Gods composition.
Ten FFVI art rows are fully transparent. The individual statue targets
need an encounter position and tier mapping to the composite artwork.

All 1,392 catalog rows are accounted for. The installed assets yielded
1,380 visible sprites, eleven fully transparent entries and one missing
FFIV palette. The external `enemy-coverage.json` records the decoded
alpha and catalog coverage check.

## NPC and party appearance review

Map placement IDs join to `mapobject`. Party costumes also join to
`field_character_asset` and `character_asset` by exact art, with job and
actor assignments kept separate. The contact sheets use the available
front, side and rear field frames, and the normal, ready and victory
battle frames. All six games yielded sheets for 762 field candidates and
233 battle character variants. The field count includes actors with
prop, vehicle and text-rendering sprites.

The [FF1 appearance pass](VISUAL_DESCRIPTIONS_FF1_COSTUMES_20261119.md)
covers its 76 field candidates and 12 battle outfits, including all base
and promoted jobs. The [FFII appearance pass](VISUAL_DESCRIPTIONS_FF2_COSTUMES_20261119.md)
covers its 72 field candidates, nine battle outfits and nine separately
drawn portraits. The [FFIII appearance pass](VISUAL_DESCRIPTIONS_FF3_COSTUMES_20261119.md)
covers 142 field candidates and 25 battle outfits. Its Ranger and Scholar
costumes use different numeric suffixes in field and battle art. The
[FFIV appearance pass](VISUAL_DESCRIPTIONS_FF4_COSTUMES_20261119.md)
covers 166 field candidates and 30 battle art and portrait variants,
including changed outfits for Cecil, Rydia and Kain. The
[FFV appearance pass](VISUAL_DESCRIPTIONS_FF5_COSTUMES_20261119.md)
covers 124 field candidates and 109 battle outfits. Galuf's Mime resolves
to his Freelancer sprite in the native master. The
[FFVI appearance pass](VISUAL_DESCRIPTIONS_FF6_COSTUMES_20261119.md)
covers 182 field candidates and 39 battle art and portrait variants,
including shared guest and moogle art. The installed costume inventory is
accounted for across all six games. The [equipment artwork pass](VISUAL_DESCRIPTIONS_EQUIPMENT_20261119.md)
covers 414 visible art keys and 371 representative appearances, with
shared category icons, guard and bare-hand sprites accounted for. Scene
props and exact visibility during battle use their own presentation context.

## Opening and early scene review

The [opening cue review](VISUAL_DESCRIPTIONS_OPENINGS_20261119.md)
compares the six openings with their native text and records the verified
puppet, art and effect references. Its broader scene inventory includes
6,163 field scripts with visual candidates or scripted presentation calls.

| Game | Opening source to follow | Review state |
| --- | --- | --- |
| I | World `sc_e_0001`, then bridge prologue `Map_40001/sc_e_0006_1` after the early Cornelia events. | Rough-sea and sandstorm calls and two art-backed scene actions reviewed. The bridge prologue's scroll and remaining visuals need comparison. |
| II | `Map_20020/sc_e_0001` and the continuations `sc_e_0001_11` and `sc_e_0001_12` on map 355. | Two initial branches located. Compare the motion, battle entry and native narration of each. |
| III | `Map_40001/sc_e_0001`, Altar Cave `sc_e_0001_1` and the other prologue branches. | The native prologue message and initial Altar Cave sequence are located. The camera shake precedes the dialogue. The dialogue already identifies the fall through the sinkhole. |
| IV | World `sc_e_0001`, followed by `Map_20230/sc_e_0001_1` and the Mysidia flashback `Map_20241_1/sc_e_0001_2`. | Opening formation, flashback assault and crystal effect reviewed. Some shipboard movements still need exact actor binding. |
| V | `Map_20250/sc_e_0001` and `Map_30030/sc_e_0001_1`. | Reviewed the Wind Drake takeoff following the castle dialogue. Audit the other actions and the outdoor continuation against the relevant audio-option branch. |
| VI | `Map_40037/sc_e_0001` continues through `Map_20010`, `Map_20260` and `Map_40000/sc_e_0001_4`. | The Magitek march on map 572 uses three exact armor puppets. Check the new-game and advertisement-demo entry conditions before assigning the runtime cue. |

| Verified source | Scene or appearance description | Speech placement |
| --- | --- | --- |
| FF1 `Map_20170/sc_e_0005_2`. Ten soldier puppets use `mo_ff1_n101_c00`. The first arm-motion group starts at instructions 74, 77 and onward. | Red-uniformed soldiers begin building the bridge, raising and lowering their arms together. | Once at the first visible group movement. The sprite frames show raised arms. The native messages cover the construction dialogue, and later repeats belong to the same work sequence. |
| FF1 Cornelia soldier, field art `mo_ff1_n101_c00`, Idle and arm-motion frames. | A small, square-faced soldier with dark eyes under a gold helmet. Red armor, boots and sleeves are banded in gold. The arm-motion frames show the soldier's bare hands raised. | Appearance candidate for this sprite in dialogue or an event. |
| FF1 Garland, field art `mo_ff1_n038_c00`, map 72 entity 32. | A compact dark-armored knight with sweeping silver horns and a narrow silver-gray visor. Gold and violet bands stripe the chest and shoulders. From behind, the purple cape has a gold border. | Use this field appearance during dialogue. The larger battle sprite has its own appearance under art ID 115. |
| FFII the Emperor, field art `mo_ff2_n003_c00`. | A small pale-faced figure with swept lilac hair, dark eyes and a high pointed collar. He wears a violet mantle and cape with gold trim, a gold chest piece and gold boots. | The normal field costume has a separate description from the armored battle Emperor and the final battle form, art IDs 85 and 128. |
| FFII Palamecian soldier, field art `mo_ff2_n016_c00`. | A helmeted soldier with a pale visor that shades the face, a red plume and blue-gray armor. White shoulder plates and a purple cape flare behind the shoulders. | Appearance candidate for the soldiers used in the Fynn opening. Check any individual battle identity against the equipped art. |
| FFIV `Map_10010/sc_e_0001`. Actor IDs 174–178 use `mo_ff4_v003_c01`. The five placements form a V, followed by simultaneous flight motions. | Five red-and-gold Red Wings airships fly in a V formation, their pale rotors turning. | Once as the formation begins its visible flight, before the interior shipboard dialogue. |
| FFV King Tycoon, field art `mo_ff5_n012_c00`. The takeoff pairs actor 32 with the Wind Drake's head and body. | A bearded man with dark eyes and gold trim across his blue headpiece. A pale-blue cloak flares over a dark blue, gold-banded outfit. | Use the pose and clothing of the visible field sprite. |
| FFV `Map_20250/sc_e_0001`, actors 48 and 50, `mo_ff5_n042_c00`. The takeoff sequences begin at instructions 300 and 305 and move the king with the dragon. | A gray Wind Drake with pale horns spreads its broad teal wings and lifts King Tycoon into the air. | Once after the last castle exchange, at the first coordinated takeoff. The native dialogue explains his destination. |
| FFVI `Map_40000/sc_e_0001_4`, three actors use `mo_ff6_n076_c00`. Instructions 60, 63 and 66 move them to three separate positions. | Three dark blue-gray Magitek walkers advance in a staggered formation, their heavy legs striding below broad metal cabins. | Candidate for the first simultaneous movement after the native text scroll. Confirm the new-game presentation condition. |

The cropped sprites and the exact action positions are verified from local
assets and script extracts. Follow-up is needed for the runtime presentation
signal and speech order. The scene candidates have no in-game narration yet.

## FF1 enemy art

The table uses the installed `monster_asset` row ID and the English battle name.
The IDs refer to the visible art including alternate palettes. Wording comes
from the decoded sprites with nearest-neighbor enlargement.

| Art ID | Battle name | Visible features |
| ---: | --- | --- |
| 1 | Goblin | A squat, pink-skinned figure with large pointed ears, wide eyes and a protruding nose. It wears a red cap with a trailing tip, a rusty-orange tunic and red boots, and holds a hooked silver blade overhead. |
| 2 | Goblin Guard | A squat, tan-skinned figure with pointed ears, bulging eyes and a protruding nose. A trailing blue cap matches its blue tunic and boots. It holds a hooked silver blade overhead. |
| 3 | Wolf | A lean, rusty-orange wolf crouches on four legs. Pointed ears rise above yellow eyes and a long, lowered muzzle. Its long tail curls high over its back. |
| 4 | Warg Wolf | A lean indigo wolf crouches on four legs with a lowered muzzle, a bright yellow eye and pointed ears. Its long, dark tail curls up over its back. |
| 5 | Werewolf | A vivid green wolf crouches on four legs, with a raised, curled tail. The pointed ears, red eye and long, dark-tipped muzzle are low and forward. |
| 6 | Winter Wolf | A chalk-white wolf with icy-blue shading crouches on four legs, its curled tail raised. Its pointed ears frame a red eye and a long, low muzzle. |
| 7 | Lizard | A low dark-blue lizard with four splayed, clawed feet, two large golden eyes and a long, low jaw. Gold stripes mark its curling tail and the swept crests above its head. A green forked tongue protrudes. |
| 8 | Fire Lizard | A low red-orange lizard with four splayed, clawed feet, golden eyes and a long jaw. Its tail curls upward with a gold stripe. A blue forked tongue extends below the blue and gold head crests. |
| 9 | Basilisk | A low green lizard with a red-striped curled tail, four splayed claws and red eyes. Pink crests sweep back above the head, and a pink forked tongue hangs from the jaw. |
| 10 | Hill Gigas | A muscular green giant with pointed ears, a broad nose, a clenched jaw and oversized fists. Purple bands cross its bare torso and wrists. A dark blue cloak flares behind its shoulders as it lifts one thick leg. |
| 11 | Ice Gigas | A muscular pale-blue giant with pointed ears, oversized fists and a grimacing face. Purple bands cross its bare torso and wrists, with a magenta cloak flaring behind one raised shoulder. |
| 12 | Fire Gigas | A muscular orange-red giant with pointed ears, clenched fists and bared teeth. Purple bands and a flaring purple cloak frame its broad bare torso and bent legs. |
| 13 | Sahagin | A hunched teal fishlike creature with forward-reaching, webbed claws and a bent tail. Pale spines fan out along its head and back above a yellow eye and a long snout. |
| 14 | Sahagin Chief | A hunched red fishlike creature with reaching claws, a long snout and a bent tail. Salmon-colored spines fan out along the head and back above a pale eye. |
| 15 | Sahagin Prince | A hunched yellow-green fishlike creature with an open snout, reaching claws and a bent tail. Cream spines fan out behind a bright eye. |
| 16 | Pirate | A tan, crouching swordsman with a green cap, a dark cloth over his lower face and a loose green coat. One boot is braced behind him as he holds a curved silver cutlass forward. |
| 17 | Buccaneer | A crouching swordsman with a blue cap, a dark cloth over his lower face, a blue coat and blue boots. His bare forearms brace a drawn silver cutlass with a curved tip. |
| 18 | Shark | A blue shark with a tall triangular dorsal fin, a pointed tail and swept side fins. Pale gill marks and a lighter underside frame an open jaw lined with triangular teeth. |
| 19 | White Shark | A turquoise shark with a swept tail, a tall dorsal fin and a light green flank. Its white head and underside frame an open mouth with rows of sharp white teeth. |
| 20 | Bigeyes | Two unequal, cream-colored eyeballs protrude from a round orange tangle. Their small dark pupils look forward, and long thin orange tendrils trail from the body. |
| 21 | Deepeyes | Two unequal mint-green eyeballs protrude from a dark blue tangle. A green pupil shows in the larger eye, and thin blue tendrils dangle from the round body. |
| 22 | Skeleton | A pale skull with deep eye sockets sits above a narrow rib cage and spread bony limbs. The figure crouches with a long, pale blade raised horizontally overhead. Green details show near its feet. |
| 23 | Bloodbones | A pink-tinged skeleton with dark eye sockets and exposed ribs crouches with its legs spread. It grips a long, pale sword above its skull. Red accents mark its feet and joints. |
| 24 | Gigas Worm | A red and purple segmented worm curls up with its huge circular mouth open. Long ivory fangs ring the dark throat. Tubular protrusions and hooked ends fan around the head and base. |
| 25 | Crawler | A dark green and blue segmented creature curls up beneath a huge circular mouth. Long yellow-white fangs point inward. Green and blue protrusions curl around its flared head and base. |
| 26 | Hyenadon | A lean blue hound bounds with long legs gathered below it. Pointed ears and orange eyes stand out on its lowered head. Orange markings cross its shoulders and its thin tail curls back over its body. |
| 27 | Hellhound | A lean magenta hound bounds with gathered legs, sharp ears and a pointed muzzle. Gold markings cross its shoulders and head. Its long tail curls loosely over its back. |
| 28 | Ogre | A broad purple humanoid with a heavy jaw, a scowl and thick arms. It wears gold-edged straps and tall gold boots, and grips a large knobbly brown club across its waist. |
| 29 | Ogre Chief | A broad green humanoid with a heavy jaw, a scowl and thick arms. Blue straps and boots frame its bulky torso. It grips a large knobbly brown club across its waist. |
| 30 | Ogre Mage | A broad blue humanoid with a heavy jaw, a scowl and thick arms. Brown straps, wristbands and boots frame its bulky torso. It grips a large knobbly reddish club across its waist. |
| 31 | Cobra | A thick magenta snake with orange markings coils beneath a raised head and curved neck. A yellow eye shows above its dark open mouth and red mouth lining. |
| 32 | Anaconda | A thick green snake flecked with violet coils beneath its raised neck. Its yellow eye and open red mouth face to the side. The long body curls back under the head. |
| 33 | Sea Snake | A long blue and lavender snake twists into three overlapping coils. Its small red eye and open purple mouth face outward from the raised head. |
| 34 | Scorpion | A low red-orange scorpion with large pincers, a dark segmented body and multiple green legs. Its thick tail arcs high over its back and ends in a hooked stinger. |
| 35 | Sea Scorpion | A low dark-blue scorpion with large pincers and pale cyan joints on its legs. Its segmented tail arches overhead and bends into a hooked stinger. |
| 36 | Minotaur | A bulky magenta bull-headed humanoid with a broad muzzle, curved gold horns and clenched fists. A blue strap crosses its bare chest. Its thin tail curls behind widely planted hooves. |
| 37 | Minotaur Zombie | A heavy green and teal bull-headed humanoid with mottled skin, red horn tips and a visible red mouth. It raises its fists beside its exposed chest, its thin tail curled behind reddish hooves. |
| 38 | Troll | A tall green humanoid with long, widely braced legs and huge forearms lifted beneath its face. Its gaping red mouth, exposed teeth and purple spines and shoulder tufts stand out against the green body. |
| 39 | Sea Troll | A tall blue humanoid with long, spread legs and huge forearms gathered beneath its face. A red mouth and white teeth show below pale hair and ragged pale shoulder tufts. |
| 40 | Shadow | A floating, tattered blue hooded cloak curls around a dark, largely concealed face with a red glint. A raised hand or sleeve holds a small glowing blue cluster, and the cloak tapers into long pointed ends. |
| 41 | Wraith | An icy-blue hooded cloak floats above pointed tatters. A yellow glare shows in the hollow hood, and a raised sleeve presents a pale blue and yellow cluster. |
| 42 | Specter | A lime-green hooded cloak floats above ragged points. A luminous green face peers from the shadow of the hood, and a raised sleeve holds a small green cluster. |
| 43 | Ghost | A pale gray-blue cloak and pointed hood float around a dark face with red and blue highlights. A raised sleeve presents a blue cluster. The hem hangs in sharp tatters. |
| 44 | Zombie | A thin, hunched humanoid with green flesh, red eyes and a sunken face. It shambles with both arms bent and raised. A dark green coat hangs in long strips over reddish-brown legs. |
| 45 | Ghoul | A gaunt gray humanoid with red eyes and hollow cheeks, reaching with both hands. A blue-violet coat with pale edges trails in strips around slender legs. |
| 46 | Ghast | A thin blue-skinned humanoid with a red glare and arms raised to reach. Its torn mustard coat flares in strips over reddish-brown feet. |
| 47 | Wight | A narrow skeletal figure with a red glare, brownish skin and raised hands. A long lime-green coat hangs in ragged strips around dark blue feet. |
| 48 | Purple Worm | A massive purple-blue worm coils around the base of an upright, orange-striped trunk. Its wide round mouth has long pale teeth and a dark violet throat. Two pale feelers jut into the air behind its head. |
| 49 | Sand Worm | A sand-colored, ringed worm coils at the base and raises an enormous dark mouth. Red teeth line the opening and red feelers trail behind the head. |
| 50 | Lava Worm | A heavy green, ringed worm coils around its own upright trunk. Its open mouth has a bright red throat and long white fangs, with pale feelers extending from behind the head. |
| 51 | Evil Eye | A single huge pale-violet eye sits inside a wrinkled, sandy oval body. Its small reddish pupil looks to one side. Orange teeth or bars line the narrow underside, and violet vapor curls around the body. |
| 52 | Death Eye | A single huge orange eye sits in a bright green, wrinkled oval. A dark pupil looks to one side. Gold teeth or bars line the underside, and blue vapor curls up around it. |
| 53 | Medusa | A pale humanoid with a hunched, partly concealed body and a mass of red and magenta snakes radiating from her head. Yellow eyes glint among the snakes. Her bent arms frame the draped pink torso. |
| 54 | Earth Medusa | A hunched green and violet humanoid with a dense mass of dark green snakes radiating from her head. Small highlights pick out the eyes. A bent green arm rests in front of her draped body. |
| 55 | Weretiger | A squat, broad-shouldered golden tiger creature with a striped head, red eyes and a huge open mouth. Long white fangs frame its red tongue. Thick forelegs end in broad paws. |
| 56 | Rakshasa | A squat jade-green tiger creature with dark facial stripes, bright red eyes and a roaring mouth. Pale fangs surround its red tongue, and broad paws support its heavy forequarters. |
| 57 | Ankheg | A long armored insect with a red segmented back, many gray-green clawed legs and a raised tapering tail. Green mandibles frame the broad red head at the front. |
| 58 | Remorazz | A long green segmented insect with many blue clawed legs and blue spines along its back. Its narrow tail points upward, and hooked violet claws frame the larger pale-green head. |
| 59 | Lesser Tiger | A golden-orange cat with dark stripes, a curled tail and sturdy paws. It lowers its head, narrows its eyes and bares two long white saber fangs. |
| 60 | Sabertooth | A stocky blue cat with violet stripes, a long curled tail and broad paws. Its eyes glare from a lowered face above two long white saber fangs. |
| 61 | Vampire | A pale, sharp-featured figure with violet hair, a high-collared blue cloak and dark trousers. He leans forward and holds the cloak near his face, its long hem flowing behind him. |
| 62 | Vampire Lord | A pale, angular-faced figure with swept blue hair and a high-collared crimson cloak. He crouches behind one raised arm, the cloak spread behind his dark clothes and boots. |
| 63 | Gargoyle | A crouched blue creature with a horned head, red eyes and a toothy snarl. Large bat-like wings arch over its shoulders. Long arms reach forward with red claws, above bent clawed legs. |
| 64 | Horned Devil | A crouched orange-red demon with pointed horns, a violet face and bright eyes. Great membranous wings arch above its shoulders. Long hands extend forward with blue-violet claws. |
| 65 | Earth Elemental | A tan upper body with long red hair, sharp features and two reaching arms. Below the waist, it fans into curled green shapes like leaves or tendrils. Bright green arcs frame its hands. |
| 66 | Fire Elemental | A red-skinned upper body with swept blue hair and sharp features rises out of yellow and orange flames. Two long arms spread beside it, with fire ribbons curling around the hands and torso. |
| 67 | White Dragon | A crouched blue-white dragon with a long coiled tail, a horned snout and bright eyes. Violet bat wings arch over its back, yellow spines rise behind it, and its bent limbs end in claws. |
| 68 | Red Dragon | A crouched scarlet and pink dragon with a long coiled tail, a horned head and an open jaw. Wing membranes flare above its back, and orange spines and claws catch the light. |
| 69 | Dragon Zombie | A gaunt green-gray dragon with a long coiled tail, red eyes and an exposed, bony head. Ragged dark-wing membranes, red ribs and pale claws give the body a skeletal look. |
| 70 | Green Slime | A bright green gelatinous cap hangs in soft drips from a wide dome. A thinner strand joins the top to a small lower lobe with two white eyes and a tiny pointed drip beneath. |
| 71 | Gray Ooze | An electric-blue, dome-shaped mass droops in wet strands. A hanging lower lobe has two pale eyes and a small red mouth, with a thin blue drip at the bottom. |
| 72 | Ochre Jelly | An orange-red jelly hangs from a rounded cap with thick drips. The lower lobe shows yellow eyes and a dark mouth, and a single red drop trails underneath. |
| 73 | Black Flan | A dark violet-gray jelly has a broad cap that trails into long drips. On the bulb below, two light-blue eyes and a small red mouth stand out. |
| 74 | Black Widow | A large cobalt spider with an oval, patterned abdomen and eight bent, jointed legs. Small orange eyes or markings sit at the front above pale pointed mouthparts. |
| 75 | Tarantula | A large green and yellow spider with an oval patterned abdomen and eight black-edged, bent legs. Magenta eyes face forward above pale pointed mouthparts. |
| 76 | Manticore | A yellow beast with a curled scorpion-like tail, broad brown bat wings with pale tips and a red-framed, snarling face. A clawed forepaw extends below the wings. |
| 77 | Sphinx | A gold-bodied beast with a violet mane around a pale face. Bright green bat-like wings frame its body and a curved green tail rises behind them. A clawed forepaw hangs below. |
| 78 | Baretta | A low red reptilian beast with a heavy spined shell and four stout legs. Its lowered head has a dark eye and a toothy jaw. Lips, toes and shell spikes are tipped with pale highlights. |
| 79 | Desert Baretta | A low olive-green armored beast with pink-white shell spikes and stout clawed feet. Its large lowered head shows a sharp eye and teeth below the broad plates of its back. |
| 80 | Mummy | A stooping humanoid with olive and gray wrappings, a red glare and elongated arms. Purple strips hang from its waist and wrists, and its heavy, wrapped legs are spread apart. |
| 81 | King Mummy | A stooping blue-wrapped humanoid with orange eyes, thick legs and long arms. Pale strips cross its chest and waist and hang downward in ragged ends. |
| 82 | Cockatrice | A small blue and white birdlike creature with raised feathered wings, a narrow pointed head, orange talons and a hooked, dark-edged tail. Its legs and wings spread as it takes flight. |
| 83 | Pyrolisk | A small red and white birdlike creature with raised cream wings edged in scarlet. A narrow red head points forward, and orange talons and a hooked tail trail beneath. |
| 84 | Allosaurus | A tall red dinosaur with an open, tooth-filled jaw, tiny bent forearms and strong hind legs. Its long thick tail curves up behind it, opposite its raised head. |
| 85 | Piranha | A green fish with an enormous red eye, a blunt snout and visible white teeth. Red and turquoise crests rise along its upper body, which tapers into a curved tail with a turquoise fin. |
| 86 | Red Piranha | A reddish-orange fish with a large green eye, a blunt snout and white teeth. Green and yellow fins rise along its back, and its curved tail ends in a split golden fin. |
| 87 | Crocodile | A vivid green crocodile with a long tapered jaw, four clawed feet and a thick scaly tail curled up behind it. Magenta markings stripe its armored back. |
| 88 | White Croc | A pale blue-white crocodile with beige plates down its back, a long low muzzle and a small red eye. Four clawed legs flank the body, and its long tail curves in a hook. |
| 89 | Ochu | A mostly green plant creature with a huge triangular mouth full of pale fangs. Broad magenta petals ring the mouth above dark roots. Two long tendrils curl up, one ending in a small pink tip. |
| 90 | Neochu | A red-orange plant creature with a wide black maw lined with gold fangs. Green petals and yellow roots flare beneath it. Two raised tendrils curl above, one ending in a red headlike tip. |
| 91 | Hydra | Several green, long-necked reptile heads and open jaws coil around a dark ridged trunk. Red crests and little red eyes mark the snarling heads. The narrow lower bodies tangle together. |
| 92 | Fire Hydra | Several orange-red reptile necks and toothed heads coil around a brown striped trunk. Blue spines and crests tip the twisted heads and lower coils. |
| 93 | Guardian | A squat blue-gray mechanical walker with a glowing orange circle in its round chest. Thin jointed arms end in pale claws, two struts angle behind its head, and broad feet brace underneath. |
| 94 | Water Elemental | A tapered turquoise and white vortex with a broad dark opening at the top. Striped spirals taper to a curling point, and pale droplets scatter around its edge. |
| 95 | Air Elemental | A tapered lilac and white vortex with a dark open center and striped spiral sides. Fine pale fragments float around it, and its narrow lower end curls off to the side. |
| 96 | Water Naga | A pale face with an orange forehead mark and long teal tendrils rises from a coiled blue serpent body. Hornlike points frame the head. One raised arm and the thick tail curl above the broad base coil. |
| 97 | Spirit Naga | A pale, masklike face with hornlike points and bronze tendrils sits above a coiled slate-blue serpent body. One arm lifts beside the face, and the thick tail curls into a broad dark coil below. |
| 98 | Chimera | A golden lionlike body with clawed legs carries multiple goatlike and feline heads beneath red wings. A long red serpent tail loops under the body and ends in a separate yellow-headed jaw. |
| 99 | Rhyos | A dark teal and pale-green beast carries multiple feline and horned heads beneath blue wings. A blue serpent tail loops below its clawed feet and ends in a separate light-colored head. |
| 100 | Piscodemon | A hunched humanoid with an orange, many-parted head in a pale robe with brown trim. Long gray tentacles protrude where the mouth would be, a dark cloak frames the shoulders, and one hand holds a staff. |
| 101 | Mindflayer | A hunched violet-robed humanoid with a long pale tentacled face. Broad dark shoulders frame its robed body and one hand holds a crooked blue and violet staff. |
| 102 | Green Dragon | A green dragon crouches low with an elongated red-eyed head and clawed limbs. White-and-gold wing membranes arch high on either side, and its narrow tail curves over its back. |
| 103 | Blue Dragon | A bright blue dragon crouches with a low red-eyed snout and small visible claws. Blue-white wings curve over its body, and a long thin tail arcs back over the wings. |
| 104 | Astos | A crouched, broad-shouldered green humanoid with a skeletal gray face, red eyes and pointed ears. Spines frame the shoulders, bright tassels trail behind the head, and long arms end in sharp red claws. |
| 105 | Crazy Horse | An orange rearing horse with a creamy mane, a single green horn, a raised chest and bent forelegs. Its long tail trails behind with a pale curl and green tip. |
| 106 | Nightmare | A white and lavender rearing horse with a purple mane and a single narrow blue horn. Its forelegs lift high, its blue hooves extend below, and its purple tail curls into a tight spiral. |
| 107 | Lich | A pale skull and rib cage rise above a mass of torn purple robes. Crownlike points and red streamers frame the skull. Bony hands reach outward, and bright yellow-green rings spiral around the lower body. |
| 108 | Kraken | A large purple tentacled figure with a flared head, green facial accents and curling arms. Long violet tentacles hang, coil and loop around the body. A streak of yellow runs down the curled central limb. |
| 109 | Tiamat | A mass of teal and green dragon necks and horned heads twists around a vivid pink-and-teal body. Eyes and teeth jut out in several directions. Spines, dark wing membranes and coiled tails fill the edges. |
| 110 | Chaos | A huge gold-armored demon with horns, sharp claws and heavy bent legs. Leathery green-gold wings span above its shoulders. Dark joints and violet knee and waist ornaments break up the metallic body. |
| 111 | Wyvern | A blue-green flying reptile with elongated jaws, hooked feet and a sinuous curved tail. Tall membranous wings angle upward, and a red tongue projects below the small head. |
| 112 | Wyrm | An orange-red winged reptile with a curved tail, hooked feet and a long, narrow head. Black-and-orange wing membranes rise above its body, and a violet tongue hangs from the jaw. |
| 113 | Tyrannosaur | A tall teal dinosaur with a long curved tail, large hind legs and tiny bent arms. Its raised jaw is open, showing pale teeth and a bright orange mouth. |
| 114 | Soldier | A squat purple-gray mechanical walker with a glowing red circle in its chest. Two bent jointed arms end in clawlike tips, and dark struts angle behind a narrow head with a red eye slit. |
| 115 | Garland | A broad silver-armored knight with a horned helmet, thin dark face slit and gold accents on his massive boots. A layered violet cape flares behind him as he braces a weapon across his front. |
| 116 | Clay Golem | A huge brick-red figure with a small square head, blocky torso and thick, bent limbs. One large, four-fingered hand opens toward the viewer while the other hangs in a fist. |
| 117 | Stone Golem | A huge gray-green figure with a square head, slablike chest and thick, braced limbs. One enormous hand is raised with spread fingers, the other hangs beside its torso. |
| 118 | Iron Golem | A massive blue metallic figure with a broad torso, tiny square head and thick segmented limbs. One hand is raised and spread, the other hangs beside its heavy legs. |
| 119 | Black Knight | A black and silver armored swordsman with a pointed helmet and heavy blue-black cape. He lunges with one knee bent and thrusts a long silver sword diagonally up, its guard marked in gold. |
| 120 | Death Knight | A dark bronze and brown armored swordsman with a pointed helmet and black cape. One leg is stretched into a lunge. A raised hand holds a long violet-edged sword with a gold guard. |
| 121 | Dark Wizard | A broad blue and violet armored figure behind a flat, horned headpiece. Its pale face has small glowing accents. Wide shoulders, angular gloves and braced feet fill out the dark robes and armor. |
| 122 | Dark Fighter | A broad teal and bronze armored figure with a wide, horned headpiece and a pale face. Thick shoulder plates and angular gauntlets frame a crouched stance with bronze legs and muted green boots. |
| 123 | Warmech | A red and dark-gray mechanical walker on two long bent legs. Two large rounded housings jut from its compact body, and a gold-framed circular part with a green center protrudes at the rear. Segmented tubing and pale highlights edge the armor. |
| 124 | Marilith | A many-armed figure with a long coiled purple snake body. A huge spray of pointed gold hair surrounds the pale face and red-pink torso. Curved silver blades radiate around her outstretched arms, and the coiled tail rises behind them. |
