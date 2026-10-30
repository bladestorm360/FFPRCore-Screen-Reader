"""Compare an archived bake set with a proposed scripted-transfer classification.

Only reviewed ordinary-transfer flags/new arcs may change; native objects,
terrain, dimensions, masters and existing route destinations must be preserved.
Usage: python tools/check_scripted_bakes.py BEFORE_DIR GENERATED_DIR REPORT
BEFORE_DIR contains ffN_maps.bin; GENERATED_DIR contains ffN/maps.bin.
"""
import collections,hashlib,json,struct,sys
from pathlib import Path

def decode(path):
    data=path.read_bytes();header=struct.unpack_from('<8sIB3s12I',data)
    strings=[]
    for i in range(header[5]):
        at=struct.unpack_from('<I',data,header[4]+4*i)[0]
        strings.append(data[at:data.index(0,at)].decode())
    def grid(at,count):
        if not at:return b''
        start=at;cells=0
        while cells<count:
            run,value=struct.unpack_from('<HB',data,at);assert run;cells+=run;at+=3
        assert cells==count
        return data[start:at]
    rooms={}
    for i in range(header[7]):
        row=struct.unpack_from('<IihhBBBB10I',data,header[6]+56*i)
        layers=[];at=row[15]
        for layer in range(row[6]):
            size=struct.unpack_from('<I',data,at)[0];layers.append(data[at+4:at+4+size]);at+=4+size
        objects=[]
        for k in range(row[17]):
            obj=struct.unpack_from('<BBhhHHiiiiII',data,row[16]+34*k)
            objects.append((*obj[:10],strings[obj[10]],strings[obj[11]]))
        rooms[strings[row[0]]]=dict(shape=row[1:8],grids=[grid(at,row[2]*row[3]) for at in row[8:15]],transport=layers,objects=objects)
    names=list(rooms)
    maps=[(mid,names[index]) for mid,index in (struct.unpack_from('<iI',data,header[8]+8*i) for i in range(header[9]))]
    transports=[]
    for i in range(header[11]):
        row=struct.unpack_from('<iiiiI',data,header[10]+20*i);transports.append((*row[:4],strings[row[4]]))
    groups=[data[header[12]:header[12]+8*header[13]],data[header[14]:header[14]+8*header[15]]]
    return rooms,(header[:4],maps,transports,groups)

def main():
    before,after,report=map(Path,sys.argv[1:]);result={}
    from event_label_identities import WORLD_EVENT_REVIEWS
    approved_submarines={(r['game'],r['submap'],r['entity_id'],*r['primary_transfer'][:2])
                         for r in WORLD_EVENT_REVIEWS if r.get('vehicle_entry')=='submarine'}
    approved_conditionals={(r['game'],r['submap'],r['entity_id'],r['raw'],r['group'],*target[:2])
                           for r in WORLD_EVENT_REVIEWS for target in r.get('conditional_transfers',[])}
    chains={};synthetic={}
    for r in WORLD_EVENT_REVIEWS:
        if 'final_transfer'not in r:continue
        n=r['game'];corrections=json.loads((Path(__file__).resolve().parents[1]/f'data/ff{n}/warp_arrival_overrides.json').read_text(encoding='utf-8'))
        index,correction=next((i,c)for i,c in enumerate(corrections)if (c['submap'],c['entity_id'],c.get('object_id'))==(r['submap'],r['entity_id'],r['id']))
        assert n==4 and r['script_name']=='sc_e_0031'and correction['arrival_baked_cell']==[6,6,0]
        target=(correction['destination_map'],-100000-index)
        chains[n,r['submap'],r['entity_id'],r['raw'],r['group']]=(tuple(r['primary_transfer'][:2]),target)
        synthetic[n,'Map_20021_5']=(target[1],6,6,0)
    for n in range(1,7):
        old,old_masters=decode(before/f'ff{n}_maps.bin');new,new_masters=decode(after/f'ff{n}/maps.bin')
        assert old_masters==new_masters and old.keys()==new.keys(),('Masters/submaps changed',n)
        promoted=[];added=[];submarine=[];remapped=[];arrivals=[]
        for name,a in old.items():
            b=new[name]
            assert a['shape']==b['shape'] and a['grids']==b['grids'] and a['transport']==b['transport'],('Geometry changed',n,name)
            def virtual(o):
                expected=synthetic.get((n,name))
                return expected and o[0]==0 and (o[6],o[2],o[3],o[1])==expected and o[8]==-1 and o[10:]==('Reviewed trap landing','')
            native=lambda rows:collections.Counter(o for o in rows if o[0]!=100 and not virtual(o))
            assert native(a['objects'])==native(b['objects']),('Native objects changed',n,name)
            arrivals.extend([name,o[6],o[2],o[3],o[1]]for o in b['objects']if virtual(o)and o not in a['objects'])
            normalized=lambda o:(*o[:9],o[9]&~(65536|262144),*o[10:])
            original=collections.Counter()
            for o in a['objects']:
                if o[0]!=100:continue
                chain=chains.get((n,name,o[8],o[10],o[11]))
                if chain:
                    assert o[6:8]==chain[0],('Original chained transfer changed',n,name)
                    replacement=(*o[:6],*chain[1],*o[8:]);original[normalized(replacement)]+=1
                    remapped.append([name,o[8],list(chain[0]),list(chain[1])])
                else:original[normalized(o)]+=1
            proposed=collections.Counter(normalized(o) for o in b['objects'] if o[0]==100)
            assert not original-proposed,('Existing route arc changed',n,name)
            old_flags={o for o in a['objects'] if o[0]==100 and o[9]&65536}
            assert old_flags<={o for o in b['objects'] if o[0]==100 and o[9]&65536},('Ordinary transfer lost',n,name)
            for o in b['objects']:
                if o[0]!=100:continue
                if o[9]&262144:
                    assert (n==3 and name=='Map_10030' and (o[8],o[6],o[7]) in {(4,242,1),(5,242,1),(6,234,1),(7,269,1)}) or (n,name,o[8],o[6],o[7])in approved_submarines,('Unreviewed submarine entry',n,name,o)
                    submarine.append([name,o[8],o[6],o[7]])
                if proposed[normalized(o)]>original[normalized(o)]:
                    assert (o[9]&65536 or (n,name,o[8],o[10],o[11],o[6],o[7])in approved_conditionals) and any(p[0] in (2,28) and p[1:6]==o[1:6] and p[8]==o[8] and p[10:]==o[10:] for p in b['objects']),('New arc lacks native trigger',n,name)
                    added.append([name,o[8],o[6],o[7]])
                if o[9]&65536 and o not in old_flags:promoted.append([name,o[8],o[6],o[7]])
        result[str(n)]={'promoted':promoted,'submarine_entries':submarine,'added_arcs':added,'remapped_chains':remapped,'synthetic_arrivals':arrivals,'sha256':hashlib.sha256((after/f'ff{n}/maps.bin').read_bytes()).hexdigest()}
        print(f'FF{n}: {len(promoted)} ordinary transfers promoted, {len(added)} new verified arcs; native objects/geometry/masters/old arcs preserved')
    report.write_text(json.dumps(result,indent=2)+'\n')

if __name__=='__main__':main()
