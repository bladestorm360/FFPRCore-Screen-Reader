"""Original stay/wait/irreversible-chest guidance, all12 languages."""
from pathlib import Path
import json,sys
from author_floating_continent_guide import STAY,WAIT,MISSED
ROOT=Path(__file__).resolve().parents[1]
ROWS={
 STAY:dict(en=STAY,ja='飛空艇の選択肢では2番目を選んでここに残り、もう一度ガイドを呼び出してください。',fr='Au choix de l’aéronef, choisissez la deuxième option pour rester ici, puis redemandez le guide.',it='Alla scelta dell’aeronave, scegli la seconda opzione per restare qui, poi richiedi di nuovo la guida.',de='Wähle beim Luftschiff die zweite Option, um hierzubleiben. Rufe dann die Hilfe erneut auf.',es='En la elección del barco volador, elige la segunda opción para quedarte y vuelve a pedir la guía.',ko='비공정 선택에서 두 번째 항목을 골라 이곳에 남은 뒤 안내를 다시 요청하세요.',zht='在飛空艇選項中選擇第二項留在此處，然後再次請求引導。',zhc='在飞空艇选项中选择第二项留在此处，然后再次请求引导。',ru='В меню воздушного корабля выберите второй вариант, чтобы остаться, затем снова вызовите подсказку.',pt='Na escolha da aeronave, selecione a segunda opção para ficar aqui e peça o guia novamente.',th='เมื่อมีตัวเลือกเรือเหาะ ให้เลือกข้อที่สองเพื่ออยู่ที่นี่ แล้วขอคำแนะนำอีกครั้ง'),
 WAIT:dict(en=WAIT,ja='飛空艇の選択肢では2番目を選んでシャドウを待ちます。到着するまで崖の端にいてください。',fr='Au choix de l’aéronef, choisissez la deuxième option pour attendre Shadow. Restez au bord jusqu’à son arrivée.',it='Alla scelta dell’aeronave, scegli la seconda opzione per aspettare Shadow. Resta sul bordo fino al suo arrivo.',de='Wähle beim Luftschiff die zweite Option, um auf Shadow zu warten. Bleibe bis zu seiner Ankunft am Rand.',es='En la elección del barco volador, elige la segunda opción para esperar a Shadow. Quédate al borde hasta que llegue.',ko='비공정 선택에서 두 번째 항목을 골라 섀도를 기다리세요. 도착할 때까지 가장자리에 머무르세요.',zht='在飛空艇選項中選擇第二項等待暗影。留在邊緣直到他到達。',zhc='在飞空艇选项中选择第二项等待暗影。留在边缘直到他到达。',ru='В меню воздушного корабля выберите второй вариант, чтобы ждать Шэдоу. Оставайтесь у края до его прихода.',pt='Na escolha da aeronave, selecione a segunda opção para esperar Shadow. Fique na borda até ele chegar.',th='เมื่อมีตัวเลือกเรือเหาะ ให้เลือกข้อที่สองเพื่อรอชาโดว์ อยู่ที่ขอบจนกว่าเขาจะมาถึง'),
 MISSED:dict(en=MISSED,ja='崩壊により、その宝箱には到達できません。{0}へ進んでください。',fr='Ce coffre est inaccessible après l’effondrement. Continuez vers {0}.',it='Quel forziere è irraggiungibile dopo il crollo. Prosegui verso {0}.',de='Diese Truhe ist nach dem Einsturz nicht mehr erreichbar. Gehe weiter zu {0}.',es='Ese cofre queda aislado tras el derrumbe. Continúa hacia {0}.',ko='붕괴 후에는 그 보물 상자에 갈 수 없습니다. {0}(으)로 계속 이동하세요.',zht='崩塌後已無法到達那個寶箱。繼續前往{0}。',zhc='崩塌后已无法到达那个宝箱。继续前往{0}。',ru='После обвала этот сундук недоступен. Продолжайте к {0}.',pt='Esse baú ficou inacessível após o desabamento. Continue até {0}.',th='หลังทางถล่มจะไปถึงหีบใบนั้นไม่ได้แล้ว ไปต่อที่ {0}')}
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));langs=set(d['Exit to {0}']);assert len(langs)==12
 for key,row in ROWS.items():
  assert set(row)==langs and all(row.values())
  if'--check'in sys.argv:assert d.get(key)==row,key
  else:assert key not in d or d[key]==row,key;d[key]=row
 if'--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Floating Continent choices/recovery:three instructions/all12 languages.')
