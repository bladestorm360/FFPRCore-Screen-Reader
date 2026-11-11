"""Original Ghido guide resource/locale author. Primary bodies stay external."""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump';LANGS=('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')
TEXT={
 'Take the stone and choose Yes.':['Take the stone and choose Yes.','石を取り、「はい」を選んでください。','Prenez la pierre et choisissez Oui.','Prendi la pietra e scegli Sì.','Nimm den Stein und wähle Ja.','Toma la piedra y elige Sí.','돌을 꺼내고 예를 선택하세요.','取出石頭，並選擇「是」。','取出石头，并选择“是”。','Возьмите камень и выберите «Да».','หยิบหินแล้วเลือกใช่','Pegue a pedra e escolha Sim.'],
 'Place the stone in the northwest chest and choose Yes.':['Place the stone in the northwest chest and choose Yes.','北西の宝箱に石を入れ、「はい」を選んでください。','Placez la pierre dans le coffre au nord-ouest et choisissez Oui.','Metti la pietra nel forziere a nord-ovest e scegli Sì.','Lege den Stein in die nordwestliche Truhe und wähle Ja.','Pon la piedra en el cofre del noroeste y elige Sí.','북서쪽 상자에 돌을 넣고 예를 선택하세요.','將石頭放入西北方的寶箱，並選擇「是」。','将石头放入西北方的宝箱，并选择“是”。','Положите камень в северо-западный сундук и выберите «Да».','วางหินในหีบทางตะวันตกเฉียงเหนือแล้วเลือกใช่','Coloque a pedra no baú a noroeste e escolha Sim.'],
 'Place the stone in the southwest chest and choose Yes.':['Place the stone in the southwest chest and choose Yes.','南西の宝箱に石を入れ、「はい」を選んでください。','Placez la pierre dans le coffre au sud-ouest et choisissez Oui.','Metti la pietra nel forziere a sud-ovest e scegli Sì.','Lege den Stein in die südwestliche Truhe und wähle Ja.','Pon la piedra en el cofre del suroeste y elige Sí.','남서쪽 상자에 돌을 넣고 예를 선택하세요.','將石頭放入西南方的寶箱，並選擇「是」。','将石头放入西南方的宝箱，并选择“是”。','Положите камень в юго-западный сундук и выберите «Да».','วางหินในหีบทางตะวันตกเฉียงใต้แล้วเลือกใช่','Coloque a pedra no baú a sudoeste e escolha Sim.'],
 'Return to the chest room.':['Return to the chest room.','宝箱の部屋に戻ってください。','Retournez dans la salle des coffres.','Torna nella stanza dei forzieri.','Kehre in den Raum mit den Truhen zurück.','Vuelve a la sala de los cofres.','상자가 있는 방으로 돌아가세요.','返回寶箱房間。','返回宝箱房间。','Вернитесь в комнату с сундуками.','กลับไปที่ห้องหีบ','Volte à sala dos baús.'],
 'Follow the opened passage.':['Follow the opened passage.','開いた通路を進んでください。','Suivez le passage ouvert.','Segui il passaggio aperto.','Folge dem geöffneten Durchgang.','Sigue el pasaje abierto.','열린 통로를 따라가세요.','沿著開啟的通道前進。','沿着开启的通道前进。','Пройдите по открытому проходу.','เดินตามทางที่เปิดออก','Siga pela passagem aberta.']}
def main():
 source=ROOT.parent/'tools/ff5_stone_20261102';objects=json.loads((source/'objects.json').read_text(encoding='utf-8'));programs=json.loads((source/'programs.json').read_text(encoding='utf-8'))
 maps={int(r['id']):r['map_name']for r in csv.DictReader((DUMP/'ff5_master/map.txt').open(encoding='utf-8-sig'))};info=json.loads((DUMP/'ff5_submaps.json').read_text(encoding='utf-8'))
 def actor(map_id,entity,kind,text,point=None):
  room=maps[map_id];rows=[r for r in objects if r['submap']==room];selected=[r for r in rows if r['type_name']==kind and r['props'].get('entity_id')==entity and r['group']=='entity_default'];assert len(selected)==1
  o=selected[0]
  if point is not None:assert o['props']['object_id']==point
  ox,oy,_,_=bake_maps.map_bounds(info[room],rows)
  return dict(map=map_id,entity=entity,raw=o['name'],layer=bake_maps.layer_index(o['layer']),x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,text=text)
 chests=[];facts=[]
 for i,entity in enumerate((30,29,28,31,32)):
  row=actor(371,entity,'Entity','Take the stone and choose Yes.');row.update(flag=174+i,occupied_value=0 if i==2 else 1);chests.append(row)
  p=next(p for p in programs if p['name']==f'sc_e_0252_{i+1}');branch=p['mnemonics'][1];assert branch['mnemonic']=='Branch'and branch['operands']['sValues'][0]=='ScenarioFlag1'and branch['operands']['iValues'][:3]==[174+i,1-row['occupied_value'],18]
  facts.append(dict(script=p['name'],sha256=hashlib.sha256(json.dumps(p['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
 guide=dict(id='ghido_stone_chests',maps=[370,371,372,373],chests=chests,first_switch=actor(370,15,'Entity','Examine the switch'),wall_switch=actor(372,15,'Entity','Examine the switch'),lower_entry=actor(372,2,'PointIn','Follow the opened passage.',1),finish_entry=actor(373,2,'PointIn','Follow the opened passage.',1),room_return=dict(map=371,layer=0,x=9,y=13,text='Return to the chest room.'),place_northwest='Place the stone in the northwest chest and choose Yes.',place_southwest='Place the stone in the southwest chest and choose Yes.')
 path=ROOT/'data/ff5/puzzles.json';data=json.loads(path.read_text(encoding='utf-8'));expected=dict(data);expected['stone_guide']=guide
 textpath=ROOT/'data/ff5/mod_text.json';catalog=json.loads(textpath.read_text(encoding='utf-8'));translated=dict(catalog)
 for key,values in TEXT.items():assert len(values)==12 and values[0]==key;translated[key]=dict(zip(LANGS,values))
 if '--check'in sys.argv:assert data==expected and catalog==translated,'Ghido resource/locales differ'
 else:path.write_text(json.dumps(expected,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');textpath.write_text(json.dumps(translated,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 (ROOT/'build_check/ff5_stone_contract.json').write_text(json.dumps(dict(controls=facts,scope_maps=[370,371,372,373],native_flags=[174,175,176,177,178,179,294],center_occupied_value=0),indent=2)+'\n')
 print('Ghido resource: five exact chest identities/inverted center receipt, native switches and actual floor points; five new instructions/12locales; other guide resources preserved.')
if __name__=='__main__':main()
