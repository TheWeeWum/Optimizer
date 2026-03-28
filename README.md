# Optimizer

A lightweight, open-source C++14 runtime profiling library.  
Record wall-clock timings for named code sections and export the results to a
CSV file that can be used alongside [Valgrind](https://valgrind.org/) /
[KCachegrind](https://kcachegrind.github.io/) profiles to correlate
statistical measurements across code changes.

---

## Features

* **Header-only public API** – include a single header via `#include "optimizer/optimizer.h"`.
* **`#pragma once` include guard** – safe for use in any modern C/C++ project.
* **RAII scoped profiling** – `ScopedProfiler` / `OPTIMIZER_PROFILE(name)` macro automatically stop timing when a scope exits.
* **Per-section statistics** – call count, total, min, max, and average elapsed time (milliseconds).
* **CSV output** – `write_stats("file.csv")` / `OPTIMIZER_WRITE_STATS("file.csv")` writes a human-readable file that can be imported into spreadsheets or compared with KCachegrind data.
* **CMake** build system with install rules.

---

## Building

### Requirements

* CMake ≥ 3.14
* A C++14-capable compiler (GCC, Clang, MSVC)

### Steps

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

To run the test suite:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Usage

### Include the header

```cpp
#include "optimizer/optimizer.h"
```

### Manual begin / end

```cpp
optimizer::Profiler::instance().begin("my_function");
// ... work ...
optimizer::Profiler::instance().end("my_function");
```

### RAII scope helper

```cpp
void my_function() {
    OPTIMIZER_PROFILE("my_function");  // timing stops at scope exit
    // ... work ...
}
```

### Write statistics to a file

```cpp
// Returns true on success.
OPTIMIZER_WRITE_STATS("runtime_stats.csv");
```

The output CSV has the following columns:

| Column | Description |
|---|---|
| `name` | Section label passed to `begin()` / `OPTIMIZER_PROFILE()` |
| `call_count` | Number of times the section was timed |
| `total_ms` | Total elapsed time (ms) |
| `min_ms` | Minimum single-call time (ms) |
| `max_ms` | Maximum single-call time (ms) |
| `avg_ms` | Average elapsed time (ms) |

### Reset statistics

```cpp
OPTIMIZER_RESET();
// or
optimizer::Profiler::instance().reset();
```

---

## Using with Valgrind / KCachegrind

Run your program under `callgrind` to collect a call-graph profile, then use
the CSV written by this library to augment the per-function data in KCachegrind:

```bash
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./my_program
kcachegrind callgrind.out
```

The `runtime_stats.csv` file produced by `OPTIMIZER_WRITE_STATS` can be opened
in any spreadsheet application and compared against the KCachegrind profile to
track changes in hotspot timing across code iterations.

---

## License

See [LICENSE](LICENSE).