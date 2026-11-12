"""Build the normal FFPR payload with named checks for a local delivery."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

import pefile

from build_one_check import CMAKE, ROOT

RUNNER = ROOT / 'scripts/run-build.py'
EVIDENCE = ROOT / 'build_check/local-delivery'
OUT = ROOT / 'out'

# Retain the argument variants used by the complete build for these fixtures.
CHECK_CASES = {
    'controller_worker_check': [[], ['failure']],
    'map_geometry_check': [['--verify-relays']],
    'npc_label_audit_check': [[f'ff{number}'] for number in range(1, 7)],
    'npc_oscillation_check': [['--verify']],
    'route_check': [['data/ff2/maps.bin', 'data/ff1/maps.bin']],
}


def run(label, command, env):
    report = EVIDENCE / f'{label}.txt'
    print(f'{label}...', flush=True)
    with report.open('wb') as output:
        subprocess.run(command, cwd=ROOT, env=env, stdout=output,
                       stderr=subprocess.STDOUT, check=True)


def verify_resources(dll):
    generated = (ROOT / 'build/data.rc').read_text(encoding='utf-8')
    inputs = {
        key: Path(filename.replace('\\\\', '\\'))
        for key, filename in re.findall(r'^(\w+) RCDATA "([^"\r\n]+)"$', generated, re.MULTILINE)
    }
    if not inputs:
        raise ValueError('The generated resource manifest is empty')
    with pefile.PE(str(dll)) as pe:
        resource = next(entry for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries if entry.id == 10)
        embedded = {str(entry.name): entry.directory.entries[0].data.struct
                    for entry in resource.directory.entries if entry.name}
        if inputs.keys() != embedded.keys():
            raise ValueError('The built resource names do not match the generated manifest')
        for key, path in inputs.items():
            entry = embedded[key]
            if not path.resolve().is_relative_to(ROOT / 'data'):
                raise ValueError(f'Unexpected resource input: {path}')
            if pe.get_data(entry.OffsetToData, entry.Size) != path.read_bytes():
                raise ValueError(f'Stale or invalid built resource: {key}')
    return len(inputs)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('checks', nargs='+', metavar='CHECK',
                        help='Maintained C++ checks for the affected behavior')
    checks = parser.parse_args().checks
    if len(set(checks)) != len(checks) or any(
        not re.fullmatch(r'[a-z][a-z0-9_]*_check', name) or
        not (ROOT / 'tools' / (name + '.cpp')).is_file() for name in checks
    ):
        parser.error('Specify distinct existing *_check targets')

    if not os.environ.get('CODEX_BUILD_JOB_NAME'):
        return subprocess.call(
            [sys.executable, '-B', str(RUNNER), '--receipt', str(EVIDENCE / 'resource-job.json'), '--',
             sys.executable, '-B', str(Path(__file__).resolve()), *checks], cwd=ROOT)
    subprocess.run([sys.executable, '-B', str(RUNNER), '--verify'], cwd=ROOT, check=True)
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    receipt = EVIDENCE / 'candidate.json'
    receipt.unlink(missing_ok=True)
    scratch = EVIDENCE / 'temp'
    scratch.mkdir(exist_ok=True)
    env = dict(os.environ, TEMP=str(scratch), TMP=str(scratch), CMAKE_BUILD_PARALLEL_LEVEL='1',
               PYTHONDONTWRITEBYTECODE='1')
    began = time.monotonic()
    python = [sys.executable, '-B']
    run('source-boundary', python + ['tools/repository_content_check.py'], env)
    run('pinned-openal', python + ['tools/openal_dependency.py', 'check'], env)
    run('configure', [str(CMAKE), '-S', '.', '-B', 'build', '-A', 'x64'], env)
    run('production-and-check-targets', python + [
        'tools/build_projects_serial.py', str(CMAKE), '--targets', 'hid', 'smoke_host', *checks
    ], env)
    for name in checks:
        executable = OUT / 'checks' / (name + '.exe')
        for index, arguments in enumerate(CHECK_CASES.get(name, [[]]), 1):
            run(f'{name}-{index}', [str(executable), *arguments], env)
    run('native-names', python + ['tools/name_check.py'], env)
    dll = OUT / 'hid.dll'
    resources = verify_resources(dll)
    with dll.open('rb') as stream:
        sha256 = hashlib.file_digest(stream, 'sha256').hexdigest()
    elapsed = time.monotonic() - began
    receipt.write_text(json.dumps({
        'checks': checks, 'resources': resources, 'dll_sha256': sha256,
        'bytes': dll.stat().st_size, 'elapsed_seconds': round(elapsed, 3)
    }, indent=2) + '\n', encoding='utf-8')
    print(f'Local build passed in {elapsed:.1f}s. Verified {resources} embedded resources.')
    print('Run smoketest.bat and the exact payload scan before local deployment.')
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (OSError, ValueError, StopIteration, subprocess.CalledProcessError) as error:
        print(f'Local build failed: {error}. See build_check/local-delivery.', file=sys.stderr)
        raise SystemExit(1)
