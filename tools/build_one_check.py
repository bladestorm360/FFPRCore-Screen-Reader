"""Canonical build.bat --check NAME development slice; no deployment/game run.

Build and run one actual production-consumer check at a time. This is not a
complete candidate build: the ordinary build.bat and smoke/security checks
remain mandatory before delivery. Use existing CMake inputs and one compiler.
"""
from pathlib import Path
import os
import re
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
CMAKE = Path('C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe')


def main():
    if len(sys.argv) != 2 or not re.fullmatch(r'[a-z][a-z0-9_]*_check', sys.argv[1]):
        raise SystemExit('Expected one existing check target name')
    name = sys.argv[1]
    if not (ROOT/'tools'/(name+'.cpp')).is_file():
        raise SystemExit('The named check has no owned source file')
    evidence = ROOT/'build_check'
    evidence.mkdir(exist_ok=True)
    env = os.environ.copy()
    env['CMAKE_BUILD_PARALLEL_LEVEL'] = '1'
    began = time.monotonic()
    commands = [[sys.executable, 'tools/openal_dependency.py', 'check'],
                [str(CMAKE), '-S', '.', '-B', 'build', '-A', 'x64'],
                [str(CMAKE), '--build', 'build', '--config', 'Release', '--target', name,
                 '--', '/m:1', '/nr:false', '/v:minimal']]
    with (evidence/(name+'_target_build.txt')).open('wb') as output:
        for command in commands:
            result = subprocess.run(command, cwd=ROOT, env=env, stdout=output, stderr=subprocess.STDOUT)
            if result.returncode:
                print('Targeted build failed:', name, 'exit', result.returncode,
                      '; see build_check/'+name+'_target_build.txt')
                return result.returncode
    executable = ROOT/'out/checks'/(name+'.exe')
    # Match the ordinary build's report name; do not leave a stale canonical
    # report alongside a differently named successful development result.
    with (evidence/(name.removesuffix('_check')+'_report.txt')).open('wb') as output:
        result = subprocess.run([str(executable)], cwd=ROOT, env=env, stdout=output, stderr=subprocess.STDOUT)
    print('Targeted check:', name, 'exit', result.returncode, 'elapsed %.3fs' % (time.monotonic()-began))
    print('Development evidence only; full canonical build/smoke/security required before deployment.')
    return result.returncode


if __name__ == '__main__':
    sys.exit(main())
