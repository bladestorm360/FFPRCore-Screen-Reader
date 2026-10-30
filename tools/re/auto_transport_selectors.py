"""Bound the automatic-vehicle selector-order uncertainty using real bakes.

Read-only: compare every indexed world selector at an automatic vehicle's
terrain with its membership for Player1 and that actual automatic transport ID.
This does not execute native movement or infer hidden progression/ownership.
"""
from pathlib import Path
import csv,hashlib,json,struct,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from check_scripted_bakes import decode

ROOT=Path(__file__).resolve().parents[2]

def run(destination):
    report=[]
    for number in range(1,7):
        bake=ROOT/f'data/ff{number}/maps.bin'
        metadata=ROOT/f'data/ff{number}/mapstate.json'
        rooms,_=decode(bake)
        state=json.loads(metadata.read_text(encoding='utf-8-sig'))
        master=ROOT.parent/f'tools/mapdump/ff{number}_master'
        with (master/'transportation.txt').open(encoding='utf-8-sig')as source:
            autos=[r for r in csv.DictReader(source)if r['type_id']=='5']
        with (master/'transportation_group.txt').open(encoding='utf-8-sig')as source:
            groups={int(r[0]):{i for i,v in enumerate(r[1:],1)if v=='1'}for r in list(csv.reader(source))[1:]if r}
        differences=[];worlds=0;selectors=0
        for name,room in rooms.items():
            if not room['shape'][3]or not room['grids'][-1]:continue
            worlds+=1;terrain=[]
            raw=room['grids'][-1]
            for at in range(0,len(raw),3):
                count,value=struct.unpack_from('<HB',raw,at);terrain.extend([value]*count)
            width,height=room['shape'][1:3];assert len(terrain)==width*height
            for fingerprint,value in state.get(name,{}).get('transport',{}).items():
                kind,eid,layer,x,y=map(int,fingerprint.split(':'))
                if not (0<=x<width and 0<=y<height):continue
                selectors+=1;attribute=terrain[y*width+x]
                for group,variants in value.items():
                    for ids in variants.values():
                        for auto in autos:
                            aid=int(auto['id'])
                            if attribute in groups[int(auto['transportation_group_id'])]and ((1 in ids)!=(aid in ids)):
                                differences.append(dict(map=name,fingerprint=fingerprint,group=group,ids=ids,terrain=attribute,auto=aid))
        sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
        report.append(dict(game=number,worlds=worlds,indexed_world_selectors=selectors,
                           automatic_ids=[int(r['id'])for r in autos],differences=differences,
                           inputs={p.name:sha(p)for p in (bake,metadata,master/'transportation.txt',master/'transportation_group.txt')}))
        print(f'FF{number}: {worlds} worlds, {selectors} indexed world selectors, {len(differences)} foot/auto boundary differences')
    Path(destination).write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')

if __name__=='__main__':run(sys.argv[1])
