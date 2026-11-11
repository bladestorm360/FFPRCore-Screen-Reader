"""Short Mobliz puzzle operations in every supported UI language."""
from pathlib import Path
import json,sys,re
ROOT=Path(__file__).resolve().parents[1]
TEXT={
'en':('Talk to {0}, then close the shop menu.','Leave the village, then return and request guidance again.'),
'ja':('{0}に話しかけ、店のメニューを閉じてください。','村を出てから戻り、もう一度ガイドを呼び出してください。'),
'fr':('Parlez à {0}, puis fermez le menu de la boutique.','Quittez le village, puis revenez et demandez à nouveau le guide.'),
'it':('Parla con {0}, poi chiudi il menu del negozio.','Esci dal villaggio, poi torna e richiedi di nuovo la guida.'),
'de':('Sprich mit {0} und schließe dann das Ladenmenü.','Verlasse das Dorf, kehre zurück und rufe die Hilfe erneut auf.'),
'es':('Habla con {0} y cierra el menú de la tienda.','Sal del pueblo, vuelve y pide la guía otra vez.'),
'ko':('{0}에게 말을 건 다음 상점 메뉴를 닫으세요.','마을을 나갔다 돌아온 뒤 안내를 다시 요청하세요.'),
'zht':('與{0}交談，然後關閉商店選單。','離開村莊後返回，再次請求引導。'),
'zhc':('与{0}交谈，然后关闭商店菜单。','离开村庄后返回，再次请求引导。'),
'ru':('Поговорите с {0}, затем закройте меню магазина.','Выйдите из деревни, затем вернитесь и снова запросите подсказку.'),
'pt':('Fale com {0} e feche o menu da loja.','Saia da vila, volte e peça orientação novamente.'),
'th':('คุยกับ{0} แล้วปิดเมนูร้านค้า','ออกจากหมู่บ้านแล้วกลับมา จากนั้นขอคำแนะนำอีกครั้ง'),
}
if __name__=='__main__':
 p=ROOT/'data/ff6/mod_text.json';d=json.loads(p.read_text(encoding='utf-8-sig'))
 assert set(TEXT)==set(json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf-8'))['Exit to {0}'])
 for i,key in enumerate(TEXT['en']):
  row={lang:v[i]for lang,v in TEXT.items()};assert all(set(re.findall(r'\{\d+\}',s))==set(re.findall(r'\{\d+\}',key))for s in row.values())
  if '--check'in sys.argv:assert d.get(key)==row,key
  else:
   assert key not in d or d[key]==row,key
   d[key]=row
 if '--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Mobliz:two original instruction keys/all12 languages.')
