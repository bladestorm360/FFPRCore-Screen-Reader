"""Inspect four exact FF2 keyword/item script assets offline.

The older mapdump omitted script-local declarations. Native CreateNbrSystem
uses those declarations to select successful Ask/Item continuations. Archive
the exact raw assets outside Git, and print only schema/declaration metadata.
"""
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT.parent/'tools'))
import dump_map_objects as archive

SPECS={'map_20041':('Map_20041_7','sc_e_0063'),
 'map_20101':('Map_20101_8','sc_e_0065'),
 'map_30061':('Map_30061_5','sc_e_0050'),
 'map_30071':('Map_30071_5','sc_e_0052')}
OUT=ROOT.parent/'tools/ff2_keyword_puzzles_20261101/scripts'
EXPECTED={'sc_e_0063':('a4bbbc781603649196f82dbe0cc3b95d55193075c47b3552aad6670330cd2ed2',[(931,5)]),
 'sc_e_0065':('0cc0e3b34a928ccb80501513281beaf01a3c150099774c22276a2886065fdb94',[(12,8)]),
 'sc_e_0050':('7244101cabad0322e2473e089d101a6dcd4cd02e1d8decca4618df943f20a647',[(926,12),(927,16)]),
 'sc_e_0052':('45fcb52938b4e24c9348a386cc25f752869220d890a22429f947142a914147c7',[(11,6)])}

def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
    return h.hexdigest()

def main():
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    check='--check' in sys.argv;facts=[]
    for prefix,(room,name) in SPECS.items():
        paths=list(Path(archive.aa_path('ff2')).glob(prefix+'*.bundle'));assert len(paths)==1,(prefix,len(paths))
        path=paths[0];by_path,by_name=archive.load_texts(str(path))
        text=archive.resolve(room+'/'+name,by_path,by_name);assert text is not None,(room,name)
        script=json.loads(text);target=OUT/(room+'_'+name+'.json')
        if check:assert target.read_text(encoding='utf-8')==text,'Archived script changed'
        else:target.write_text(text,encoding='utf-8')
        metadata={key:script[key] for key in ('ScriptLocal','ScriptLocalValue')}
        keys=metadata['ScriptLocal'];values=metadata['ScriptLocalValue'];assert len(keys)==len(values)
        bindings=[(values[i],values[i+1]) for i,key in enumerate(keys) if key.startswith('Key_')]
        source_hash=hashlib.sha256(text.encode()).hexdigest();assert (source_hash,bindings)==EXPECTED[name]
        facts.append(dict(locator=room+'/'+name,bundle=path.name,bundle_sha256=digest(path),script_sha256=source_hash,bindings=bindings))
    dll=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY II PR/GameAssembly.dll')
    assert digest(dll)=='dbc8d9f674d3097b5bfa3c3b3ab877c32d92947bb40df545202ee574f2b0b19b','Native dispatch/learning producer changed'
    inside=False;typed=False
    for line in (ROOT.parent/'ff2/dump.cs').open(encoding='utf-8-sig'):
        if line.startswith('public class UserDataManager :'):inside=True
        if inside and '<OwnedKeyWaordList>k__BackingField;' in line:
            typed='List<int>' in line and '// 0x90' in line;break
    assert typed,'Native learned-word field contract differs'
    (ROOT/'build_check/ff2_keyword_bindings.json').write_text(json.dumps(dict(bindings=facts,learned_word_field_offset=144,word_size_limit=32),indent=2)+'\n',encoding='utf-8')
    print('FF2 keyword contracts: four exact native binding tables, matched dispatch/learning DLL and typed learned-word field.')

if __name__=='__main__':main()
