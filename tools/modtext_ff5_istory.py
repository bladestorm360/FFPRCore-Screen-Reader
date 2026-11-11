"""One concise source-qualified recovery instruction in all twelve languages."""
from pathlib import Path
import json,sys
R=Path(__file__).resolve().parents[1]
KEY='Take this exit, then request guidance again.'
TEXT=dict(en=KEY,ja='この出口から出て、もう一度ガイドを呼び出してください。',fr='Prenez cette sortie, puis redemandez le guidage.',it='Prendi questa uscita, poi richiedi di nuovo la guida.',de='Nimm diesen Ausgang und fordere dann erneut die Führung an.',es='Toma esta salida y vuelve a pedir indicaciones.',ko='이 출구로 나간 뒤 안내를 다시 요청하세요.',zht='從這個出口離開，然後再次要求引導。',zhc='从这个出口离开，然后再次请求引导。',ru='Выйдите здесь и снова запросите подсказку.',pt='Saia por aqui e peça orientação novamente.',th='ออกทางนี้ แล้วขอคำแนะนำอีกครั้ง')
if __name__=='__main__':
 p=R/'data/ff5/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'));assert len(TEXT)==12 and all(TEXT.values())
 if '--check'in sys.argv:assert data.get(KEY)==TEXT
 else:
  assert KEY not in data or data[KEY]==TEXT;data[KEY]=TEXT;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Istory recovery instruction:all12 supported languages.')
