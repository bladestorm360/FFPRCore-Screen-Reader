"""One party-setup instruction, with the canonical localized Terra name."""
from pathlib import Path
import csv,json,sys
ROOT=Path(__file__).resolve().parents[1];D=ROOT.parent/'tools/mapdump/ff6_master'
KEY='Bring Terra and at least one other character in the active party.'
TEXT={
'en':'Bring {0} and at least one other character in the active party.',
'ja':'{0}ともう一人以上のキャラクターを現在のパーティに入れてください。',
'fr':'Placez {0} et au moins un autre personnage dans le groupe actif.',
'it':'Metti {0} e almeno un altro personaggio nel gruppo attivo.',
'de':'Nimm {0} und mindestens einen weiteren Charakter in die aktive Gruppe auf.',
'es':'Incluye a {0} y al menos otro personaje en el grupo activo.',
'ko':'{0}와 다른 캐릭터를 한 명 이상 현재 파티에 넣으세요.',
'zht':'將{0}及至少另一名角色加入目前隊伍。',
'zhc':'将{0}及至少另一名角色加入当前队伍。',
'ru':'Включите {0} и хотя бы ещё одного персонажа в активную группу.',
'pt':'Coloque {0} e pelo menos mais um personagem no grupo ativo.',
'th':'ใส่{0}และตัวละครอื่นอย่างน้อยหนึ่งคนในปาร์ตี้ปัจจุบัน',
}
FLY='Interact to fly the airship.'
FLIGHT=dict(zip(TEXT,(
 'Interact to fly the airship.',
 '調べて飛空艇を操縦してください。',
 'Interagissez pour piloter le vaisseau.',
 "Interagisci per pilotare l'aeronave.",
 'Interagiere, um das Luftschiff zu fliegen.',
 'Interactúa para pilotar el barco volador.',
 '상호작용하여 비공정을 조종하세요.',
 '互動以駕駛飛空艇。',
 '互动以驾驶飞空艇。',
 'Взаимодействуйте, чтобы управлять воздушным кораблём.',
 'Interaja para pilotar a aeronave.',
 'โต้ตอบเพื่อขับเรือเหาะ',
)))
def main():
 actor=next(r for r in csv.DictReader((D/'character_default_name.txt').open(encoding='utf-8-sig'))if r['character_id']=='1');assert actor['tag_name']=='TINA'
 values={}
 for lang,template in TEXT.items():
  system=dict(line.split('\t',1)for line in(D/('system_'+lang+'.txt')).read_text(encoding='utf-8-sig').splitlines()if'\t'in line);values[lang]=template.format(system[actor['mes_id_name']])
 assert values['en']==KEY and all(values.values())
 p=ROOT/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert data.get(KEY)==values and data.get(FLY)==FLIGHT
 else:
  assert KEY not in data or data[KEY]==values;assert FLY not in data or data[FLY]==FLIGHT
  data[KEY]=values;data[FLY]=FLIGHT;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Imperial party setup and manual flight:two keys/all12 languages,actual canonical character name.')
if __name__=='__main__':main()
