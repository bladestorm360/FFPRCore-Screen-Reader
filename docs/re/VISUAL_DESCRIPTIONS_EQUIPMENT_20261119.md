# Distinct equipment artwork audit

Inventory entries in the six games use text icon tags for their shared
category markings. The content masters assign zero to the per-item
`icon_id` column. The distinct equipment here comes from combat art and
its native weapon and guard assignments. Narrative objects in the field
use separate art, tracked in the field or scene audit.

The local equipment worksheets join the combat art keys to their items
and show a nonempty representative frame. The audit IDs refer to the
game and representative row on those sheets. Matches with identical
cropped pixels share a row. The local `ff*-equipment.json` indexes carry
the exact installed keys, assignment names, bundle, frame and fingerprint.
Descriptions of slash arcs describe the displayed attack effect.
Shared guard and bare-hand placeholders need no item-specific description.

The first runtime entry describes the FF1 `wp_ff1_sw02_c00` blade shared
by Broadsword and Great Sword. All four decoded frames show a broad silver
blade, dark edge, gold guard and red grip. The later frames also contain
a slash arc. Ctrl+I adds the common physical description only when the
acting party member's weapon renderer is visible and has a sprite. The
native loader supplies the complete group key with its palette. The
four poses share generic frame names. The held, hidden and different
palette cases passed the battle focus fixture. All 12 localizations are
present. The new entry is outside the installed mod.

## FF1

| Art rows | Artwork | Description |
| --- | --- | --- |
| 1-01 to 1-04 | Axes | The Battle Axe has a gray-violet blade against a brown haft. The Mythril Axe has a blue blade. The Great Axe has a broader heavy violet-gray head. The Light Axe has a splayed pale blue and white blade. Each leaves a broad pale crescent with rose or violet tint. |
| 1-05 to 1-07 | Hammers | Hammer has a gray-mauve square head and brown shaft under a cream and gold arc. Mythril Hammer has a blue head, blue detail on the shaft and a pale violet arc. Thor's Hammer has a heavier angular magenta head and a white-gray sweep. |
| 1-08 to 1-09 | Sasuke's Blade, Masamune | Both have a long narrow silver blade, a black edge, a little gold guard and a purple grip. Thin pale pink blows finish in a violet streak. Masamune is longer and straighter. |
| 1-10 to 1-12 | Knife, Dagger, Mythril Knife | Small upright blades curve into a narrow slash. Knife has a red grip, gold guard and off-white edge. Dagger has a blue grip and bright silver edge. Mythril Knife uses a blue edge, purple grip and bright blue slash. |
| 1-13 | Cat Claws | Narrow silver claws fan from a small gold and red fitting and leave a long red curved trail. |
| 1-14 | Protect Cloak guard | A squarish orange-gold patch of folded cloth flashes brighter along the upper edge. |
| 1-15 to 1-16 | Nunchaku, Iron Nunchaku | Two short linked sticks leave a broad fan-shaped trail. The first has red-orange sticks and a warm salmon trail. The iron version has dark gray-brown sticks and a tan trail. |
| 1-17 to 1-18 | Shared bare hand and shield | A plain fist with a red wrist wrap is the unarmed effect. A rounded orange-gold shield with a broad gray metal rim is shared by the guard entries. |
| 1-19 to 1-20 | Staff, Power Staff | Each has a slender crook and curled round head. Staff is ochre and brown with a pink-violet sweep. Power Staff is pale green with a mint sweep. |
| 1-21 | Crosier | A silver curled head sits on a narrow brown haft with a pale pink-violet trail. |
| 1-22 to 1-24 | Healing Staff, Mage's Staff, Wizard's Staff | A small bright orb tops a slim violet shaft. Healing Staff has a green orb and sweep, Mage's Staff has blue, and Wizard's Staff has coral-red. |
| 1-25 to 1-27 | Longsword, Mythril Sword, Sun Blade | The straight tapered blade has a small gold crossguard. Longsword uses a silver blade and red grip. Mythril Sword has a blue blade and violet grip. Sun Blade is orange and gold with a green grip. Their swing leaves an icy-blue line. |
| 1-28 to 1-29 | Shared rapier art and Wyrmkiller | A fine blade has a small looped guard. The rapier art has a gray steel blade and warm gold grip. Wyrmkiller has a green blade and green-gold grip. |
| 1-30 to 1-32 | Broadsword or Great Sword, Werebuster, Defender | A broad flared blade and short gold guard leave a wide icy-blue arc. The common broad blade is gray with a red grip, Werebuster is amber with a violet grip, and Defender has a bright green blade and blue grip. |
| 1-33 to 1-34 | Rune Blade, Ice Brand | Each has a gold crossguard and violet grip. Rune Blade has a narrow pale turquoise blade. Ice Brand has a wide blue blade and a thicker frosty-blue arc. |
| 1-35 to 1-36 | Scimitar, Falchion | Scimitar has a strong silver curve, looped gold guard and red grip. Falchion has a shorter and broader silver curve with a blue arm on the guard and a violet grip. |
| 1-37 | Flame Sword | The curved blade has a glowing red-orange edge and a gold guard around a red grip. Its attack leaves a broad ring of orange, gold and pale flame. |
| 1-38 | Coral Sword | A mottled rose-red, branching blade projects from a straight brown grip and gold guard, leaving a thin pale violet arc. |
| 1-39 | Excalibur | A straight silver blade has an inset gold stripe, an elaborate yellow crossguard and a red grip. It swings through a broad white and icy-blue arc. |

