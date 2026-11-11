"""Exact clock choices and native hidden-chest completion; no game bodies."""
from pathlib import Path
import json,hashlib,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(v):return hashlib.sha256(json.dumps(v,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def author(o):
    manifest=json.loads((ROOT/'docs/re/zozo_clock_native_20261103.json').read_text(encoding='utf-8'))
    for locator,wanted in manifest['source_script_hashes'].items():
        room,name=locator.split('/');assert sha(o.SCRIPTS[room,name]['mnemonics'])==wanted,locator
    assert manifest['choice_positions']==[3,1,5]and manifest['treasure_receipt']==115
    chest=o.physical(386,14);assert chest['type_name']=='TreasureBox'and chest['props']['flag_id']==115
    t=o.target;c=o.condition
    clock=t(386,11,'Set the clock to 6:10:50. Choose options 3, 1, 5.')
    reward=t(386,14,'Open the chest.')
    # The real captured wall/clock visibility decides which operation is
    # reachable. A native arrival behind the wall can already reach the chest;
    # it must not be sent through its reset barrier to the other side's clock.
    g=o.guide('zozo_clock_reward',[386,387],'',[
        o.rule([c('map',0,387)],o.entrance(386,1)),
        o.rule([c('map',0,386)],clock,reward)])
    g.update(complete_when=[c('treasure',115,1)],source_script_hashes=manifest['source_script_hashes'],evidence='docs/re/ZOZO_CLOCK_REWARD_20261103.md')
    for rule in g['rules']:
        for goal in rule['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True)
    return g
if __name__=='__main__':
    import author_ff6_story_guides as o
    expected=author(o);p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));old=[g for g in d['story_guides']if g['id']==expected['id']]
    if'--check'in sys.argv:assert old==[expected]
    else:
        assert not old or '--update'in sys.argv
        d['story_guides']=[g for g in d['story_guides']if g['id']!=expected['id']]+[expected];p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Zozo: actual clock choices3/1/5, captured wall state, hidden chest115 completion and floor387 approach.')
