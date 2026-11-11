"""Original concise native-unlock prompt, all12 languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
KEY='Interact to unlock the door.'
ROW=dict(en=KEY,ja='調べて扉の鍵を開けてください。',fr='Interagissez pour déverrouiller la porte.',it='Interagisci per aprire la serratura della porta.',de='Interagiere, um die Tür aufzuschließen.',es='Interactúa para abrir la cerradura de la puerta.',ko='상호작용하여 문의 잠금을 해제하세요.',zht='互動以解開門鎖。',zhc='互动以解开门锁。',ru='Взаимодействуйте, чтобы отпереть дверь.',pt='Interaja para destrancar a porta.',th='โต้ตอบเพื่อปลดล็อกประตู')
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));assert set(ROW)==set(d['Exit to {0}'])and len(ROW)==12
 if'--check'in sys.argv:assert d.get(KEY)==ROW
 else:assert KEY not in d or d[KEY]==ROW;d[KEY]=ROW;p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Narshe native-unlock instruction:all12 languages.')