## FFII

| Art rows | Artwork | Description |
| --- | --- | --- |
| 2-01 to 2-07 | Axe, Battle Axe, Demon Axe, Mythril Axe, Ogrekiller, Poison Axe, Rune Axe | Axe has a dark head with a bright silver edge and brown haft. Battle Axe has a pale green-gray double edge. Demon Axe has a gray and cream-gold head. Mythril Axe is bright blue and cyan. Ogrekiller has a copper head and a narrow blue grip. Poison Axe has an emerald head and purple grip. Rune Axe has a broad golden crescent head and a teal shaft. Each axe's sweep follows its head color. |
| 2-08 to 2-11, 2-14 | Bow, Flame Bow, Ice Bow, Dark Bow, Yoichi Bow | A short recurved bow holds a fine arrow and taut string. The ordinary Bow is brown and cream. Flame Bow is dark red and gold. Ice Bow is bright blue and cyan. Dark Bow is dark blue and rose. Yoichi Bow has violet trim and a narrow yellow arrow. |
| 2-12 to 2-13 | Longbow, Mythril Bow | A horizontal bow curve supports an upright arrow at its center. Longbow has a silver and dark gray arc with gold center accents. Mythril Bow has a steel-blue arc and blue arrow. |
| 2-15 | Cat Claws | Several pale parallel blades or scratch lines streak horizontally from dark blue metal. |
| 2-16 to 2-21 | Knife, Orichalcum, Main Gauche, Dagger, Ripper, Mythril Knife | A short blade curves into a slashing arc. Knife has a thin gray-white edge and small blue grip. Orichalcum uses a thick rose edge and gold fittings. Main Gauche uses a gold arc and a little green hilt. Dagger has a broad pale silver arc and simple gold hilt. Ripper has a purple arc with little glints. Mythril Knife arcs in bright blue and cyan over a red-and-gold grip. |
| 2-22 to 2-23 | Shared bare hand and shield | A tan fist in a blue cuff is the unarmed effect. A rounded rust-orange guard with a pale gray rim represents the shield items. |
| 2-24, 2-28 to 2-30, 2-32 | Spear, Javelin, Demon Spear, Mythril Spear, Trident | A narrow horizontal shaft has a highlight at its blunt end and a small metallic point. Spear has a red-brown shaft and silver point. Javelin has a brown and dark blue shaft. Demon Spear has a pale mint point on a violet shaft. Mythril Spear has a blue point and gold shaft. Trident has a split silver head, ochre shaft and blue end band. |
| 2-25 to 2-27 | Flame Lance, Holy Lance, Ice Lance | Flame Lance leaves two thin horizontal red-orange streaks. Holy Lance leaves two broad white-blue streaks. Ice Lance leaves two trails of angular ice-blue shards. |
| 2-31 | Thunder Spear | A shaft with a pale banded grip flashes around a bright gold electric point and short branching rays. |
| 2-33 to 2-41 | Staves and maces | Each has a short oblique handle and a wide strike arc. Power Staff has a red-orange curved head and warm orange arc. Diamond Mace has a rounded blue-green head with little yellow details. Healing Staff uses a rose-pink trail. Mace has a small gray head and broad icy-blue arc. Wizard's Staff has a small gold pronged head and ochre trail. Mage's Staff has a deep blue head and turquoise arc. Mythril Mace has a spiked bright blue head and cyan arc. Staff has a simple brown cap and gold-brown arc. Werebuster has very pale spikes and a broad pale green arc. |
| 2-42 to 2-43 | Broadsword, Blood Sword | Broadsword has a smooth gray-white edge and green hilt. Blood Sword has a bright red sweeping edge with a tapering dark red blade and grip. |
| 2-44 to 2-46 | Gaia Blade, Defender, Excalibur | Gaia Blade has a thick gold fan-shaped stroke and a bright jagged gold hilt. Defender has a narrow silver blade, green sweep and small orange guard. Excalibur has a wide silver blade, pale blue arc and red and gold guard. |
| 2-47 to 2-48 | Flame Sword, Ice Brand | Flame Sword traces a jagged orange and gold slash. Ice Brand traces a cyan-white curve with little icy stars above a dark blue and silver hilt. |
| 2-49 to 2-53 | Longsword, Ancient Sword, Masamune, Mythril Sword, Sleep Blade | Longsword has a broad silver sweep, steel-blue guard and purple grip. Ancient Sword has a rough-grained brown-gold blade and ochre sweep. Masamune has a narrow gray blade at the edge of a violet arc with a bright downward point. Mythril Sword is cobalt and cyan with a little green-white guard. Sleep Blade has a thin pink crescent and a narrow violet-red hilt. |
| 2-54 to 2-55 | Sun Blade, Wing Sword | Sun Blade shows a concentric gold, orange and white disk with a glowing center. Wing Sword leaves a slim icy blue slash and has a narrow dark blue grip. |

