"""Offline native-rule witness on real FFIII terrain, not a game/runtime probe.

Native IsOverTheMountain permits at most two consecutive terrain19 cells and
requires the first far-side terrain to be in the current transport group's list.
Caller limits this to Invincible10/11; scripts910/911 move two/three tiles.
This deliberately excludes story/actor claims: it proves the missing geometric
edge before integration with production snapshots and visibility restrictions.
"""
import csv,heapq,json,struct,sys,time
from pathlib import Path
from check_scripted_bakes import decode

ROOT=Path(__file__).resolve().parents[1]

def run():
    rooms,_=decode(ROOT/'data/ff3/maps.bin');room=rooms['Map_10040'];w,h=room['shape'][1:3]
    def expand(raw):
        out=[]
        for at in range(0,len(raw),3):
            count,value=struct.unpack_from('<HB',raw,at);out.extend([value]*count)
        assert len(out)==w*h
        return out
    terrain=expand(room['grids'][-1]);air=expand(room['transport'][5])
    with (ROOT.parent/'tools/mapdump/ff3_master/transportation_group.txt').open(encoding='utf-8-sig') as f:
        row=next(r for r in csv.reader(f) if r and r[0]=='6')
    cross={i for i,value in enumerate(row[1:],1) if value=='1'}
    at=lambda x,y:(y%h)*w+x%w
    start=at(83,62);goal=at(36,100);report={}
    for vehicle in (8,9,10,11):
        began=time.perf_counter();queue=[(0,start)];dist={start:0};came={};popped=0
        while queue:
            cost,node=heapq.heappop(queue)
            if cost!=dist[node]:continue
            popped+=1
            if node==goal:break
            x,y=node%w,node//w
            for dx,dy in ((0,-1),(1,0),(0,1),(-1,0)):
                step=1;target=at(x+dx,y+dy)
                if vehicle in (10,11) and terrain[target]==19:
                    while step<=2 and terrain[at(x+dx*step,y+dy*step)]==19:step+=1
                    target=at(x+dx*step,y+dy*step)
                    if step>3 or terrain[target]==19:continue
                if air[target] or terrain[target] not in cross:continue
                next_cost=cost+step
                if next_cost>=dist.get(target,10**9):continue
                dist[target]=next_cost;came[target]=(node,step);heapq.heappush(queue,(next_cost,target))
        path=[];cursor=goal
        if goal in dist:
            while cursor!=start:
                previous,length=came[cursor]
                if length>1:path.append({'from':[previous%w,previous//w],'to':[cursor%w,cursor//w],'mountain_cells':length-1})
                cursor=previous
        report[str(vehicle)]={'reachable':goal in dist,'distance':dist.get(goal),'crossings':list(reversed(path)),
                              'popped':popped,'compute_ms':1000*(time.perf_counter()-began)}
        assert report[str(vehicle)]['reachable']==(vehicle in (10,11))
        print(f"FFIII vehicle{vehicle}: valley reachable={goal in dist}; distance={dist.get(goal)}; crossings={len(path)}; expanded={popped}")
    Path(sys.argv[1]).write_text(json.dumps(report,indent=2)+'\n')

if __name__=='__main__':run()
