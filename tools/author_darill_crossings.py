"""Exact native turtle travel metadata; no script implementation is shipped."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
HASHES={
 'Map_30651_8/sc_e_0690':'626e351645cd8c72a3cc7cb2c244b7e33a7e54f7f525f0f7888a180b2cc1c9c3',
 'Map_30651_8/sc_e_0691':'9a98cb073cab91da681a0a58306a6f623b98636efc3104b464d7fca927a2cc19',
 'Map_30651_8/sc_e_0687':'b52af75563db1a888e2aeca7430610739fd340e37844075b73e4f53b1d05b827',
 'Map_30651_7/sc_e_0687_1':'ef1628a14671ad4fcb42957cf0ad3bb4b43339fdef5cee5946485cd14edd343f'}
def sha(value):return hashlib.sha256(json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def author(owner):
    import bake_maps
    for locator,wanted in HASHES.items():
        room,name=locator.split('/');assert sha(owner.SCRIPTS[room,name]['mnemonics'])==wanted,locator
    rows=[];infos=json.loads((owner.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))
    for entity,group,name,landing in((22,'entity_default','sc_e_0690',[79,8,1]),(23,'entity_default','sc_e_0690',[79,8,1]),(20,'ev_e_0688','sc_e_0691',[71,13,1]),(26,'entity_default','sc_e_0687_1',[56,14,1])):
        actor=owner.physical(463,entity,group);dest=462 if entity==26 else 463;room=owner.MAPS[dest]['map_name'];body=owner.SCRIPTS[room,name]
        # The last player-puppet block ends with an absolute Lerp. Native
        # puppet coordinates have no tile-object Y-1 adjustment.
        entries=[m['operands']['iValues'][2]for m in body['mnemonics']if m['mnemonic']=='SetPuppet'and m['operands']['iValues'][1]==-1001]
        moves=[]
        for entry in entries:
            for m in body['mnemonics'][entry:]:
                if m['mnemonic']=='Exit':break
                if m['mnemonic']=='Lerp':moves.append(m['operands']['iValues'][:2])
        assert moves[-1]==landing[:2]
        ox,oy,*_=bake_maps.map_bounds(infos[room],[o for o in owner.OBJECTS if o['submap']==room])
        row=dict(submap=actor['submap'],entity_id=entity,object_id=actor['id'],destination_map=dest,original_point=2 if entity==26 else 0,continuation=name,continuation_sha256=sha(body),arrival_cell=landing,arrival_baked_cell=[landing[0]-ox,landing[1]-oy,1],evidence='Exact native turtle player-puppet final landing; area1 records the second crossing side. docs/re/DARILLS_TOMB_20261103.md')
        if entity!=26:row.update(same_map=True,interaction_travel=True,approach_mask=actor['props']['accept_action_direction'])
        rows.append(row)
    return rows
if __name__=='__main__':
    import author_ff6_story_guides as owner
    path=ROOT/'data/ff6/warp_arrival_overrides.json';old=json.loads(path.read_text(encoding='utf-8'));expected=author(owner)
    for row in expected:
        matches=[r for r in old if(r['submap'],r.get('object_id'))==(row['submap'],row['object_id'])]
        if'--check'in sys.argv:assert matches==[row]
        elif matches:assert len(matches)==1;old[old.index(matches[0])]=row
        else:old.append(row)
    if'--check'not in sys.argv:path.write_text(json.dumps(old,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Darill turtle metadata: three exact same-floor manual arcs and one corrected return landing/four source hashes.')
