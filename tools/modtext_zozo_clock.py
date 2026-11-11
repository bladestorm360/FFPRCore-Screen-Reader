"""One concise original clock-choice instruction in twelve supported languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
ROW=dict(en='Set the clock to 6:10:50. Choose options 3, 1, 5.',ja='時計を6時10分50秒に合わせてください。選択肢を3番、1番、5番の順に選んでください。',fr='Réglez l’horloge sur 6 h 10 min 50 s. Choisissez les options 3, 1, 5.',it='Imposta l’orologio sulle 6:10:50. Scegli le opzioni 3, 1, 5.',de='Stelle die Uhr auf 6:10:50. Wähle die Optionen 3, 1, 5.',es='Pon el reloj a las 6:10:50. Elige las opciones 3, 1, 5.',ko='시계를 6시 10분 50초로 맞추세요. 3, 1, 5번 항목을 차례로 선택하세요.',zht='將時鐘設為6時10分50秒。依序選第3、1、5項。',zhc='将时钟设为6时10分50秒。依次选第3、1、5项。',ru='Установите время 6:10:50. Выберите варианты 3, 1, 5.',pt='Ajuste o relógio para 6:10:50. Escolha as opções 3, 1, 5.',th='ตั้งนาฬิกาเป็น 6:10:50 เลือกตัวเลือก 3, 1, 5 ตามลำดับ')
if __name__=='__main__':
    p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));assert len(ROW)==12
    if'--check'in sys.argv:assert d.get(ROW['en'])==ROW
    else:assert ROW['en']not in d or d[ROW['en']]==ROW;d[ROW['en']]=ROW;p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Zozo clock choice instruction:all12 supported languages.')
