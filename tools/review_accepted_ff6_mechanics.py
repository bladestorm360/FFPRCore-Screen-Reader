"""Explicit native-behavior reconciliation of already qualified FFVI guides.

This does not infer completeness from a target reference or map membership.
The finite cases below were compared to exact native receipts and the accepted
complete-episode evidence. Earlier Phantom Train travel remains a separate gap.
"""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
GUIDE_MAPS={
 'opera':[*range(112,119),576],
 'kefkas_tower':[*range(542,572),590,595,596],
 'narshe_light_path':[320],
 'phoenix_cave':[497,498,499],
 'dream_train':list(range(522,528)),
 'narshe_battle':[19],
 'burning_house':list(range(412,421))}
PHOENIX={'sc_e_0794':'held area1 opens the left wall','sc_e_0795':'held area2 opens the right wall','sc_e_0796':'held area3 removes spikes','sc_e_0797':'held area4 removes spikes','sc_e_0798':'held area5 removes spikes','sc_e_0800':'held area6 moves the rocks','sc_e_0801':'held area7 supplies the next stones','sc_e_0804':'held scenario400 is one side of the final twin latch','sc_e_0805':'held scenario401 is the other side of the final twin latch','sc_e_0808':'persistent403 extends the bridge','sc_e_0811':'persistent405 raises the stones','sc_e_0816':'sets406 and clears407; no holder is required','sc_e_0817':'sets407 and clears406; no holder is required','sc_e_0799':'persistent399 drains water/cools lava and performs the actual499 handoff'}
TOWER={'sc_e_0973':475,'sc_e_0980':480,'sc_e_0988':488,'sc_e_0989':489,'sc_e_0990':490,'sc_e_1001':500,'sc_e_0999':498,'sc_e_1000':499,'sc_e_0982':482,'sc_e_0976':478,'sc_e_0993':493,'sc_e_0984':484,'sc_e_0985':485,'sc_e_0986':486,'sc_e_0987':487}
OPERA={'sc_e_0315':'director preparation starts the actual576 performance scene','sc_e_0316':'dressing-room preparation sets159','sc_e_0323':'retry conversation sets164','sc_e_0322':'first timed retry sets161 and continues the actual reset scene','sc_e_0325':'later retry sets area6 before the letter stage','sc_e_0327':'last retry sets area6 before the letter stage','sc_e_0342':'retry letter sets175','sc_e_0329':'initial sabotage letter sets166','sc_e_0332':'the far-right lever sets168 and opens the rafters','sc_e_0331':'stagehand dialogue clears its native collision approach; the checked lever journey retains that interaction prefix','sc_e_0333':'wrong lever causes a real return/drop scene','sc_e_0334':'wrong lever causes a real return/drop scene','sc_e_0335':'wrong lever is an unneeded stage effect, not the correct far-right control'}
TRAIN={'sc_e_0903':'initial flashback sets442','sc_e_0904':'second lever pull stops the four-second crate timer; local0 is the actual receipt','sc_e_0904_1':'native passage records local0=3 after the crate crossing','sc_e_0906':'native moving-chest encounter positions the actor using area2; re-request uses its current identity','sc_e_0906_1':'left approach selects moving-chest area2=1','sc_e_0906_2':'right approach selects moving-chest area2=2','sc_e_0907':'feeding interaction sets443 and uses the actual chest-item receipt','sc_e_0908':'first-car reset/display lever supplies the reference six-chest arrangement; the checked later-car solution is explicit','sc_e_0929':'second-car checking lever uses the six actual locals and sets448','sc_e_0921':'left maze switch toggles447','sc_e_0917':'right maze switch toggles445','sc_e_0919':'middle maze switch toggles446'}
for i in range(6):
    TRAIN[f'sc_e_{910+i:04}']=f'first-car reference chest toggles local{i+1}; no reward is inferred from its visual open state'
    TRAIN[f'sc_e_{923+i:04}']=f'second-car chest toggles local{i+1}; all64 patterns select the next actual correction'
