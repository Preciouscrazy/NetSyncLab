# NetSync Lab - Windows Validation

Date: 2026-11-15
OS: Windows 10
Compiler: MSVC 19.51
Build: Visual Studio 2026 / MSBuild 18
Python: 3.15.2

## Verified Results

- CMake configuration: PASS
- C++ compilation: PASS
- C++ unit tests: 23/23 PASS
- CLI integration tests: 5/5 PASS
- CTest checks: 5/5 PASS
- Source-hygiene scan: PASS
- Demo convergence: PASS

## Synthetic Demo

- Simulation ticks: 14
- Issued inputs: 12
- Upstream drops: 3
- Downstream drops: 4
- Peak state gap: 1
- Final server position: 14
- Final client position: 14
- Pending inputs: 0

## Limitations

These results describe synthetic networking
conditions and local Windows verification.

GitHub Actions must be verified separately.
Source-hygiene checks do not prove authorship.