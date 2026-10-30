"""Maintain native dive/surface availability announcements in all12 locales."""
from pathlib import Path
import json

ROOT=Path(__file__).resolve().parents[1]
VALUES={'en':'Can dive','ja':'潜水可能','fr':'Plongée possible','it':'Puoi immergerti',
        'de':'Tauchen möglich','es':'Puedes sumergirte','ko':'잠수 가능',
        'zht':'可以潛水','zhc':'可以潜水','ru':'Можно погрузиться',
        'th':'สามารถดำน้ำได้','pt':'Pode mergulhar'}
SURFACE={'en':'Can surface','ja':'浮上可能','fr':'Remontée possible','it':'Puoi riemergere',
         'de':'Auftauchen möglich','es':'Puedes salir a la superficie','ko':'부상 가능',
         'zht':'可以上浮','zhc':'可以上浮','ru':'Можно всплыть',
         'th':'สามารถขึ้นสู่ผิวน้ำได้','pt':'Pode emergir'}
if __name__=='__main__':
    path=ROOT/'data/common/mod_text.json'
    catalog=json.loads(path.read_text(encoding='utf-8'))
    assert set(VALUES)==set(catalog['Can land'])
    assert set(SURFACE)==set(VALUES)
    catalog['Can dive']=VALUES
    catalog['Can surface']=SURFACE
    path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Can dive/Can surface: all12 supported languages updated')
