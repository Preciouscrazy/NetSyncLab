# NetSync Lab v0.2 — GitHub Actions verification

**Date:** 2026-11-15  
**Source:** merged `main` commit [`3d72b82399a1fe3c05e28a38da093718c489b497`](https://github.com/Preciouscrazy/NetSyncLab/commit/3d72b82399a1fe3c05e28a38da093718c489b497)  
**Workflow:** [`.github/workflows/ci.yml`](../.github/workflows/ci.yml)  
**Hosted run:** [#43689844368 — Build and test](https://github.com/Preciouscrazy/NetSyncLab/actions/runs/43689844368)

## Verified hosted jobs

| Runner | Configure | Build | CTest | Source hygiene | Overall |
| --- | --- | --- | --- | --- | --- |
| `windows-latest` | PASS | PASS | PASS | PASS | **SUCCESS** |
| `ubuntu-latest` | PASS | PASS | PASS | PASS | **SUCCESS** |

Both jobs completed successfully on the **v0.2 post-merge `main` commit**. The workflow uses Python 3.12 for testing and source checks. It registers six CTest entries, including the C++ unit suite and Python golden-output checks.

The separate [Windows v0.2 local validation](windows-v02-validation.md) records an individual 29/29 unit-test run and 7/7 CLI-test run on Windows 10 / MSVC 19.51. This hosted run verifies that the configured CI test steps succeeded on both operating systems; it is not evidence of performance under live packet transport.

The older [initial CI verification](github-actions-validation.md) corresponds to the v0.1 source and is retained as historical evidence.
