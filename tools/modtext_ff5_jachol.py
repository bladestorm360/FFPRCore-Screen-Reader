"""Original concise Jachol instructions in all twelve supported languages."""
from pathlib import Path
import json,sys
from author_ff5_jachol import LIVE,WAIT,STAGE,DOOR,CLIMB
R=Path(__file__).resolve().parents[1]
TEXT={
'en':(LIVE,WAIT,STAGE,DOOR,CLIMB,'Resume timers until the skull switch appears.'),
'ja':('今です。ドクロのスイッチを操作してください。','ドクロのスイッチが現れるまで待ってください。','ここでドクロのスイッチを待ってください。','宝箱を調べ、最初の選択肢を選んで通路を開いてください。','壁を調べて登ってください。','ドクロのスイッチが現れるまでタイマーを再開してください。'),
'fr':('Allez-y. Actionnez l’interrupteur en forme de crâne.','Attendez que l’interrupteur en forme de crâne apparaisse.','Attendez ici l’interrupteur en forme de crâne.','Examinez le coffre. Choisissez la première option pour ouvrir le passage.','Examinez le mur pour grimper et sortir.','Relancez les minuteries jusqu’à l’apparition de l’interrupteur en forme de crâne.'),
'it':('Vai ora. Aziona l’interruttore a teschio.','Attendi che appaia l’interruttore a teschio.','Attendi qui l’interruttore a teschio.','Esamina il forziere. Scegli la prima opzione per aprire il passaggio.','Esamina il muro per arrampicarti e uscire.','Riattiva i timer finché appare l’interruttore a teschio.'),
'de':('Jetzt los. Betätige den Totenkopfschalter.','Warte, bis der Totenkopfschalter erscheint.','Warte hier auf den Totenkopfschalter.','Untersuche die Truhe. Wähle die erste Option, um den Durchgang zu öffnen.','Untersuche die Wand, um hinauszuklettern.','Lasse die Timer weiterlaufen, bis der Totenkopfschalter erscheint.'),
'es':('Ve ahora. Acciona el interruptor de calavera.','Espera a que aparezca el interruptor de calavera.','Espera aquí al interruptor de calavera.','Examina el cofre. Elige la primera opción para abrir el paso.','Examina la pared para trepar y salir.','Reanuda los temporizadores hasta que aparezca el interruptor de calavera.'),
'ko':('지금 이동해 해골 스위치를 조작하세요.','해골 스위치가 나타날 때까지 기다리세요.','여기서 해골 스위치를 기다리세요.','상자를 조사하고 첫 번째 선택지로 통로를 여세요.','벽을 조사해 올라가세요.','해골 스위치가 나타날 때까지 타이머를 재개하세요.'),
'zht':('現在前進，操作骷髏開關。','等待骷髏開關出現。','在這裡等待骷髏開關。','調查寶箱，選擇第一個選項以開啟通道。','調查牆壁，爬出去。','恢復計時器，直到骷髏開關出現。'),
'zhc':('现在前进，操作骷髅开关。','等待骷髅开关出现。','在这里等待骷髅开关。','调查宝箱，选择第一个选项以开启通道。','调查墙壁，爬出去。','恢复计时器，直到骷髅开关出现。'),
'ru':('Идите сейчас. Нажмите переключатель с черепом.','Дождитесь появления переключателя с черепом.','Ждите переключатель с черепом здесь.','Осмотрите сундук. Выберите первый вариант, чтобы открыть проход.','Осмотрите стену, чтобы взобраться и выйти.','Возобновите таймеры до появления переключателя с черепом.'),
'pt':('Vá agora. Acione o interruptor de caveira.','Espere o interruptor de caveira aparecer.','Espere aqui pelo interruptor de caveira.','Examine o baú. Escolha a primeira opção para abrir a passagem.','Examine a parede para subir e sair.','Retome os temporizadores até o interruptor de caveira aparecer.'),
'th':('ไปตอนนี้ ใช้งานสวิตช์กะโหลก','รอให้สวิตช์กะโหลกปรากฏ','รอสวิตช์กะโหลกที่นี่','ตรวจสอบหีบ เลือกตัวเลือกแรกเพื่อเปิดทาง','ตรวจสอบผนังเพื่อปีนออกไป','เดินตัวจับเวลาต่อจนกว่าสวิตช์กะโหลกจะปรากฏ'),
}
if __name__=='__main__':
 assert len(TEXT)==12 and all(len(v)==6 for v in TEXT.values());p=R/'data/ff5/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 for i,key in enumerate(TEXT['en']):
  wanted={lang:v[i]for lang,v in TEXT.items()};assert all(wanted.values())
  if '--check'in sys.argv:assert data.get(key)==wanted
  else:assert key not in data or data[key]==wanted;data[key]=wanted
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Jachol:six short instructions/all12 supported languages;existing operate/open strings reused.')
