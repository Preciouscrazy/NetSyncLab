# NetSync Lab — local validation evidence

**Date:** 2026-11-15
**Scope:** Local Linux environment only. The Windows and Ubuntu GitHub Actions jobs are *configured*, not yet executed on GitHub. Windows/MSVC compilation remains for the user's own Windows environment.

## Toolchain

| Tool | Version / environment |
| --- | --- |
| CMake | 3.31.6 |
| GCC | g++ (Debian 14.2.0-19) 14.2.0 |
| Clang | clang++ 17.0.0 |
| Python (CLI checks) | Python 3.13.5 |
| C++ version | C++20 |

## Commands and results

For **GCC**, configured with `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`, compiled using `cmake --build build`, and executed:

```text
./build/netsync_tests                    RESULT 23/23 tests passed
ctest --test-dir build                   5/5 CTest checks passed
python tests/test_cli.py ./build/netsync 5/5 CLI tests passed
```

For **Clang**, configured with `cmake -S . -B build-clang -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++` and performed equivalent commands:

```text
./build-clang/netsync_tests                    RESULT 23/23 tests passed
ctest --test-dir build-clang                   5/5 CTest checks passed
python tests/test_cli.py ./build-clang/netsync 5/5 CLI tests passed
```

Both builds compile with strict warnings as errors (`-Wall -Wextra -Wpedantic -Werror`). The complete `demo` summary and full `trace` CSV were byte-for-byte identical between GCC and Clang on this environment (compared using `diff`). This is local toolchain agreement, not a proof of portability to untested systems.

## Synthetic scenario results

The included [demo summary](demo-summary.txt) and [per-tick CSV](demo-trace.csv) were generated using the GCC executable and matched the Clang outputs.

- 12 input commands, finishing in 14 simulated ticks (including drain)
- Final server acknowledgment 12 and client acknowledgment 12
- Final server and predicted positions both 14
- Peak per-tick client/server position gap 1 grid cell
- Upstream: 14 sent, 3 dropped, 5 counted as reordered
- Downstream: 14 sent, 4 dropped, 2 counted as reordered
- Convergence flag: `true`

These are deterministic outcomes from a scripted fault schedule, **not** packet statistics, latency measurements or gameplay evidence from a real online game.

## Limits of validation

The separate **source-hygiene** scanner checks UTF-8 decoding, BOM and selected invisible characters. It cannot establish code authorship or guarantee the absence of every watermark/provenance mechanism.

GitHub CI and Microsoft compiler results must be verified **after** this starter is tested on the user's machine and pushed. The repository has no external runtime dependencies and uses standard C++20 features, but that is not a substitute for compiling it on Windows.
