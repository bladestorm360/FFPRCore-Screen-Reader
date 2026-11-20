"""Check exact appearance keys against the frozen local artwork indexes."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
LANGUAGES = {"en", "ja", "fr", "it", "de", "es", "ko", "zht", "zhc", "ru", "th", "pt"}


def load(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def read_index(directory, files, name):
    path = directory / name
    found = hashlib.sha256(path.read_bytes()).hexdigest()
    if files[name] != found:
        raise ValueError(f"Frozen index changed: {path}")
    return load(path)


def expected_placements(directory, baseline, coverage, title):
    facts = baseline["games"][title]
    files = facts["prior_visual_index_fingerprints"]
    enemies = read_index(directory, files, f"{title}-inventory.json")["enemies"]
    invisible = {int(identity) for identity in coverage[title]["transparent_rows"]}
    invisible.update(int(identity) for identity in coverage[title]["missing"])
    yield from (f"Enemy appearance {art}" for art, row in enemies.items() if row["id"] not in invisible)
    for kind, suffix in (("Field", "field-costumes"), ("Character", "battle-costumes"), ("Weapon", "equipment")):
        for art, row in read_index(directory, files, f"{title}-{suffix}.json").items():
            if row.get("bundle") and (kind != "Weapon" or row.get("id")):
                yield f"{kind} appearance {art}"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("index_dir", type=Path, help="The external frozen artwork index directory")
    parser.add_argument("titles", nargs="+", choices=[f"ff{n}" for n in range(1, 7)])
    parser.add_argument("--all-locales", action="store_true", help="Require all supported languages")
    args = parser.parse_args()
    baseline = load(ROOT / "docs" / "re" / "visual_inventory_baseline_20261120.json")
    coverage = read_index(args.index_dir, baseline["local_shared_indexes"], "enemy-coverage.json")
    failures = 0
    for title in args.titles:
        expected = set(expected_placements(args.index_dir, baseline, coverage, title))
        directory = ROOT / "data" / title
        shipped = load(directory / "appearances.json")
        overrides = load(directory / "mod_text.json")
        missing = expected - shipped.keys()
        unexpected = shipped.keys() - expected
        for key in sorted(missing):
            print(f"{title} missing {key}")
        for key in sorted(unexpected):
            if not key.startswith("Status appearance "):
                print(f"{title} unexpected {key}")
                failures += 1
        failures += len(missing)
        absent = Counter()
        for key in sorted(expected & shipped.keys()):
            localized = {**shipped[key], **overrides.get(key, {})}
            if not isinstance(shipped[key].get("en"), str) or not shipped[key]["en"].strip():
                print(f"{title} empty English appearance: {key}")
                failures += 1
            for language in LANGUAGES:
                if not localized.get(language, "").strip():
                    absent[language] += 1
        if args.all_locales:
            failures += sum(absent.values())
        print(f"{title}: {len(expected) - len(missing)}/{len(expected)} exact appearance keys. "
              f"Outstanding locale entries: {sum(absent.values())}.")
    return failures != 0


if __name__ == "__main__":
    sys.exit(main())
