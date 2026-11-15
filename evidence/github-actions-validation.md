# NetSync Lab — GitHub Actions verification

**Date (UTC):** 2026-11-15  
**Source commit:** `d3af23800d978d6dfb968f68a2e49d8d86abbdd7`  
**Workflow:** `.github/workflows/ci.yml` — Build and test  
**GitHub Actions run:** https://github.com/Preciouscrazy/NetSyncLab/actions/runs/43688559330

## Hosted CI results

| GitHub runner | Configure | Build | CTest | Source-hygiene scan |
| --- | --- | --- | --- | --- |
| `windows-latest` | PASS | PASS | PASS | PASS |
| `ubuntu-latest` | PASS | PASS | PASS | PASS |

Both independent jobs completed with GitHub Actions conclusion **success**.

The five CTest checks comprise `unit_tests` (23 C++ cases), `demo_converges`, `ideal_converges`, `trace_converges`, and `cli_golden_output` (5 CLI tests).

The workflow uses Python 3.12 to execute the CLI and source-hygiene tests; the compiled application itself has no Python runtime dependency.

## Local test corroboration

The separate [Windows validation record](windows-validation.md) documents the user's Windows 10 / MSVC 19.51 run: **23/23** C++ unit tests, **5/5** CLI integration tests, **5/5** CTest checks, and deterministic demo convergence.

The prior [Linux validation record](local-validation.md) contains independent GCC and Clang evidence from the starter's development environment. Those files are preserved as dated records; any pre-publication wording about GitHub Actions was correct at the time and has now been superseded by the successful hosted run above.

## Interpretation and limitations

These tests support the specified **deterministic synthetic network simulator** and its documented input contracts. They do not demonstrate performance on actual network sockets, security under hostile clients, real-world latency, production readiness, or human authorship. The UTF-8 scan detects selected source-hygiene issues; it does not certify the absence of all possible provenance or watermark signals.
