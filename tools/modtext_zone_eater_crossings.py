"""Four original crossing instructions in all twelve supported languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
LANGS=('en','ja','fr','it','de','es','ko','zht','zhc','ru','pt','th')
ROWS=[
('Cross the bridges. Avoid the moving guards. If you fall, request guidance again.',
 '橋を渡ってください。動いている見張りを避けてください。落ちたら、もう一度ガイドを呼び出してください。',
 'Traversez les ponts. Évitez les gardes mobiles. Si vous tombez, redemandez le guide.',
 'Attraversa i ponti. Evita le guardie in movimento. Se cadi, richiedi di nuovo la guida.',
 'Überquere die Brücken. Weiche den patrouillierenden Wachen aus. Wenn du fällst, rufe die Anleitung erneut auf.',
 'Cruza los puentes. Evita a los guardias en movimiento. Si caes, vuelve a solicitar la guía.',
 '다리를 건너세요. 움직이는 경비병을 피하세요. 떨어지면 안내를 다시 요청하세요.',
 '穿過橋樑。避開移動中的守衛。如果掉下去，請再次要求指引。',
 '穿过桥梁。避开移动中的守卫。如果掉下去，请再次请求指引。',
 'Перейдите мосты. Избегайте движущихся стражников. Если упадёте, снова запросите подсказку.',
 'Atravesse as pontes. Evite os guardas em movimento. Se cair, peça o guia novamente.',
 'ข้ามสะพาน หลบยามที่กำลังเคลื่อนที่ หากตกลงไป ให้ขอคำแนะนำอีกครั้ง'),
('Use the stepping stones to reach Gogo. Request guidance after each jump and switch.',
 '足場を渡ってゴゴのところへ進んでください。跳ぶたび、またスイッチを踏むたびにガイドを呼び出してください。',
 'Utilisez les plateformes pour rejoindre Gogo. Redemandez le guide après chaque saut et chaque interrupteur.',
 'Usa le piattaforme per raggiungere Gogo. Richiedi la guida dopo ogni salto e interruttore.',
 'Erreiche Gogo über die Trittsteine. Rufe die Anleitung nach jedem Sprung und Schalter erneut auf.',
 'Usa las plataformas para llegar hasta Gogo. Solicita la guía después de cada salto e interruptor.',
 '디딤돌을 이용해 고고에게 가세요. 점프하거나 스위치를 밟을 때마다 안내를 요청하세요.',
 '利用踏腳石前往格格身邊。每次跳躍和踩下開關後都請要求指引。',
 '利用踏脚石前往格格身边。每次跳跃和踩下开关后都请请求指引。',
 'Доберитесь до Гого по выступам. Запрашивайте подсказку после каждого прыжка и переключателя.',
 'Use as plataformas para chegar a Gogo. Peça o guia após cada salto e interruptor.',
 'ใช้แท่นเหยียบเพื่อไปหาโกโก ขอคำแนะนำหลังการกระโดดและการเหยียบสวิตช์แต่ละครั้ง'),
('Step on {0} to open the return passage, then request guidance again.',
 '{0}を踏んで帰り道を開き、もう一度ガイドを呼び出してください。',
 'Marchez sur {0} pour ouvrir le passage de retour, puis redemandez le guide.',
 'Sali su {0} per aprire il passaggio di ritorno, poi richiedi di nuovo la guida.',
 'Tritt auf {0}, um den Rückweg zu öffnen, und rufe dann die Anleitung erneut auf.',
 'Pisa {0} para abrir el pasaje de vuelta y vuelve a solicitar la guía.',
 '{0}을(를) 밟아 돌아가는 통로를 연 뒤 안내를 다시 요청하세요.',
 '踩下{0}以打開返回通道，然後再次要求指引。',
 '踩下{0}以打开返回通道，然后再次请求指引。',
 'Встаньте на {0}, чтобы открыть обратный проход, затем снова запросите подсказку.',
 'Pise em {0} para abrir a passagem de volta e peça o guia novamente.',
 'เหยียบ {0} เพื่อเปิดทางกลับ แล้วขอคำแนะนำอีกครั้ง'),
('Step on {0} to raise the stepping stones, then request guidance again.',
 '{0}を踏んで足場を出し、もう一度ガイドを呼び出してください。',
 'Marchez sur {0} pour faire apparaître les plateformes, puis redemandez le guide.',
 'Sali su {0} per far comparire le piattaforme, poi richiedi di nuovo la guida.',
 'Tritt auf {0}, um die Trittsteine anzuheben, und rufe dann die Anleitung erneut auf.',
 'Pisa {0} para elevar las plataformas y vuelve a solicitar la guía.',
 '{0}을(를) 밟아 디딤돌을 올린 뒤 안내를 다시 요청하세요.',
 '踩下{0}以升起踏腳石，然後再次要求指引。',
 '踩下{0}以升起踏脚石，然后再次请求指引。',
 'Встаньте на {0}, чтобы поднять выступы, затем снова запросите подсказку.',
 'Pise em {0} para erguer as plataformas e peça o guia novamente.',
 'เหยียบ {0} เพื่อยกแท่นเหยียบขึ้น แล้วขอคำแนะนำอีกครั้ง')]

def main():
    path=ROOT/'data/common/mod_text.json';data=json.loads(path.read_text(encoding='utf-8'))
    for row in ROWS:
        assert len(row)==len(LANGS)
        expected=dict(zip(LANGS,row));key=row[0]
        assert all(s and s.count('{0}')==key.count('{0}')for s in expected.values())
        if '--check'in sys.argv:assert data.get(key)==expected,key
        else:
            assert key not in data or data[key]==expected,key
            data[key]=expected
    if '--check'not in sys.argv:path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Crossing instructions: four keys/twelve languages; placeholders checked.')
if __name__=='__main__':main()
