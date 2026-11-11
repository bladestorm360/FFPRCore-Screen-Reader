"""One later-lever instruction in every supported game locale."""
from pathlib import Path
import json,sys
from author_late_opera import LEVER
R=Path(__file__).resolve().parents[1]
TEXT=dict(en=LEVER,ja='{0}を操作し、戦闘に勝ってください。',fr='Actionnez {0}, puis gagnez le combat.',it='Aziona {0}, poi vinci la battaglia.',de='Betätige {0} und gewinne dann den Kampf.',es='Acciona {0} y gana la batalla.',ko='{0}(을)를 작동한 뒤 전투에서 승리하세요.',zht='操作{0}，然後贏得戰鬥。',zhc='操作{0}，然后赢得战斗。',ru='Используйте {0}, затем победите в бою.',pt='Acione {0} e vença a batalha.',th='ใช้{0} แล้วชนะการต่อสู้')
if __name__=='__main__':
 assert len(TEXT)==12 and all('{0}'in v for v in TEXT.values());p=R/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert data.get(LEVER)==TEXT
 else:assert LEVER not in data or data[LEVER]==TEXT;data[LEVER]=TEXT;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Later Opera:one lever/battle instruction/all12 locales.')
