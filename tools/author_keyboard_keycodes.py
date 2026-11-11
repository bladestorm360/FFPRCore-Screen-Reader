"""Compare factual Unity key numbers in all six current declarations."""
from pathlib import Path
import hashlib,json,re,sys
ROOT=Path(__file__).resolve().parents[1]
def author():
 header=(ROOT/'src/core/input/input.h').read_text(encoding='utf-8');body=re.search(r'enum class Key[^\{]*\{(.*?)\};',header,re.S)[1]
 names=[s.strip()for s in re.sub(r'//[^\n]*','',body).split(',')if s.strip()and s.strip()!='Count'];result=None;sources={}
 for n in range(1,7):
  p=ROOT.parent/('FF4'if n==4 else'FF5'if n==5 else f'ff{n}')/'dump.cs';raw=p.read_bytes();text=raw.decode('utf-8-sig');enum=re.search(r'public enum KeyCode[^\n]*\n\{(.*?)\n\}',text,re.S)[1]
  values={k:int(v)for k,v in re.findall(r'public const KeyCode (\w+) = (-?\d+);',enum)}
  missing=[k for k in names if k not in values];assert missing==[f'F{i}'for i in range(16,25)],missing
  row=[values.get(k,-1)for k in names];assert result is None or result==row,n;result=row;sources[str(n)]=hashlib.sha256(raw).hexdigest()
 lines=['#pragma once','#include "input.h"','// Factual Unity key values agree across all six current engine declarations.','// F16–F24 have no legacy Unity equivalent and cannot collide with KeyConfig.','namespace input::native_keyboard {','inline constexpr int kUnityKeys[]={']
 lines +=['    '+','.join(str(v)for v in result[i:i+12])+','for i in range(0,len(result),12)]
 lines+=['};','static_assert(sizeof(kUnityKeys)/sizeof(kUnityKeys[0])==static_cast<int>(Key::Count));','}']
 out='\n'.join(lines)+'\n';path=ROOT/'src/core/input/native_keycodes.h';manifest=dict(source_declarations=sources,keys=dict(zip(names,result)))
 if'--check'in sys.argv:assert path.read_text(encoding='utf-8')==out;assert json.loads((ROOT/'docs/re/keyboard_keycodes_20261104.json').read_text(encoding='utf-8'))==manifest
 else:path.write_text(out,encoding='utf-8');(ROOT/'docs/re/keyboard_keycodes_20261104.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
 print('All-six native keycode mapping:',len(names),'mod keys;',len(missing),'extended Windows-only function keys; no native source copied.')
if __name__=='__main__':author()
