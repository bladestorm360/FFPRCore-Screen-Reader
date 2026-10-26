"""Maintain the localized riding instruction; does not modify game text."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
VALUES={'en':'dismount','ja':'降りる','fr':'descendez','it':'scendi',
        'de':'absteigen','es':'desmonta','ko':'내리기','zht':'下坐騎',
        'zhc':'下坐骑','ru':'спешьтесь','th':'ลงจากพาหนะ','pt':'desmonte'}
if __name__=='__main__':
    path=ROOT/'data/common/mod_text.json';catalog=json.loads(path.read_text(encoding='utf8'))
    assert set(VALUES)==set(catalog['land'])
    catalog['dismount']=VALUES
    path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print('Dismount: all12 supported locales updated')
