# NetSync Lab v0.2 — Windows validation record

**Validation date:** 2026-11-15  
**Project version:** v0.2, bounded retransmission  
**Verified on:** Windows 10 (10.0.19045), Visual Studio Community 18 (2026), MSVC 19.51.36260.0, MSBuild 18.11.1; Windows SDK 10.0.26100.0  
**Python for CLI tests:** 3.15.2  
**GitHub main merge commit:** [`3d72b82`](https://github.com/Preciouscrazy/NetSyncLab/commit/3d72b82399a1fe3c05e28a38da093718c489b497)

## Observed local results

These values reproduce the Windows PowerShell console output supplied after checkout of `feature/bounded-retransmission-v02` and again after updating local `main` to `3d72b82`.

| Verification step | Result |
| --- | --- |
| CMake configuration (Visual Studio 18 2026) | PASS |
| MSVC C++20 build in Debug | PASS |
| CTest suite on feature branch | **6/6 PASS** |
| Individual C++ unit cases | **29/29 PASS** |
| Individual Python CLI integration cases | **7/7 PASS** |
| Source-hygiene scan | **22 UTF-8 text files PASS** |
| CTest after fast-forward of `main` | **6/6 PASS** |
| Final local Git status on `main` | Clean; up to date with `origin/main` |

### Reproduce on the user's Windows toolchain

```powershell
cmake -S . -B build -G "Visual Studio 18 2026"
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure

.\build\Debug\netsync_tests.exe
python tests\test_cli.py .\build\Debug\netsync.exe
.\build\Debug\netsync.exe compare
python tools\check_text.py
```

### Observed deterministic comparison

```csv
policy,ticks,converged,server_ack,client_ack,upstream_packets_sent,upstream_packets_dropped,command_entries_sent,largest_batch,peak_state_gap
baseline,14,true,12,12,14,3,33,4,1
bounded,19,true,12,12,10,2,27,3,2
```

The output matched [the checked-in reference CSV](retransmission-comparison.csv). Both policies acknowledged all 12 inputs and converged.

## Evidence boundaries

This records an **observed Windows/MSVC run**, not a claim about every Windows environment. The scenario is deterministic and synthetic: ticks are not milliseconds, position gaps are abstract grid cells, and command-entry counts do not measure real bandwidth. The separate [GitHub Actions v0.2 record](ci-v02-validation.md) verifies hosted Windows and Ubuntu checks for the merged source. Earlier [v0.1 Windows evidence](windows-validation.md) remains preserved as a historical baseline.
