"""Maintain the connected-world filter and manual dive wording in all locales."""
from pathlib import Path
import json

VALUES = {
 'On. Exits to the same destination are grouped separately on each floor. The closest usable exit on each floor remains listed.': {
  'en':'On. Exits to the same destination are grouped separately on each floor. The closest usable exit on each floor remains listed.',
  'ja':'オン。同じ行き先への出口を階ごとにまとめ、各階で最も近い通行可能な出口を表示します。',
  'fr':'Activé. Les sorties vers une même destination sont regroupées par étage. La sortie utilisable la plus proche de chaque étage reste affichée.',
  'it':'Attivo. Le uscite verso la stessa destinazione sono raggruppate per piano. Resta visibile l’uscita utilizzabile più vicina di ogni piano.',
  'de':'Ein. Ausgänge zum selben Ziel werden je Etage gruppiert. Der nächste nutzbare Ausgang jeder Etage bleibt aufgelistet.',
  'es':'Activado. Las salidas al mismo destino se agrupan por planta. Se conserva la salida utilizable más cercana de cada planta.',
  'ko':'켜짐. 같은 목적지로 가는 출구를 층별로 묶습니다. 각 층에서 이용할 수 있는 가장 가까운 출구가 표시됩니다.',
  'zht':'開啟。通往同一目的地的出口按樓層分組，每層保留最近且可通行的出口。',
  'zhc':'开启。通往同一目的地的出口按楼层分组，每层保留最近且可通行的出口。',
  'ru':'Вкл. Выходы в одно место группируются отдельно на каждом этаже. Остаётся ближайший доступный выход каждого этажа.',
  'th':'เปิด จัดกลุ่มทางออกที่ไปยังจุดหมายเดียวกันแยกตามชั้น โดยแสดงทางออกที่ใช้งานได้และใกล้ที่สุดของแต่ละชั้น',
  'pt':'Ativado. As saídas para o mesmo destino são agrupadas por andar. A saída utilizável mais próxima de cada andar continua listada.'},
 'World map': {
  'en':'World map','ja':'ワールドマップ','fr':'Carte du monde','it':'Mappa del mondo',
  'de':'Weltkarte','es':'Mapa del mundo','ko':'월드 맵','zht':'世界地圖','zhc':'世界地图',
  'ru':'Карта мира','th':'แผนที่โลก','pt':'Mapa do mundo'},
 'dive here': {
  'en':'dive here','ja':'ここで潜水','fr':'plongez ici','it':'immergiti qui',
  'de':'hier abtauchen','es':'sumergirse aquí','ko':'여기서 잠수','zht':'在此潛水','zhc':'在此潜水',
  'ru':'погрузиться здесь','th':'ดำลงที่นี่','pt':'mergulhe aqui'},
 'World map filter help': {
  'en':'World map: destinations on connected world maps, including their town and dungeon entrances. Does not list objects inside towns or dungeons.',
  'ja':'ワールドマップ：つながっているワールドマップ上の目的地。町やダンジョンの入口を含みますが、内部のものは表示しません。',
  'fr':'Carte du monde : destinations des cartes du monde reliées, y compris les entrées des villes et donjons. Les objets à l’intérieur ne sont pas affichés.',
  'it':'Mappa del mondo: destinazioni sulle mappe del mondo collegate, inclusi gli ingressi di città e dungeon. Non elenca gli oggetti al loro interno.',
  'de':'Weltkarte: Ziele auf verbundenen Weltkarten, einschließlich Stadt- und Dungeoneingängen. Objekte innerhalb von Städten oder Dungeons werden nicht aufgelistet.',
  'es':'Mapa del mundo: destinos de los mapas del mundo conectados, incluidas las entradas de pueblos y mazmorras. No enumera objetos de su interior.',
  'ko':'월드 맵: 연결된 월드 맵의 목적지와 마을 및 던전 입구를 표시합니다. 마을이나 던전 내부의 대상은 표시하지 않습니다.',
  'zht':'世界地圖：顯示相連世界地圖上的目的地，包括城鎮與迷宮入口，不列出內部物件。',
  'zhc':'世界地图：显示相连世界地图上的目的地，包括城镇与迷宫入口，不列出内部物件。',
  'ru':'Карта мира: места на связанных картах мира, включая входы в города и подземелья. Объекты внутри них не перечисляются.',
  'th':'แผนที่โลก: จุดหมายบนแผนที่โลกที่เชื่อมต่อกัน รวมทางเข้าเมืองและดันเจี้ยน แต่ไม่แสดงวัตถุภายใน',
  'pt':'Mapa do mundo: destinos nos mapas do mundo conectados, incluindo entradas de cidades e masmorras. Não lista objetos dentro delas.'}
}
if __name__=='__main__':
 path=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
 catalog=json.loads(path.read_text(encoding='utf-8'))
 for key,values in VALUES.items():
  assert set(values)==set(catalog['Can surface']) and all(values.values())
  catalog[key]=values
 path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('World filter/dive/floor grouping:4 keys,12 supported languages')
