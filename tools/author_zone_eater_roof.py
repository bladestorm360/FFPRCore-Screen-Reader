"""Original roof route/safety facts, with exact external callback fingerprints.

No game body is emitted. The automatic callback is covered even though the
immutable object-based discovery inventory did not bind it to an actor.
"""
from pathlib import Path
import hashlib,json,sys
import bake_maps
ROOT=Path(__file__).resolve().parents[1]
HASHES={
 'Map_30791_5/sc_e_0850':'0a1d32dcf77edb6a38a526e04cdfcfdb5dc83bb2ae6fa1c468762428a78e181c',
 'Map_30791_5/sc_e_0850_0':'f2852e83890251c94efc9787cf8740aaff4946e4a9cae37c6db170d12141bc86',
 'Map_30791_5/sc_e_0850_1':'6f3fde549d73d3c522582955283bbc28eeb4a8fea4f969c29fb866898caf4395',
 'Map_30791_5/sc_e_0850_2':'295cfafcbcff4ecf4a1227ebeea6c8d356799b92c04a41de88290c742b4b3488',
 'Map_30791_5/sc_map_30791_5':'2e365aa0a7432d5cd5644c2ceec30ae77ca89656168675d97fd439c36cdf3a45',
 'Map_30791_4/sc_e_0850_in1':'e3077db1d3ac880a1c653119be0f833aec27b2c1db34a8a0c6dfeea6f3830f62',
 'Map_30791_6/sc_e_0850_in2':'6ea0c6eb6e480435c3783b4463c648932976c81c542684f617ebb93dcb5558be'}
STATIONS=[[1,37,18],[1,37,12],[1,33,9],[1,29,14],[1,24,10],[1,18,13],[1,13,9],[1,10,15],[1,9,19]]

def author(physical,scripts,objects):
    for name,wanted in HASHES.items():
        room,script=name.split('/');body=scripts[room,script]['mnemonics']
        assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,name
    room='Map_30791_5';local=[o for o in objects if o['submap']==room]
    info=json.loads((ROOT.parent/'tools/mapdump/ff6_submaps.json').read_text(encoding='utf-8'))[room]
    ox,oy,width,height=bake_maps.map_bounds(info,local)
    points=[];kinds={1135:0,1136:0}
    for entity in range(12,46):
        o=physical(504,entity,'ev_e_0850');p=o['props'];assert o['type_name']=='Event'and p['action_id']==2 and p['script_id'] in kinds
        kinds[p['script_id']]+=1;points.append([bake_maps.layer_index(o['layer']),o['tx']-ox,bake_maps.tile_row(o)-oy])
    assert kinds=={1135:16,1136:18}and len({tuple(p)for p in points})==34
    assert all(p in points for p in STATIONS)
    entrances={p['props']['object_id']:[bake_maps.layer_index(p['layer']),p['tx']-ox,bake_maps.tile_row(p)-oy]for p in local if p['type_name']=='PointIn'}
    assert entrances=={1:STATIONS[0],2:STATIONS[-1]},entrances
    exits={p['props']['entity_id']:(p['props']['map_id'],[bake_maps.layer_index(p['layer']),p['tx']-ox,bake_maps.tile_row(p)-oy])for p in local if p['type_name']=='GotoMap'}
    assert exits=={3:(503,[1,37,19]),4:(505,[1,9,20])},exits
    return dict(id='zone_eater_roof',maps=[504],rules=[],
        introduction='Cross the falling-ceiling room from one safe area to the next.',
        blocked='Story guidance is waiting for current game state.',
        safe_cells=points,stations=STATIONS,source_script_hashes=HASHES,
        source_dimensions=[width,height],evidence='docs/re/ZONE_EATER_ROOF_20261103.md')

if __name__=='__main__':
    assert sys.argv[1:]==['--check']
    import author_ff6_story_guides as owner
    expected=author(owner.physical,owner.SCRIPTS,owner.OBJECTS)
    data=json.loads((ROOT/'data/ff6/puzzles.json').read_text(encoding='utf-8'))
    rows=[g for g in data['story_guides'] if g['id']=='zone_eater_roof']
    assert rows==[expected],'Roof source/recipe changed; review exact callback and safe-area contract'
    print('Roof:34 safe markers/nine stations/two physical arrivals and exits/seven exact callback hashes checked.')
