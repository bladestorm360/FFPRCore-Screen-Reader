"""Original complete Floating Continent objectives on actual native receipts."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
STAY='At the airship prompt, choose the second option to stay here, then request guidance again.'
WAIT='At the airship prompt, choose the second option to wait for Shadow. Stay at the edge until he arrives.'
MISSED='That chest is cut off after the collapse. Continue to {0}.'

def author(o):
    m=json.loads((ROOT/'docs/re/floating_continent_native_20261103.json').read_text(encoding='utf-8'))
    for locator,wanted in m['source_script_hashes'].items():
        room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
        assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
    c=o.condition;f=o.flag;r=o.rule;t=o.target;rules=[]
    # The exact save-room exit uses physical PointIn3 at90,50, not PointIn2.
    rules.append(r([c('map',0,432)],o.entrance(431,3)))
    for mid in(431,631):
        normal=[c('map',0,mid),f(292)]
        def add(conditions,entity,text='Operate {0}, then request guidance again.',group=None):
            rules.append(r([*normal,*conditions],t(mid,entity,text,group)))
        add([f(287)],157,'Talk to {0}, then request guidance again.','ev_e_0566')
        for stage,entity in((0,151),(1,149),(2,147)):
            add([c('area',1,stage)],entity)
        add([c('area',1,3,'ge'),c('area',6)],145)
        add([c('area',6,1),c('treasure',150)],6,'Open the Treasure Chest.')
        add([c('area',6,1),c('treasure',247),f(290)],154,'Open the Treasure Chest.')
        add([c('area',6,1),c('area',2)],135)
        for prior,current,entity in((2,3,96),(3,4,87),(4,5,80)):
            add([c('area',prior,1),c('area',current)],entity)
        add([c('area',5,1),c('treasure',151)],8,'Open the Treasure Chest.')
        # Prefer the path-opening marker after the native decline. Before that,
        # its route is blocked by the actual airship prompt; offer the prompt as
        # fallback. Its two position predicates omit that completed fallback at
        # the exact restored tile, so the controller cannot retain it forever.
        branch=[c('area',1,3),c('area',5,1)]
        add(branch,74,'Step on {0} to open the passage, then request guidance again.')
        add([*branch,c('player_y',0,37,'ne')],155,STAY)
        add([*branch,c('player_y',0,37),c('player_x',0,70,'ne')],155,STAY)
        add([c('area',1,4),f(291)],71,'Continue to {0}, then request guidance again.','ev_e_0592')
        add([f(291,1)],160,'Continue through the passage.','ev_e_0594')
        escape=[c('map',0,mid),f(292,1)]
        def flee(conditions,entity,text='Step on {0} to open the passage, then request guidance again.'):
            rules.append(r([*escape,*conditions],t(mid,entity,text)))
        # The west-side collapse299 spawns encounter18. Its native victory
        # callback records area7=1; it is absent on the normal safe approach.
        flee([f(299,1),c('area',7),c('area',1,11,'lt')],18,'Continue to {0}, then request guidance again.')
        flee([c('area',1,4)],52)
        flee([c('area',1,5)],43)
        flee([c('area',1,6),f(293)],20,'Continue to {0}, then request guidance again.')
        for entity in(40,41):flee([c('area',1,6),f(293,1)],entity)
        for entity in(35,36):flee([c('area',1,7)],entity)
        flee([c('area',1,8),f(294)],19,'Continue to {0}, then request guidance again.')
        flee([c('area',1,8),f(294,1)],32)
        flee([c('area',1,9)],29)
        flee([c('area',1,10),c('treasure',152)],10,'Open the Treasure Chest.')
        for entity in(21,22):flee([c('area',1,10),c('treasure',152,1)],entity)
        for obtained in(0,1):
            flee([c('area',1,11),f(295),c('treasure',152,obtained)],17,
                 'Continue to {0}, then request guidance again.'if obtained else MISSED)
        end=r([*escape,c('area',1,11),f(295,1)],t(mid,11,WAIT))
        end.update(finish_on_exit=m['escape_completion']['final_ready_map'],
                   finish_through=m['escape_completion']['through'])
        rules.append(end)
    g=o.guide('floating_continent',[431,631,432],'',rules)
    g.update(complete_scenario=297,source_script_hashes=m['source_script_hashes'],
             evidence='docs/re/FLOATING_CONTINENT_GUIDANCE_20261103.md')
    for rule in rules:
        for target in rule.get('targets',[]):target.update(avoid_other_events=True,allow_scripted_crossings=True)
    return g

if __name__=='__main__':
    import author_ff6_story_guides as o
    p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o)
    if'--check'in sys.argv:assert[row for row in d['story_guides']if row['id']==g['id']]==[g]
    else:
        d['story_guides']=[row for row in d['story_guides']if row['id']!=g['id']]+[g]
        p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Floating Continent: full normal/escape objectives, four treasures, native stay/wait choices, save-room return and final ready191 completion.')
