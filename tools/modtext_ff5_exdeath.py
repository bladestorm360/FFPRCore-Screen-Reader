"""Castle Exdeath guide instructions in all supported game languages."""
import json
import sys

from author_ff5_exdeath import ROOT, STOP, RETRY, REARM, RESTORE

KEYS = STOP, RETRY, REARM, RESTORE
TEXT = {
    'en': KEYS,
    'ja': ('床を止めてください。停止後か戦闘終了後にもう一度ガイドを呼び出してください。', '床を動かし直し、もう一度ガイドを呼び出してください。', '南に一歩、次に北に一歩進んで床を動かし直してください。', 'このマスを踏んで橋を元に戻してください。'),
    'fr': ('Arrêtez le sol. Redemandez le guide après son arrêt ou après le combat.', 'Relancez le sol, puis redemandez le guide.', 'Faites un pas au sud puis un au nord pour relancer le sol.', 'Marchez sur cette case pour rétablir le pont.'),
    'it': ('Ferma il pavimento. Richiedi la guida quando si ferma o dopo lo scontro.', 'Riavvia il pavimento, poi richiedi la guida.', 'Fai un passo a sud, poi uno a nord, per riavviare il pavimento.', 'Calpesta questa casella per ripristinare il ponte.'),
    'de': ('Halte den Boden an. Fordere nach dem Stillstand oder dem Kampf erneut Führung an.', 'Setze den Boden erneut in Bewegung und fordere dann Führung an.', 'Gehe einen Schritt nach Süden und dann einen nach Norden, um den Boden wieder zu starten.', 'Betritt dieses Feld, um die Brücke wiederherzustellen.'),
    'es': ('Detén el suelo. Pide la guía cuando se detenga o después del combate.', 'Vuelve a mover el suelo y pide la guía de nuevo.', 'Da un paso al sur y otro al norte para volver a mover el suelo.', 'Pisa esta casilla para restaurar el puente.'),
    'ko': ('바닥을 멈추세요. 멈춘 뒤나 전투가 끝난 뒤 안내를 다시 요청하세요.', '바닥을 다시 움직인 뒤 안내를 다시 요청하세요.', '남쪽으로 한 걸음, 북쪽으로 한 걸음 이동해 바닥을 다시 움직이세요.', '이 칸을 밟아 다리를 복구하세요.'),
    'zht': ('停下地板。停止後或戰鬥結束後再次要求導引。', '重新啟動地板，然後再次要求導引。', '先向南一步，再向北一步，重新啟動地板。', '踩上這一格以恢復橋樑。'),
    'zhc': ('停下地板。停止后或战斗结束后再次请求引导。', '重新启动地板，然后再次请求引导。', '先向南一步，再向北一步，重新启动地板。', '踩上这一格以恢复桥梁。'),
    'ru': ('Остановите пол. Запросите подсказку после его остановки или после боя.', 'Запустите пол снова, затем запросите подсказку.', 'Сделайте шаг на юг, затем шаг на север, чтобы снова запустить пол.', 'Наступите на эту клетку, чтобы восстановить мост.'),
    'pt': ('Pare o piso. Peça orientação quando ele parar ou após a batalha.', 'Acione o piso de novo e peça orientação novamente.', 'Dê um passo ao sul e outro ao norte para acionar o piso de novo.', 'Pise neste quadrado para restaurar a ponte.'),
    'th': ('หยุดพื้น แล้วขอคำแนะนำอีกครั้งเมื่อพื้นหยุดหรือหลังการต่อสู้', 'เริ่มให้พื้นเคลื่อนอีกครั้ง แล้วขอคำแนะนำใหม่', 'ก้าวไปทางใต้หนึ่งก้าว แล้วไปทางเหนือหนึ่งก้าวเพื่อให้พื้นเคลื่อนอีกครั้ง', 'เหยียบช่องนี้เพื่อคืนสภาพสะพาน'),
}

def main():
    check = '--check' in sys.argv
    path = ROOT / 'data/ff5/mod_text.json'
    data = json.loads(path.read_text(encoding='utf-8'))
    for index, key in enumerate(KEYS):
        expected = {language: lines[index] for language, lines in TEXT.items()}
        if check:
            assert data[key] == expected
        else:
            assert key not in data or data[key] == expected
            data[key] = expected
    if not check:
        path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Castle Exdeath: four instructions in all 12 locales.')

if __name__ == '__main__':
    main()
