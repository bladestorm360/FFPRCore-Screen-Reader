"""Independent solution-type controls in all twelve supported UI languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
NAMES={
'en':('Navigation puzzle guidance','Non-navigation puzzle guidance','Navigation puzzle guidance is on','Navigation puzzle guidance is off','Non-navigation puzzle guidance is on','Non-navigation puzzle guidance is off','Puzzle guidance is off'),
'ja':('移動パズルの案内','移動以外のパズルの案内','移動パズルの案内はオン','移動パズルの案内はオフ','移動以外のパズルの案内はオン','移動以外のパズルの案内はオフ','パズルの案内はオフ'),
'fr':('Guide des énigmes de déplacement','Guide des autres énigmes','Guide des énigmes de déplacement activé','Guide des énigmes de déplacement désactivé','Guide des autres énigmes activé','Guide des autres énigmes désactivé','Guide des énigmes désactivé'),
'it':('Guida agli enigmi di navigazione','Guida agli altri enigmi','Guida agli enigmi di navigazione attiva','Guida agli enigmi di navigazione disattivata','Guida agli altri enigmi attiva','Guida agli altri enigmi disattivata','Guida agli enigmi disattivata'),
'de':('Hilfe bei Navigationsrätseln','Hilfe bei anderen Rätseln','Hilfe bei Navigationsrätseln ein','Hilfe bei Navigationsrätseln aus','Hilfe bei anderen Rätseln ein','Hilfe bei anderen Rätseln aus','Rätselhilfe aus'),
'es':('Guía de puzles de navegación','Guía de otros puzles','Guía de puzles de navegación activada','Guía de puzles de navegación desactivada','Guía de otros puzles activada','Guía de otros puzles desactivada','Guía de puzles desactivada'),
'ko':('이동 퍼즐 안내','이동 외 퍼즐 안내','이동 퍼즐 안내 켜짐','이동 퍼즐 안내 꺼짐','이동 외 퍼즐 안내 켜짐','이동 외 퍼즐 안내 꺼짐','퍼즐 안내 꺼짐'),
'zht':('導航謎題引導','非導航謎題引導','導航謎題引導已開啟','導航謎題引導已關閉','非導航謎題引導已開啟','非導航謎題引導已關閉','謎題引導已關閉'),
'zhc':('导航谜题引导','非导航谜题引导','导航谜题引导已开启','导航谜题引导已关闭','非导航谜题引导已开启','非导航谜题引导已关闭','谜题引导已关闭'),
'ru':('Подсказки к навигационным загадкам','Подсказки к другим загадкам','Подсказки к навигационным загадкам включены','Подсказки к навигационным загадкам выключены','Подсказки к другим загадкам включены','Подсказки к другим загадкам выключены','Подсказки к загадкам выключены'),
'pt':('Guia de enigmas de navegação','Guia de outros enigmas','Guia de enigmas de navegação ativado','Guia de enigmas de navegação desativado','Guia de outros enigmas ativado','Guia de outros enigmas desativado','Guia de enigmas desativado'),
'th':('คำแนะนำปริศนาการเดินทาง','คำแนะนำปริศนาประเภทอื่น','เปิดคำแนะนำปริศนาการเดินทาง','ปิดคำแนะนำปริศนาการเดินทาง','เปิดคำแนะนำปริศนาประเภทอื่น','ปิดคำแนะนำปริศนาประเภทอื่น','ปิดคำแนะนำปริศนา'),
}
ROWS={key:{lang:values[i]for lang,values in NAMES.items()}for i,key in enumerate(NAMES['en'])}
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 assert set(NAMES)==set(data['Exit to {0}'])and all(len(v)==7 for v in NAMES.values())
 for key,want in ROWS.items():
  if '--check'in sys.argv:assert data.get(key)==want,key
  else:
   assert key not in data or data[key]==want,key
   data[key]=want
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Puzzle type settings: seven keys/all12 languages.')
