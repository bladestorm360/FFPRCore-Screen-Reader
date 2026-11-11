"""Original five-action Catapult solution and localized instructions.

The initial push switch only gives a hint; no game progress changes there.
Every required action below has an exact native persistent receipt.
"""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
LANGS=('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')
SPECS=((30,'sc_e_0224_1',309,'Examine the planter.'),(31,'sc_e_0224_2',310,'Read the notepad.'),
 (22,'sc_e_0224_3',311,'Examine the urn.'),(23,'sc_e_0224_4',312,'Read the fallen book.'),
 (18,'sc_e_0224_5',313,'Pull the switch and choose Yes.'))
TEXT={
 'Continue into the room. If asked to rest, choose No.':['Continue into the room. If asked to rest, choose No.','部屋の中へ進んでください。休むか聞かれたら「いいえ」を選んでください。','Entrez dans la pièce. Si on vous propose de vous reposer, choisissez Non.','Entra nella stanza. Se ti viene chiesto di riposare, scegli No.','Gehe in den Raum. Wenn du dich ausruhen sollst, wähle Nein.','Entra en la habitación. Si te preguntan si quieres descansar, elige No.','방 안으로 들어가세요. 쉴지 물으면 아니요를 선택하세요.','進入房間。若詢問是否休息，請選擇「否」。','进入房间。若询问是否休息，请选择“否”。','Пройдите в комнату. Если предложат отдохнуть, выберите «Нет».','เดินเข้าไปในห้อง หากถามว่าจะพักหรือไม่ ให้เลือกไม่','Entre no quarto. Se perguntarem se deseja descansar, escolha Não.'],
 'Examine the planter.':['Examine the planter.','花壇を調べてください。','Examinez la jardinière.','Esamina la fioriera.','Untersuche den Blumenkasten.','Examina la jardinera.','화단을 조사하세요.','調查花壇。','调查花坛。','Осмотрите клумбу.','สำรวจแปลงดอกไม้','Examine a jardineira.'],
 'Read the notepad.':['Read the notepad.','メモを読んでください。','Lisez le bloc-notes.','Leggi il blocco degli appunti.','Lies den Notizblock.','Lee el bloc de notas.','메모를 읽으세요.','閱讀記事本。','阅读记事本。','Прочитайте записку.','อ่านสมุดบันทึก','Leia o bloco de notas.'],
 'Examine the urn.':['Examine the urn.','壺を調べてください。','Examinez l’urne.','Esamina l’urna.','Untersuche die Urne.','Examina la urna.','항아리를 조사하세요.','調查罈子。','调查坛子。','Осмотрите урну.','สำรวจไห','Examine a urna.'],
 'Read the fallen book.':['Read the fallen book.','落ちた本を読んでください。','Lisez le livre tombé au sol.','Leggi il libro caduto.','Lies das heruntergefallene Buch.','Lee el libro caído.','떨어진 책을 읽으세요.','閱讀掉落的書。','阅读掉落的书。','Прочитайте упавшую книгу.','อ่านหนังสือที่ตกลงมา','Leia o livro caído.'],
 'Pull the switch and choose Yes.':['Pull the switch and choose Yes.','スイッチを引き、「はい」を選んでください。','Tirez l’interrupteur et choisissez Oui.','Tira l’interruttore e scegli Sì.','Ziehe den Schalter und wähle Ja.','Tira del interruptor y elige Sí.','스위치를 당기고 예를 선택하세요.','拉動開關，並選擇「是」。','拉动开关，并选择“是”。','Потяните рычаг и выберите «Да».','ดึงสวิตช์แล้วเลือกใช่','Puxe o interruptor e escolha Sim.']}
def main():
 maps={int(r['id']):r for r in csv.DictReader((DUMP/'ff5_master/map.txt').open(encoding='utf-8-sig'))};room=maps[293]['map_name'];assert room=='Map_20231_4'
 names={int(r['id']):r['script_name']for r in csv.DictReader((DUMP/'ff5_master/script.txt').open(encoding='utf-8-sig'))}
 objects=[]
 for line in (DUMP/'ff5_objects.jsonl').open(encoding='utf-8'):
  row=json.loads(line)
  if row['submap']==room:objects.append(row)
 info=json.loads((DUMP/'ff5_submaps.json').read_text(encoding='utf-8'))[room];bounds=bake_maps.map_bounds(info,objects);assert bounds[:2]==(0,0)or list(bounds[:2])==[0,0]
 wanted={s[1]for s in SPECS}|{'sc_e_0224'};native={}
 for line in (DUMP/'ff5_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap']==room and r['name']in wanted:native[r['name']]=r
 assert native.keys()==wanted;steps=[];facts=[]
 for entity,script,receipt,text in SPECS:
  actors=[r for r in objects if r['props'].get('entity_id')==entity and names.get(r['props'].get('script_id'))==script];assert len(actors)==1
  actor=actors[0];body=native[script]['mnemonics']
  assert any(m['mnemonic']=='SetFlag'and m['operands']['sValues'][0]=='ScenarioFlag2'and m['operands']['iValues'][0]+256==receipt for m in body)
  steps.append(dict(map=293,entity=entity,raw=actor['name'],layer=bake_maps.layer_index(actor['layer']),x=actor['tx'],y=bake_maps.tile_row(actor),examine=True,text=text,done_scenario=receipt))
  facts.append(dict(entity=entity,script=script,receipt=receipt,sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
 assert not any(m['mnemonic']in('SetFlag','ResetFlag','Calc')for m in native['sc_e_0224']['mnemonics'])
 path=ROOT/'data/ff5/puzzles.json';data=json.loads(path.read_text(encoding='utf-8'));original=json.loads(path.read_text(encoding='utf-8'))
 data['catapult_guide']=dict(id='catapult_clues',maps=[293],done_scenario=313,
  _about='Five native clue operations and actor-visible recovery. The push switch supplies a repeatable hint. Book23 is staged outside the room; current exact native pose owns routing. Final313 opens chest access; item contents remain undisclosed.',steps=steps)
 entry=[r for r in objects if r['props'].get('entity_id')==17 and names.get(r['props'].get('script_id'))=='sc_e_0432'];assert len(entry)==1
 actor=entry[0];data['catapult_guide']['entry_step']=dict(map=293,entity=17,raw=actor['name'],layer=bake_maps.layer_index(actor['layer']),x=actor['tx'],y=bake_maps.tile_row(actor),text='Continue into the room. If asked to rest, choose No.')
 text_path=ROOT/'data/ff5/mod_text.json';catalog=json.loads(text_path.read_text(encoding='utf-8'));expected=dict(catalog)
 for key,values in TEXT.items():assert len(values)==12 and values[0]==key;expected[key]=dict(zip(LANGS,values))
 if '--check'in sys.argv:assert data==original and catalog==expected,'Catapult recipe or locales differ'
 else:path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');text_path.write_text(json.dumps(expected,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 (ROOT/'build_check/ff5_catapult_contract.json').write_text(json.dumps(dict(map=293,controls=facts,optional_hint='sc_e_0224: no persistent state change',final_receipt=313),indent=2)+'\n')
 print('Catapult: five native receipt-backed controls plus rest-contact entry, six instructions/12 locales; staged book identity retained; initial optional hint is not invented progress.')
if __name__=='__main__':main()
