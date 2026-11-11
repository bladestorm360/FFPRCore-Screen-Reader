"""Original complete Gogo instructions in every supported language."""
from pathlib import Path
import json,sys
from author_ff5_gogo import AIR,GOGO,LEAVE
R=Path(__file__).resolve().parents[1]
TEXT={
'en':(AIR,GOGO,LEAVE),
'ja':('空気を補給して制限時間をリセットしてください。','オートバトルを解除してください。クリスタルを調べ、2番目の選択肢を選んでください。戦闘中はメインコマンドを開いたまま、敵が去るまで何も行動しないでください。','制限時間が切れる前に塔から出てください。'),
'fr':('Prenez la réserve d’air pour réinitialiser le compte à rebours.','Désactivez le combat automatique. Examinez le cristal et choisissez la deuxième option. En combat, gardez le menu principal des commandes ouvert sans agir jusqu’au départ de l’ennemi.','Quittez la tour avant la fin du compte à rebours.'),
'it':('Prendi la riserva d’aria per azzerare il conto alla rovescia.','Disattiva la battaglia automatica. Esamina il cristallo e scegli la seconda opzione. In battaglia, lascia aperto il menu principale dei comandi e non agire finché il nemico non se ne va.','Esci dalla torre prima che scada il tempo.'),
'de':('Nimm den Luftvorrat, um das Zeitlimit zurückzusetzen.','Schalte den automatischen Kampf aus. Untersuche den Kristall und wähle die zweite Option. Lass im Kampf das Hauptbefehlsmenü offen und handle nicht, bis der Gegner geht.','Verlasse den Turm, bevor die Zeit abläuft.'),
'es':('Toma la reserva de aire para reiniciar el tiempo.','Desactiva la batalla automática. Examina el cristal y elige la segunda opción. En batalla, mantén abierto el menú principal de órdenes y no actúes hasta que el enemigo se vaya.','Sal de la torre antes de que se agote el tiempo.'),
'ko':('공기를 보충해 제한 시간을 초기화하세요.','자동 전투를 끄세요. 크리스탈을 조사하고 두 번째 선택지를 고르세요. 전투에서는 기본 명령 메뉴를 열어 둔 채 적이 떠날 때까지 아무 행동도 하지 마세요.','제한 시간이 끝나기 전에 탑을 나가세요.'),
'zht':('取得空氣補給，重設倒數計時。','關閉自動戰鬥。調查水晶，選擇第二個選項。戰鬥中保持主要指令選單開啟，不要採取任何行動，直到敵人離開。','在倒數結束前離開塔。'),
'zhc':('取得空气补给，重置倒计时。','关闭自动战斗。调查水晶，选择第二个选项。战斗中保持主要指令菜单开启，不要采取任何行动，直到敌人离开。','在倒计时结束前离开塔。'),
'ru':('Возьмите запас воздуха, чтобы сбросить таймер.','Отключите автобой. Осмотрите кристалл и выберите второй вариант. В бою оставьте открытым главное меню команд и ничего не делайте, пока противник не уйдёт.','Покиньте башню до истечения времени.'),
'pt':('Pegue a reserva de ar para reiniciar a contagem regressiva.','Desative a batalha automática. Examine o cristal e escolha a segunda opção. Na batalha, mantenha o menu principal de comandos aberto e não faça nada até o inimigo ir embora.','Saia da torre antes que o tempo acabe.'),
'th':('รับอากาศสำรองเพื่อเริ่มเวลานับถอยหลังใหม่','ปิดการต่อสู้อัตโนมัติ ตรวจสอบคริสตัลและเลือกตัวเลือกที่สอง ระหว่างต่อสู้ให้เปิดเมนูคำสั่งหลักไว้ และไม่ทำสิ่งใดจนกว่าศัตรูจะจากไป','ออกจากหอคอยก่อนหมดเวลา'),
}
if __name__=='__main__':
 assert len(TEXT)==12 and all(len(v)==3 for v in TEXT.values());p=R/'data/ff5/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));check='--check'in sys.argv
 for i,key in enumerate(TEXT['en']):
  want={lang:values[i]for lang,values in TEXT.items()};assert all(want.values())
  if check:assert d.get(key)==want
  else:assert key not in d or d[key]==want;d[key]=want
 if not check:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Gogo:air/second-choice/inaction/full return instructions,all12 supported languages.')