## FFIII

| Art rows | Artwork | Description |
| --- | --- | --- |
| 3-01 to 3-02 | Dual Tomahawk, Dual Haken | Both are double-edged axes. Dual Tomahawk has a short red haft and a gray-green faceted head with pale edges. Dual Haken has a longer silver shaft and smaller dark olive crescent blades edged in yellow. |
| 3-03 to 3-08 | Books and tomes | A square book has a black edge, pale page stripe and dark trim. Book of Fire is plain red, Book of Ice plain blue and Book of Light plain gold. The matching tomes add an angular ring of red-gold, light blue or pale yellow markings respectively. |
| 3-09 to 3-11 | Great Bow, Killer Bow, Rune Bow | A short upright bow has a taut white string. Great Bow has copper-red edges. Killer Bow is plum and wine-colored. Rune Bow has a yellow and pale turquoise curve. |
| 3-12 | Kaiser Knuckles | Pointed pale blue claws extend from a dark gray knuckle guard with large gold fittings. |
| 3-13 to 3-14 | Madhura Harp, Loki Harp | A slender lyre has two high raised curved posts, a rounded base and straight pale strings. Madhura Harp has gold and green wood. Loki Harp is carved in dark violet and gray. |
| 3-15 | Triton Hammer | A short deep blue shaft supports a split blue and white metal head with a bright cyan end. |
| 3-16 to 3-18 | Nunchaku, Tonfa, Sansetsukon | Nunchaku has two orange-gold sticks joined by a short dark segment. Tonfa is a gray and lilac bar with a short black side handle. Sansetsukon has three narrow joined blue sticks with red-orange bands. |
| 3-19 to 3-20 | Mythril Rod, Omnirod | A four-pointed bright star tops a short banded brown and gold handle. Mythril Rod's head is pale cyan. Omnirod's head shines white and silver. |
| 3-21 to 3-24 | Thunder Spear, Wind Spear, Blood Lance, Holy Lance | A silver diamond-shaped point sits on a long jointed straight shaft. The shaft markings are yellow, green, crimson and cyan, in order. |
| 3-25 to 3-30 | Fire, Ice, Light, Golem, Rune and Elder staves | A wide framed jewel tops a short angled staff. Fire has a red and orange jewel. Ice has a deep blue jewel with white facets. Light has a yellow-white center. Golem has a silvery gray head and olive shaft. Rune has a cyan and cobalt center with an olive shaft. Elder has a bright green star inside a golden frame and a dark brown shaft. |
| 3-31 to 3-36 | Golden Sword, Wightslayer, Serpent Sword, Tyrfing, Royal Sword, Break Blade | Golden Sword is a gold blade with a rounded bright red and gold guard. Wightslayer has a straight crimson blade and gold crossguard. Serpent Sword has a curved aqua blade, Tyrfing a heavier curved violet blade, Royal Sword a straight lilac-white blade with an ornate rounded gold guard, and Break Blade a slim pale yellow blade with a dark ochre grip. |
| 3-37 to 3-38 | Onion Sword, Boomerang | Onion Sword has a ridged diagonal reddish blade or grip with a tiny gold hook. Boomerang bends into an L with a warm brown middle and blue-white tips. |
| 3-39 | Shared arrow | A common narrow arrow has a straight brown shaft, a little orange-gold point, a silver band and a split pale tail. |
| 3-40 to 3-42 | Faerie or Wyvern Claws, Hellish Claws, Cat Claws | A small hooked gauntlet tips narrow claws beside a dark grip and gold blocks. The claws use bright green, red-orange and orange-yellow blades respectively. |
| 3-43 | Battleaxe | A small axe has a short golden haft and a broad pale-lilac blade edged in mint. |
| 3-44 to 3-46 | Diamond Bell, Earthen Bell, Rune Bell | Diamond Bell has a small golden flared head and a short orange shaft. Earthen Bell has a wide yellow head and pale triangular face. Rune Bell has a mint and white head with a tiny gold knob. |
| 3-47 to 3-48 | Bow, Yoichi Bow | A narrow bronze or gold bow has a white angular string. Yoichi Bow shows a more irregular curve with bright yellow edging. |
| 3-49 | Morning Star | A small rounded bright-blue spiked head hangs from a short dark chain with a gold and orange handle. |
| 3-50 | Lamia Harp | A broad gold lyre with tall capped arms holds four pale vertical strings above a thick brown-gold base. |
| 3-51 | Thorian Hammer | A pale violet hammerhead has a turquoise crossing band at the top and a gold corner detail. Its dark shaft is edged with gold. |
| 3-52 to 3-53 | Shared curved katana, Kotetsu | Ashura, Kiku-Ichimonji and Masamune share a fine pale mint and violet curved slash. Kotetsu has a straight narrow steel-lilac blade, orange crossguard and dark grip. |
| 3-54 to 3-57 | Shared dagger art, Mythril Knife, Orichalcum, Air Knife | A small blade has a short diagonal handle. The shared dagger uses lilac-white metal and gold fittings. Mythril Knife has a blue blade. Orichalcum has a hot pink blade and blue fittings. Air Knife leaves a pale mint streak shaped like a crescent. |
| 3-58 to 3-60 | Fire Rod, Ice Rod, Light Rod | A four-pointed star with a bright white center caps a short gold and brown shaft. The star glows orange-gold, cyan-blue or yellow-white, in order. |
| 3-61 | Staff | A rounded white-and-blue gem sits inside a mint ring on an angled orange and brown shaft. |
| 3-62 to 3-69 | Swords | Longsword has a slim pale lilac blade and short gold grip. Mythril Sword uses a bright blue edge. Ancient Sword has a wider irregular pale blade with a mint center. Blood Sword has a long hot pink blade and red-black guard. Defender and Excalibur share a wide lilac and mint blade with gold grip. Freezing Blade has a broad blue-violet body and frosty white edges. Ragnarok has a long straight striped pale blade with a gold drop-shaped guard. Salamand Sword has a yellow-orange flame instead of a smooth blade edge. |
| 3-70 to 3-71 | Moonring Blade, Shuriken | Moonring Blade is an angled blue and white oval ring with a dark opening. Shuriken is a compact four-pointed steel-blue star with a dark center. |

