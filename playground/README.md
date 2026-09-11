# Sentinel — Playground Stage

Small, standalone concepts built and manually verified one at a time before
any real project structure exists. Each file here proves exactly one thing.

## Concepts proven so far

### 1. `concurrent_demo.cpp` — two threads genuinely run at the same time

Two 2-second "tasks" run on separate `std::thread`s. Verified manually:
total wall-clock time comes out to ~2000ms, not ~4000ms, proving the two
tasks overlapped rather than running one after another.

**Bug found and fixed along the way**: both threads writing to `std::cout`
without synchronization produced torn/garbled output (e.g.
`"Task ATask B finished\n finished"`), because `std::cout << a << b` is
multiple operations, not one atomic one. Fixed with a `std::mutex` around
just the print statements — the `sleep_for()` calls stay unlocked, so the
concurrency itself isn't lost, only the shared print stream is protected.

### 2. `shared_counter_demo.cpp` — a shared number needs the same protection

Two threads each increment a shared `int` 1,000,000 times (expected total:
2,000,000). Without a lock, `shared_counter = shared_counter + 1` is a
read-modify-write with three separate steps, and interleaving between
threads silently loses updates — reproduced losing exactly 1,000,000
updates on some runs. Fixed the same way: a `std::mutex` around the
read-modify-write.

### 3. `producer_consumer_demo.cpp` — waiting for another thread, correctly

A consumer needs data a producer thread creates after a 1-second delay.

**Naive version (broken)**: consumer checks a plain `bool ready` flag
exactly once, immediately, with no actual waiting. Reproduced reliably:
`"Consumer: data not ready yet!"` printed before the producer had even
started its delay — the consumer never gave the producer a chance.

**Fix**: `std::condition_variable` + `std::mutex`. The consumer calls
`cv.wait(lock, predicate)`, which genuinely sleeps (no CPU spinning) until
`notify_one()` is called *and* the predicate (`ready == true`) holds — the
predicate form matters even with a real notify, since it guards against
spurious wakeups and against a notify arriving before `wait()` was called.
Verified across 5 runs: consumer always got `data = 42`, never "not ready".

This is the same wait/notify mechanism Sentinel's stampede protection will
reuse later: concurrent callers for the same in-flight key will wait on a
condition variable instead of duplicating work.

## How to build and run (Windows / VS Code Code Runner, or any g++)

```
g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -g -O0 concurrent_demo.cpp -o concurrent_demo
./concurrent_demo

g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -g -O0 shared_counter_demo.cpp -o shared_counter_demo
./shared_counter_demo

g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -g -O0 producer_consumer_demo.cpp -o producer_consumer_demo
./producer_consumer_demo
```

## Status

Both concepts manually verified, multiple runs, on two separate machines.
No automated scripts yet by design — these are still small, single-concept
demos. Automated testing (pytest) starts once work reaches feature level.
