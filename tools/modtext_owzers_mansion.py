"""Original concise mansion instructions in all twelve supported languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
ROWS=[
dict(en='Read the diary.',ja='日記を読んでください。',fr='Lisez le journal.',it='Leggi il diario.',de='Lies das Tagebuch.',es='Lee el diario.',ko='일기를 읽으세요.',zht='閱讀日記。',zhc='阅读日记。',ru='Прочитайте дневник.',pt='Leia o diário.',th='อ่านไดอารี่'),
dict(en='Examine the bookshelf.',ja='本棚を調べてください。',fr='Examinez la bibliothèque.',it='Esamina la libreria.',de='Untersuche das Bücherregal.',es='Examina la estantería.',ko='책장을 조사하세요.',zht='調查書架。',zhc='调查书架。',ru='Осмотрите книжный шкаф.',pt='Examine a estante.',th='สำรวจชั้นหนังสือ'),
dict(en='Left door: 1 North. Request the guide again when ready, then wait for Go now.',ja='左の扉へ北に1歩です。準備ができたらもう一度ガイドを呼び出し、「今です」の合図を待ってください。',fr='Porte de gauche : 1 pas au nord. Redemandez le guidage quand vous êtes prêt, puis attendez le signal.',it='Porta sinistra: 1 passo a nord. Quando sei pronto, richiedi di nuovo la guida e attendi il via.',de='Linke Tür: 1 Schritt nach Norden. Fordere die Führung erneut an, wenn du bereit bist, und warte auf das Startsignal.',es='Puerta izquierda: 1 paso al norte. Cuando estés listo, pide indicaciones de nuevo y espera la señal.',ko='왼쪽 문: 북쪽으로 1칸. 준비되면 안내를 다시 요청하고 출발 신호를 기다리세요.',zht='左門：向北1步。準備好後再次請求引導，然後等待出發提示。',zhc='左门：向北1步。准备好后再次请求引导，然后等待出发提示。',ru='Левая дверь: 1 шаг на север. Когда будете готовы, снова запросите подсказку и ждите сигнала.',pt='Porta esquerda: 1 passo ao norte. Quando estiver pronto, peça orientação de novo e espere o sinal.',th='ประตูซ้าย: ไปเหนือ 1 ก้าว เมื่อพร้อมให้ขอคำแนะนำอีกครั้ง แล้วรอสัญญาณให้ไป')]
if __name__=='__main__':
    p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'))
    for row in ROWS:
        assert len(row)==12
        if'--check'in sys.argv:assert d.get(row['en'])==row
        else:assert row['en']not in d or d[row['en']]==row;d[row['en']]=row
    if'--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Owzer: three instructions/all12 supported languages.')
