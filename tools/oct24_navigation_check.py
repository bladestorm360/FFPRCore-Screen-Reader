"""Catalog, actual activation data and all-six NPC dispatch audit; no game execution."""
from pathlib import Path
import csv
import json
import re

ROOT = Path(__file__).resolve().parents[1]
LANGS = {'en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt'}
common = json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf-8-sig'))
ff2 = json.loads((ROOT/'data/ff2/mod_text.json').read_text(encoding='utf-8-sig'))
keys=['Objects','Walk {0}, then {1}','Control interception','Toggle control interception',
      'On. Mod controls intercept native input. F9 toggles this setting.',
      'Off. Game controls pass through and game-context mod shortcuts pause. Mod dialogs remain usable. F9 restores shortcuts in any menu.',
      'Map marker','World map exit','Area transition','No map markers displayed','Wall near bed',
      '{0}, about {1} steps {2}','Inn','White magic shop','Black magic shop','Accessory shop']
for key, catalog in [(k,common) for k in keys]+[('Statue Room',ff2)]:
    assert set(catalog[key]) == LANGS, key
    tokens = set(re.findall(r'\{\d+\}', key))
    for lang, value in catalog[key].items():
        assert value and set(re.findall(r'\{\d+\}', value)) == tokens, (key,lang)
from modtext_oct24 import RECORDS
for scope,key,values,_ in RECORDS:
    assert (common if scope=='common' else ff2)[key]==values,(scope,key,'recipe drift')
titles = json.loads((ROOT/'data/ff2/map_titles.json').read_text(encoding='utf-8-sig'))
assert titles['98'] == {'text':'Statue Room'}
assert titles['230'] == {'room':'ラウンドウォーム'}
translation=json.loads((ROOT/'data/ff2/translation.json').read_text(encoding='utf-8-sig'))
assert set(translation['ラウンドウォーム'])==LANGS
assert translation['@map/230/ビックホーン']==translation['ラウンドウォーム']
assert translation['ラウンドウォーム']['en']=='Roundworm' and translation['ビックホーン']['en']=='Big Horn'

# These extracted asset inputs are existing offline prerequisites. Report facts
# and aggregate counts only; do not copy game scripts or native text into Git.
source = ROOT.parent/'tools/mapdump'
for number in range(1,7):
    folder=('ff1','ff2','ff3','FF4','FF5','ff6')[number-1]
    dump=(ROOT.parent/folder/'dump.cs').read_text(encoding='utf-8-sig')
    blocks={}
    for block in dump.split('// Namespace: ')[1:]:
        cls=re.search(r'^public class (\w+)\b',block,re.M)
        if cls:blocks[(block.splitlines()[0].strip(),cls[1])]=block
    contracts={
      ('Last.UI.KeyInput','ExpandMinimapController'):[' Show(',' Hide(','mapController','keyHelpController'],
      ('Last.UI.KeyInput','GpsMenuController'):[' Show(',' Close(','keyHelpController'],
      ('Last.UI.KeyInput','GpsMenuInfoView'):['contentParent','fixedChestText','numChestText','maxChestText','shopIconList'],
      ('Last.UI.Common','ExpandMinimapMapController'):['pointItemList','minimapIconPropertyList','playerPos',' UpdateScroll(',' Scaling('],
      ('Last.UI.Common.Library','LibraryInfoParamText'):['iconView','detailText'],
      ('Last.UI.KeyInput','KeyIconView'):['iconTextList','keyText','keyText2','separateText'],
      ('Last.UI.KeyInput','KeyHelpController'):['contentList','view'],
      ('Last.UI.KeyInput','BattlePlayerInfomationController'):['contentList','targetSelectContentList',' UpdateView('],
      ('Last.UI.KeyInput','BattlePlayerInfomationContentController'):['<TargetData>k__BackingField'],
      ('Last.UI.KeyInput','BattlePlayerInfomationTargetSelectContentController'):['<TargetData>k__BackingField'],
    }
    for cls,fields in contracts.items():
        block=blocks[cls]
        for member in fields:assert member in block,(number,cls,member)
    for investigation in ('map_panels_20261024','ally_targets_20261024'):
        receipt=json.loads((ROOT.parent/'tools'/investigation/f'ff{number}'/'provenance.json').read_text())
        assert receipt['functions'] and len(receipt['dll_sha256'])==64
    scripts = {int(row['id']):row['script_name'] for row in csv.DictReader((source/f'ff{number}_master/script.txt').open(encoding='utf-8-sig'))}
    companions = [json.loads(line) for line in (source/f'ff{number}_objects.jsonl').open(encoding='utf-8-sig')
                  if scripts.get(int(json.loads(line)['props'].get('script_id') or 0),'').startswith('sc_transition_')]
    assert len(companions)==(1 if number==2 else 0), (number,'transition companion census changed: review exact new identities')
    if companions:
        obj=companions[0]
        assert obj['submap']=='Map_20041_7' and obj['id']==104 and obj['props']['entity_id']==96
    master = source/f'ff{number}_master/action.txt'
    actions = {int(row['id']):row['action_name'] for row in csv.DictReader(master.open(encoding='utf-8-sig'))}
    assert actions[2]=='RunScript' and actions[4]=='Talk' and actions[8]=='Shop'
    manual = auto = staged = 0
    script_modes = {}
    for line in (source/f'ff{number}_objects.jsonl').open(encoding='utf-8-sig'):
        obj = json.loads(line); prop = obj['props']; kind = obj['type_name']
        script = int(prop.get('script_id') or 0)
        if kind in ('Entity','AnimEntity') and (script or prop.get('action_id')):
            manual += 1
            if script: script_modes.setdefault(script,set()).add('manual')
        elif kind in ('Event','RandomEvent','SwitchEvent') and (script or prop.get('action_id')):
            auto += 1
            if script: script_modes.setdefault(script,set()).add('auto')
        if kind in ('NPC','ShopNPC') and prop.get('action_id')==2 and not script and not prop.get('message_key'):
            staged += 1
    reused = sum(len(modes)==2 for modes in script_modes.values())
    print(f'FF{number}: {manual} manual object placements, {auto} automatic trigger placements; '
          f'{reused} scripts reused by both activation modes; {staged} staged RunScript NPC placements')

print(f'October24: {12*(len(keys)+1)} localized values/placeholder contracts; all-six native map, drop, hint and party field contracts pass')
from npc_story_label_overrides import check as check_npc_story_labels
check_npc_story_labels()
