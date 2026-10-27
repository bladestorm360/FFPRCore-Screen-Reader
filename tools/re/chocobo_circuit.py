"""Verify the exact FFIII circuit contract without exposing its exchange contents.

Original investigation tooling only. Matching scripts/assets and the resulting
evidence stay outside every Git tree. No game process or save is accessed.
"""
from pathlib import Path
import csv
import hashlib
import json
import subprocess

ROOT = Path(__file__).resolve().parents[2]
DUMP = ROOT.parent / "tools/mapdump"
OUT = ROOT.parent / "tools/chocobo_circuit_20261026"
OUT.mkdir(parents=True, exist_ok=True)
assert subprocess.run(["git", "-C", str(OUT), "rev-parse", "--show-toplevel"],
                      capture_output=True).returncode != 0


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, ensure_ascii=False,
                                    separators=(",", ":")).encode()).hexdigest()


scripts = [json.loads(line) for line in
           (DUMP / "ff3_scripts.jsonl").read_text(encoding="utf-8").splitlines()]
objects = [json.loads(line) for line in
           (DUMP / "ff3_objects.jsonl").read_text(encoding="utf-8").splitlines()]
master = list(csv.DictReader((DUMP / "ff3_master/script.txt").open(encoding="utf-8-sig")))
marker = next(s for s in scripts if s["name"] == "sc_e_0286")
exchange = next(s for s in scripts if s["name"] == "sc_e_0301")
marker_id = int(next(r["id"] for r in master if r["script_name"] == marker["name"]))
exchange_id = int(next(r["id"] for r in master if r["script_name"] == exchange["name"]))
review = next(r for r in json.loads((ROOT / "tools/world_event_reviews.json").read_text())
              if r["game"] == 3 and r["entity_id"] == 2368)
assert digest(marker) == review["script_sha256"] and marker_id == 380

# Restrict analysis to flag operands. Never inspect dialogue, item IDs, quantities,
# reward messages or other contents of the native exchange.
references = []
for script in scripts:
    hits = []
    for pc, row in enumerate(script.get("mnemonics") or []):
        values = row.get("operands") or {}
        strings = values.get("sValues") or []
        integers = values.get("iValues") or []
        if "ScenarioFlag1" in strings and 192 in integers:
            hits.append((pc, row["mnemonic"]))
    if hits:
        references.append({"submap": script["submap"], "script": script["name"], "hits": hits})
assert len(references) == 2
assert [r["mnemonic"] for r in marker["mnemonics"]] == ["Nop", "SetFlag", "Exit"]
assignment = marker["mnemonics"][1]["operands"]
assert assignment["sValues"][0] == "ScenarioFlag1" and assignment["iValues"][0] == 192
branch = exchange["mnemonics"][1]
assert branch["mnemonic"] == "Branch"
assert branch["operands"]["sValues"][:3] == ["ScenarioFlag1", "＝", "imm"]
assert branch["operands"]["iValues"][:3] == [192, 1, 4]
reset = exchange["mnemonics"][11]
assert reset["mnemonic"] == "ResetFlag"
assert reset["operands"]["sValues"][0] == "ScenarioFlag1"
assert reset["operands"]["iValues"][0] == 192
assert exchange_id == 247 and exchange["submap"] == "Map_20110"
assert marker["system_flag"]["RidingVehicle"] == 1

events = [o for o in objects if o["submap"] == "Map_10010"
          and o.get("props", {}).get("script_id") == marker_id]
assert len(events) == 1
event = events[0]
p = event["props"]
assert event["id"] == 5426 and p["entity_id"] == 2368 and p["action_id"] == 2
assert p["target_transportation_ids"] == "12" and p["gimmick"]
assert (event["tx"], event["ty"] - 1, event["w"], event["h"]) == (86, 25, 16, 16)

transport = next(r for r in csv.DictReader(
    (DUMP / "ff3_master/transportation.txt").open(encoding="utf-8-sig")) if r["id"] == "12")
assert (transport["transportation_group_id"], transport["type_id"], transport["landing_group_id"]) == ("2", "9", "2")
record = {
    "script_count_checked": len(scripts), "flag_references": references,
    "checkpoint": {"map_id": 1, "source_map": event["submap"], "entity_id": 2368,
                   "editor_id": 5426, "cell": [1, 86, 25], "transportation_id": 12,
                   "script_id": marker_id, "script": marker["name"], "sha256": digest(marker)},
    "completion": {"bank": "ScenarioFlag1", "index": 192, "absolute_index": 192,
                   "required_value": 1, "word_index": 6, "bit_index": 0,
                   "exchange_map_id": 60, "exchange_script_id": exchange_id,
                   "exchange_script": exchange["name"], "sha256": digest(exchange),
                   "reset_after_native_exchange": True},
    "lap_sequence_or_start_flag": False, "distance_or_direction_test": False,
    "reward_contents_exported": False, "game_launched_or_probed": False,
    "input_sha256": {str(p.relative_to(ROOT.parent)): hashlib.sha256(p.read_bytes()).hexdigest()
                     for p in (DUMP / "ff3_scripts.jsonl", DUMP / "ff3_objects.jsonl",
                               DUMP / "ff3_master/script.txt", DUMP / "ff3_master/transportation.txt")},
}
(OUT / "contract.json").write_text(json.dumps(record, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print("FFIII circuit: one mounted checkpoint, one completion bit, no ordered lap flags; exchange contents withheld.")
