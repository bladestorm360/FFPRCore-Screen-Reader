"""Maintain localized pre-action movement verbs; no game text is modified."""
from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[1]
VALUES = {
    'Fly {0}, then {1}': {
        'en': 'Fly {0}, then {1}', 'ja': '{0}飛行してから、{1}',
        'fr': 'Volez {0}, puis {1}', 'it': 'Vola per {0}, poi {1}',
        'de': 'Fliege {0}, dann {1}', 'es': 'Vuela {0}, luego {1}',
        'ko': '{0} 비행한 후 {1}', 'zht': '飛行{0}，然後{1}',
        'zhc': '飞行{0}，然后{1}', 'ru': 'Пролетите {0}, затем {1}',
        'th': 'บิน {0} จากนั้น {1}', 'pt': 'Voe {0}, depois {1}',
    },
    'Sail {0}, then {1}': {
        'en': 'Sail {0}, then {1}', 'ja': '{0}航行してから、{1}',
        'fr': 'Naviguez {0}, puis {1}', 'it': 'Naviga per {0}, poi {1}',
        'de': 'Fahre {0}, dann {1}', 'es': 'Navega {0}, luego {1}',
        'ko': '{0} 항해한 후 {1}', 'zht': '航行{0}，然後{1}',
        'zhc': '航行{0}，然后{1}', 'ru': 'Проплывите {0}, затем {1}',
        'th': 'แล่นเรือ {0} จากนั้น {1}', 'pt': 'Navegue {0}, depois {1}',
    },
}

if __name__ == '__main__':
    path = ROOT / 'data/common/mod_text.json'
    catalog = json.loads(path.read_text(encoding='utf-8'))
    for key, translations in VALUES.items():
        assert set(translations) == set(catalog['Walk {0}, then {1}'])
        assert all(value.count('{0}') == 1 and value.count('{1}') == 1
                   for value in translations.values())
        catalog[key] = translations
    path.write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Fly/Sail approach phrases: 2 keys, all 12 supported locales')
