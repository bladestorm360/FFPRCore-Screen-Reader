"""The exact facing instruction in every supported language."""
from pathlib import Path
import json,sys
from author_ff5_fire_ship import FACE
R=Path(__file__).resolve().parents[1]
TEXT=dict(en=FACE,ja='{0}を向いて、スイッチを操作してください。',fr='Tournez-vous vers {0}. Actionnez le levier.',it='Rivolgiti verso {0}. Aziona l’interruttore.',de='Blicke nach {0}. Betätige den Schalter.',es='Mira hacia {0}. Acciona el interruptor.',ko='{0}쪽을 바라보고 스위치를 조작하세요.',zht='面向{0}。操作開關。',zhc='面向{0}。操作开关。',ru='Повернитесь в направлении {0}. Нажмите переключатель.',pt='Vire para {0}. Acione o interruptor.',th='หันไปทาง{0} แล้วใช้สวิตช์')
if __name__=='__main__':
 assert len(TEXT)==12 and all(v.count('{0}')==1 for v in TEXT.values());p=R/'data/ff5/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert d.get(FACE)==TEXT
 else:assert FACE not in d or d[FACE]==TEXT;d[FACE]=TEXT;p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Ship:exact-facing instruction in all12 game languages.')
