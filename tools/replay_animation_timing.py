"""Replay Pixel Remaster field-script timing from local, extracted inputs.

The clock models interpreter dependencies. Unity rendering, motion completion
and dialogue dismissal are observations supplied by a scenario. This tool
never writes or embeds game scripts.
"""

import argparse
import csv
from dataclasses import dataclass
from functools import lru_cache
import json
import math
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
VISUAL = {"ColorFade", "CueAnim", "FadeIn", "FadeOut", "Flash", "Hide", "LookAt",
          "Move", "MoveBirdView", "MoveTo", "PlayAnim", "SetPos", "Shake", "Show"}
OTHER = {"Blank", "Nop", "PitchEnvSe", "PlayBGM", "PlayEnvSe", "PlaySE",
         "SetFlag", "StopBGM", "StopEnvSe"}
TRANSITIONS = {"ChangeMap", "EncountBoss", "Encount"}
LANGUAGES = {"en", "ja", "fr", "it", "de", "es", "ko", "zht", "zhc", "ru", "th", "pt"}


def need(condition, detail):
    if not condition:
        raise ValueError(detail)


def operands(op, name):
    return (op.get("operands") or {}).get(name) or []


@dataclass(frozen=True)
class Event:
    tick: int
    pc: int
    command: str
    actor: int | None
    page: str = ""


class FieldReplay:
    def __init__(self, body, fps, page_seconds, ready_seconds, branches=None, durations=None):
        self.body = body
        self.fps = fps
        self.page_seconds = page_seconds
        self.ready_seconds = ready_seconds
        self.branches = branches or {}
        self.durations = durations or {}
        self.events = []
        self.steps = 0

    def ticks(self, seconds):
        need(math.isfinite(seconds) and seconds >= 0, "Replay durations must be finite and nonnegative")
        return math.ceil(seconds * self.fps - 1e-9)

    def track(self, pc, tick, actor=None):
        pending = {}
        active_until = tick
        returns = []
        while True:
            self.steps += 1
            need(self.steps <= 10_000, "Instruction limit reached")
            need(0 <= pc < len(self.body), f"Instruction {pc} is outside the extract")
            op = self.body[pc]
            command = op["mnemonic"]
            ints, reals, strings = (operands(op, k) for k in ("iValues", "rValues", "sValues"))
            after = pc + 1
            if command == "Wait":
                tick += self.ticks(reals[0])
            elif command == "Msg":
                self.events.append(Event(tick, pc, command, actor, strings[0]))
                tick += self.ticks(self.page_seconds)
            elif command == "Call":
                need(actor is None and len(returns) < 32, f"Unsupported call at {pc}")
                returns.append(after)
                after = ints[0]
            elif command == "Return":
                need(bool(returns), f"Return without a call at {pc}")
                after = returns.pop()
            elif command == "Jump":
                after = ints[0]
            elif command == "Branch":
                need(pc in self.branches, f"Supply the condition outcome for branch {pc}")
                if self.branches[pc]:
                    after = ints[0]
            elif command == "SetPuppet":
                slot, entity, entry = ints[:3]
                need(actor is None and 0 <= slot < 32, f"Unsupported puppet slot at {pc}")
                pending[slot] = (entity, entry)
            elif command == "ExecPuppet":
                need(actor is None, f"Nested puppet launch at {pc}")
                tick = max(tick, active_until)
                active_until = max([tick] + [
                    self.track(entry, tick, entity) for _, (entity, entry) in sorted(pending.items())
                ])
                pending.clear()
                if not ints or ints[0] != 1:
                    tick = active_until
            elif command == "FinishPuppet":
                active_until = tick
                pending.clear()
                self.events[:] = [event for event in self.events if event.actor is None or event.tick <= tick]
            elif command == "Exit" or command in TRANSITIONS:
                if command in TRANSITIONS:
                    self.events.append(Event(tick, pc, command, actor))
                return tick
            elif command in VISUAL:
                self.events.append(Event(tick, pc, command, actor))
                tick += self.ticks(self.durations.get(pc, 0))
            else:
                need(command in OTHER, f"Unsupported instruction {command} at {pc}")
            pc = after

    def run(self):
        end = self.track(0, self.ticks(self.ready_seconds))
        # Parallel actors scheduled beyond a transition cannot still be drawn.
        return sorted((event for event in self.events if event.tick <= end), key=lambda row: row.tick)