def identity(r):return r['submap'],r['group'],r['entity'],r['raw']
def explain(g,r):
    script=r['script'];action=r['action']
    if g=='narshe_light_path':
        assert r['submap']=='Map_30011_4'and r['group']=='ev_e_0090'
        if not script:
            assert action==0 and r['entity']in(9,10,11,12,13,14,15,16,42)
            return 'decorative counterpart','Automatic light/rotation puppet; no player action. Exact demonstration, thirteen ordered regions, trap/restart and final317 arrival are qualified.'
        if script=='sc_e_0090':return 'guided mechanic','Actual demonstration sets local1=1 and68; guide first selects this event, then each native predecessor-qualified region.'
        if script=='sc_e_0091':return 'guided mechanic','Trap-resolution producer clears559 and records either recovery progress or restart100. Guide requests a native direction, then selects the actual resulting state.'
        assert script=='sc_e_0092'or script.startswith('sc_e_0092_')
        return 'guided mechanic','Native ordered-region/trap counterpart: local2 encodes the region and local1 its predecessor. The guide routes cardinally through thirteen safe regions, avoids other events, explains559 and restart>=99, and ends only at ready317.'
    if g=='phoenix_cave':
        if not script:
            assert r['group']=='ev_e_0806'and action==0 and r['entity']in(33,34,35,37,38,39)
            return 'gate counterpart','Six native collision actors belong to the two-party latch402. The complete15-action/29-transfer journey and premature-plate recovery retain these gates and actual release receipts.'
        if script=='sc_e_0793':return 'ordinary return transport','Optional crane return resets the native multi-party mode and returns to268. It is a native return conversation, not a required Phoenix mechanism or completion receipt.'
        assert script in PHOENIX
        return 'guided mechanic',PHOENIX[script]+'. Complete entrance-to-reunion and premature-plate recovery exercise the actual hold/release, geometry and ready262 completion; no switch is certified by proximity.'
    if g=='kefkas_tower':
        if not script:
            assert action in(0,4)
            if r['group']=='ev_e_1016':return 'decorative counterpart','Ending-only puppet switch/floor; action0 has no manual script. These do not add a pre-completion puzzle control.'
            assert r['group']in('ev_e_0973','ev_e_0980','ev_e_0983','ev_e_0993')
            return 'gate counterpart','Exact native floor/fence/hidden-door counterpart. The qualified58-hop/27-action episode retains its native latch, occupied-party release and directional crossings.'
        assert script in TOWER
        return 'guided mechanic',f'Exact scenario{TOWER[script]} producer. The accepted three-party guide retains its actual plate/weight/fence/door handoff and tested release;58 hops/27 actions reach the real final latch without inventing completion.'
    if g=='opera':
        if not script:
            assert action==0
            return 'decorative counterpart','Automatic Celes replacement/orchestra/curtain puppet; no separate native interaction. The complete performance/retry/letter/lever/rafters/Ultros episode owns its surrounding stage.'
        assert script in OPERA
        return 'guided mechanic',OPERA[script]+'. Eight complete first-performance/retry/resume trajectories cover native choice1/2/1, real timer, three reachable rats/Ultros and ready177 completion; wrong levers are explicitly excluded.'
    if g=='dream_train':
        if script in('sc_e_0930','sc_e_0931','sc_e_0932'):
            return 'decorative counterpart','Locomotive lever only changes its animation/local. Native departure event8, not a fabricated combination of these three, owns arrival528/449; the qualified guide selects that actual departure.'
        assert script in TRAIN
        return 'guided mechanic',TRAIN[script]+'. Full first-car, maze/chest and departure checks use actual item/treasure/local receipts and physical native landings; all64 later chest patterns and16 layout episodes are covered.'
    if g=='narshe_battle':
        assert script in{f'sc_e_{i:04}'for i in range(250,256)}and r['group']=='ev_e_0250'
        return 'native party selection','Pre-defense native party-composition selector, not an ordered traversal puzzle. It arranges the displayed native party slots; the defense guide becomes eligible at137 and protects Banon until138. No hidden composition or movement is invented.'
    assert g=='burning_house'and script=='sc_e_0524'
    return 'automatic scene','Native entrance urgency conversation sets274. The existing fire/boss guide uses actual live flame obstacles and the rescue objective; this automatic crossing is not an additional manual puzzle switch.'

def main():
    rows={};facts={};sources=None
    for filename in ('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/filename).read_text(encoding='utf-8'))['games']['ff6']
        if sources is not None:assert sources==d['source_sha256']
        sources=d['source_sha256'];facts.update(d['script_facts'])
        for r in d['candidates']:
            g=next((g for g,maps in GUIDE_MAPS.items()if any(m in maps for m in r['maps'])),None)
            if g:rows.setdefault(identity(r),(g,[]))[1].append(r)
    # Select only the originally unresolved exact identities, never rewrite an
    # earlier accepted disposition or turn a map match into a completeness claim.
    path=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(path.read_text(encoding='utf-8'))
    existing={identity(r):r for r in old['decisions']if r['game']=='ff6'}
    authored=[]
    for k,(g,variants)in rows.items():
        if k in existing and 'ACCEPTED_FF6_MECHANICS_20261103.md'not in existing[k]['evidence']:continue
        r=variants[0];assert len(variants)==1,k
        disposition,reason=explain(g,r);locator=r['submap']+'/'+r['script']
        authored.append(dict(game='ff6',**{key:r[key]for key in ('submap','group','entity','raw','script')},script_sha256=facts.get(locator,{}).get('sha256'),disposition=disposition,guides=[g],reason=reason,evidence='docs/re/ACCEPTED_FF6_MECHANICS_20261103.md; exact native fingerprints and accepted complete-episode source/route/model/controller receipts.'))
    assert len(authored)==139,('Expected exact reconciled selection',len(authored))
    for role,name in {'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
        assert h.hexdigest()==sources[role],role
    replacements={identity(r):r for r in authored};new=dict(old);seen=set();new['decisions']=[]
    for r in old['decisions']:
        if r['game']=='ff6'and identity(r)in replacements:new['decisions'].append(replacements[identity(r)]);seen.add(identity(r))
        else:new['decisions'].append(r)
    new['decisions'].extend(r for r in authored if identity(r)not in seen)
    if '--check'in sys.argv:assert old==new,'Accepted-mechanic decisions changed'
    else:path.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('139 exact FFVI accepted-guide discoveries reconciled; pre-engine Phantom Train remains outside this disposition set. No guide/source/route behavior changed.')
if __name__=='__main__':main()
