"""Exact native chained teleport landing; source bodies never enter Git."""
from pathlib import Path
import hashlib,json,sys

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
def sha(value):return hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()

def main():
    source={(s['submap'],s['name']):s for s in map(json.loads,(DUMP/'ff4_scripts.jsonl').open(encoding='utf-8-sig'))}
    family=next(f for f in json.loads((ROOT.parent/'tools/map_event_audit_20261028/ff4_context_triage.json').read_text(encoding='utf-8'))['families']if f['script']=='sc_e_0031')
    assert not family['gaps']
    for row in family['bodies']:assert sha(source[row['submap'],row['script']]['mnemonics'])==row['body_sha256']
    root=source['Map_20061_9','sc_e_0031'];middle=source['Map_10010','sc_e_0031_1'];final=source['Map_20021_5','sc_e_0031_2']
    def transfers(s):
        main=next(p for p in s['segments']if p['Label']=='Main')
        return [(*m['operands']['iValues'][:2],m['operands']['sValues'][0])for m in s['mnemonics'][main['EntryPoint']:main['EntryPoint']+main['Count']]if m['mnemonic']=='ChangeMap']
    assert transfers(root)==[(1,14,'sc_e_0031_1')]
    assert transfers(middle)==[(29,1,'sc_e_0031_2')]
    assert transfers(final)==[]
    player_positions=[]
    for m in final['mnemonics']:
        if m['mnemonic']!='SetPuppet'or m['operands']['iValues'][1]not in(-1001,0):continue
        for move in final['mnemonics'][m['operands']['iValues'][2]:]:
            if move['mnemonic']=='Exit':break
            assert move['mnemonic']not in {'Move','MoveTo','Lerp','Route'},'Final player movement now needs another source review'
            if move['mnemonic']=='SetPos':player_positions.append(move['operands']['iValues'][:3])
    assert player_positions==[[6,6,0],[6,6,0]],player_positions
    entry=dict(submap='Map_20061_9',entity_id=5,object_id=5,destination_map=29,original_point=1,
               continuation='sc_e_0031_2',continuation_sha256=sha(final),arrival_cell=[6,6,0],arrival_baked_cell=[6,6,0],
               evidence='Complete native Main chain through an intermediate world scene; final controlled player and first-party SetPos6,6, unchanged Bottom layer; source hashes in tools/silent_family_decisions.json')
    if '--apply'in sys.argv:
        path=ROOT/'data/ff4/warp_arrival_overrides.json';rows=json.loads(path.read_text(encoding='utf-8'))if path.exists()else[]
        matched=[r for r in rows if r['submap']==entry['submap']and r['entity_id']==5]
        if matched:assert matched==[entry]
        else:rows.append(entry);path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FFIV Devil Road: exact two native Main transfers and two final player SetPos6,6,L0; final source SHA256',sha(final))

if __name__=='__main__':main()
