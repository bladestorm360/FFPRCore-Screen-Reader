"""Maintained original mod labels for the optional hidden enemy-health disclosure."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
LOCALES=('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')
ROWS={
 'Enemy critical status':[
  'Enemy critical status','敵の瀕死状態','État critique des ennemis','Stato critico dei nemici','Kritischer Zustand von Gegnern','Estado crítico de enemigos','적의 빈사 상태','敵人瀕死狀態','敌人濒死状态','Критическое состояние врагов','สถานะใกล้ตายของศัตรู','Estado crítico dos inimigos'],
 "On. Announce enemies' hidden critical HP status.":[
  "On. Announce enemies' hidden critical HP status.",'オン。画面に表示されない敵の瀕死状態を読み上げます。',"Activé. Annonce l'état critique des PV ennemis, même s'il est masqué.",'Attivo. Annuncia lo stato critico dei PV nemici anche se non è visibile.','Ein. Sagt den verborgenen kritischen HP-Zustand von Gegnern an.','Activado. Anuncia el estado crítico de PV de los enemigos aunque esté oculto.','켬. 화면에 표시되지 않는 적의 빈사 상태를 읽습니다.','開啟。朗讀畫面未顯示的敵人瀕死狀態。','开启。朗读画面未显示的敌人濒死状态。','Включено. Озвучивать скрытое критическое состояние ОЗ врагов.','เปิด ประกาศสถานะ HP ใกล้หมดของศัตรูที่ไม่ได้แสดงบนหน้าจอ.','Ativado. Anuncia o estado crítico de PV dos inimigos mesmo quando oculto.'],
 "Off. Keep enemies' hidden critical HP status silent.":[
  "Off. Keep enemies' hidden critical HP status silent.",'オフ。画面に表示されない敵の瀕死状態は読み上げません。',"Désactivé. N'annonce pas l'état critique masqué des PV ennemis.",'Disattivo. Non annuncia lo stato critico nascosto dei PV nemici.','Aus. Sagt den verborgenen kritischen HP-Zustand von Gegnern nicht an.','Desactivado. No anuncia el estado crítico oculto de PV de los enemigos.','끔. 화면에 표시되지 않는 적의 빈사 상태를 읽지 않습니다.','關閉。不朗讀畫面未顯示的敵人瀕死狀態。','关闭。不朗读画面未显示的敌人濒死状态。','Выключено. Не озвучивать скрытое критическое состояние ОЗ врагов.','ปิด ไม่ประกาศสถานะ HP ใกล้หมดของศัตรูที่ไม่ได้แสดงบนหน้าจอ.','Desativado. Não anuncia o estado crítico oculto de PV dos inimigos.']}
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';catalog=json.loads(p.read_text(encoding='utf8'))
 for key,values in ROWS.items():
  assert len(values)==len(LOCALES)
  catalog[key]=dict(zip(LOCALES,values))
 p.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
 print('Enemy critical status:3 keys,36 localized values written')
