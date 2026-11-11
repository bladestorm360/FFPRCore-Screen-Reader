"""One complete-rescue instruction in every supported locale."""
from pathlib import Path
import json,sys
from author_veldt_cave import RESCUE
R=Path(__file__).resolve().parents[1]
TEXT=dict(en=RESCUE,ja='{0}に進んでください。戦闘に勝ち、救助の場面が終わるまで待ってください。',fr='Rejoignez {0}. Gagnez le combat et attendez la fin du sauvetage.',it='Raggiungi {0}. Vinci la battaglia e attendi la fine del salvataggio.',de='Erreiche {0}. Gewinne den Kampf und warte, bis die Rettung abgeschlossen ist.',es='Llega hasta {0}. Gana la batalla y espera a que termine el rescate.',ko='{0}(으)로 가세요. 전투에서 승리한 뒤 구조 장면이 끝날 때까지 기다리세요.',zht='前往{0}。贏得戰鬥，等待救援場景結束。',zhc='前往{0}。赢得战斗，等待救援场景结束。',ru='Доберитесь до {0}. Победите в бою и дождитесь завершения спасения.',pt='Chegue a {0}. Vença a batalha e espere o resgate terminar.',th='ไปยัง{0} ชนะการต่อสู้ แล้วรอให้ฉากช่วยเหลือจบลง')
if __name__=='__main__':
 assert len(TEXT)==12 and all('{0}'in v for v in TEXT.values());p=R/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert data.get(RESCUE)==TEXT
 else:assert RESCUE not in data or data[RESCUE]==TEXT;data[RESCUE]=TEXT;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Veldt cave:one full-rescue instruction/all12 locales.')
