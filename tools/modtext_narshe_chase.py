"""Original native-wait/rescue instructions, all12 supported locales."""
from pathlib import Path
import json,sys
from author_narshe_chase_guide import ROOM_WAIT,LEDGE_START,LEDGE_WAIT,RESCUE
ROOT=Path(__file__).resolve().parents[1]
ROWS={
 ROOM_WAIT:dict(en=ROOM_WAIT,ja='宝物庫のイベントが終わるまで待ち、Shift+Gを押してください。',fr='Attendez la fin de la scène dans la salle aux trésors, puis appuyez sur Maj+G.',it='Attendi la fine della scena nella stanza dei tesori, poi premi Maiusc+G.',de='Warte, bis die Szene in der Schatzkammer endet. Drücke dann Umschalt+G.',es='Espera a que termine la escena de la sala del tesoro y pulsa Mayús+G.',ko='보물 방 장면이 끝날 때까지 기다린 뒤 Shift+G를 누르세요.',zht='等待寶物室劇情結束，然後按Shift+G。',zhc='等待宝物室剧情结束，然后按Shift+G。',ru='Дождитесь конца сцены в сокровищнице, затем нажмите Shift+G.',pt='Espere a cena da sala do tesouro terminar e pressione Shift+G.',th='รอให้ฉากในห้องสมบัติจบ แล้วกด Shift+G'),
 LEDGE_START:dict(en=LEDGE_START,ja='{0}まで進み、崖のイベントが終わるまで動かずに待ってください。',fr='Rejoignez {0}, puis restez immobile jusqu’à la fin de la scène au bord de la falaise.',it='Raggiungi {0}, poi resta fermo fino alla fine della scena sul dirupo.',de='Gehe zu {0} und bleibe still, bis die Szene am Abgrund endet.',es='Llega a {0} y quédate quieto hasta que termine la escena del precipicio.',ko='{0}에 도착한 뒤 절벽 장면이 끝날 때까지 움직이지 마세요.',zht='到達{0}後，原地不動直到懸崖劇情結束。',zhc='到达{0}后，原地不动直到悬崖剧情结束。',ru='Доберитесь до {0}, затем стойте неподвижно до конца сцены у обрыва.',pt='Chegue a {0} e fique parado até a cena do penhasco terminar.',th='ไปถึง {0} แล้วอยู่นิ่งจนกว่าฉากที่หน้าผาจะจบ'),
 LEDGE_WAIT:dict(en=LEDGE_WAIT,ja='崖のイベントが終わるまで動かずに待ち、Shift+Gを押してください。',fr='Restez immobile jusqu’à la fin de la scène au bord de la falaise, puis appuyez sur Maj+G.',it='Resta fermo fino alla fine della scena sul dirupo, poi premi Maiusc+G.',de='Bleibe still, bis die Szene am Abgrund endet. Drücke dann Umschalt+G.',es='Quédate quieto hasta que termine la escena del precipicio y pulsa Mayús+G.',ko='절벽 장면이 끝날 때까지 움직이지 말고 기다린 뒤 Shift+G를 누르세요.',zht='原地不動直到懸崖劇情結束，然後按Shift+G。',zhc='原地不动直到悬崖剧情结束，然后按Shift+G。',ru='Стойте неподвижно до конца сцены у обрыва, затем нажмите Shift+G.',pt='Fique parado até a cena do penhasco terminar e pressione Shift+G.',th='อยู่นิ่งจนกว่าฉากที่หน้าผาจะจบ แล้วกด Shift+G'),
 RESCUE:dict(en=RESCUE,ja='{0}に話しかけてモグを助けてください。',fr='Parlez à {0} pour sauver Mog.',it='Parla con {0} per salvare Mog.',de='Sprich mit {0}, um Mog zu retten.',es='Habla con {0} para salvar a Mog.',ko='{0}에게 말을 걸어 모그를 구하세요.',zht='與{0}交談以救出莫古。',zhc='与{0}交谈以救出莫古。',ru='Поговорите с {0}, чтобы спасти Мога.',pt='Fale com {0} para salvar Mog.',th='พูดกับ {0} เพื่อช่วยม็อก')}
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));langs=set(d['Exit to {0}']);assert len(langs)==12
 for key,row in ROWS.items():
  assert set(row)==langs and all(row.values())
  if'--check'in sys.argv:assert d.get(key)==row,key
  else:assert key not in d or d[key]==row,key;d[key]=row
 if'--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Narshe chase:four instructions/all12 languages; native waits and original choice preserved.')