CUE_ROW = re.compile(
    r'Cue\{game::Id::(FF[1-6]), (\d+), (-?\d+), (\d+), L"([^"]+)", L"([^"]+)"'
    r'(?:, CueTrigger::(\w+))?(?:,.*?)?\},?$'
)


def registry():
    source = ROOT / "src/core/field/scene_effect_cues.cpp"
    for text in source.read_text(encoding="utf-8").splitlines():
        row = text.strip()
        if not row.startswith("Cue{"):
            continue
        match = CUE_ROW.fullmatch(row)
        need(match is not None, "Unrecognized production scene cue row")
        title, map_id, actor, pc, scene, key, trigger = match.groups()
        yield (title.lower(), int(map_id), scene), (int(actor), int(pc), trigger or "PlayAnim"), key


@lru_cache(maxsize=6)
def map_names(extract, game):
    master = extract / f"{game}_master/map.txt"
    with master.open(encoding="utf-8-sig") as records:
        return {int(row["id"]): row["map_name"] for row in csv.DictReader(line for line in records if line.strip())}


@lru_cache(maxsize=128)
def load_field(extract, game, map_id, scene):
    location = map_names(extract, game)[map_id]
    with (extract / f"{game}_scripts.jsonl").open(encoding="utf-8-sig") as data:
        matches = [row for line in data if scene in line and (row := json.loads(line))["name"] == scene
                   and row["submap"] == location]
    need(len(matches) == 1, f"Expected one {scene} at {location}, found {len(matches)}")
    return matches[0]["mnemonics"]


@lru_cache(maxsize=6)
def mod_text(game):
    texts = {}
    for catalog in ("common", game):
        texts.update(json.loads((ROOT / "data" / catalog / "mod_text.json").read_text(encoding="utf-8-sig")))
    return texts


def verify_locale(game, key):
    phrases = mod_text(game).get(key, {})
    need(set(phrases) == LANGUAGES and all(isinstance(value, str) and value.strip() for value in phrases.values()),
         f"Incomplete locales for {key!r}")


def verify_bindings(extract):
    count = 0
    faults = []
    seen = set()
    for scope, identity, key in registry():
        try:
            actor, pc, command = identity
            need((scope, identity) not in seen, "Duplicate animation binding")
            seen.add((scope, identity))
            body = load_field(extract, *scope)
            need(0 <= pc < len(body) and body[pc]["mnemonic"] == command, "Wrong opcode or instruction offset")
            assignments = []
            for op in body:
                if op["mnemonic"] != "SetPuppet":
                    continue
                _, entity, entry = operands(op, "iValues")[:3]
                if 0 <= entry <= pc and not any(row["mnemonic"] in {"Exit", "Return", "SetPuppet"}
                                               for row in body[entry:pc]):
                    assignments.append(entity)
            need(actor in assignments or (actor == 0 and command == "ColorFade"),
                 "Wrong puppet binding")
            verify_locale(scope[0], key)
            count += 1
        except (KeyError, ValueError) as error:
            faults.append(f"{scope[0]} {scope[1]} {scope[2]} instruction {identity[1]}: {error}")
    for fault in faults:
        print(fault)
    print(f"{count} matching scene bindings and catalogs, {len(faults)} unresolved")
    return 1 if faults else 0


def audit(extract, location, fps, page, ready, branches=None, durations=None):
    game = location[0]
    model = FieldReplay(load_field(extract, *location), fps, page, ready, branches, durations)
    trace = model.run()
    matches = {(event.actor or 0, event.pc, event.command) for event in trace}
    bound = {}
    for scope, identity, key in registry():
        if scope != location:
            continue
        need(identity in matches, f"{scope} cue {key!r} did not run in this scenario")
        need(identity not in bound, f"{scope} duplicate cue {identity}")
        verify_locale(game, key)
        bound[identity] = key
    return model, trace, bound


