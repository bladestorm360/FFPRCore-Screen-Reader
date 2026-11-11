"""Complete native switch/reward/summit/battle/return objective; original data."""
from pathlib import Path
import hashlib,json,sys
from ordinary_interaction_fixtures import author_recipe, retired_recipes
ROOT=Path(__file__).resolve().parents[1]
def author(owner):
    hashes=json.loads((ROOT/'docs/re/cultists_tower_native_20261103.json').read_text(encoding='utf-8'))['source_script_hashes']
    for locator,wanted in hashes.items():
        room,name=locator.split('/');body=owner.SCRIPTS[room,name]['mnemonics'];assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
    f=owner.flag;c=owner.condition;t=owner.target;r=owner.rule
    rules=[r([f(421)],t(478,5,'Examine the hidden switch')),
        r([f(421,1),c('treasure',179)],t(479,3,'Open the chest.')),
        r([f(421,1),c('treasure',179,1),f(423)],t(487,3,'Open the chest.')),
        r([f(421,1),c('treasure',179,1),f(423,1),f(424)],t(486,1,'Fight {0}.','ev_e_0881',spot=True)),
        dict(when=[f(421,1),c('treasure',179,1),f(424,1)],targets=[owner.entrance(476,2)],finish_on_exit=476,finish_through=[486,484,482,480,477])]
    guide=owner.guide('cultists_tower',list(range(476,488)),'',rules)
    guide.update(complete_when=[c('treasure',179,1),f(424,1),c('map',0,476)],source_script_hashes=hashes,evidence='docs/re/CULTISTS_TOWER_20261103.md')
    for rule in rules:
        for goal in rule['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True)
    return guide
if __name__=='__main__':
    import author_ff6_story_guides as owner
    d=json.loads((ROOT/'data/ff6/puzzles.json').read_text(encoding='utf-8'))
    author_recipe(d,author(owner),retired_recipes(ROOT,'ff6','story_guides'),'story_guides')
    print('Cultists Tower: exact switch421/hidden chest179/summit423/boss424/actual entrance return, no invented proximity completion.')
