"""Check serialized EXP-row ownership against the recursive text reader.

Reads installed bundles offline; exports factual identities/counts, no asset text
or recovered game implementation. Runtime visibility/pool reuse remains separate.
"""
from pathlib import Path
import json
import UnityPy

ROOT = Path(__file__).resolve().parents[2]
STEAM = Path('D:/Games/SteamLibrary/steamapps/common')
GAMES = ['FINAL FANTASY PR', 'FINAL FANTASY II PR', 'FINAL FANTASY III PR',
         'FINAL FANTASY IV PR', 'FINAL FANTASY V PR', 'FINAL FANTASY VI PR']
records = []
for n, game in enumerate(GAMES, 1):
    data = next(p for p in (STEAM / game).iterdir() if p.name.endswith('_Data') and p.is_dir())
    paths = list((data / 'StreamingAssets/aa/StandaloneWindows64').glob('key_battle_assets_all_*.bundle'))
    assert len(paths) == 1
    env = UnityPy.load(str(paths[0]))
    trees, transforms = {}, {}
    for obj in env.objects:
        if obj.type.name in ('MonoBehaviour', 'Transform', 'RectTransform'):
            tree = obj.read_typetree()
            trees[obj.path_id] = tree
            if obj.type.name in ('Transform', 'RectTransform'):
                transforms[tree['m_GameObject']['m_PathID']] = tree
    views = {pid: t for pid, t in trees.items() if 'nameText' in t and 'expValueText' in t}
    assert views
    headings = [t for t in trees.values() if 'nextFixText' in t and 'expFixText' in t]
    assert headings, f'FF{n}: native member-column headings missing'
    assert all('nextValueText' in t for t in views.values())
    if n == 5:
        assert all('abpValueText' in t for t in views.values()) and all('abpFixText' in t for t in headings)
    text_ids = {pid for pid, t in trees.items() if 'm_Text' in t}
    def descends(child, parent):
        seen = set()
        while child and child not in seen:
            if child == parent:
                return True
            seen.add(child)
            transform = transforms.get(child, {})
            father = trees.get(transform.get('m_Father', {}).get('m_PathID'), {})
            child = father.get('m_GameObject', {}).get('m_PathID')
        return False
    rows = []
    for pid, controller in trees.items():
        view_id = controller.get('view', {}).get('m_PathID')
        if view_id not in views:
            continue
        view = views[view_id]
        root = controller['m_GameObject']['m_PathID']
        own_text = view['nameText']['m_PathID']
        names_in_tree = [other['nameText']['m_PathID'] for other in views.values()
                         if descends(trees[other['nameText']['m_PathID']]['m_GameObject']['m_PathID'], root)]
        fields = {key: value['m_PathID'] for key, value in view.items()
                  if isinstance(value, dict) and value.get('m_PathID') in text_ids}
        row = {'controller_path_id': pid, 'view_path_id': view_id,
               'name_is_descendant': own_text in names_in_tree,
               'result_names_in_controller_subtree': len(names_in_tree),
               'text_fields': sorted(fields)}
        rows.append(row)
        assert row['name_is_descendant'], f'FF{n}: recursive reader misses its own row name {pid}'
        assert len(names_in_tree) == 1, f'FF{n}: recursive reader crosses character rows {pid}'
    assert rows
    records.append({'game': n, 'bundle': paths[0].name, 'rows': rows,
                    'member_heading_views': len(headings),
                    'structured_columns': ['expValueText', 'nextValueText'] + (['abpValueText'] if n == 5 else [])})
    print(f'FF{n}: {len(rows)} serialized EXP row controllers each contain exactly their own name')
(ROOT / 'docs/re/result_name_views.json').write_text(json.dumps(records, indent=2) + '\n', encoding='utf-8')
