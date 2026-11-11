"""Original dream-train instructions in all twelve supported mod languages."""
import json,sys
from pathlib import Path
from modtext_opera_guide import LOCALES
from author_dream_train_guide import TWO_PULL,STOP_TIMER
P=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
ROWS={
'Follow the train puzzle. Use Shift+G after each action.':(
 'Follow the train puzzle. Use Shift+G after each action.','列車の仕掛けを順に解いてください。操作のたびにShift+Gで案内を確認してください。','Résolvez les mécanismes du train. Appuyez sur Maj+G après chaque action.','Risolvi gli enigmi del treno. Premi Maiusc+G dopo ogni azione.','Löse die Zugrätsel. Drücke nach jeder Aktion Umschalt+G.','Resuelve los mecanismos del tren. Pulsa Mayús+G después de cada acción.','열차 퍼즐을 진행하세요. 각 조작 후 Shift+G를 누르세요.','依序解開列車機關。每次操作後按Shift+G。','依次解开列车机关。每次操作后按Shift+G。','Решайте загадки поезда. После каждого действия нажимайте Shift+G.','Resolva os mecanismos do trem. Pressione Shift+G após cada ação.','ไขกลไกบนรถไฟ กด Shift+G หลังทำแต่ละขั้นตอน'),
'Open the upper-left chest.':(
 'Open the upper-left chest.','左上の宝箱を開けてください。','Ouvrez le coffre en haut à gauche.','Apri il forziere in alto a sinistra.','Öffne die Truhe oben links.','Abre el cofre de arriba a la izquierda.','왼쪽 위 보물상자를 여세요.','打開左上方的寶箱。','打开左上方的宝箱。','Откройте верхний левый сундук.','Abra o baú superior esquerdo.','เปิดหีบด้านซ้ายบน'),
'Open the upper-middle chest.':(
 'Open the upper-middle chest.','上段中央の宝箱を開けてください。','Ouvrez le coffre en haut au centre.','Apri il forziere in alto al centro.','Öffne die Truhe oben in der Mitte.','Abre el cofre de arriba en el centro.','가운데 위 보물상자를 여세요.','打開上排中央的寶箱。','打开上排中央的宝箱。','Откройте верхний средний сундук.','Abra o baú superior central.','เปิดหีบตรงกลางด้านบน'),
'Close the upper-right chest.':(
 'Close the upper-right chest.','右上の宝箱を閉じてください。','Fermez le coffre en haut à droite.','Chiudi il forziere in alto a destra.','Schließe die Truhe oben rechts.','Cierra el cofre de arriba a la derecha.','오른쪽 위 보물상자를 닫으세요.','關閉右上方的寶箱。','关闭右上方的宝箱。','Закройте верхний правый сундук.','Feche o baú superior direito.','ปิดหีบด้านขวาบน'),
'Close the lower-left chest.':(
 'Close the lower-left chest.','左下の宝箱を閉じてください。','Fermez le coffre en bas à gauche.','Chiudi il forziere in basso a sinistra.','Schließe die Truhe unten links.','Cierra el cofre de abajo a la izquierda.','왼쪽 아래 보물상자를 닫으세요.','關閉左下方的寶箱。','关闭左下方的宝箱。','Закройте нижний левый сундук.','Feche o baú inferior esquerdo.','ปิดหีบด้านซ้ายล่าง'),
'Close the lower-middle chest.':(
 'Close the lower-middle chest.','下段中央の宝箱を閉じてください。','Fermez le coffre en bas au centre.','Chiudi il forziere in basso al centro.','Schließe die Truhe unten in der Mitte.','Cierra el cofre de abajo en el centro.','가운데 아래 보물상자를 닫으세요.','關閉下排中央的寶箱。','关闭下排中央的宝箱。','Закройте нижний средний сундук.','Feche o baú inferior central.','ปิดหีบตรงกลางด้านล่าง'),
'Open the lower-right chest.':(
 'Open the lower-right chest.','右下の宝箱を開けてください。','Ouvrez le coffre en bas à droite.','Apri il forziere in basso a destra.','Öffne die Truhe unten rechts.','Abre el cofre de abajo a la derecha.','오른쪽 아래 보물상자를 여세요.','打開右下方的寶箱。','打开右下方的宝箱。','Откройте нижний правый сундук.','Abra o baú inferior direito.','เปิดหีบด้านขวาล่าง'),
}
ROWS.update({
TWO_PULL:(
 TWO_PULL,'レバーを調べ、操作できるようになったらすぐにもう一度調べてください。2回目の操作で4秒のタイマーが止まります。','Actionnez le levier, puis actionnez-le à nouveau dès que vous retrouvez le contrôle. La seconde action arrête le compte à rebours de quatre secondes.','Aziona la leva, poi azionala di nuovo appena riprendi il controllo. La seconda azione ferma il timer di quattro secondi.','Betätige den Hebel und gleich noch einmal, sobald du wieder steuern kannst. Beim zweiten Mal stoppt der Timer von vier Sekunden.','Acciona la palanca y vuelve a accionarla en cuanto recuperes el control. La segunda vez detiene el temporizador de cuatro segundos.','레버를 조작한 뒤 조작이 가능해지면 즉시 다시 조작하세요. 두 번째 조작으로 4초 타이머가 멈춥니다.','調查拉桿，恢復操作後立即再調查一次。第二次操作會停止四秒計時。','调查拉杆，恢复操作后立即再调查一次。第二次操作会停止四秒计时。','Осмотрите рычаг, затем снова осмотрите его, как только вернётся управление. Второе действие останавливает четырёхсекундный таймер.','Acione a alavanca e acione-a novamente assim que recuperar o controle. A segunda ação para o temporizador de quatro segundos.','สำรวจคันโยก แล้วสำรวจอีกครั้งทันทีที่ควบคุมได้ การสำรวจครั้งที่สองจะหยุดตัวจับเวลา 4 วินาที'),
STOP_TIMER:(
 STOP_TIMER,'今すぐレバーをもう一度調べ、タイマーを止めて木箱の位置を固定してください。','Actionnez à nouveau le levier maintenant pour arrêter le compte à rebours et immobiliser les caisses.','Aziona di nuovo la leva ora per fermare il timer e bloccare le casse.','Betätige den Hebel jetzt erneut, um den Timer zu stoppen und die Kisten festzuhalten.','Acciona otra vez la palanca ahora para detener el temporizador y fijar las cajas.','지금 레버를 다시 조작하여 타이머를 멈추고 상자를 고정하세요.','現在再次調查拉桿，停止計時並固定木箱。','现在再次调查拉杆，停止计时并固定木箱。','Снова осмотрите рычаг сейчас, чтобы остановить таймер и закрепить ящики.','Acione a alavanca novamente agora para parar o temporizador e fixar as caixas.','สำรวจคันโยกอีกครั้งตอนนี้เพื่อหยุดตัวจับเวลาและตรึงลังไว้'),
'Open the chest beside the crates.':(
 'Open the chest beside the crates.','木箱のそばの宝箱を開けてください。','Ouvrez le coffre à côté des caisses.','Apri il forziere accanto alle casse.','Öffne die Truhe neben den Kisten.','Abre el cofre junto a las cajas.','나무 상자 옆의 보물상자를 여세요.','打開木箱旁的寶箱。','打开木箱旁的宝箱。','Откройте сундук рядом с ящиками.','Abra o baú ao lado das caixas.','เปิดหีบที่อยู่ข้างลัง'),
'Open the chest at the left end of this car.':(
 'Open the chest at the left end of this car.','この車両の左端にある宝箱を開けてください。','Ouvrez le coffre à l’extrémité gauche de ce wagon.','Apri il forziere all’estremità sinistra di questo vagone.','Öffne die Truhe am linken Ende dieses Wagens.','Abre el cofre del extremo izquierdo de este vagón.','이 객차의 왼쪽 끝에 있는 보물상자를 여세요.','打開這節車廂最左端的寶箱。','打开这节车厢最左端的宝箱。','Откройте сундук в левом конце этого вагона.','Abra o baú na extremidade esquerda deste vagão.','เปิดหีบที่ปลายด้านซ้ายของตู้รถไฟนี้'),
'Use the item from the opened chest on the moving chest.':(
 'Use the item from the opened chest on the moving chest.','開けた宝箱から入手した品を、動く宝箱に使ってください。','Utilisez l’objet du coffre ouvert sur le coffre mobile.','Usa l’oggetto del forziere aperto sul forziere mobile.','Benutze den Gegenstand aus der geöffneten Truhe an der beweglichen Truhe.','Usa el objeto del cofre abierto en el cofre móvil.','열었던 보물상자에서 얻은 물건을 움직이는 보물상자에 사용하세요.','將已打開寶箱中取得的物品用在移動寶箱上。','将已打开宝箱中取得的物品用在移动宝箱上。','Используйте предмет из открытого сундука на движущемся сундуке.','Use o item do baú aberto no baú que se move.','ใช้ไอเทมจากหีบที่เปิดแล้วกับหีบที่เคลื่อนที่'),
'Open the chest that the moving chest was guarding.':(
 'Open the chest that the moving chest was guarding.','動く宝箱が守っていた宝箱を開けてください。','Ouvrez le coffre que le coffre mobile protégeait.','Apri il forziere custodito dal forziere mobile.','Öffne die Truhe, die von der beweglichen Truhe bewacht wurde.','Abre el cofre que protegía el cofre móvil.','움직이는 보물상자가 지키고 있던 보물상자를 여세요.','打開原本由移動寶箱看守的寶箱。','打开原本由移动宝箱看守的宝箱。','Откройте сундук, который охранял движущийся сундук.','Abra o baú que o baú móvel estava protegendo.','เปิดหีบที่หีบเคลื่อนที่เคยเฝ้าอยู่'),
})
def main():
 original=P.read_text(encoding='utf-8');catalog=json.loads(original)
 for key,values in ROWS.items():assert len(values)==12 and values[0]==key and all(values)
 additions={key:dict(zip(LOCALES,values)) for key,values in ROWS.items() if key not in catalog}
 if '--check' in sys.argv:assert not additions,'Missing dream-train translations'
 if additions:
  insertion=json.dumps(additions,ensure_ascii=False,indent=2)[2:-2]
  P.write_text(original[:original.rfind('}')].rstrip()+',\n'+insertion+'\n}\n',encoding='utf-8');catalog.update(additions)
 for key in ROWS:assert catalog[key]==dict(zip(LOCALES,ROWS[key]))
 print(f'Dream train: {len(ROWS)} instructions in twelve languages; {len(additions)} added')
if __name__=='__main__':main()
