"""Author/check native loop axes in existing own bakes without changing geometry.

The normal full bake uses the same map-master resolver/flag encoder. Only each
56-byte submap record's flag byte may change; every other byte is preserved.
"""
from pathlib import Path
import argparse, hashlib, json, struct
import bake_maps as bake

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--check',action='store_true')
    args=parser.parse_args();result={}
    for game in bake.GAMES:
        master=Path(bake.MAPDUMP)/(game+'_master/map.txt')
        header,rows=bake.read_csv(str(master));axes=bake.native_loop_axes(rows,{c:i for i,c in enumerate(header)})
        path=ROOT/'data'/game/'maps.bin';before=path.read_bytes();data=bytearray(before)
        h=struct.unpack_from('<8sIB3x12I',data)
        assert h[0]==b'FFPRMAP1' and h[1] in(1,2)
        def string(index):
            offset=struct.unpack_from('<I',data,h[3]+4*index)[0]
            return data[offset:data.index(0,offset)].decode('utf-8')
        changed=[];looping=[]
        for i in range(h[6]):
            offset=h[5]+56*i;record=struct.unpack_from('<IiHH4B10I',data,offset)
            name=string(record[0]);old=record[4]
            assert old&~7==0
            expected=bake.loop_flags(bool(old&1),axes.get(name,0))
            if expected&6:looping.append(dict(submap=name,axes=expected>>1,width=record[2],height=record[3]))
            if old!=expected:
                assert not args.check,('Missing/wrong native loop axes',game,name,old,expected)
                changed.append(dict(submap=name,before=old,after=expected,offset=offset+12))
                data[offset+12]=expected
        allowed={row['offset']for row in changed}
        assert all(a==b or index in allowed for index,(a,b) in enumerate(zip(before,data,strict=True)))
        if changed:path.write_bytes(data)
        result[game]=dict(changed=changed,looping_collision_maps=looping,
                          map_master_sha256=hashlib.sha256(master.read_bytes()).hexdigest(),
                          before_sha256=hashlib.sha256(before).hexdigest(),after_sha256=hashlib.sha256(data).hexdigest())
        print(game,'looping collision maps',looping,'flag-byte changes',len(changed))
    if not args.check:
        target=ROOT/'build_check/sunken_shrine_20261102/loop_metadata.json'
        assert not target.exists(),'Preserve initial metadata author evidence'
        target.write_text(json.dumps(result,indent=2)+'\n')
    print('Native loop metadata matches; geometry, objects, gates and world bytes preserved.')

if __name__=='__main__':main()
