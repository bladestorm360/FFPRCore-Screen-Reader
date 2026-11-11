"""Exact manual turtle action and native follow wait in all twelve languages."""
from pathlib import Path
import json,sys
from author_gerad_guide import CROSS,WAIT
ROOT=Path(__file__).resolve().parents[1]
ROWS={
 CROSS:dict(en=CROSS,ja='{0}を向き、渡るまで繰り返し調べてください。',fr='Faites face à {0} et interagissez jusqu’à avoir traversé.',it='Voltati verso {0} e continua a interagire finché non attraversi.',de='Blicke nach {0} und interagiere wiederholt, bis du hinüber bist.',es='Mira hacia {0} e interactúa hasta cruzar.',ko='{0}쪽을 보고 건널 때까지 계속 상호작용하세요.',zht='面向{0}，持續互動直到渡過。',zhc='面向{0}，持续互动直到渡过。',ru='Повернитесь на {0} и взаимодействуйте, пока не переправитесь.',pt='Vire para {0} e continue interagindo até atravessar.',th='หันไปทาง{0} แล้วกดโต้ตอบต่อไปจนข้ามได้'),
 WAIT:dict(en=WAIT,ja='ジェラドが止まるまで待ち、再度案内を求めてください。',fr='Attendez que Gerad s’arrête, puis redemandez conseil.',it='Attendi che Gerad si fermi, poi richiedi indicazioni.',de='Warte, bis Gerad stehen bleibt, und fordere erneut Hinweise an.',es='Espera a que Gerad se detenga y vuelve a pedir indicaciones.',ko='제라드가 멈추면 다시 안내를 요청하세요.',zht='等傑拉德停下，再次要求指引。',zhc='等杰拉德停下，再次请求指引。',ru='Подождите, пока Джерад остановится, затем снова запросите подсказку.',pt='Espere Gerad parar e peça orientação novamente.',th='รอให้เจราดหยุด แล้วขอคำแนะนำอีกครั้ง')}
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));languages=set(d['Exit to {0}'])
 for key,row in ROWS.items():
  assert set(row)==languages and len(row)==12
  if '--check'in sys.argv:assert d.get(key)==row
  else:assert key not in d or d[key]==row;d[key]=row
 if '--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Gerad: two manual/wait instructions, all12 languages.')