def verify_fire_scenes(extract):
    for fps, page, ready in ((30, .1, 0), (60, .5, .7), (144, 2.5, 4)):
        for map_id, scene, expected in (
            (271, "sc_e_0046", (110, 17, 117, 197, 45)),
            (271, "sc_e_0046_1", (6, 172, 262, 24)),
            (104, "sc_e_0047", (70, 517, 568, 614, 112, 643, 132, 787, 858, 160, 877, 888, 176)),
        ):
            # Allow a native update for a nonzero-length puppet move. Actual
            # speed and presentation are independently witnessed by the mod.
            motion = {197: 1 / fps} if scene == "sc_e_0046" else {}
            model, trace, bound = audit(extract, ("ff5", map_id, scene), fps, page, ready, durations=motion)
            by_pc = {event.pc: event for event in trace}
            ticks = [by_pc[pc].tick for pc in expected]
            need(ticks == sorted(ticks) and all(earlier < later for earlier, later in zip(ticks, ticks[1:])),
                 f"Unexpected phase ordering in {scene} at {fps} Hz")
            need(len(bound) == {"sc_e_0046": 3, "sc_e_0046_1": 2, "sc_e_0047": 8}[scene],
                 f"Unexpected registered cue coverage for {scene}")
            if scene == "sc_e_0046":
                need(by_pc[110].tick + model.ticks(.21) < by_pc[17].tick, "Late mist must fit ahead of the page")
            if scene == "sc_e_0046_1":
                need(by_pc[6].tick + model.ticks(.7) < by_pc[172].tick, "Blasts must follow the return fade")
                need(by_pc[172].tick + model.ticks(.4) < by_pc[262].tick, "Late blast must fit before the queen falls")
            print(f"{scene} at {fps} Hz: {len(bound)} reachable cues, phase order verified")


def pairs(items, convert):
    return {int(pc): convert(value) for pc, value in (item.split("=", 1) for item in items)}


def branch_result(value):
    need(value in {"0", "1"}, "Supply 0 or 1 for a branch outcome")
    return value == "1"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("extract", type=Path, help="Local map extraction directory")
    parser.add_argument("--game", default="ff5", choices=[f"ff{n}" for n in range(1, 7)])
    parser.add_argument("--map", type=int)
    parser.add_argument("--scene")
    parser.add_argument("--fps", type=int, default=60)
    parser.add_argument("--page-seconds", type=float, default=.5)
    parser.add_argument("--ready-seconds", type=float, default=0)
    parser.add_argument("--branch", action="append", default=[], metavar="PC=0_OR_1")
    parser.add_argument("--duration", action="append", default=[], metavar="PC=SECONDS")
    parser.add_argument("--verify-ff5-fire-scenes", action="store_true")
    parser.add_argument("--verify-bindings", action="store_true")
    options = parser.parse_args()
    if options.verify_ff5_fire_scenes:
        verify_fire_scenes(options.extract)
        return
    if options.verify_bindings:
        return verify_bindings(options.extract)
    if not options.map or not options.scene or options.fps <= 0:
        parser.error("Supply a map, scene and positive frame rate")
    model, trace, cues = audit(options.extract, (options.game, options.map, options.scene),
                              options.fps, options.page_seconds, options.ready_seconds,
                              pairs(options.branch, branch_result), pairs(options.duration, float))
    for event in trace:
        identity = (event.actor or 0, event.pc, event.command)
        if event.command in VISUAL or event.command in TRANSITIONS or event.page:
            status = cues.get(identity, "review" if event.command in VISUAL else event.page)
            print(f"{event.tick / model.fps:7.3f}s  {event.pc:4}  {str(event.actor):>4}  {event.command:<14} {status}")


if __name__ == "__main__":
    raise SystemExit(main())
