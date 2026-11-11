"""Infiltration actions and native option positions in the twelve UI languages."""
from pathlib import Path
import json,sys
from author_south_figaro_guide import MERCHANT,SOLDIER,FIRST,SECOND,RECOVERY
ROOT=Path(__file__).resolve().parents[1]
ROWS={
 MERCHANT:dict(en=MERCHANT,ja='{0}に「ぬすむ」を使って商人の服を奪い、戦闘後に再度案内を求めてください。',fr='Utilisez Voler sur {0} pour prendre le déguisement de marchand, puis redemandez conseil après le combat.',it='Usa Ruba contro {0} per ottenere il travestimento da mercante, poi richiedi indicazioni dopo la battaglia.',de='Benutze Stehlen gegen {0}, um die Händlerverkleidung zu erhalten. Fordere nach dem Kampf erneut Hinweise an.',es='Usa Robar contra {0} para conseguir el disfraz de mercader y vuelve a pedir indicaciones después del combate.',ko='{0}에게 훔치기를 사용해 상인 변장을 얻은 뒤 전투 후 다시 안내를 요청하세요.',zht='對{0}使用偷竊取得商人服裝，戰鬥後再次要求指引。',zhc='对{0}使用偷窃取得商人服装，战斗后再次请求指引。',ru='Используйте кражу против {0}, чтобы получить одежду торговца, затем снова запросите подсказку после боя.',pt='Use Roubar contra {0} para conseguir o disfarce de mercador e peça orientação novamente após a batalha.',th='ใช้ขโมยกับ {0} เพื่อรับชุดปลอมตัวพ่อค้า แล้วขอคำแนะนำอีกครั้งหลังการต่อสู้'),
 SOLDIER:dict(en=SOLDIER,ja='{0}に「ぬすむ」を使って兵士の服を奪い、戦闘後に再度案内を求めてください。',fr='Utilisez Voler sur {0} pour prendre le déguisement de soldat, puis redemandez conseil après le combat.',it='Usa Ruba contro {0} per ottenere il travestimento da soldato, poi richiedi indicazioni dopo la battaglia.',de='Benutze Stehlen gegen {0}, um die Soldatenverkleidung zu erhalten. Fordere nach dem Kampf erneut Hinweise an.',es='Usa Robar contra {0} para conseguir el disfraz de soldado y vuelve a pedir indicaciones después del combate.',ko='{0}에게 훔치기를 사용해 병사 변장을 얻은 뒤 전투 후 다시 안내를 요청하세요.',zht='對{0}使用偷竊取得士兵服裝，戰鬥後再次要求指引。',zhc='对{0}使用偷窃取得士兵服装，战斗后再次请求指引。',ru='Используйте кражу против {0}, чтобы получить солдатскую форму, затем снова запросите подсказку после боя.',pt='Use Roubar contra {0} para conseguir o disfarce de soldado e peça orientação novamente após a batalha.',th='ใช้ขโมยกับ {0} เพื่อรับชุดปลอมตัวทหาร แล้วขอคำแนะนำอีกครั้งหลังการต่อสู้'),
 FIRST:dict(en=FIRST,ja='1番目の選択肢を選んでください。',fr='Choisissez la première option.',it='Scegli la prima opzione.',de='Wähle die erste Option.',es='Elige la primera opción.',ko='첫 번째 선택지를 고르세요.',zht='選擇第一個選項。',zhc='选择第一个选项。',ru='Выберите первый вариант.',pt='Escolha a primeira opção.',th='เลือกตัวเลือกแรก'),
 SECOND:dict(en=SECOND,ja='2番目の選択肢を選んでください。',fr='Choisissez la deuxième option.',it='Scegli la seconda opzione.',de='Wähle die zweite Option.',es='Elige la segunda opción.',ko='두 번째 선택지를 고르세요.',zht='選擇第二個選項。',zhc='选择第二个选项。',ru='Выберите второй вариант.',pt='Escolha a segunda opção.',th='เลือกตัวเลือกที่สอง')}
ROWS[RECOVERY]={lang:ROWS[SECOND][lang]+' '+ROWS[MERCHANT][lang]for lang in ROWS[SECOND]}
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));langs=set(d['Exit to {0}']);assert len(langs)==12
 for key,row in ROWS.items():
  assert set(row)==langs and all(row.values())
  if '--check' in sys.argv:assert d.get(key)==row,key
  else:assert key not in d or d[key]==row,key;d[key]=row
 if '--check' not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('South Figaro: five instructions/all12 languages.')
