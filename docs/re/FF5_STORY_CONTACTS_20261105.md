# Western Valley, Black Chocobo Forest and Moogle Village

The three Western Valley contact tiles use the same native event, limited
to Boko's local transportation 3. Entering any of them automatically
completes both jumps. Black Chocobo Forest stages recruitment on approach.
The subsequent conversation starts the native flight and return scene.
In Moogle Village, talking to the rescued moogle opens its house and
transfers the party inside. These contacts require no puzzle guide.

Western Valley's copied initializer treated three native flag indices
above 255 as unknown. The matching FFV flag reader and writer add the
segment offset before checking the full flag-array bounds. The offline
bake now represents 417, 418 and 419 as their equivalent bank and index
pairs. No native code, runtime hook or other map-state program changed.
The native jump event still owns actor hiding on the loaded map. The
initializer retains its original reload behavior.

The final story-contact fixture covers all three mounted approaches, the
two goblin receipts, the forest entrance and conversation, and the moogle
house gate and return. See the [fixture](../../tools/ff5_story_contacts_route_check.cpp)
and [source fingerprints](ff5_story_contacts_native_20261105.json).