## FFIV

| Art rows | Artwork | Description |
| --- | --- | --- |
| 4-01 to 4-06 | Arrow palettes | A long straight narrow shaft has a tiny metal head and two pale tail feathers. The shafts are navy blue, charcoal, violet, green, dark red and burnt orange. The violet arrow has a green head accent, and the others have yellow or orange accents. |
| 4-07 to 4-10 | Dwarven Axe, Ogrekiller, Poison Axe, Rune Axe | A short diagonal haft bears a broad curved pale edge and dark rear spur. Dwarven Axe has brass and reddish colors above a gray handle. Ogrekiller uses blue and cyan. Poison Axe has a violet blade and green handle. Rune Axe has a green and yellow blade and gold handle. |
| 4-11 to 4-17 | Bow palettes | A short bowed frame has angled ends and a contrasting taut string. The first is brown with a pale gray string. Crossbow is blue and pale gold. Great Bow is gray with an ochre string. Killer Bow is violet and lime. Elven Bow is green and yellow. Yoichi's Bow is red and gold. Artemis's Bow is orange-red and pale yellow. |
| 4-18 to 4-21 | Fire Claw, Ice Claw, Thunder Claw, gray-blue claw | A low knuckle fitting has short pale claws and squared fittings. Fire Claw has red metal and gold offsets. Ice Claw is cyan and cobalt. Thunder Claw is yellow with lavender bands. The remaining art has slate-violet metal with gold and white fittings. |
| 4-22 to 4-24 | Dreamer's Harp, Lamia's Harp, gold and blue harp | A triangular harp has an upright column and vertical strings beneath a sloped arm. Dreamer's Harp has an orange frame and golden strings. Lamia's Harp has a magenta frame and golden strings. The other harp uses pale blue strings and a gold-orange frame. |
| 4-25 to 4-28 | Mythril Hammer, Gaia Hammer, Wooden Hammer, gray hammer | An angled handle holds a small double-sided hammerhead. Mythril Hammer is silver-lilac with a green handle. Gaia Hammer has gold fittings and a blue handle. Wooden Hammer has an orange-red head and golden shaft. The remaining hammer has a silver-gray head and red handle. |
| 4-29 to 4-33 | Ashura or Kunai, Kotetsu, Kikuichimonji, Murasame, Masamune | Long narrow blades have compact, angled guards. The shared art is deep blue with a gold guard. Kotetsu is magenta with green below. Kikuichimonji is yellow-green with a cobalt guard. Murasame is ice blue with a blue guard. Masamune is steel-gray with an amber guard. |
| 4-34 to 4-37 | Knives | Knife and Mythril Knife share a short lilac-white blade and green guard. Dancing Dagger has a golden-orange blade and yellow guard. Another knife has a lilac-white blade with a yellow guard. Mage Masher has a narrow bright green blade and gold-green guard. |
| 4-38 | Bare hand | A square tan fist and long red sleeve form the unarmed effect. |
| 4-39 to 4-46 | Rods | A short diagonal shaft supports a squared bright gem. Rod is brown and gray with an amber head. Ice Rod is blue and cyan. Flame Rod is red and gold. Thunder Rod has yellow and blue. Rod of Change has lime and green. Fairy Rod has red-orange and amber. Stardust Rod has pink and gold. Lilith Rod has brick red, scarlet and gold. |
| 4-47 to 4-54 | Spears and lances | A slim shaft has a bright small faceted point and angled wings or flanges. Spear has a gold shaft and white-gray point. Wind Spear has green shaft and violet point. Fire Lance has orange and yellow shaft and an orange point. Ice Lance has cobalt shaft and pale cyan point. Holy Lance uses blue and lilac steel. Blood Lance has a gold haft and red tip and shoulder. Gungnir has a gold shaft and violet-blue head. Wyvern Lance has a blue shaft and narrow yellow head. |
| 4-55 to 4-61 | Crooked staves | Each slim staff bends into a small rounded raised top. Staff is colored brown, Healing Staff olive-gold, Mythril Staff violet-blue, Power Staff fiery orange, Kinesis Staff yellow and white, Sage's Staff red, and Rune Staff green. |
| 4-62 to 4-69 | Broad swords | Light Sword and Sword of Legend share a yellow blade with blue fittings. Ragnarok is pale lilac with a bright green guard. Flame Sword has a red and orange blade with gold. Icebrand is blue and white. Several swords share a vivid pink-white blade with gold hilt. Blood Sword is red and white with a brighter yellow guard. Ancient Sword has a mottled amber blade and pale gray guard. Gorgon Blade has a pastel violet blade and green fittings. |
| 4-70 to 4-72 | Dark Sword, Shadowblade, Deathbringer | Straight narrow dark-edged blades are blue with a gold guard, yellow-green with a blue guard, and red with gold guard, respectively. |
| 4-73 to 4-75 | Boomerang, Moonring Blade, shuriken | Boomerang has a sharp bent dark orange body with gold-brown edges. Moonring Blade forms a long olive and yellow ring. Fuma Shuriken and Shuriken share a four-pointed gray-white star. |
| 4-76 to 4-80 | Whip, Chain Whip, Blitz Whip, Flame Whip, Dragon's Whisker | A long thin lash curls into loops. The handle and lash are cream and brown-orange, gold and silver, violet and yellow, yellow and red-orange, and yellow and green, respectively. |

