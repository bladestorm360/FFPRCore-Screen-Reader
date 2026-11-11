"""Build generated dependencies in small serial groups inside one capped tree.

Historical all-project retention exceeded1GB; fresh processes per target bounded
it but repeated startup151 times. Three-project groups halve measured warm
startup with identical artifacts at the same limits, while bounding retention.
One worker, no compiler/project parallelism, no reused nodes or build servers.
"""
from pathlib import Path
import argparse
import re
import subprocess
import sys
import time
import uuid
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cmake', type=Path)
    parser.add_argument('--clean-first', action='store_true')
    parser.add_argument('--group-size', type=int, choices=(1,2,3), default=3,
                        help='Bounded serial project retention;1 keeps historical per-target mode.')
    parser.add_argument('--targets', nargs='+', help='Optional existing generated dependencies for focused development.')
    args = parser.parse_args()
    subprocess.run([sys.executable, str(ROOT / 'scripts/run-build.py'), '--verify'],
                   cwd=ROOT, check=True)
    project = ROOT / 'build/ALL_BUILD.vcxproj'
    namespace = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}
    references = ET.parse(project).findall('./m:ItemGroup/m:ProjectReference', namespace)
    targets = []
    for reference in references:
        path = Path(reference.attrib['Include']).resolve()
        if path.parent != project.parent.resolve() or not path.is_file():
            raise RuntimeError('ALL_BUILD dependency is outside the generated project directory')
        name = path.stem
        if name == 'ZERO_CHECK':
            continue  # CMake performs this dependency for each real target.
        if not re.fullmatch(r'[A-Za-z0-9_]+', name) or name == 'ALL_BUILD':
            raise RuntimeError('Unexpected generated target name')
        if name not in targets:
            targets.append(name)
    if not targets:
        raise RuntimeError('No ALL_BUILD dependencies; refuse an empty successful build')
    if args.targets:
        if len(set(args.targets))!=len(args.targets) or any(t not in targets for t in args.targets):
            raise RuntimeError('Focused targets must be distinct existing ALL_BUILD dependencies')
        targets=args.targets
    command = [str(args.cmake), '--build', 'build', '--config', 'Release', '--target']
    options = ['--', '/m:1', '/nr:false', '/v:minimal']
    if args.clean_first:
        # Clean once, rather than repeatedly erasing common dependencies.
        subprocess.run(command + ['clean'] + options, cwd=ROOT, check=True)
    # Match CMake's actual64-bit host and VisualStudioVersion. An x86 host made
    # the first comparison relink artifacts and is deliberately not a fallback.
    cache=(ROOT/'build/CMakeCache.txt').read_text(encoding='utf-8')
    instance=re.search(r'^CMAKE_GENERATOR_INSTANCE:INTERNAL=(.+)$',cache,re.M)
    version=re.search(r'^CMAKE_GENERATOR:INTERNAL=Visual Studio (\d+)\b',cache,re.M)
    if not instance or not version:raise RuntimeError('Expected the configured Visual Studio generator/instance')
    msbuild=Path(instance.group(1))/'MSBuild/Current/Bin/amd64/MSBuild.exe'
    if not msbuild.is_file():raise RuntimeError('Configured64-bit MSBuild is unavailable; do not silently change hosts')
    evidence=ROOT/'build_check'/('serial_groups_'+uuid.uuid4().hex)
    evidence.mkdir(parents=True)
    for first in range(0,len(targets),args.group_size):
        group=targets[first:first+args.group_size]
        print(f'Serial projects {first+1}..{first+len(group)}/{len(targets)}: '+', '.join(group),flush=True)
        started=time.monotonic()
        if args.group_size==1:
            result=subprocess.run(command+group+options,cwd=ROOT)
        else:
            # The MSBuild task uses this process's existing node. Exact project
            # references preserve normal dependencies and every required target;
            # a first failure stops the group and all subsequent groups.
            root=ET.Element('Project',DefaultTargets='Build',xmlns=namespace['m'])
            items=ET.SubElement(root,'ItemGroup')
            for target in group:ET.SubElement(items,'OwnedProject',Include=str(ROOT/'build'/(target+'.vcxproj')))
            task=ET.SubElement(root,'Target',Name='Build')
            ET.SubElement(task,'MSBuild',Projects='@(OwnedProject)',Targets='Build',BuildInParallel='false',StopOnFirstFailure='true',
                          Properties='Configuration=Release;Platform=x64;VisualStudioVersion='+version.group(1)+'.0')
            path=evidence/(str(first//args.group_size)+'.proj')
            ET.ElementTree(root).write(path,encoding='utf-8',xml_declaration=True)
            result=subprocess.run([str(msbuild),str(path),*options[1:]],cwd=ROOT)
        print(f'Serial group completed in {time.monotonic()-started:.3f}s; exit{result.returncode}',flush=True)
        if result.returncode:return result.returncode
    print(f'All {len(targets)} ALL_BUILD dependencies completed serially.', flush=True)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
