"""Four exact first-car exits: real facing-dependent landing and story guard.

This corrects an existing first-Main-transfer bake, without running scripts or
adding runtime parsing. The native message-only branch before109 stays a valid
physical event; only its derived transfer is unavailable.
"""
import hashlib,json
SOURCE='118ed803735cf895ee4ff3b1b5cfbb01f0a78dff4cb4353817593f9521cd558d'
ROWS={11:(1,33,11,1,8),12:(13,33,12,1,8),20:(16,4,11,2,2),21:(17,4,12,2,2)}
def identity(obj):
    if obj['submap']!='Map_30441_1'or obj['id']not in ROWS:return None
    entity,x,y,point,mask=ROWS[obj['id']];p=obj['props']
    assert(obj['type_name'],obj['group'],p.get('entity_id'),p.get('action_id'),p.get('script_id'),obj['tx'],obj['ty'],obj['w'],obj['h'],obj['layer'])==('Event','entity_default',entity,2,191,x,y,16,16,'GroundObject'),'Phantom exit identity changed'
    return point,mask
def validate(obj,script):
    row=identity(obj)
    if row is None:return None
    assert script['name']=='sc_e_0162'and hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==SOURCE,'Phantom exit source changed'
    return row
def destination(obj,name,scripts):
    if identity(obj)is None:return None
    assert name=='sc_e_0162'
    point,_=validate(obj,scripts[obj['submap'],name]);return(624,point,'')
def entry_flags(obj):
    row=identity(obj);return(row[1]<<8)if row else 0
def guard(obj,script):
    row=validate(obj,script)
    if row is None:return None
    # Original mod predicate in the existing state VM: only the matching
    # derived transfer depends on109. The native message event is retained.
    return dict(entry=0,code=[[2,0,109,0,-1,1,3],[6],[0],[1,'@action'],[6]],destination=[624,row[0]])
