# NetSync Lab v0.2 — Bounded retransmission validation

**Date:** 2026-11-15
**Scope:** Deterministic, synthetic in-memory packet scheduling (no real sockets or network measurements)

## Reproduce

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/netsync compare
```

On Windows with Visual Studio, use `cmake --build build --config Debug`, `ctest --test-dir build -C Debug --output-on-failure`, and `./build/Debug/netsync.exe compare` in PowerShell.

The exact CSV output is committed as [`retransmission-comparison.csv`](retransmission-comparison.csv), and a Python CLI regression test compares program output byte for byte with it.

## Controls and policies

Both runs use the same 12 scripted movement inputs, initial state, cyclic upstream/downstream fault patterns, and drain limit.

- **Baseline (v0.1 compatibility):** transmit all unacknowledged commands on every tick with pending inputs.
- **Bounded:** transmit at most **3** oldest unacknowledged commands every **2** simulation ticks (ticks 0, 2, 4, ...). This is a deterministic go-back-N-style retry schedule, not an adaptive RTT-based algorithm.

## Measured simulation outputs

| Metric | Baseline | Bounded | Interpretation |
| --- | ---: | ---: | --- |
| Converged | yes | yes | All 12 commands processed and acknowledged |
| Simulated ticks to convergence | 14 | 19 | Bounded took 5 more ticks (+35.7%) |
| Upstream packets transmitted | 14 | 10 | 4 fewer (-28.6%) |
| Command entries transmitted | 33 | 27 | 6 fewer (-18.2%) |
| Largest upstream command batch | 4 | 3 | Bounded to three commands |
| Peak client/server position gap | 1 | 2 | Temporary prediction error increased |
| Upstream packets dropped | 3 | 2 | Different number of sends means different positions in the cyclic fault pattern |

**Definition:** "command entries transmitted" counts command records *placed into upstream packets*, including packets later dropped. It is not unique commands, bytes, bandwidth, wire overhead, or elapsed wall-clock time. "Simulated ticks" are discrete steps without a configured millisecond duration. The peak gap is measured in abstract grid cells, not player-visible latency.

## Verification

- Local Linux **GCC 14** and **Clang 17**: 29/29 C++ tests, 7/7 CLI tests, 6/6 CTest entries, same exact comparison CSV output. Both builds treat compiler warnings as errors.
- C++ tests exercise oldest-first batch selection, policy bounds, repeatability, one-command packets, loss recovery, and baseline-vs-bounded payload comparisons.
- The original `demo`, `ideal`, and `trace` CLI behaviors and output are preserved and covered by existing golden CLI tests.
- **Windows/MSVC and hosted Windows/Ubuntu CI for v0.2 must be checked separately.** Do not claim those environments passed solely from the local GCC/Clang results.

## Interpretation and limits

The experiments show a **specific traffic-versus-convergence tradeoff**, not a universal performance improvement. Because packet drop/delay patterns index **send order**, changing the send rate changes which transmissions are dropped; these are the same deterministic fault *rules*, not a matched set of identical dropped packets. Multiple traces and stronger fairness controls would be needed for generalizable conclusions.

The client batches oldest pending commands, so later inputs wait for earlier acknowledgments. This can increase time to converge and does not guarantee progress under total or adversarial loss. There are no sockets, retransmission timers in milliseconds, selective acknowledgments, congestion control, throughput measurements, or production security protections.
