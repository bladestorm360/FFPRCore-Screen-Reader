"""Map-entry availability notice in the twelve supported UI languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
KEY='Puzzle guide active'
ROW=dict(en=KEY,ja='パズルガイドあり',fr='Guide des énigmes actif',it='Guida agli enigmi attiva',de='Rätselhilfe aktiv',es='Guía de puzles activa',ko='퍼즐 안내 활성',zht='謎題引導已啟用',zhc='谜题引导已启用',ru='Подсказки к загадке активны',pt='Guia de enigmas ativo',th='คำแนะนำปริศนาพร้อมใช้งาน')
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));assert set(ROW)==set(d['Exit to {0}'])and len(ROW)==12
 if '--check'in sys.argv:assert d.get(KEY)==ROW
 else:
  assert KEY not in d or d[KEY]==ROW
  d[KEY]=ROW;p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Puzzle map-entry notice: one key/all12 languages.')
