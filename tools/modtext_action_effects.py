"""Localized acknowledgements of the two observed FFIII visual result effects."""
from pathlib import Path
import json

VALUES = {
    'A glowing aura appears around {0}': {
        'en':'A glowing aura appears around {0}','ja':'{0}の周りに輝くオーラが現れる',
        'fr':'Une aura lumineuse apparaît autour de {0}','it':'Un’aura luminosa appare intorno a {0}',
        'de':'Um {0} erscheint eine leuchtende Aura','es':'Un aura luminosa aparece alrededor de {0}',
        'ko':'{0} 주위에 빛나는 오라가 나타남','zht':'{0}周圍出現發光的光環','zhc':'{0}周围出现发光的光环',
        'ru':'Вокруг {0} появляется светящаяся аура','th':'มีออร่าเรืองแสงปรากฏรอบตัว {0}','pt':'Uma aura brilhante aparece ao redor de {0}',
    },
    'Affected': {
        'en':'Affected','ja':'効果あり','fr':'Affecté','it':'Colpito','de':'Betroffen','es':'Afectado',
        'ko':'효과 적용','zht':'受到影響','zhc':'受到影响','ru':'Под воздействием','th':'ได้รับผล','pt':'Afetado',
    },
    'Affected by {0}': {
        'en':'Affected by {0}','ja':'{0}の効果を受けた','fr':'Affecté par {0}','it':'Colpito da {0}',
        'de':'Von {0} betroffen','es':'Afectado por {0}','ko':'{0} 효과 적용','zht':'受到{0}影響',
        'zhc':'受到{0}影响','ru':'Под воздействием: {0}','th':'ได้รับผลจาก {0}','pt':'Afetado por {0}',
    },
}
if __name__ == '__main__':
    path=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
    catalog=json.loads(path.read_text(encoding='utf-8'))
    for legacy in ('Boost takes effect','Draw Attacks takes effect','A glowing aura appears'):catalog.pop(legacy,None)
    for key,value in VALUES.items():
        assert set(value)==set(catalog['Exit to {0}']) and all(value.values())
        catalog[key]=value
    path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Action target feedback:3 keys,12 locales')
