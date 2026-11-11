"""Exact FFV Pyramid initializer continuations;no native code or general VM."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
PIN={
'sc_map_30461_1':'9bd499817e809ec51616d4fb472f125700e6d6d08fff97ee41bb8a88a56751da',
'sc_e_0288_needle':'4279392592bff662bae02e9fb41aa541b272747e5495484c13807c558337fd93',
'sc_map_30461_5':'b32e4b8a5c7b94ce910353279836598744fc536b0be4ae775bf8ee3e122fb65b',
'sc_map_30461_13':'3cd6ba02c17a50f9d77587445a180b8e61125afde5c4f095b412137323ede914',
'sc_map_30461_15':'692760c5e9b73a2a258066a7278a6d36e7f17f82a6a33b4752e4519054310a85',
'sc_map_30461_14':'f58e798d9dc6e6b7a47c2fd9ad89b74d991b2e104ec02538779e9311ce57cb08',
'sc_map_30461_7':'04b7738780c1b3f110f7dbd179d31124f22a5d600473d4e60fb0b268d0eeca95',
}
def digest(s):return hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def extend(script,game,region,code):
 name=script.get('name');room=script.get('submap')
 if game!='ff5'or name not in PIN or name=='sc_e_0288_needle':return code
 assert room==name[3:].replace('map_','Map_',1)and digest(script)==PIN[name],(room,name)
 if name=='sc_map_30461_1':
  rows=script['mnemonics'];assert code[3]==[9]and rows[3]['mnemonic']=='ChangeScript'and rows[3]['operands']['sValues'][0]=='sc_e_0288_needle'and rows[3]['operands']['iValues'][0]==1
  # The native HookedProcess saves pc+1 and Integrator.Update restores the
  # original mnemonic bank after the helper exits. Reuse the existing bounded
  # Call/Return projection,retaining every original initializer PC and branch.
  matches=[]
  with(R.parent/'tools/mapdump/ff5_scripts.jsonl').open(encoding='utf-8')as f:
   for line in f:
    s=json.loads(line)
    if(s['submap'],s['name'])==(room,'sc_e_0288_needle'):matches.append(s)
  helper,=matches;assert digest(helper)==PIN['sc_e_0288_needle']
  import bake_map_state as b
  extra=b.initializer_code(helper,'ff5',region);base=len(code);code[3]=[4,base]
  for row in extra:
   out=list(row)
   if out[0]in(3,4):out[1]+=base
   elif out[0]==2:out[6]+=base
   elif out[0]==6:out=[5]
   code.append(out)
 elif name=='sc_map_30461_7':
  # Exact unchanged-layer FFV SetPos units were qualified by2DBD20 and
  # MoveEntityLayer2D7C80 in the preserved Library spatial contract. Both
  # coffin enemies otherwise retain an off-map home in future-floor routes.
  import bake_map_state as b
  for pc,actor,entry,x,y in((88,129,91,10,13),(107,131,110,18,9)):
   m=script['mnemonics'][pc];assert m['mnemonic']=='SetPuppet'and m['operands']['iValues'][:3]==[0,actor,entry]
   effect=b.puppet_navigation_effect(script,entry,region,True);assert effect==[1,x-region[0],y-region[1]]
   code[pc]=[10,0,actor,*effect]
 elif name=='sc_map_30461_14':
  # Current's exact setup chooses the same phase as AreaLocal0,then moves
  # these six actors synchronously and writes that same receipt. Facts are
  # bound to actual metadata blobs,not inferred from the dormant tile homes.
  import struct
  proof=json.loads((R/'docs/re/ff5_pyramid_native_20261104.json').read_text(encoding='utf-8'));arrays=proof['native_arrays']
  for row in arrays.values():assert hashlib.sha1(struct.pack('<'+'i'*len(row['values']),*row['values'])).hexdigest().upper()==row['field']
  ids=arrays['triggers8']['values'];assert ids==list(range(78,84))and code[3]==[9]
  base=len(code);code[3]=[3,base];code.append([2,9,0,0,-1,0,base+9])
  for phase in(1,0):
   cells=arrays['cells8phase'+str(phase)]['values'];assert len(cells)==12
   code.extend([[10,i,actor,1,cells[i]-region[0],cells[i+6]-region[1]]for i,actor in enumerate(ids)])
   code.extend([[11,0],[6]])
 else:
  # These exact calls only unregister the previous timed-floor callbacks.
  # They cannot change actor groups or the subsequent initializer facts.
  for pc,m in enumerate(script['mnemonics']):
   if m['mnemonic']=='SysCall':
    assert m['operands']['sValues'][0]in('ピラミッド3階のギミック解除','ピラミッド8階のギミック解除')and code[pc]==[9];code[pc]=[0]
  if name=='sc_map_30461_5':
   # These are three physical doors and two snakes,not merely scenery. Their
   # actual unchanged-layer placements must block the corridors before each
   # corresponding native switch/battle receipt retires them.
   import bake_map_state as b
   for pc,actor,entry,x,y in((37,115,63,0,0),(38,118,66,0,0),(39,116,69,0,0),(75,115,86,7,10),(76,118,89,11,14),(77,116,92,9,18),(160,113,163,10,12),(174,114,177,11,11)):
    m=script['mnemonics'][pc];assert m['mnemonic']=='SetPuppet'and m['operands']['iValues'][1:3]==[actor,entry]
    effect=b.puppet_navigation_effect(script,entry,region,True);assert effect==[1,x-region[0],y-region[1]]
    code[pc]=[10,m['operands']['iValues'][0],actor,*effect]
 return code
