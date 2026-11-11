"""Two concise rescue instructions in all twelve supported locales."""
from pathlib import Path
import json,sys
from author_tzen_guide import START,EXIT
ROOT=Path(__file__).resolve().parents[1]
TEXT={
'en':(START,EXIT),
'ja':('救助を始めてください。６分以内に子供を見つけ、家から脱出してください。','子供を連れて家から出てください。'),
'fr':('Commencez le sauvetage. Trouvez l’enfant et sortez de la maison en six minutes.','Sortez de la maison avec l’enfant.'),
'it':('Inizia il salvataggio. Trova il bambino ed esci dalla casa entro sei minuti.','Esci dalla casa con il bambino.'),
'de':('Beginne die Rettung. Finde das Kind und verlasse das Haus innerhalb von sechs Minuten.','Verlasse das Haus mit dem Kind.'),
'es':('Inicia el rescate. Encuentra al niño y sal de la casa en seis minutos.','Sal de la casa con el niño.'),
'ko':('구조를 시작하세요. 6분 안에 아이를 찾아 집에서 나오세요.','아이를 데리고 집에서 나오세요.'),
'zht':('開始救援。在六分鐘內找到孩子並離開房屋。','帶著孩子離開房屋。'),
'zhc':('开始救援。在六分钟内找到孩子并离开房屋。','带着孩子离开房屋。'),
'ru':('Начните спасение. Найдите ребёнка и выйдите из дома за шесть минут.','Выйдите из дома с ребёнком.'),
'pt':('Inicie o resgate. Encontre a criança e saia da casa em seis minutos.','Saia da casa com a criança.'),
'th':('เริ่มการช่วยเหลือ หาตัวเด็กและออกจากบ้านภายในหกนาที','ออกจากบ้านพร้อมกับเด็ก'),
}
if __name__=='__main__':
 assert len(TEXT)==12
 p=ROOT/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 for index,key in enumerate(TEXT['en']):
  want={lang:values[index]for lang,values in TEXT.items()};assert all(want.values())
  if '--check'in sys.argv:assert data.get(key)==want
  else:assert key not in data or data[key]==want;data[key]=want
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Tzen:two instructions/all12 locales;timer information precedes the start operation.')
