# Ship Graveyard jump sequence — Closed offline

This audit slice covers the required jump sequence on the Ship Graveyard
exterior. The native controls combine a timed start, several distinct
manual approaches, and a chest that clears the route to the next encounter.
The chest by itself is one ordinary interaction.

The native timer start arms a two-second callback. The callback runs a
three-part jump chain and ends on the southwest shore. The reverse control
uses a different chain. Other jumps depend on the tile and side used to
interact. The two middle deck controls also branch on the player's facing.
Each continued script must be included before deciding its final landing.
The installed game is read only. No runtime probe or gameplay capture is
needed to resolve these source facts.

The old bake lacked the same-floor jumps. Four production route checks
reproduced the failures in both directions. The updated bake adds 12 exact
approach and landing pairs, with the final arrival of each script chain.
The timed crossing leads to the optional southwest chest. The ordinary
route announces that the player should wait at the crossing. The main
guide uses the actual introductory and rest scene contacts, the required
manual jumps, the bridge-opening chest and the encounter. It ends after
the full native encounter callback. The north jump stays unavailable until
that callback returns. All existing terrain and actors are preserved.

The focused replay passed 47 assertions over 26 route legs, including 11
stairs and three jumps in the connected main guide. The source record pins
43 field programs and five native movement methods. Only the exact timed
crossing adds a localized action phrase. It runs during route formatting on
the existing worker. Guidance reuses the Navigation toggle and adds no
game-thread hook, timer observation or automatic input. Actual gameplay
remains unconfirmed. Exact movement methods and the primary source tables
are retained in the sibling non-Git archive.

All-six route worker and guide checks passed. The source verification passed
115 canonical producers, 1362 functional occurrences and 245 contextual
closures. The two enforced verification trees drained within their resource
limits. Combined build and deployment remain pending in the broader audit.
