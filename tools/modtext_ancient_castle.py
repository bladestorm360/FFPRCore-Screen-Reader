"""One concise reused statue instruction in all twelve supported languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
ROW=dict(en='Examine the statue',ja='石像を調べてください',fr='Examinez la statue',it='Esamina la statua',de='Untersuche die Statue',es='Examina la estatua',ko='석상을 조사하세요',zht='調查石像',zhc='调查石像',ru='Осмотрите статую',pt='Examine a estátua',th='สำรวจรูปปั้น')
def main():
    path=ROOT/'data/common/mod_text.json';data=json.loads(path.read_text(encoding='utf-8'));key=ROW['en']
    if'--check'in sys.argv:assert data.get(key)==ROW
    else:
        assert key not in data or data[key]==ROW
        data[key]=ROW;path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Statue instruction: twelve languages, no reward-name disclosure.')
if __name__=='__main__':main()
