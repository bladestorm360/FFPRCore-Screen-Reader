"""Minimal pre-engine integration; preserve the accepted engine1/0/1 recipe."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def engine_template(owner):
    f=owner.flag;c=owner.condition;r=owner.rule;t=owner.target
    g=owner.guide('phantom_train',list(range(355,373))+[624],'',[
        r([f(120)],t(371,14,'Operate {0}.')),
        r([f(121,1)],t(371,10,'Operate {0}.')),
        r([f(122)],t(371,6,'Operate {0}.')),
        r([f(120,1),f(121),f(122,1)],t(356,5,'Fight {0}.','ev_e_0214')),
        r([c('map',0,372)],t(372,1,'Talk to {0}.','ev_e_0215'))])
    g['complete']='Continue through the passage.';return g
def extend(owner,engine):
    manifest=json.loads((ROOT/'docs/re/phantom_train_native_20261103.json').read_text(encoding='utf-8'))
    for locator,wanted in manifest['script_sha256'].items():
        room,name=locator.split('/');body=owner.SCRIPTS[room,name]['mnemonics']
        assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
    c=owner.condition;f=owner.flag;t=owner.target;r=owner.rule
    passage='Continue through the passage.'
    stages=[
      r([c('map',0,355),f(107)],t(355,1,passage,'ev_e_0160',spot=True)),
      r([c('map',0,355),f(107,1),f(108)],t(355,2,passage,spot=True)),
      r([f(108,1),f(109)],t(357,2,passage,spot=True)),
      r([f(109,1),f(110),f(519)],t(361,8,passage,'ev_e_1050',spot=True)),
      r([f(109,1),f(110),f(519,1)],t(361,6,'Speak to {0} to continue.','ev_e_0185')),
      r([f(110,1),f(111)],t(624,1,passage,'ev_e_0186',spot=True)),
      r([f(111,1),f(112)],t(624,2,passage,'ev_e_0187',spot=True)),
      r([f(112,1),f(113)],t(624,3,passage,'ev_e_0188',spot=True)),
      r([f(113,1),f(114)],t(362,20,'Operate {0}, then request guidance again.','ev_e_0189')),
      r([f(114,1),f(115),c('map',0,624)],t(624,22,passage,spot=True)),
      r([f(114,1),f(115),c('map',0,624,'ne')],t(362,20,'Operate {0}, then request guidance again.','ev_e_0189'))]
    assert len(engine['rules'])==5
    # The same original engine objectives remain, after the actual detachment
    # passage opens. The final Cyan conversation owns the real world-map exit.
    for row in engine['rules'][:4]:row['when']=[f(115,1),f(123),*row['when']]
    last=engine['rules'][4];last['when'].append(f(123,1));last['finish_on_exit']=1
    engine['rules']=stages+engine['rules'];engine['introduction']='Follow the train puzzle. Use Shift+G after each action.'
    engine['source_script_hashes']=manifest['script_sha256'];engine['evidence']='docs/re/PHANTOM_TRAIN_JOURNEY_20261103.md'
    return engine

if __name__=='__main__':
    assert sys.argv[1:]==['--check']
    import author_ff6_story_guides as owner
    original=engine_template(owner)
    expected=extend(owner,original)
    current=json.loads((ROOT/'data/ff6/puzzles.json').read_text(encoding='utf-8'))
    assert next(g for g in current['story_guides']if g['id']=='phantom_train')==expected
    print('Phantom Train:11 native pre-engine stages; retained1/0/1 engine and actual final-world exit.')
