"""Maintain the observed Guard barrier description in all supported locales."""
from pathlib import Path
import json
VALUES={'en':'A glowing barrier appears around {0}',
 'ja':'{0}の周囲に光る障壁が現れます','fr':'Une barrière lumineuse apparaît autour de {0}',
 'it':'Una barriera luminosa appare intorno a {0}','de':'Eine leuchtende Barriere erscheint um {0}',
 'es':'Una barrera luminosa aparece alrededor de {0}','ko':'{0} 주위에 빛나는 장벽이 나타납니다',
 'zht':'{0}周圍出現一道發光屏障','zhc':'{0}周围出现一道发光屏障',
 'ru':'Вокруг {0} появляется светящийся барьер','th':'ม่านแสงปรากฏรอบตัว {0}',
 'pt':'Uma barreira luminosa aparece ao redor de {0}'}
if __name__=='__main__':
 p=Path(__file__).resolve().parents[1]/'data/common/mod_text.json';c=json.loads(p.read_text(encoding='utf-8'))
 assert set(VALUES)==set(c['A glowing aura appears around {0}'])
 assert all(v.count('{0}')==1 for v in VALUES.values())
 c['A glowing barrier appears around {0}']=VALUES
 p.write_text(json.dumps(c,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Guard visual description:12 complete localized templates')