## FFV

| Art rows | Artwork | Description |
| --- | --- | --- |
| 5-01 | Shared shield | A small oval guard has a brilliant blue-white center and gray metal rim. Several shield items share it. |
| 5-02 to 5-06 | Ogre Killer, Death Sickle, Poison Axe, Rune Axe, Titan's Axe | Ogre Killer has a pale green curved blade, gold-brown handle and side hook. Death Sickle has a long dark gray hooked blade and white shaft. Poison Axe has two hot pink crescent blades on blue metal. Rune Axe has gold crescent edges and violet shaft. Titan's Axe has dark gray crescent blades and pale shaft. |
| 5-07 | Tinklebell | A pointed rose-pink bell has a pale triangular face and short dark blue handle. |
| 5-08 to 5-10 | Flame Bow, Elven Bow, green-framed bow | A short curved bow has a taut pale string. Flame Bow has a steel-blue arc. Elven Bow has a dark pale-violet frame edged with mint and a yellow string. The third uses bright mint and pale yellow in the curve. |
| 5-11 | Flail | A short orange-gold handle joins a bent dark chain carrying a small violet-blue ball with bright projections. |
| 5-12 to 5-13 | Silver Harp, Apollo's Harp | A two-post lyre holds straight pale strings. Silver Harp has pale cyan framing, violet shadow and orange crossbars. Apollo's Harp has a deep red frame with a pink-white set of strings. |
| 5-14 | Gaia or Mythril Hammer | A bright blue and gray double-sided head has a little gold badge and a gold-orange handle. Both names use this art. |
| 5-15 | Assassin's Dagger | A short red-black weapon angles up into a narrow red and pink-edged tip, with a triangular dark hook under the grip. |
| 5-16 | Bare hand | A gray-white blocky fist is the shared unarmed effect. |
| 5-17 to 5-19 | Shared rod, Poison Rod, Lilith Rod | A diagonal shaft supports a four-pointed star. The shared star is white-mint above a brown and gold shaft. Poison Rod has paler mint points with a white center. Lilith Rod has a pink star and blue shaft. |
| 5-20 | Gold guard effect | An angular gold shape has two long prongs above a broad tapered base and a narrow loose point at the side. |
| 5-21 to 5-25 | Spears, lances and trident | The first three long gold shafts have pale violet, icy blue and green spear points respectively. Dragon Lance has a red shaft and pink point. Trident has several bright blue and white prongs at the end of a gold shaft. |
| 5-26 to 5-28 | Healing Staff, Staff of Light, shared Judgment and Sage's Staff | A short golden-brown handle stands under a faceted ringed head. The heads glow ice blue, gold-yellow and pale emerald, in order. |
| 5-29 to 5-31 | Coral Sword, Sleep Blade, Rune Blade | Coral Sword has a narrow rose and white blade with a cobalt guard. Sleep Blade has a wide checkered blue-white blade and gold guard. Rune Blade has a gold and white blade and muted lilac-green hilt. |
| 5-32 to 5-34 | Thrown weapons | The first is a narrow silver-blue twin-tipped shape with a small central gold band. The second is a bright red-orange ring with dark opening. Fuma Shuriken is a cyan and blue four-pointed star with curved edges. |
| 5-35 to 5-39 | Whips | A cropped curved lash has a short colored handle. Beast Killer and Whip share a pale green lash and gold handle. Chain Whip has a blue lash. Blitz Whip has a yellow lash. Fire Lash is orange and red. Dragon's Whisker is pink-red with a red handle. |
| 5-40 to 5-41 | Crossed blade art | Both frames show two crossed pale blades with dark green outlines. The first has a small gold end, while the second has a larger gold guard above a more symmetrical pair of blades. |

