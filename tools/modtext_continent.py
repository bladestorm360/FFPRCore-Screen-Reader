"""Maintain the two geographic names used by reviewed airship travel targets."""
from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[1]
VALUES = {
    'Floating Continent': {
        'en': 'Floating Continent', 'ja': '浮遊大陸',
        'fr': 'Continent flottant', 'it': 'Continente fluttuante',
        'de': 'Schwebender Kontinent', 'es': 'Continente flotante',
        'ko': '부유 대륙', 'zht': '浮游大陸', 'zhc': '浮游大陆',
        'ru': 'Парящий континент', 'th': 'ทวีปลอยฟ้า', 'pt': 'Continente Flutuante',
    },
    'Surface World': {
        'en': 'Surface World', 'ja': '地上世界',
        'fr': 'Monde de la surface', 'it': 'Mondo di superficie',
        'de': 'Oberwelt', 'es': 'Mundo de la superficie',
        'ko': '지상 세계', 'zht': '地表世界', 'zhc': '地表世界',
        'ru': 'Наземный мир', 'th': 'โลกพื้นผิว', 'pt': 'Mundo da superfície',
    },
}

if __name__ == '__main__':
    path = ROOT / 'data/common/mod_text.json'
    catalog = json.loads(path.read_text(encoding='utf-8'))
    for key, translations in VALUES.items():
        assert set(translations) == set(catalog['Exit to {0}'])
        assert all(translations.values())
        catalog[key] = translations
    path.write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Continent travel names: 2 keys, all 12 supported locales')
