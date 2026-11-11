"""Four original navigation/party/choice instructions in all twelve locales."""
from pathlib import Path
import json,sys
from author_yeti_guide import ENTRY,FALL,SKULL,MOG
ROOT=Path(__file__).resolve().parents[1]
TEXT={
'en':(ENTRY,FALL,SKULL,MOG),
'ja':('最初の選択肢を選び、洞窟へ飛び込んでください。','スイッチを操作して下の部屋へ落ちてください。','最初の選択肢で魔石を取り、その後の戦闘に勝ってください。','雪男と話す前に、モグをパーティーに加えてください。'),
'fr':('Choisissez la première option pour sauter dans la grotte.','Actionnez l’interrupteur pour tomber dans la salle en dessous.','Choisissez la première option pour prendre la magilithe, puis gagnez le combat.','Placez Mog dans l’équipe active avant de parler au yéti.'),
'it':('Scegli la prima opzione per saltare nella caverna.','Aziona l’interruttore per cadere nella stanza sottostante.','Scegli la prima opzione per prendere la magicite, poi vinci la battaglia.','Metti Mog nel gruppo attivo prima di parlare con lo yeti.'),
'de':('Wähle die erste Option, um in die Höhle zu springen.','Betätige den Schalter, um in die Kammer darunter zu fallen.','Wähle die erste Option, um den Maginit zu nehmen, und gewinne dann den Kampf.','Nimm Mog in die aktive Gruppe auf, bevor du mit dem Yeti sprichst.'),
'es':('Elige la primera opción para saltar a la cueva.','Acciona el interruptor para caer a la cámara inferior.','Elige la primera opción para tomar la magicita y gana el combate.','Incluye a Mog en el grupo activo antes de hablar con el yeti.'),
'ko':('첫 번째 선택지를 골라 동굴로 뛰어드세요.','스위치를 조작해 아래 방으로 떨어지세요.','첫 번째 선택지를 골라 마석을 가져간 뒤 전투에서 승리하세요.','설인과 대화하기 전에 모그를 전투 파티에 넣으세요.'),
'zht':('選擇第一個選項跳入洞窟。','操作開關，落入下方的房間。','選擇第一個選項取得魔石，然後贏得戰鬥。','與雪人交談前，請讓莫古加入出戰隊伍。'),
'zhc':('选择第一个选项跳入洞窟。','操作开关，落入下方的房间。','选择第一个选项取得魔石，然后赢得战斗。','与雪人交谈前，请让莫古加入出战队伍。'),
'ru':('Выберите первый вариант, чтобы прыгнуть в пещеру.','Нажмите переключатель, чтобы упасть в комнату внизу.','Выберите первый вариант, чтобы взять магицит, затем выиграйте бой.','Возьмите Мога в действующую группу, прежде чем говорить с йети.'),
'pt':('Escolha a primeira opção para pular na caverna.','Acione o interruptor para cair na câmara abaixo.','Escolha a primeira opção para pegar a magicita e vença a batalha.','Coloque Mog no grupo ativo antes de falar com o yeti.'),
'th':('เลือกตัวเลือกแรกเพื่อกระโดดลงถ้ำ','ใช้สวิตช์เพื่อร่วงลงสู่ห้องด้านล่าง','เลือกตัวเลือกแรกเพื่อหยิบศิลาเวทมนตร์ แล้วชนะการต่อสู้','ให้ม็อกอยู่ในกลุ่มต่อสู้ก่อนคุยกับเยติ'),
}
if __name__=='__main__':
 assert len(TEXT)==12
 p=ROOT/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 for index,key in enumerate(TEXT['en']):
  wanted={lang:values[index]for lang,values in TEXT.items()};assert all(wanted.values())
  if '--check'in sys.argv:assert data.get(key)==wanted
  else:assert key not in data or data[key]==wanted;data[key]=wanted
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Yeti:four instructions/all12 locales;Navigation entrance/fall and Other magicite choice/Mog setup.')
