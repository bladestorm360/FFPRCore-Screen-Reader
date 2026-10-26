# FFIII Vikings' Cove merchant audit

Status: complete, offline source/bake/log comparison. No missing merchant or
production change found; no build/deployment or game launch/control/probe needed.

The user's pathfinder showed Item Shop and Magic Shop, with otherwise generic
pirates. All four native Cove submaps in area13 were inspected: map51/52/53/54,
Map_20091_1 through Map_20091_4. Both extracted object data and the current shipped
bake have exactly two ShopNPC objects, both unconditional entity_default actors
on map53, the inn/shop room.

| Merchant | Native entity | Product group | Baked layer/cell |
| --- | ---: | ---: | --- |
| Magic Shop | 5 | 17 | Ground16,10 |
| Item Shop, pirate sprite | 6 | 16 | Ground15,7 |

The innkeeper is ordinary scripted NPC7, also a pirate sprite, using script602
(sc_sty_20091_3). It is not a hidden equipment merchant. Product groups16/17
contain five item and seven magic entries respectively; no weapon/armor merchant
or additional product group is assigned to any Cove actor. Other pirate/dancer/
boss scripts contain dialogue/scene behavior, no merchant action or shop-opening
script. The inn's companion SysCall is not a shop opener. Story groups change
dialogue actors, not the two unconditional merchants.

Source locators are ../tools/mapdump/ff3_objects.jsonl, ff3_scripts.jsonl and
ff3_master/{map,script,product,product_group}.txt. The actual packed FFPRMAP1 v2
bake was decoded using baked_map.cpp's56-byte SubmapRec/34-byte ObjectRec layout;
both (submap,entity_id) shop identities match source exactly. Bake SHA256:
724def7b7ff992c1c32c9454a11149e6d8a2b3f826a7c097fb71137b32520a4b.
The user's own log111019.343/111019.656 speaks Magic Shop and Item Shop;
111014.375 also speaks Inn (Pirate). These agree with the static merchant count.

Evidence hashes and bounded own-log excerpt are in
build_check/vikings_cove_20261025/. No NPC was renamed, no story gate/route was
weakened, and no speculative equipment shop was added. This is a content audit,
not new all-six runtime coverage or a general absence proof for other towns.
