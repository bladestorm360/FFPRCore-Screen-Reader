"""Full native light/painting/door/treasure/story recipe, original instructions."""
from pathlib import Path
import json,sys,hashlib
ROOT=Path(__file__).resolve().parents[1]
def author(o):
    manifest=json.loads((ROOT/'docs/re/owzers_mansion_native_20261103.json').read_text(encoding='utf-8'))
    for locator,wanted in manifest['source_script_hashes'].items():
        room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
        assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
    c=o.condition;f=o.flag;t=o.target;r=o.rule;rules=[]
    # A completed story must not erase an unfinished map mechanism. Native391
    # disables the timer on a later visit; the same real passages reach any
    # remaining hanging-chest encounters without replaying the story/boss.
    late=[f(391,1)]
    for receipt,entity,group in((385,43,'ev_e_0767'),(386,45,'ev_e_0768'),(387,47,'ev_e_0769'),(388,49,'ev_e_0770')):
        missing=[*late,f(receipt)]
        rules.append(r([*missing,c('map',0,445,'ne')],o.entrance(445,1)))
        rules.append(r([*missing,c('map',0,445),c('area',1)],t(445,56,'Continue through the passage.')))
        rules.append(r([*missing,c('map',0,445),c('area',1,1)],t(445,entity,'Fight {0}.',group)))
        late.append(f(receipt,1))
    # Native eligibility is298 OR378. Two disjoint conjunctions reuse the
    # existing bounded predicate model; neither branch invents party state.
    for eligibility in([f(298,1)],[f(298),f(378,1)]):
        prefix=[*eligibility,f(391)]
        for receipt,mid,entity,text,group in((379,442,19,'Continue through the passage.','ev_e_0756'),
            (577,442,20,'Read the diary.','ev_e_0757'),(380,442,18,'Operate {0}, then request guidance again.','ev_e_0758'),
            (381,442,15,'Examine the painting','entity_default'),(382,442,13,'Examine the painting','ev_e_0762'),
            (384,444,2,'Continue through the passage.','ev_e_0763')):
            rules.append(r([*prefix,f(receipt)],t(mid,entity,text,group)))
            prefix.append(f(receipt,1))
        rules.append(r([*prefix,f(389),c('map',0,445,'ne')],o.entrance(445,1)))
        floor=[*prefix,f(389),c('map',0,445)]
        rules.append(r([*floor,c('local',2)],t(445,88,'Continue through the passage.','ev_e_1064')))
        stage=dict(map=445,x=23,y=23,layer=1,kind='spot',label='Passage',text='Continue through the passage.')
        timed=[*floor,c('local',2,1)]
        rules.append(dict(when=timed,at=stage,note='Left door: 1 North. Request the guide again when ready, then wait for Go now.',owzer_door=True))
        rules.append(r(timed,stage))
        rules.append(r([*floor,c('local',2,2),c('area',1)],t(445,56,'Continue through the passage.')))
        chest_prefix=[*floor,c('area',1,1)]
        for receipt,entity,group in((385,43,'ev_e_0767'),(386,45,'ev_e_0768'),(387,47,'ev_e_0769'),(388,49,'ev_e_0770')):
            rules.append(r([*chest_prefix,f(receipt)],t(445,entity,'Fight {0}.',group)))
            chest_prefix.append(f(receipt,1))
        rules.append(r(chest_prefix,t(445,1,'Examine the painting','ev_e_0771')))
        rules.append(r([*prefix,f(389,1),f(390)],t(446,7,'Talk to {0}.','entity_default')))
        final=[*prefix,f(389,1),f(390,1)]
        rules.append(r([*final,c('map',0,446),c('player_y',0,19,'ge')],t(446,9,'Continue through the passage.','ev_e_0774')))
        rules.append(r([*final,c('map',0,446),c('player_y',0,19,'lt')],t(446,8,'Examine the bookshelf.','ev_e_0774')))
        rules.append(r([*final,c('map',0,446,'ne')],t(446,8,'Examine the bookshelf.','ev_e_0774')))
        for last in rules[-2:]:last['finish_on_scenario']=391
    rules.append(dict(when=[f(391),f(298),f(378)],note='Story gated'))
    guide=o.guide('owzers_mansion',list(range(442,447)),'',rules)
    guide.update(complete_when=[f(391,1),*[f(i,1)for i in range(385,389)]],source_script_hashes=manifest['source_script_hashes'],evidence='docs/re/OWZERS_MANSION_20261103.md')
    for rule in rules:
        for row in rule.get('targets',[]):row.update(avoid_other_events=True,allow_scripted_crossings=True)
    return guide
if __name__=='__main__':
    import author_ff6_story_guides as owner
    expected=author(owner);p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));old=[g for g in d['story_guides']if g['id']==expected['id']]
    if'--check'in sys.argv:assert old==[expected]
    else:
        assert not old or '--update'in sys.argv
        d['story_guides']=[g for g in d['story_guides']if g['id']!=expected['id']]+[expected];p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Owzer full journey: diary/lights/paintings/native timed left door/four encounters/final painting/boss/bookshelf391.')