## FFVI

| Art rows | Artwork | Description |
| --- | --- | --- |
| 6-01 to 6-02 | Metal Knuckles, Venom Claws | Three parallel silver fingers project from a dark teal-gray knuckle guard. Metal Knuckles has a gold-tinted side plate. Venom Claws has a dark olive side plate. |
| 6-03 to 6-04 | Mythril Claws, Burning Fist or Dragon Claws | A blunt hooked dark claw curves out in three segmented silver-gray tips. Mythril Claws has a copper wrist marking and the shared second set has a gold wrist marking. |
| 6-05 to 6-06 | Kaiser Knuckles, Tigerfang | Pale angular claws fan around a small red-brown knuckle pad. Tigerfang uses more gold and warm yellow at the tips. |
| 6-07 to 6-09 | Shuriken, Pinwheel, Fuma Shuriken | Shuriken has four small mint-white points. Pinwheel has four vermilion and pink points and a hollow center. Fuma Shuriken is a wide dark mint and gray four-pointed star. |
| 6-10 to 6-14 | Narrow blade poses | The five shapes are a narrow vertical white-edged blade and gold guard, a wider vertical steel blade and gold guard, a thin horizontal gold shaft with a pale pointed tip, a wide horizontal silver-green blade with a gold guard, and a very narrow gold vertical piece capped by turquoise. |
| 6-15 to 6-16 | Shared flail, Bone Club | Chain Flail and Morning Star show a pointed dark gray ball on a linked gray chain and red-brown grip. Bone Club has a white and cream branching bone head with a brown wrapped handle. |
| 6-17 to 6-20 | Dice, cards, darts | Dice and Fixed Dice share two cream cubic dice with dark pips. Cards and Death Tarot share a cream and pale green rectangular card with two gold markings. Darts show a white narrow point and gold tail. Viper Darts use an orange-pink branched tail. |
| 6-21 to 6-22 | Kodachi or Kunai, Sakura | The shared slanted short blade has a gray-green edge and gold grip. Sakura is a small, bright pink-red blade with a dark blocky grip. |
| 6-23 to 6-26 | Katanas | Several katanas share the thin silver-green curved steel and gold grip. Ichigeki and Kazekiri share a cobalt edge and blue-gray grip. Kagenui uses a gold-brown blade and red accents. Ashura has bright steel and a brown-red grip. |
| 6-27 to 6-30 | Murasame, Masamune, Murakumo, Mutsunokami | All have longer straight narrow steel. Murasame is pale silver-green, Masamune teal, Murakumo lavender-blue, and Mutsunokami bronze and gold with a little red hilt. |
| 6-31 to 6-36 | Daggers and knives | The common broad dagger has pale steel, green shadows and gold grip. Assassin's Dagger is salmon and pale red. Main Gauche uses pale gray-green with a little orange guard, and Man-Eater adds gold guard tips. Air Knife has a narrower irregular pale edge and small red hilt. Valiant Knife has a bright blue line along its broad steel blade and gold fittings at the wrist. |
| 6-37 | Blue curved blade effect | Two bright cyan lobes sweep around a short pale vertical stem with a little golden band. |
| 6-38 | Zephyr Cloak | A short red cloth hangs in a slanted fold. A small bright green clasp or patch marks the upper corner. |
| 6-39 to 6-42 | Chocobo Brush, Da Vinci Brush, Magical Brush, Rainbow Brush | Chocobo Brush has orange and bright yellow on a gray-green handle. Da Vinci Brush has a cream rounded brush head and a long brown handle. Magical Brush has a narrow hot yellow tip and dark gray handle. Rainbow Brush has a pale gray point and a brick-colored banded handle. |
| 6-43 | Bare hand | A small pale block in a dark green and gray cuff is the unarmed effect. |
| 6-44 to 6-48 | Star-tipped rods | A tiny four-pointed cross lights one end of a straight narrow handle. The shared rod is pale green with a dark olive handle. Flame and Healing rods use rose and red-brown. Holy and Ice rods use pale blue with a blue handle. Thunder Rod has an orange star and white-blue handle. Magus Rod has a bright red star on a dark gray handle. |
| 6-49 | Punisher | A long brown and salmon staff has a small rounded gold grip or handle at one end and a thin gold band at the other. |
| 6-50 to 6-51 | Guard art | Most shields share a shallow pointed red and gold convex guard with a pale rim. A separate narrow gold and olive form has several hooked bright projections. |
| 6-52 to 6-55 | Mythril Spear, Heavy Lance, Golden Spear, Radiant Lance | Straight narrow hafts have small glowing blade tips. Mythril Spear is pale green-gray. Heavy Lance has a longer gold shaft. Golden Spear uses a gold tip and salmon shaft. Radiant Lance is gray-brown with a broad blue-white head. |
| 6-56 to 6-59 | Trident, Holy Lance, Partisan, Impartisan | Trident has three small pale mint prongs on a dark green shaft. Holy Lance has a barbed blue point and gray haft. Partisan has a red and pink point. Impartisan has a broad curved teal crescent and narrow gold shaft. |
| 6-60 to 6-62 | Broad swords | A group of broad angular blades uses silver-white and gray-green. The first has a yellow guard. Crystal Sword and Gladius share an orange guard. Stoneblade has a pale green guard. |
| 6-63 to 6-67 | Excalibur or Ragnarok, Blood Sword, Flametongue, Icebrand, Thunder Blade | Excalibur and Ragnarok share a straight pale-gold edge with a dry red-brown crossguard. Blood Sword has a slim bright red-pink blade. The wider Flametongue glows red and pink, and Icebrand white and blue. Thunder Blade is tan-gold with red guard corners. |
| 6-68 to 6-71 | Soul Saber, Organyx, Falchion, Lightbringer | Soul Saber has a long narrow silver-green blade with pale guard edges. Organyx is a long dark teal blade with a small gold guard. Falchion has a large curving silver-green blade with gold guard. Lightbringer is a long bright cream and gold blade with a little red grip. |
| 6-72 | Ultima Weapon | A straight narrow blade appears as a vertical brilliant blue bar around a pale blue-white core. |
| 6-73 to 6-75 | Rune Blade, Enhancer, Zantetsuken | Rune Blade has a broad pale gold blade and red hilt. Enhancer has a slender mint-white blade and gold fittings. Zantetsuken has a swept charcoal-green edge in a tapering diagonal curve. |
| 6-76 to 6-78 | Additional blade effects | One shows a short curled gold and gray piece with a tiny blue middle. Another shows a pale diagonal blade or handle over two brilliant cyan splash shapes. The last is a very long horizontal silver-green blade with pale gold guard tips. |
| 6-79 to 6-82 | Moonring Blade, Rising Sun, Boomerang, Wing Edge | Moonring Blade is an oval silver-green ring. Rising Sun is an orange and red oval with yellow stripes. Boomerang is a pale steel crescent with gold ends. Wing Edge has a blue and white crescent with slate green tips. |
| 6-83 to 6-85 | Hawkeye, Sniper, unassigned thrown art | Hawkeye has a broad gray-blue winglike head and a gold-edged diagonal shaft. Sniper has a fiery red and yellow blade with a narrow trailing grip. The third crop shows a pale triangular blade on a thin green and gold bent stem. |

The representative frames resolve 414 art keys in the common equipment
masters, with 371 distinct cropped appearances. The FFVI separate
`battle_weapon_asset` table also names two local asset keys, `WP_FF5_B001_C00`
and `WP_FF5_S001_C01`, with no matching installed bundle or content
assignment in this audit. No description is assigned to these references.
