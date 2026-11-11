"""Independent finite census of all FF2 native Ask/Learn/Item bindings.

The mnemonic archive omitted declarations. Exact known NbrSelect callers are
the target set; no game execution or runtime capture. Raw assets stay outside
Git; only identities, hashes, binding IDs and state-effect indices are emitted.
"""
import csv,hashlib,json,re,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'
sys.path.insert(0,str(ROOT.parent/'tools'));import dump_map_objects as archive

def sha(value):return hashlib.sha256(value).hexdigest()

def main():
    targets={}
    for line in (DUMP/'ff2_scripts.jsonl').open(encoding='utf-8'):
        row=json.loads(line)
        if any(m['mnemonic']=='SysCall' and m['operands'].get('sValues',[None])[0]=='NbrSelect' for m in row['mnemonics']):
            targets.setdefault(row['map'],[]).append(row)
    assert sum(map(len,targets.values()))==63 and len(targets)<=64
    names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff2_master/script.txt').open(encoding='utf-8-sig'))}
    objects=[]
    for line in (DUMP/'ff2_objects.jsonl').open(encoding='utf-8'):objects.append(json.loads(line))
    out=ROOT.parent/'tools/ff2_keyword_puzzles_20261101/all_scripts';out.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    facts=[];bundles=[]
    print('FF2 finite keyword census:63 exact scripts in',len(targets),'bundles.',flush=True)
    for parent,rows in sorted(targets.items()):
        paths=list(Path(archive.aa_path('ff2')).glob(parent.lower()+'_assets_*.bundle'));assert len(paths)==1,(parent,len(paths))
        path=paths[0];by_path,by_name=archive.load_texts(str(path))
        bundles.append(dict(name=path.name,sha256=sha(path.read_bytes())))
        for old in rows:
            room=old['submap'];name=old['name'];assert re.fullmatch(r'Map_[0-9_]+',room) and re.fullmatch(r'sc_[A-Za-z0-9_]+',name)
            text=archive.resolve(room+'/'+name,by_path,by_name);assert text is not None,(room,name)
            native=json.loads(text);assert native['Mnemonics']==old['mnemonics'],(room,name,'mnemonic archive differs')
            (out/(room+'_'+name+'.json')).write_text(text,encoding='utf-8')
            labels=native.get('ScriptLocal',[]);values=native.get('ScriptLocalValue',[]);assert len(labels)==len(values)
            bindings=[dict(label=label,content=values[i],success_pc=values[i+1]) for i,label in enumerate(labels) if label.startswith('Key_')]
            actors=[dict(entity=o['props'].get('entity_id'),raw=o['name'],group=o['group'],type=o['type_name'],action=o['props'].get('action_id'))
                for o in objects if o['submap']==room and names.get(o['props'].get('script_id'))==name]
            flags=[]
            for m in native['Mnemonics']:
                v=m['operands'];sv=v.get('sValues',[]);iv=v.get('iValues',[])
                if m['mnemonic'] in ('SetFlag','ResetFlag') and sv and iv:flags.append([m['mnemonic'],sv[0],iv[0]])
            facts.append(dict(submap=room,script=name,script_sha256=sha(json.dumps(old['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()),
                asset_sha256=sha(text.encode()),bindings=bindings,owners=actors,state_effects=flags))
    result=dict(about='63 exact native NbrSelect callers, independently recovered declarations. Factual discovery only; semantic dispositions/complete guides are separate. Raw assets remain external.',bundles=bundles,scripts=facts)
    (ROOT/'docs/re/ff2_keyword_mechanics_inventory_20261101.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FF2 keyword census complete:',len(facts),'scripts,',sum(len(r['bindings']) for r in facts),'bindings; raw sources outside Git.')

if __name__=='__main__':main()
