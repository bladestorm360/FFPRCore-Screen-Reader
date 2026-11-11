"""Execute actual batch guard structure with cheap stand-ins for its tools.

This regression checks the canonical audit-check fragment without compiling or
running the real tools. Success must visit every producer; a failure must stop
at its exact producer. The full canonical build remains separately required.
"""
from pathlib import Path
import json,re,subprocess
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'build_check/broader_audit_20261104';OUT.mkdir(parents=True,exist_ok=True)
body=(ROOT/'build.bat').read_text(encoding='utf-8-sig');a=body.index('python tools\\modtext_puzzle_verbosity.py');b=body.index('"out\\checks\\puzzle_guide_check.exe"',a);fragment=body[a:b]
lines=[];names=[]
for line in fragment.splitlines():
 if line.startswith('python '):
  name=re.search(r'build_check\\([^ >]+)',line).group(1);names.append(name);lines.append('call :producer '+str(len(names)))
 else:lines.append(line)
assert len(names)>=30
script='@echo off\nsetlocal\nset "fail=%~1"\n'+'\n'.join(lines)+'\nexit /b 0\n:producer\necho PRODUCER %~1\nif "%~1"=="%fail%" exit /b 1\nexit /b 0\n'
p=OUT/'build_guard_fixture.bat';p.write_text(script,encoding='utf-8');results=[];errors=[]
for fail in (0,1,len(names)//2,len(names)):
 r=subprocess.run(['cmd.exe','/d','/c','call',str(p),str(fail)],cwd=ROOT,capture_output=True,text=True,timeout=30)
 seen=[int(i)for i in re.findall(r'^PRODUCER (\d+)\s*$',r.stdout,re.M)];wanted=list(range(1,(fail or len(names))+1));ok=seen==wanted and r.returncode==int(fail!=0)
 results.append(dict(failed_producer=fail,expected_visits=len(wanted),actual_visits=seen,exit_code=r.returncode,passed=ok))
 if not ok:errors.append(fail)
(OUT/'build_guard_flow_report.json').write_text(json.dumps(dict(producers=names,cases=results),indent=2)+'\n',encoding='utf-8')
print(f'Canonical audit guard flow:{len(names)} producers/four paths/{len(errors)} failures;cheap stand-ins,no real build.')
raise SystemExit(bool(errors))
