"""Original complete bridge/fall/jump-room recipes; native bodies stay external.

The existing reviewed manual-travel edges own every jump and its real landing.
These recipes only select the next native switch, passage or joining objective.
"""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
HASHES={
 'Map_30791_2/sc_e_0833':'03300dc97e2c3548f95ea9fee57c042eb158b31eaa2336614c4c430b0405d2ba',
 'Map_30791_3/sc_e_0833_1':'e196819114d94de3a7f10f298ee93cf077d79e3e38ab5b15e0d5e6294e83622e',
 'Map_30791_3/sc_e_0849':'9d00d9d192969f9fad1831fcf63d4923d4d233f13b47c89f2ba39d4884853c1a',
 'Map_30791_3/sc_map_30791_3':'1cee572cd819422b6c2db835d98a46cebcfff01f17127b7d3742825979c80879',
 'Map_30791_7/sc_e_0857':'9b02d41a8cf079e3393653e8f2ee9217005dd16df5a3f33d07ae577b989d4d27',
 'Map_30791_7/sc_map_30791_7':'26fe51b9c06a1f47df4cd18aae6d2d004f44634f237ee328624fdaec4e9bc22b',
 'Map_30791_8/sc_e_0865':'272f2b0603b27df466dcd31146aceb89a8937ff8d7542a9b19cdd7288fd0cb0b',
 'Map_30791_8/sc_map_30791_8':'dca0e0df5778b160fbd23df9488c4c477b36d3171058d8229150ec426a2b89e2'}

def author(owner):
    for name,wanted in HASHES.items():
        room,script=name.split('/')
        body=owner.SCRIPTS[room,script]['mnemonics']
        assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,name
    rows=json.loads((ROOT/'data/ff6/warp_arrival_overrides.json').read_text(encoding='utf-8'))
    jumps=[r for r in rows if r['submap'] in ('Map_30791_2','Map_30791_7') and r.get('same_map')]
    assert len(jumps)==65
    for r in jumps:
        body=owner.SCRIPTS[r['submap'],r['continuation']]
        assert r['interaction_travel'] and r['approach_mask'] and r['arrival_baked_cell'][2]==1
        assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==r['continuation_sha256']
    c=owner.condition;t=owner.target;rule=owner.rule
    def exit(map_id,entity):
        row=t(map_id,entity,'Continue through the passage.')
        assert owner.physical(map_id,entity)['type_name']=='GotoMap'
        row['kind']='transition';return row
    bridges=owner.guide('zone_eater_bridges',[500,501,502,503],
        'Cross the bridges. Avoid the moving guards. If you fall, request guidance again.',[
        rule([c('map',0,500)],owner.entrance(501,1)),
        rule([c('map',0,501)],exit(501,6)),
        rule([c('map',0,502),c('local',1,0)],t(502,8,'Step on {0} to open the return passage, then request guidance again.',spot=True)),
        rule([c('map',0,502),c('local',1,1)],exit(502,2)),
        rule([c('map',0,503)],owner.entrance(504,1))])
    stones=owner.guide('zone_eater_stepping_stones',[505,506,507],
        'Use the stepping stones to reach Gogo. Request guidance after each jump and switch.',[
        rule([c('map',0,505)],owner.entrance(506,1)),
        rule([c('map',0,506),owner.flag(409)],t(506,25,'Step on {0} to raise the stepping stones, then request guidance again.','ev_e_0857',spot=True)),
        rule([c('map',0,506),owner.flag(409,1)],exit(506,2)),
        rule([c('map',0,507)],t(507,1,'Speak to {0} to continue.','ev_e_0865'))])
    stones.update(complete_scenario=410,finish_on_completion=True)
    for g in (bridges,stones):
        g.update(source_script_hashes=HASHES,evidence='docs/re/ZONE_EATER_CROSSINGS_20261103.md')
        # Do not use contact guards or unrelated triggers as shortcuts. Keep
        # reviewed jumps, exact native approach sides and landing receipts.
        for rule_ in g['rules']:
            for goal in rule_['targets']:
                goal.update(avoid_other_events=True,allow_scripted_crossings=True)
    return [bridges,stones]

if __name__=='__main__':
    import author_ff6_story_guides as owner
    expected=author(owner)
    path=ROOT/'data/ff6/puzzles.json';data=json.loads(path.read_text(encoding='utf-8'))
    ids={g['id']for g in expected};old=[g for g in data['story_guides']if g['id']in ids]
    if '--check'in sys.argv:assert old==expected,'Crossing recipes/source contract changed'
    else:
        assert not old or '--update'in sys.argv,'Existing records require a scoped update'
        data['story_guides']=[g for g in data['story_guides']if g['id']not in ids]
        data['story_guides'].extend(expected)
        path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Zone Eater crossings: two complete guides/eight exact state scripts/65 existing direction-qualified manual landings.')
