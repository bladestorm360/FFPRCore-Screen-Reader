"""Original Vargas instruction; twelve game-supported languages."""
from pathlib import Path
import json,sys
from author_mt_kolts import FIGHT
R=Path(__file__).resolve().parents[1]
TEXT={
'en':FIGHT,
'ja':'{0}と戦ってください。マッシュ一人になったら、必殺技の爆裂拳を選び、左、右、左、決定の順に入力してください。',
'fr':'Combattez {0}. Quand Sabin est seul, utilisez Poing foudroyant dans Blitz : Gauche, Droite, Gauche, puis Confirmer.',
'it':'Combatti {0}. Quando Sabin è solo, usa Colpo iracondo da Blitz: Sinistra, Destra, Sinistra, poi Conferma.',
'de':'Bekämpfe {0}. Wenn Sabin allein kämpft, nutze Faustwirbel unter Blitz: Links, Rechts, Links, dann Bestätigen.',
'es':'Lucha contra {0}. Cuando Sabin luche solo, usa Puños de ira en Blitz: Izquierda, Derecha, Izquierda y Confirmar.',
'ko':'{0}와 싸우세요. 매시 혼자 싸우게 되면 필살기의 폭렬권을 선택하고 왼쪽, 오른쪽, 왼쪽, 확인 순서로 입력하세요.',
'zht':'與{0}戰鬥。馬修單獨作戰時，選擇必殺技的爆裂拳，依序輸入左、右、左，然後確認。',
'zhc':'与{0}战斗。马修单独作战时，选择必杀技的爆裂拳，依次输入左、右、左，然后确认。',
'ru':'Сразитесь с {0}. Когда Сабин останется один, выберите «Кулак ярости» в «Блиц»: Влево, Вправо, Влево, затем Подтвердить.',
'pt':'Enfrente {0}. Quando Sabin lutar sozinho, use Punho Furioso em Blitz: Esquerda, Direita, Esquerda e Confirmar.',
'th':'ต่อสู้กับ {0} เมื่อซาบินต่อสู้คนเดียว ให้ใช้หมัดรุนแรงจากบลิตซ์: ซ้าย ขวา ซ้าย แล้วกดยืนยัน',
}
if __name__=='__main__':
 assert len(TEXT)==12 and all(v.count('{0}')==1 for v in TEXT.values());p=R/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert data.get(FIGHT)==TEXT
 else:assert FIGHT not in data or data[FIGHT]==TEXT;data[FIGHT]=TEXT;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Mt. Kolts:one mandatory combat solution/all12 locales.')
