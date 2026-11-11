"""Original current/encounter instructions in every supported game language."""
from pathlib import Path
import json,sys
from author_ff5_desert import RIDE,STEP,FIGHT,RETRY
R=Path(__file__).resolve().parents[1];CUE='Ride the current'
TEXT={
'en':(CUE,RIDE,STEP,FIGHT,RETRY),
'ja':('流砂に乗る','流砂に運ばれた後、もう一度ガイドを呼び出してください。','南へ一歩進み、もう一度ガイドを呼び出してください。','最初の選択肢を選び、戦闘に勝ってください。','話しかけてから、もう一度ガイドを呼び出してください。'),
'fr':('Suivez le courant','Laissez le courant vous emporter, puis redemandez le guide.','Faites un pas vers le sud, puis redemandez le guide.','Choisissez la première option, puis gagnez le combat.','Interagissez, puis redemandez le guide.'),
'it':('Segui la corrente','Lasciati trasportare dalla corrente, poi richiedi la guida.','Fai un passo a sud, poi richiedi la guida.','Scegli la prima opzione, poi vinci la battaglia.','Interagisci, poi richiedi la guida.'),
'de':('Lass dich treiben','Lass dich von der Strömung tragen und fordere dann erneut Führung an.','Gehe einen Schritt nach Süden und fordere erneut Führung an.','Wähle die erste Option und gewinne den Kampf.','Interagiere und fordere dann erneut Führung an.'),
'es':('Sigue la corriente','Deja que la corriente te lleve y vuelve a pedir la guía.','Da un paso al sur y vuelve a pedir la guía.','Elige la primera opción y gana la batalla.','Interactúa y vuelve a pedir la guía.'),
'ko':('흐름을 타세요','흐름을 따라 이동한 뒤 안내를 다시 요청하세요.','남쪽으로 한 걸음 이동한 뒤 안내를 다시 요청하세요.','첫 번째 선택지를 고른 뒤 전투에서 승리하세요.','대화한 뒤 안내를 다시 요청하세요.'),
'zht':('順著流沙移動','讓流沙帶你移動，然後再次要求導引。','向南走一步，然後再次要求導引。','選擇第一個選項，然後贏得戰鬥。','互動後再次要求導引。'),
'zhc':('顺着流沙移动','让流沙带你移动，然后再次请求引导。','向南走一步，然后再次请求引导。','选择第一个选项，然后赢得战斗。','互动后再次请求引导。'),
'ru':('Двигайтесь по течению','Позвольте течению перенести вас, затем снова запросите подсказку.','Сделайте шаг на юг и снова запросите подсказку.','Выберите первый вариант и победите в бою.','Взаимодействуйте и снова запросите подсказку.'),
'pt':('Siga a corrente','Deixe a corrente levar você e peça orientação novamente.','Dê um passo ao sul e peça orientação novamente.','Escolha a primeira opção e vença a batalha.','Interaja e peça orientação novamente.'),
'th':('เคลื่อนตามกระแส','ปล่อยให้กระแสพาไป แล้วขอคำแนะนำอีกครั้ง','ก้าวไปทางใต้หนึ่งก้าว แล้วขอคำแนะนำอีกครั้ง','เลือกตัวเลือกแรก แล้วชนะการต่อสู้','โต้ตอบ แล้วขอคำแนะนำอีกครั้ง'),
}
def main():
 check='--check'in sys.argv;assert len(TEXT)==12 and all(len(v)==5 for v in TEXT.values())
 for path,keys in((R/'data/common/mod_text.json',[CUE]),(R/'data/ff5/mod_text.json',[RIDE,STEP,FIGHT,RETRY])):
  d=json.loads(path.read_text(encoding='utf-8'))
  for key in keys:
   i=TEXT['en'].index(key);wanted={lang:values[i]for lang,values in TEXT.items()}
   if check:assert d.get(key)==wanted,key
   else:assert key not in d or d[key]==wanted;d[key]=wanted
  if not check:path.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Desert:five concise current/encounter keys in all12 supported languages.')
if __name__=='__main__':main()
