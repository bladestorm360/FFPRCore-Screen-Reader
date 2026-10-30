"""Retain the eight exact duplicate-PointIn contexts outside every Git tree.

Expected cells use the verified native rectangle-center conversion, independently
of the baker's PointIn choice. Original objects/initializers never enter Git.
"""
import csv
import hashlib
import json
import math
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools'))
import bake_maps

DUMP=ROOT.parent/'tools/mapdump'
OUT=ROOT.parent/'tools/conditional_arrivals_20261029'


def reachable(script):
    if not script:return [],[],[]
    rows=script.get('mnemonics')or[]
    todo=[next((v['EntryPoint']for v in script.get('segments',[])if v['Label']=='Main'),0)]
    seen=set();player=[];syscalls=[]
    while todo:
        pc=todo.pop()
        if pc in seen or not 0<=pc<len(rows):continue
        assert len(seen)<8192
        seen.add(pc);m=rows[pc];op=m['mnemonic'];v=m.get('operands')or{};iv=v.get('iValues')or[]
        if op=='SetPuppet'and len(iv)>2 and iv[1]in(0,-1001):player.append(pc)
        if op=='SysCall':syscalls.append((pc,(v.get('sValues')or[''])[0]))
        if op in('Exit','Return'):continue
        if op=='Jump':todo.append(iv[0]);continue
        if op=='Call':todo.append(iv[0])
        if op=='Branch':todo.append(iv[2])
        todo.append(pc+1)
    return sorted(seen),player,syscalls


def main():
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    result=[]
    for n in(3,5):
        facts=json.loads((ROOT.parent/f'tools/map_event_audit_20261028/ff{n}_facts.json').read_text(encoding='utf-8'))
        with (DUMP/f'ff{n}_master/map.txt').open(encoding='utf-8-sig')as file:
            maps={int(r['id']):r for r in csv.DictReader(file)if r.get('id')}
        needed={maps[r['destination']]['map_name']for r in facts['ambiguous_door_arrivals']}
        needed.update(r['submap']for r in facts['ambiguous_door_arrivals'])
        objects=[];scripts={}
        with (DUMP/f'ff{n}_objects.jsonl').open(encoding='utf-8')as file:
            for line in file:
                o=json.loads(line)
                if o['submap']in needed:objects.append(o)
        with (DUMP/f'ff{n}_scripts.jsonl').open(encoding='utf-8')as file:
            for line in file:
                s=json.loads(line)
                if s['submap']in needed and s['name'].lower()=='sc_'+s['submap'].lower():scripts[s['submap']]=s
        submaps=json.loads((DUMP/f'ff{n}_submaps.json').read_text(encoding='utf-8'))
        for row in facts['ambiguous_door_arrivals']:
            dest=maps[row['destination']]['map_name']
            room=[o for o in objects if o['submap']==dest]
            bounds=bake_maps.map_bounds(submaps[dest],room)
            points=[o for o in room if o['type']==0 and o['props'].get('object_id')==row['point']]
            defaults=[o for o in points if o['group']in('entity_default','default','')]
            assert defaults
            first=defaults[0];world=dest in('Map_10010','Map_10040')
            # Native ParseMapObjectGroupData centers original dimensions. Cell
            # conversion floors that center, with native Y inverted.
            cell=[1 if world else bake_maps.layer_index(first['layer']),
                  math.floor((first['x']+first['w']/2)/16)-bounds[0],
                  math.floor((first['y']-first['h']/2)/16)-bounds[1]]
            source,=[o for o in objects if o['submap']==row['submap']and o['type']==3 and o['props'].get('entity_id')==row['id']]
            assert not source['props'].get('script_id')
            initializer=scripts.get(dest);seen,player,syscalls=reachable(initializer)
            assert not player and not syscalls,(n,dest,player,syscalls)
            result.append(dict(game=n,source=row,source_object=source,submap=dest,points=points,
                               default_point=first,expected_cell=cell,initializer=initializer,
                               initializer_sha256=hashlib.sha256(json.dumps(initializer,ensure_ascii=False,sort_keys=True,separators=(',',':')).encode()).hexdigest(),
                               main_reachable=seen,main_player_tracks=player,main_syscalls=syscalls))
            print(f'FF{n} {row["submap"]}/{row["id"]}: native default entity{first["props"]["entity_id"]}, cell{cell}; no Main player track/SysCall')
    assert len(result)==8
    (OUT/'contexts.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')


if __name__=='__main__':main()
