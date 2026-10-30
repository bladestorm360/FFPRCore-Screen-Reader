"""Audit complete scene-script bodies and map-change continuations, offline.

Original analysis only in Git; proprietary script bodies remain in mapdump.
The report contains factual identities/hashes, never script snippets/dialogue.
Pure travel is deliberately a small whitelist, not absence-of-dialogue guessing.
"""
from pathlib import Path
import collections,csv,hashlib,json,subprocess

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
OUT=ROOT.parent/'tools/map_event_audit_20261028'
assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
TRAVEL={'Nop','Branch','Jump','Exit','Return','Wait','FadeOut','FadeIn','StopBGM',
        'PlayBGM','StopEnvSe','PauseEnvSe','PlayEnvSe','ChangeMap','SetVehicle',
        'AddTelepoCache','RemoveTelepoCache'}
def body_hash(s):return hashlib.sha256(json.dumps(s.get('mnemonics'),sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
def rows(path):return list(csv.DictReader(path.open(encoding='utf-8-sig')))
summaries=[]
for n in range(1,7):
    scripts=[json.loads(l) for l in (DUMP/f'ff{n}_scripts.jsonl').open(encoding='utf-8-sig')]
    exact={(s['submap'],s['name']):s for s in scripts};by_name=collections.defaultdict(list)
    for s in scripts:by_name[s['name']].append(s)
    maps={int(r['id']):r['map_name'] for r in rows(DUMP/f'ff{n}_master/map.txt') if r['id'] and int(r['id'])>0}
    def resolve(sm,name):
        if (sm,name) in exact:return exact[sm,name]
        choices=by_name[name]
        if choices and len({body_hash(s) for s in choices})==1:return choices[0]
        return None
    def closure(s,seen=None):
        seen=set() if seen is None else set(seen);key=(s['submap'],s['name'])
        if key in seen:return [],['continuation_cycle']
        seen.add(key);found=[s];unknown=[]
        for m in s.get('mnemonics') or []:
            v=m.get('operands')or {};ss=v.get('sValues')or [];ii=v.get('iValues')or []
            if m['mnemonic']=='ChangeMap' and ss and ss[0]:
                child=resolve(maps.get(ii[0],''),ss[0])
                if child:
                    extra,gaps=closure(child,seen);found+=extra;unknown+=gaps
                else:unknown.append('missing_continuation:'+ss[0])
            elif m['mnemonic']=='ChangeScript':unknown.append('native_change_script')
        return found,unknown
    facts=json.loads((OUT/f'ff{n}_facts.json').read_text(encoding='utf-8'));report=[]
    for family in facts['scene_families']:
        real=[o for o in family['occurrences'] if not o['submap'].startswith('Map_00000')]
        if not real:continue
        source=resolve(family['source_submap'],family['script']);assert source and body_hash(source)==family['body_sha256']
        bodies,gaps=closure(source);ops=collections.Counter(m['mnemonic'] for b in bodies for m in b.get('mnemonics')or [])
        pure=bool(ops.get('ChangeMap')) and not gaps and set(ops)<=TRAVEL
        report.append(dict(script=family['script'],script_id=family['script_id'],body_sha256=family['body_sha256'],
            occurrences=real,classification='proven_pure_travel' if pure else 'requires_context_review',
            operations=dict(ops),continuations=[dict(submap=b['submap'],script=b['name'],body_sha256=body_hash(b)) for b in bodies[1:]],
            unresolved=gaps))
    (OUT/f'ff{n}_scene_review.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    counts=collections.Counter(r['classification'] for r in report)
    summary=dict(game=n,families=len(report),pure_travel=counts['proven_pure_travel'],context_review=counts['requires_context_review'],
                 missing_script_occurrences=len(facts['missing_scene_scripts']),continuation_gaps=sum(bool(r['unresolved']) for r in report))
    summaries.append(summary);print(summary,flush=True)
(ROOT/'build_check/map_event_audit_20261028/scene_summary.json').write_text(json.dumps(summaries,indent=2)+'\n')
