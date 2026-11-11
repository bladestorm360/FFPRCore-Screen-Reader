"""Author/check exact, reviewed service names; no native script bodies in Git.

The review manifest freezes native identity and semantic decisions. Runtime uses
only its per-game immutable dictionary keys, never these discovery heuristics.
"""
from pathlib import Path
import argparse
from collections import Counter
import hashlib
import json

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'docs/re/service_npc_labels_20261102.json'
LANGS = ('en', 'ja', 'fr', 'it', 'de', 'es', 'ko', 'zht', 'zhc', 'ru', 'th', 'pt')
# Mod-owned role vocabulary. Personal names and already useful native role
# labels remain in their existing per-game dictionaries.
ROLE_TEXT = {
 'innkeeper': ('Innkeeper','宿屋の主人','Aubergiste','Locandiere','Gastwirt','Posadero','여관 주인','旅店老闆','旅店老板','Хозяин гостиницы','เจ้าของโรงแรม','Estalajadeiro'),
 'priest': ('Priest','神父','Prêtre','Sacerdote','Priester','Sacerdote','신부','神父','神父','Священник','บาทหลวง','Padre'),
 'weapons': ('Weapon Shop','武器屋',"Boutique d’armes",'Armeria','Waffenhändler','Tienda de armas','무기 상점','武器店','武器店','Магазин оружия','ร้านอาวุธ','Loja de Armas'),
 'armor': ('Armor Shop','防具屋',"Boutique d’armures",'Corazzeria','Rüstungshändler','Tienda de armaduras','방어구 상점','防具店','防具店','Магазин доспехов','ร้านชุดเกราะ','Loja de Armaduras'),
 'items': ('Item Shop','道具屋',"Boutique d’objets",'Emporio','Krämer','Tienda de objetos','도구 상점','道具店','道具店','Магазин предметов','ร้านไอเท็ม','Loja de Itens'),
 'magic': ('Magic Shop','魔法屋','Boutique de magie','Negozio di magia','Magieladen','Tienda de magia','마법 상점','魔法店','魔法店','Магазин магии','ร้านเวทมนตร์','Loja de Magia'),
 'white_magic': ('White Magic Shop','白魔法屋','Boutique de magie blanche','Negozio di magia bianca','Weißmagiehändler','Tienda de magia blanca','백마법 상점','白魔法店','白魔法店','Магазин белой магии','ร้านเวทมนตร์ขาว','Loja de Magia Branca'),
 'black_magic': ('Black Magic Shop','黒魔法屋','Boutique de magie noire','Negozio di magia nera','Schwarzmagiehändler','Tienda de magia negra','흑마법 상점','黑魔法店','黑魔法店','Магазин чёрной магии','ร้านเวทมนตร์ดำ','Loja de Magia Negra'),
 'accessories': ('Accessory Shop','アクセサリー屋',"Boutique d’accessoires",'Negozio di accessori','Accessoire-Laden','Tienda de accesorios','액세서리점','飾品店','饰品店','Магазин аксессуаров','ร้านเครื่องประดับ','Loja de Acessórios'),
 'equipment': ('Equipment Shop','武器防具屋',"Boutique d’équipement",'Negozio di equipaggiamento','Ausrüstungsladen','Tienda de equipo','장비 상점','裝備店','装备店','Магазин снаряжения','ร้านอุปกรณ์','Loja de Equipamentos'),
 'pub': ('Tavernkeeper','酒場の主人','Tavernier','Taverniere','Schankwirt','Tabernero','주점 주인','酒館老闆','酒馆老板','Хозяин таверны','เจ้าของโรงเหล้า','Taberneiro'),
 'ninja': ('Ninja Supplies','忍具屋','Fournitures de ninja','Articoli ninja','Ninjabedarf','Suministros ninja','닌자 도구점','忍具店','忍具店','Товары для ниндзя','ร้านอุปกรณ์นินจา','Artigos Ninja'),
 'apothecary': ('Apothecary','薬屋','Apothicaire','Speziale','Apotheker','Boticario','약사','藥師','药师','Аптекарь','เภสัชกร','Boticário'),
 'shopkeeper': ('Shopkeeper','店主','Commerçant','Negoziante','Händler','Tendero','상점 주인','店主','店主','Торговец','เจ้าของร้าน','Lojista'),
}

def sha(path):
    value = hashlib.sha256()
    with path.open('rb') as source:
        for block in iter(lambda: source.read(1024 * 1024), b''): value.update(block)
    return value.hexdigest()

def actor_key(row):
    return f"@actor/{row['map']}/{row['entity']}/{row['raw']}"

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    manifest = json.loads(MANIFEST.read_text(encoding='utf-8'))
    assert manifest['total_services'] == 485
    assert sum(len(review['rows']) for review in manifest['games'].values()) == 485
    changed = 0
    for game, review in manifest['games'].items():
        for path, expected in review['source_sha256'].items():
            assert sha(ROOT.parent / 'tools/mapdump' / path) == expected, ('Changed primary input', path)
        source_objects = {}
        for line in (ROOT.parent / 'tools/mapdump' / (game + '_objects.jsonl')).open(encoding='utf-8'):
            obj = json.loads(line)
            prop = obj.get('props', {})
            identity = (obj['submap'], int(prop.get('entity_id') or 0), obj['name'])
            source_objects.setdefault(identity, []).append((int(obj.get('type') or 0), int(prop.get('script_id') or 0), int(prop.get('product_group_id') or 0)))
        expected_keys = {}
        actual_shops = Counter((submap, actor, raw, script, group)
                              for (submap, actor, raw), variants in source_objects.items()
                              for kind, script, group in variants if kind == 17)
        reviewed_shops = Counter((row['submap'], row['entity'], row['raw'], row['script_id'], row['product_group'])
                                for row in review['rows'] if row['type'] == 17)
        assert actual_shops == reviewed_shops, ('Unreviewed typed shop', game)
        for row in review['rows']:
            identity = (row['submap'], row['entity'], row['raw'])
            assert (row['type'], row['script_id'], row['product_group']) in source_objects[identity]
            if not row['correct']: continue
            # An exact key must not rename a non-NPC sharing those identifiers.
            assert all(kind in (6, 17) for kind, _, _ in source_objects[identity])
            key = actor_key(row)
            texts = dict(zip(LANGS, ROLE_TEXT[row['role']], strict=True))
            assert all(texts.values())
            assert key not in expected_keys or expected_keys[key] == texts, ('Conflicting roles', game, key)
            expected_keys[key] = texts
        path = ROOT / 'data' / game / 'translation.json'
        dictionary = json.loads(path.read_text(encoding='utf-8'))
        for key, texts in expected_keys.items():
            if args.check:
                assert dictionary.get(key) == texts, ('Missing/wrong actor label', game, key)
            else:
                dictionary[key] = texts
        if not args.check:
            path.write_text(json.dumps(dictionary, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        changed += len(expected_keys)
        print(game, len(review['rows']), 'services reviewed,', len(expected_keys), 'exact actor corrections, 12 languages')
    assert changed == 210
    print('Service names:485 reviewed/210 corrected; exact native identities and primary hashes match.')

if __name__ == '__main__': main()
