"""Review retired guide operations as ordinary navigation without publishing them."""
import json


def retired_recipes(root, game, section='puzzles'):
    path = root / 'docs/re/ordinary_puzzle_interaction_fixtures_20261106.json'
    return json.loads(path.read_text(encoding='utf-8'))[game].get(section, [])


def author_recipe(data, definition, retired, section='puzzles'):
    name = definition['id']
    active = next((row for row in data[section] if row['id'] == name), None)
    historical = next((row for row in retired if row['id'] == name), None)
    if historical is not None:
        assert active is None and definition == historical, name
    elif active is not None:
        active.clear()
        active.update(definition)
    else:
        data[section].append(definition)
