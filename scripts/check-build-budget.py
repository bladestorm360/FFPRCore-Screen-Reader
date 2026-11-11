"""Small bounded checks for the real Windows job guard; no full-size allocation."""
import ctypes as c
from ctypes import wintypes as w
import json
import mmap
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time


def main():
    guard = Path(__file__).with_name("run-build.py")
    checks = 0
    with tempfile.TemporaryDirectory(prefix="build-budget-") as directory:
        root = Path(directory)

        def run(name, code, expected=0, memory=64):
            nonlocal checks
            receipt = root / (name + ".json")
            result = subprocess.run([sys.executable, str(guard), "--memory-mb", str(memory),
                                     "--receipt", str(receipt), "--", sys.executable, "-c", code],
                                    capture_output=True, text=True, timeout=20)
            if (result.returncode == 0) != (expected == 0):
                raise AssertionError(name + ": " + result.stdout + result.stderr)
            data = json.loads(receipt.read_text())
            if data.get("limitsVerified"):
                assert data["memoryLimitBytes"] == memory * 1_000_000 // mmap.PAGESIZE * mmap.PAGESIZE
                # Windows can include a refused commitment attempt in its raw
                # peak counter. Deliberate OOM cases prove denial instead;
                # successful ordinary children must remain below the budget.
                if expected == 0:
                    assert data["peakJobMemoryBytes"] <= data["memoryLimitBytes"], repr(data)
                assert data["childrenDrained"]
                assert 0 < data["cpuPercent"] <= 25
                processors = max(1, os.cpu_count() or 1)
                assert data["cpuPercent"] == max(1, min(2500, 30_000 // processors)) / 100
                assert data["cpuEquivalentCoreLimit"] <= 3
            checks += 1
            return result, data

        run("normal", "print('normal child')")
        _, maximum = run("approved-default", "print('2 GB limit, no large allocation')", memory=2000)
        assert maximum["sharedSlots"] == 2 and maximum["slot"] in (1, 2)
        run("page-rounded", "print('rounded child')", memory=33)
        run("verify-inside", "import subprocess,sys;sys.exit(subprocess.call([sys.executable," + repr(str(guard)) + ",'--verify']))")
        run("minimum-verify", "import subprocess,sys;sys.exit(subprocess.call([sys.executable," + repr(str(guard)) + ",'--verify']))", memory=32)
        outside = subprocess.run([sys.executable, str(guard), "--verify"], capture_output=True, timeout=10)
        assert outside.returncode != 0
        checks += 1
        result, _ = run("memory", "a=bytearray(96_000_000)", expected=1)
        assert "MemoryError" in result.stderr
        child = "a=bytearray(96_000_000)"
        result, _ = run("child-memory", "import subprocess,sys;sys.exit(subprocess.call([sys.executable,'-c'," + repr(child) + "]))", expected=1)
        assert "MemoryError" in result.stderr
        leaf = "import time;a=bytearray(32_000_000);print('allocated',flush=True);time.sleep(2)"
        aggregate = (
            "import subprocess,sys;cmd=[sys.executable,'-c'," + repr(leaf) + "];"
            "first=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True);"
            "assert first.stdout.readline().strip()=='allocated';"
            "second=subprocess.run(cmd,capture_output=True,text=True);"
            "first.communicate();sys.stderr.write(second.stderr);"
            "sys.exit(37 if second.returncode!=0 else 0)"
        )
        result, _ = run("aggregate-memory", aggregate, expected=1)
        assert "MemoryError" in result.stderr
        _, rejected = run("over-budget", "raise Exception('must never start')", expected=1, memory=2001)
        assert not rejected["limitsVerified"] and "childPid" not in rejected

        ready = [root / "ready-1", root / "ready-2"]
        release = root / "release"
        held = []
        try:
            for index in range(2):
                code = ("import pathlib,time;pathlib.Path(" + repr(str(ready[index])) + ").write_text('ready');"
                        "deadline=time.monotonic()+20\n"
                        "while not pathlib.Path(" + repr(str(release)) + ").exists() and time.monotonic()<deadline:time.sleep(.02)")
                held.append(subprocess.Popen([sys.executable, str(guard), "--memory-mb", "64",
                                              "--receipt", str(root / f"slot-{index}.json"),
                                              "--", sys.executable, "-c", code],
                                             stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True))
                deadline = time.monotonic() + 10
                while not ready[index].exists() and held[-1].poll() is None and time.monotonic() < deadline:
                    time.sleep(.02)
                assert ready[index].exists(), "Two-slot admission needs both slots free; preserve other jobs and retry this check later."
            result, busy = run("busy", "raise Exception('must never start')", expected=1)
            assert result.returncode == 75 and busy["busy"] and "childPid" not in busy
        finally:
            release.write_text("release")
            for process in held:
                process.communicate(timeout=10)
        assert all(process.returncode == 0 for process in held)
        slots = [json.loads((root / f"slot-{index}.json").read_text()) for index in range(2)]
        assert {item["slot"] for item in slots} == {1, 2}
        assert all(item["sharedSlots"] == 2 and item["childrenDrained"] for item in slots)
        checks += 1

        orphan = root / "orphan.pid"
        spawn = "import subprocess,sys,pathlib;p=subprocess.Popen([sys.executable,'-c','import time;time.sleep(30)']);pathlib.Path(" + repr(str(orphan)) + ").write_text(str(p.pid))"
        run("cleanup", spawn)
        kernel = c.WinDLL("kernel32", use_last_error=True)
        kernel.OpenProcess.argtypes, kernel.OpenProcess.restype = [w.DWORD, w.BOOL, w.DWORD], w.HANDLE
        kernel.WaitForSingleObject.argtypes, kernel.WaitForSingleObject.restype = [w.HANDLE, w.DWORD], w.DWORD
        kernel.CloseHandle.argtypes = [w.HANDLE]
        handle = kernel.OpenProcess(0x100000, False, int(orphan.read_text()))
        if handle:
            try:
                assert kernel.WaitForSingleObject(handle, 1000) == 0, "Child survived job cleanup."
            finally:
                kernel.CloseHandle(handle)
        checks += 1
    print(f"{checks} real resource-guard checks passed;64 MB test cap, no full-size allocation.")


if __name__ == "__main__":
    main()
