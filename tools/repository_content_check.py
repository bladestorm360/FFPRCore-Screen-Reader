"""Reject recovered game implementation in Git; never emit inspected bodies.

Original mod code, compiled navigation metadata and attributed dependencies are
valid repository inputs. Game dumps/disassembly and script exports belong in
external analysis directories even when a repository path would be ignored.
The marker checks enforce known violations; they do not waive the human rule
for an unfamiliar source format. Reports contain only paths, hashes and reasons.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
ASSEMBLY = re.compile(rb'(?m)^\s*(?:0x)?[0-9A-Fa-f]{4,16}\s+(?:mov|push|pop|call|sub|add|ret|jmp|lea|cmp|test|xor)\b')
DECLARATIONS = re.compile(rb'(?m)^\s*// (?:RVA: 0x|Token: 0x|Namespace:)')
BINARY = {'.dll', '.exe', '.bin', '.wav', '.png', '.ico', '.lib', '.zip', '.7z', '.pdb'}
BODY_KEYS = {'mnemonic', 'op_str', 'disassembly', 'decompiled', 'pseudocode', 'asm'}


def reasons(name, data):
    path = Path(name)
    found = []
    if path.name.lower() in {'dump.cs', 'gameassembly.dll', 'global-metadata.dat'}:
        found.append('recovered game input filename')
    if path.suffix.lower() in BINARY:
        return found
    if ASSEMBLY.search(data):
        found.append('native instruction listing')
    if DECLARATIONS.search(data):
        found.append('recovered game declarations')
    if name.startswith('docs/re/') and path.suffix.lower() in {'.c', '.asm', '.cs'}:
        found.append('native analysis body in documentation tree')
    if path.suffix.lower() in {'.json', '.jsonl'}:
        if re.search(rb'"Main"\s*:\s*\[', data) and re.search(rb'"(?:Command|command|Type|type)"\s*:', data):
            found.append('original game script body')
        try:
            if path.suffix.lower() == '.jsonl':
                documents = [json.loads(line) for line in data.splitlines() if line.strip()]
            else:
                documents = [json.loads(data.decode('utf-8-sig'))]
        except (ValueError, UnicodeError):
            documents = []
        while documents:
            item = documents.pop()
            if isinstance(item, dict):
                if BODY_KEYS.intersection(item):
                    found.append('recovered native body fields')
                    break
                documents.extend(item.values())
            elif isinstance(item, list):
                documents.extend(item)
    return found


def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args])


def blob_rows(history):
    if history:
        rows = git('rev-list', '--objects', '--all').splitlines()
        wanted = [(row.split(b' ', 1)[0], row.split(b' ', 1)[1].decode('utf-8', 'replace'))
                  for row in rows if b' ' in row]
    else:
        wanted = []
        for row in git('ls-files', '--stage', '-z').split(b'\0'):
            if not row:
                continue
            metadata, name = row.split(b'\t', 1)
            _, oid, stage = metadata.split()
            if stage != b'0':
                raise RuntimeError('Resolve the index conflict before checking content')
            wanted.append((oid, name.decode('utf-8')))
    process = subprocess.Popen(['git', '-C', str(ROOT), 'cat-file', '--batch'],
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    try:
        for oid, name in wanted:
            process.stdin.write(oid + b'\n')
            process.stdin.flush()
            header = process.stdout.readline().split()
            if len(header) != 3:
                raise RuntimeError('Cannot inspect Git object metadata')
            size = int(header[2])
            data = process.stdout.read(size)
            if len(data) != size or process.stdout.read(1) != b'\n':
                raise RuntimeError('Incomplete Git object read')
            if header[1] == b'blob':
                yield name, oid.decode('ascii'), data
    finally:
        process.stdin.close()
        process.stdout.close()
        if process.wait() != 0:
            raise RuntimeError('Git object reader failed')


def working_rows():
    paths = set(git('ls-files', '--cached', '--others', '--exclude-standard', '-z').decode('utf-8').split('\0')) - {''}
    # These ignored historical export locations are also prohibited. Native
    # exporters now validate external output paths before creating directories.
    for pattern in ('docs/**/*.c', 'docs/**/*.asm', 'docs/**/*.cs', 'docs/**/*.log'):
        paths.update(p.relative_to(ROOT).as_posix() for p in ROOT.glob(pattern) if p.is_file())
    for name in sorted(paths):
        path = ROOT / name
        if path.is_file():
            data = path.read_bytes()
            yield name, hashlib.sha256(data).hexdigest(), data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument('--history', action='store_true')
    modes.add_argument('--staged', action='store_true')
    modes.add_argument('--file', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    if args.file:
        data = args.file.read_bytes()
        rows = [(str(args.file), hashlib.sha256(data).hexdigest(), data)]
    else:
        rows = blob_rows(args.history) if args.history or args.staged else working_rows()
    checked = 0
    rejected = []
    for name, digest, data in rows:
        checked += 1
        failures = reasons(name, data)
        if failures:
            rejected.append(dict(path=name, object_id=digest, bytes=len(data), reasons=failures))
    report = dict(mode='history' if args.history else 'staged' if args.staged else 'file' if args.file else 'working',
                  checked=checked, rejected=rejected, rejected_count=len(rejected))
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    for row in rejected:
        print('REJECT:', row['path'], row['object_id'], ', '.join(row['reasons']))
    print('Repository game-source check:', checked, 'objects/files,', len(rejected), 'rejected')
    return 1 if rejected else 0


if __name__ == '__main__':
    sys.exit(main())
