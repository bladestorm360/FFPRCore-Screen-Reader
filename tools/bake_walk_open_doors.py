"""Bake the routing effects of ordinary, unconditional step-on door scripts."""
import collections
import csv
import json
from pathlib import Path
import sys

import bake_maps


# Effects with no movement, scene transfer, inventory check or player input.
PRESENTATION = {'Nop', 'Wait', 'PlaySE', 'StopSE', 'SyncSE', 'PlayAnim', 'CueAnim', 'Shake'}
STEP_TYPES = {2, 30}
BLOCKER_TYPES = {5, 10, 11}


def openings(script):
    rows = script.get('mnemonics') or []
    segments = {row['EntryPoint']: row for row in script.get('segments', ())}
    main = next((key for key, row in segments.items() if row['Label'] == 'Main'), None)
    hidden = set()
    visited = set()

    def run(start, puppet=False):
        if start in visited or start not in segments or len(visited) == 24:
            return None
        visited.add(start)
        segment = segments[start]
        body = rows[start:start + segment['Count']]
        end = next((i for i, row in enumerate(body) if row['mnemonic'] in {'Exit', 'Return'}), None)
        if end is None:
            return None
        hides = False
        puppets = {}
        for row in body[:end]:
            op = row['mnemonic']
            args = row.get('operands') or {}
            ints, strings = args.get('iValues') or [], args.get('sValues') or []
            if op in PRESENTATION:
                continue
            if puppet:
                if op == 'Hide':
                    hides = True
                    continue
                if op == 'LookAt':
                    continue
                return None
            if op == 'Call' and len(ints) >= 1 and run(ints[0]) is not None:
                continue
            if op == 'SetPuppet' and len(ints) >= 4 and ints[1] > 0 and ints[3] == 0:
                if ints[2] not in segments and start <= ints[2] < start + segment['Count']:
                    segments[ints[2]] = {'Count': start + segment['Count'] - ints[2]}
                puppets[ints[0]] = (ints[1], ints[2])
                continue
            if op == 'ExecPuppet' and puppets:
                for actor, entry in puppets.values():
                    effect = run(entry, puppet=True)
                    if effect is None:
                        return None
                    if effect:
                        hidden.add(actor)
                puppets.clear()
                continue
            if op in {'SetFlag', 'ResetFlag'} and strings and strings[0] in {
                'ScenarioFlag1', 'ScenarioFlag2', 'ScenarioFlag3', 'ScenarioFlag4'
            }:
                continue
            if op == 'Calc' and strings and strings[0] in {'MapLocalVariable', 'AreaLocalVariable'} and strings[1:3] == ['←', 'imm']:
                continue
            return None
        visited.remove(start)
        return None if puppets else hides

    return hidden if run(main) is not None else set()


def actor_key(obj, bounds):
    left, top = bounds[:2]
    props = obj['props']
    return [obj['type'], props['entity_id'], obj['group'], bake_maps.layer_index(obj['layer']),
            obj['tx'] - left, bake_maps.tile_row(obj) - top]


def generate(game, dump):
    with (dump / f'{game}_master/script.txt').open(encoding='utf-8-sig') as stream:
        names = {int(row['id']): row['script_name'].lower() for row in csv.DictReader(stream) if row['id']}
    with (dump / f'{game}_scripts.jsonl').open(encoding='utf-8') as stream:
        scripts = {(row['submap'], row['name'].lower()): row for row in map(json.loads, stream)}
    objects = collections.defaultdict(list)
    with (dump / f'{game}_objects.jsonl').open(encoding='utf-8') as stream:
        for obj in map(json.loads, stream):
            objects[obj['submap']].append(obj)
    submaps = json.loads((dump / f'{game}_submaps.json').read_text(encoding='utf-8'))
    for floor, actors in objects.items():
        if floor not in submaps:
            continue
        # Sequence-sensitive bookshelf, pyramid and multi-party puzzle state
        # continues to belong to the existing puzzle planner.
        if game == 'ff5' and floor.startswith(('Map_20221_', 'Map_30441_', 'Map_30461_')):
            continue
        if game == 'ff6' and floor.startswith(('Map_30781_', 'Map_30791_')):
            continue
        bounds = bake_maps.map_bounds(submaps[floor], actors)
        by_id = collections.defaultdict(list)
        for actor in actors:
            props = actor.get('props') or {}
            if props.get('entity_id', 0) > 0:
                by_id[props['entity_id']].append(actor)
        result = []
        for actor in actors:
            props = actor.get('props') or {}
            sid = props.get('script_id', 0)
            if (actor['type'] not in STEP_TYPES or props.get('action_id') != 2 or not sid or
                not props.get('entity_id') or props.get('target_transportation_ids')):
                continue
            script = scripts.get((floor, names.get(sid, '')))
            if script is None:
                continue
            contact = actor_key(actor, bounds)
            blockers = []
            for target_id in sorted(openings(script)):
                targets = by_id[target_id]
                if len(targets) != 1:
                    continue
                target, = targets
                properties = target['props']
                if (target['type'] not in BLOCKER_TYPES or not properties.get('collision') or
                    properties.get('action_id') or properties.get('target_transportation_ids')):
                    continue
                blocker = actor_key(target, bounds)
                if (contact[3] != blocker[3] or contact[4:] == blocker[4:] or
                    not all(0 <= value for value in contact[3:] + blocker[3:])):
                    continue
                blockers.append(blocker)
            if blockers:
                result.append({'contact': contact, 'script': sid, 'blockers': blockers, 'asset': names[sid]})
        if result:
            yield floor, result


if __name__ == '__main__':
    root = Path(__file__).resolve().parents[1]
    for game in sys.argv[1:]:
        path = root / 'data' / game / 'mapstate.json'
        maps = json.loads(path.read_text(encoding='utf-8'))
        previous = {name: row['walk_open'] for name, row in maps.items() if 'walk_open' in row}
        for row in maps.values():
            row.pop('walk_open', None)
        openings_by_map = dict(generate(game, root.parent / 'tools' / 'mapdump'))
        for floor, doors in openings_by_map.items():
            maps.setdefault(floor, {'entry': 0, 'code': [], 'initializer_unknown': True})['walk_open'] = doors
        if openings_by_map != previous:
            path.write_text(json.dumps(maps, ensure_ascii=False, separators=(',', ':')) + '\n', encoding='utf-8')
        print(game, len(openings_by_map), 'floors,', sum(map(len, openings_by_map.values())), 'contacts',
              ', '.join(openings_by_map))
