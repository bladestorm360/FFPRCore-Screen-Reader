"""Maintain the manual Nautilus surfacing marker and endpoint in12 languages."""
from pathlib import Path
import json

VALUES={
 "Surface near Doga's Village": {
  'en':"Surface near Doga's Village",'ja':'ドーガの村付近で浮上',
  'fr':'Remonter près du village de Doga','it':'Riemergi vicino al villaggio di Doga',
  'de':'Nahe Dogas Dorf auftauchen','es':'Emerger cerca de la aldea de Doga',
  'ko':'도가의 마을 근처에서 부상','zht':'在多加之村附近浮出水面','zhc':'在多加之村附近浮出水面',
  'ru':'Всплыть возле деревни Доги','th':'ลอยขึ้นใกล้หมู่บ้านโดก้า','pt':'Emergir perto da Vila de Doga'},
 'surface here': {
  'en':'surface here','ja':'ここで浮上','fr':'remontez ici','it':'riemergi qui',
  'de':'hier auftauchen','es':'emerger aquí','ko':'여기서 부상','zht':'在此浮出水面','zhc':'在此浮出水面',
  'ru':'всплыть здесь','th':'ลอยขึ้นที่นี่','pt':'emerja aqui'}
}
if __name__=='__main__':
 path=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
 catalog=json.loads(path.read_text(encoding='utf-8'))
 for key,values in VALUES.items():
  assert set(values)==set(catalog['Can surface']) and all(values.values())
  catalog[key]=values
 path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Nautilus surfacing marker/endpoint:12 supported languages')
