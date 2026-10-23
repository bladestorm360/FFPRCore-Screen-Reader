"""Mod-owned physical object labels in FFPR's twelve supported UI languages."""
LANGS = ('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')
ROWS = (
 ('Chakram','チャクラム','Chakram','Chakram','Chakram','Chakram','차크람','輪刃','轮刃','Чакрам','จักร','Chakram'),
 ('Tablet','石板','Tablette','Tavoletta','Steintafel','Losa','석판','石板','石板','Каменная плита','แผ่นศิลา','Tábua de pedra'),
 ('Horn','角','Corne','Corno','Horn','Cuerno','뿔','角','角','Рог','เขา','Chifre'),
 ('Fang','牙','Croc','Zanna','Fangzahn','Colmillo','송곳니','牙','牙','Клык','เขี้ยว','Presa'),
 ('Lute','リュート','Luth','Liuto','Laute','Laúd','류트','魯特琴','鲁特琴','Лютня','ลูต','Alaúde'),
 ('Guillotine','ギロチン','Guillotine','Ghigliottina','Guillotine','Guillotina','단두대','斷頭台','断头台','Гильотина','กิโยติน','Guilhotina'),
 ('Book','本','Livre','Libro','Buch','Libro','책','書','书','Книга','หนังสือ','Livro'),
 ('Door','扉','Porte','Porta','Tür','Puerta','문','門','门','Дверь','ประตู','Porta'),
 ('Map','地図','Carte','Mappa','Karte','Mapa','지도','地圖','地图','Карта','แผนที่','Mapa'),
 ('Letter','手紙','Lettre','Lettera','Brief','Carta','편지','信件','信件','Письмо','จดหมาย','Carta'),
 ('Machine','機械','Machine','Macchina','Maschine','Máquina','기계','機械','机械','Машина','เครื่องจักร','Máquina'),
 ('Raft','いかだ','Radeau','Zattera','Floß','Balsa','뗏목','木筏','木筏','Плот','แพ','Jangada'),
 ('Weight','おもり','Poids','Peso','Gewicht','Peso','추','重物','重物','Груз','ตุ้มน้ำหนัก','Peso'),
 ('Diary','日記','Journal intime','Diario','Tagebuch','Diario','일기','日記','日记','Дневник','ไดอารี่','Diário'),
 ('Bucket','バケツ','Seau','Secchio','Eimer','Cubo','양동이','水桶','水桶','Ведро','ถังน้ำ','Balde'),
 ('Window','窓','Fenêtre','Finestra','Fenster','Ventana','창문','窗戶','窗户','Окно','หน้าต่าง','Janela'),
 ('Candle','ろうそく','Bougie','Candela','Kerze','Vela','양초','蠟燭','蜡烛','Свеча','เทียน','Vela'),
 ('Hole','穴','Trou','Buco','Loch','Agujero','구멍','洞','洞','Отверстие','รู','Buraco'),
 ('Chain','鎖','Chaîne','Catena','Kette','Cadena','사슬','鎖鏈','锁链','Цепь','โซ่','Corrente'),
 ('Vine','つた','Liane','Rampicante','Ranke','Enredadera','덩굴','藤蔓','藤蔓','Лоза','เถาวัลย์','Cipó'),
 ('Scene trigger','シーン開始地点','Déclencheur de scène','Attivazione scena','Szenenauslöser','Activador de escena','장면 트리거','場景觸發點','场景触发点','Запуск сцены','จุดเริ่มฉาก','Gatilho de cena'),
 ('Conversation','会話','Conversation','Conversazione','Gespräch','Conversación','대화','對話','对话','Разговор','บทสนทนา','Conversa'),
 ('Vehicle interaction','乗り物の操作','Interaction avec un véhicule','Interazione con veicolo','Fahrzeuginteraktion','Interacción con vehículo','탈것 상호작용','載具互動','载具互动','Взаимодействие с транспортом','โต้ตอบกับพาหนะ','Interação com veículo'),
 ('Switch trigger','スイッチの作動地点','Déclencheur d’interrupteur','Attivazione interruttore','Schalterauslöser','Activador de interruptor','스위치 트리거','開關觸發點','开关触发点','Активация переключателя','จุดเปิดสวิตช์','Gatilho de interruptor'),
 ('Event trigger','イベント開始地点','Déclencheur d’événement','Attivazione evento','Ereignisauslöser','Activador de evento','이벤트 트리거','事件觸發點','事件触发点','Запуск события','จุดเริ่มเหตุการณ์','Gatilho de evento'),
 ('Interactive object','調べられる物','Objet interactif','Oggetto interattivo','Interaktives Objekt','Objeto interactivo','상호작용 가능한 물체','可互動物件','可互动物体','Интерактивный объект','วัตถุที่โต้ตอบได้','Objeto interativo'),
)

def extend(recipes):
    for row in ROWS:
        assert len(row)==len(LANGS)
        recipes.append(('common',row[0],dict(zip(LANGS,row)),
                        'October22 reviewed physical object identity; mod-owned translation'))
