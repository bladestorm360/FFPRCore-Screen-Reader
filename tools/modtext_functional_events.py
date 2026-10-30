"""Mod-owned entrance/barrier labels in every supported UI language."""
from pathlib import Path
import json

LANGS=('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')
ROWS=(
 ('Dance floor','ダンスフロア','Piste de danse','Pista da ballo','Tanzfläche','Pista de baile','춤추는 공간','舞池','舞池','Танцплощадка','ลานเต้นรำ','Pista de dança'),
 ('Checkpoint','チェックポイント','Point de contrôle','Punto di controllo','Kontrollpunkt','Punto de control','체크포인트','檢查點','检查点','Контрольная точка','จุดตรวจ','Ponto de controle'),
 ('Service area','サービスエリア','Zone de service','Area di servizio','Servicebereich','Zona de servicio','서비스 구역','服務區','服务区','Зона обслуживания','พื้นที่บริการ','Área de serviço'),
 ('Puzzle trigger','仕掛けの作動地点','Déclencheur de mécanisme','Attivatore del meccanismo','Rätselauslöser','Activador del mecanismo','장치 작동 지점','機關觸發點','机关触发点','Активатор головоломки','จุดกระตุ้นกลไก','Ativador de mecanismo'),
 ('Timer start','タイマー開始地点','Départ du chronomètre','Avvio del timer','Timerstart','Inicio del temporizador','타이머 시작','計時開始點','计时开始点','Запуск таймера','จุดเริ่มจับเวลา','Início do cronômetro'),
 ('Timer stop','タイマー終了地点','Arrêt du chronomètre','Arresto del timer','Timerstopp','Fin del temporizador','타이머 정지','計時停止點','计时停止点','Остановка таймера','จุดหยุดจับเวลา','Parada do cronômetro'),
 ('Moving passage','動く通路','Passage mobile','Passaggio mobile','Beweglicher Durchgang','Pasadizo móvil','움직이는 통로','移動通道','移动通道','Подвижный проход','ทางเลื่อน','Passagem móvel'),
 ('Coffin','棺','Cercueil','Bara','Sarg','Ataúd','관','棺材','棺材','Гроб','โลงศพ','Caixão'),
 ('Quicksand','流砂','Sables mouvants','Sabbie mobili','Treibsand','Arenas movedizas','유사','流沙','流沙','Зыбучий песок','ทรายดูด','Areia movediça'),
 ('Light path','光の道','Chemin de lumière','Sentiero di luce','Lichtpfad','Camino de luz','빛의 길','光之路','光之路','Световая дорожка','เส้นทางแสง','Caminho de luz'),
 ('Jump point','飛び移る地点','Point de saut','Punto di salto','Sprungstelle','Punto de salto','도약 지점','跳躍點','跳跃点','Место для прыжка','จุดกระโดด','Ponto de salto'),
 ('Safe spot','安全な場所','Endroit sûr','Posto sicuro','Sicherer Platz','Zona segura','안전 지대','安全地點','安全地点','Безопасное место','จุดปลอดภัย','Local seguro'),
 ('Rope crossing','綱渡り','Traversée sur corde','Attraversamento su corda','Seilüberquerung','Cruce por cuerda','밧줄 건너기','繩索通道','绳索通道','Переход по канату','ทางข้ามเชือก','Travessia por corda'),
 ('Water current','水流','Courant d’eau','Corrente d’acqua','Wasserströmung','Corriente de agua','물살','水流','水流','Водное течение','กระแสน้ำ','Corrente de água'),
 ('Slippery floor','滑る床','Sol glissant','Pavimento scivoloso','Rutschiger Boden','Suelo resbaladizo','미끄러운 바닥','濕滑地板','湿滑地板','Скользкий пол','พื้นลื่น','Piso escorregadio'),
 ('Map entrance','マップ入口','Entrée de zone','Ingresso dell’area','Gebietseingang','Entrada de zona','지역 입구','區域入口','区域入口','Вход в область','ทางเข้าพื้นที่','Entrada da área'),
 ('Passage','通路','Passage','Passaggio','Durchgang','Paso','통로','通道','通道','Проход','ทางผ่าน','Passagem'),
 ('Tozus entrance','トーザスの村の入口','Entrée de Tozus','Ingresso di Tozus','Eingang nach Tozus','Entrada a Tozus','토저스 마을 입구','托扎斯村入口','托扎斯村入口','Вход в Тозус','ทางเข้าโทซุส','Entrada de Tozus'),
 ('Closed forest entrance','閉ざされた森の入口','Entrée de forêt fermée','Ingresso della foresta chiuso','Verschlossener Waldeingang','Entrada del bosque cerrada','닫힌 숲 입구','封閉的森林入口','封闭的森林入口','Закрытый вход в лес','ทางเข้าป่าที่ปิดอยู่','Entrada da floresta fechada'),
 ('Airship barrier','飛空艇を阻む障壁','Barrière pour aéronefs','Barriera per aeronavi','Luftschiffbarriere','Barrera para barcos voladores','비공정 차단 장벽','飛空艇屏障','飞空艇屏障','Преграда для воздушных кораблей','สิ่งกีดขวางเรือเหาะ','Barreira para aeronaves'),
 ('Passage barrier','通行を阻む障壁','Barrière de passage','Barriera del passaggio','Durchgangsbarriere','Barrera del paso','통행 차단 장벽','通道屏障','通道屏障','Преграда на проходе','สิ่งกีดขวางทางผ่าน','Barreira de passagem'),
 ('Statue barrier','石像の障壁','Barrière de statues','Barriera delle statue','Statuenbarriere','Barrera de estatuas','석상 장벽','石像屏障','石像屏障','Преграда из статуй','สิ่งกีดขวางรูปปั้น','Barreira de estátuas'),
 ('Saronia Airspace','サロニア上空','Espace aérien de Saronia','Spazio aereo di Saronia','Luftraum von Saronia','Espacio aéreo de Saronia','사로니아 상공','薩羅尼亞空域','萨罗尼亚空域','Воздушное пространство Саронии','น่านฟ้าซาโรเนีย','Espaço aéreo de Saronia'),
)
if __name__=='__main__':
    path=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
    catalog=json.loads(path.read_text(encoding='utf-8'))
    for row in ROWS:
        assert len(row)==12 and set(LANGS)==set(catalog['Exit to {0}'])
        catalog[row[0]]=dict(zip(LANGS,row))
    path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(f'Functional events:{len(ROWS)} keys,12 locales')
