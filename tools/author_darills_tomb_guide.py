"""Full tomb objective using existing owned state/routing; native bodies external."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def author(owner):
    manifest=json.loads((ROOT/'docs/re/darills_tomb_native_20261103.json').read_text(encoding='utf-8'))
    for locator,wanted in manifest['source_script_hashes'].items():
        room,name=locator.split('/');body=owner.SCRIPTS[room,name]['mnemonics']
        assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
    assert manifest['epitaph_original_option_order']==[3,0,2,1]
    c=owner.condition;flag=owner.flag;rule=owner.rule;t=owner.target
    rules=[rule([c('map',0,577),flag(528)],t(577,2,'Talk to {0}.','entity_default')),
           dict(when=[c('map',0,577),flag(528,1)],targets=[t(577,2,'Talk to {0}.','entity_default')],finish_on_exit=2)]
    prefix=[c('map',0,577,'ne')]
    for mid,entity,receipt,text,script in((458,1,341,'Examine the gravestone',None),(459,1,342,'Raise the water, then request guidance again.',None),
        (464,0,346,'Examine the gravestone',720),(464,0,347,'Examine the gravestone',721),(464,0,348,'Examine the gravestone',722),(464,0,349,'Examine the gravestone',723),
        (466,0,350,'Examine the gravestone. Choose Yes, then options 4, 1, 2, 1.',724)):
        rules.append(rule([*prefix,flag(receipt)],t(mid,entity,text,script=script)))
        prefix.append(flag(receipt,1))
    rules.append(rule([*prefix,c('treasure',169)],t(461,9,'Open the chest.')));prefix.append(c('treasure',169,1))
    for mid,entity,receipt,text in((463,24,343,'Examine the gravestone'),(463,1,344,'Raise the water, then request guidance again.')):
        rules.append(rule([*prefix,flag(receipt)],t(mid,entity,text)))
        prefix.append(flag(receipt,1))
    rules.append(rule([*prefix,flag(352),c('area',1)],t(463,22,'Ride the turtle, then request guidance again.','entity_default')))
    rules.append(rule([*prefix,flag(352),c('area',1,1)],t(468,4,'Examine the gravestone')))
    rules.append(rule([*prefix,flag(352,1),flag(353)],t(468,5,'Continue through the passage.',spot=True)))
    g=owner.guide('darills_tomb',list(range(456,469))+[577],'',rules)
    # These independent native facts finish the complete story/map-puzzle
    # objective, including a late optional-puzzle visit after acquiring transport.
    g.update(complete_when=[flag(353,1),flag(350,1),c('treasure',169,1)],source_script_hashes=manifest['source_script_hashes'],evidence='docs/re/DARILLS_TOMB_20261103.md')
    for r in rules:
        for goal in r['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True)
    return g
if __name__=='__main__':
    import author_ff6_story_guides as owner
    expected=author(owner);path=ROOT/'data/ff6/puzzles.json';d=json.loads(path.read_text(encoding='utf-8'));prior=[g for g in d['story_guides']if g['id']==expected['id']]
    if'--check'in sys.argv:assert prior==[expected]
    else:
        assert not prior or '--update'in sys.argv
        d['story_guides']=[g for g in d['story_guides']if g['id']!=expected['id']]+[expected]
        path.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Darill: full native water/turtle/epitaph/chest/boss/memory journey, including post-story puzzle resumes.')
