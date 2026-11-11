"""One original bridge-choice instruction in all twelve supported languages."""
from pathlib import Path
import json,sys
from author_sealed_gate_guide import CHOICE
ROOT=Path(__file__).resolve().parents[1]
ROW=dict(en=CHOICE,ja='{0}を操作してください。最初の選択肢を選び、もう一度ガイドを呼び出してください。',fr='Actionnez {0}. Choisissez la première option, puis redemandez le guidage.',it='Aziona {0}. Scegli la prima opzione, poi richiedi di nuovo la guida.',de='Betätige {0}. Wähle die erste Option und fordere dann erneut die Führung an.',es='Acciona {0}. Elige la primera opción y vuelve a solicitar la guía.',ko='{0}을 조작하세요. 첫 번째 항목을 선택한 뒤 안내를 다시 요청하세요.',zht='操作{0}。選第一項，然後再次請求引導。',zhc='操作{0}。选第一项，然后再次请求引导。',ru='Используйте {0}. Выберите первый вариант, затем снова запросите подсказку.',pt='Acione {0}. Escolha a primeira opção e solicite as orientações novamente.',th='ใช้งาน {0} เลือกตัวเลือกแรก แล้วขอคำแนะนำอีกครั้ง')
ROWS=[ROW,
dict(en='Examine {0}, then request guidance again.',ja='{0}を調べて、もう一度ガイドを呼び出してください。',fr='Examinez {0}, puis redemandez le guidage.',it='Esamina {0}, poi richiedi di nuovo la guida.',de='Untersuche {0} und fordere dann erneut die Führung an.',es='Examina {0} y vuelve a solicitar la guía.',ko='{0}을 조사한 뒤 안내를 다시 요청하세요.',zht='調查{0}，然後再次請求引導。',zhc='调查{0}，然后再次请求引导。',ru='Осмотрите {0}, затем снова запросите подсказку.',pt='Examine {0} e solicite as orientações novamente.',th='สำรวจ {0} แล้วขอคำแนะนำอีกครั้ง'),
dict(en='Open the Treasure Chest.',ja='宝箱を開けてください。',fr='Ouvrez le coffre.',it='Apri lo scrigno.',de='Öffne die Schatztruhe.',es='Abre el cofre.',ko='보물 상자를 여세요.',zht='打開寶箱。',zhc='打开宝箱。',ru='Откройте сундук.',pt='Abra o baú.',th='เปิดหีบสมบัติ'),
dict(en='Step on {0} to open the passage, then request guidance again.',ja='{0}に乗って通路を開き、もう一度ガイドを呼び出してください。',fr='Marchez sur {0} pour ouvrir le passage, puis redemandez le guidage.',it='Sali su {0} per aprire il passaggio, poi richiedi di nuovo la guida.',de='Tritt auf {0}, um den Durchgang zu öffnen, und fordere dann erneut die Führung an.',es='Pisa {0} para abrir el paso y vuelve a solicitar la guía.',ko='{0}을 밟아 통로를 연 뒤 안내를 다시 요청하세요.',zht='踩上{0}打開通路，然後再次請求引導。',zhc='踩上{0}打开通路，然后再次请求引导。',ru='Наступите на {0}, чтобы открыть проход, затем снова запросите подсказку.',pt='Pise em {0} para abrir a passagem e solicite as orientações novamente.',th='เหยียบ {0} เพื่อเปิดทาง แล้วขอคำแนะนำอีกครั้ง')]
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));assert len(ROW)==12
 for row in ROWS:
  assert len(row)==12
  if'--check'in sys.argv:assert d.get(row['en'])==row
  else:assert row['en']not in d or d[row['en']]==row;d[row['en']]=row
 if'--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Sealed Gate instructions:four keys/all12 supported languages.')
