#pragma once

#include <chrono>
#include <limits>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace optimizer {

/**
 * Per-function runtime statistics collected by the Profiler.
 */
struct FunctionStats {
    std::string name;
    uint64_t call_count;
    double total_time_ms;
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
};

/**
 * Singleton runtime profiler.
 *
 * Records call counts and wall-clock timings for named code sections.
 * The collected statistics can be written to a CSV file that can be
 * viewed alongside Valgrind/KCachegrind profiling data.
 *
 * Example usage:
 *   optimizer::Profiler::instance().begin("my_function");
 *   // ... work ...
 *   optimizer::Profiler::instance().end("my_function");
 *   optimizer::Profiler::instance().write_stats("stats.csv");
 */
class Profiler {
public:
    /** Returns the process-wide singleton instance. */
    static Profiler& instance();

    /**
     * Start timing a named section.
     * Nesting the same name is not supported; call end() before begin() again.
     */
    void begin(const std::string& name);

    /**
     * Stop timing a named section and record the elapsed time.
     * Has no effect if begin() was not called first for that name.
     */
    void end(const std::string& name);

    /**
     * Write accumulated statistics to a CSV file.
     *
     * The file contains one header row followed by one data row per tracked
     * section:  name, call_count, total_ms, min_ms, max_ms, avg_ms
     *
     * @param filename  Path to the output file (will be overwritten).
     * @return          true on success, false if the file could not be opened.
     */
    bool write_stats(const std::string& filename) const;

    /** Clear all accumulated statistics. */
    void reset();

    /** Return a snapshot of the current statistics. */
    std::vector<FunctionStats> get_stats() const;

private:
    Profiler() = default;

    struct TimingEntry {
        std::chrono::high_resolution_clock::time_point start;
        bool active = false;
        uint64_t call_count = 0;
        double total_time_ms = 0.0;
        double min_time_ms = std::numeric_limits<double>::max();
        double max_time_ms = 0.0;
    };

    std::unordered_map<std::string, TimingEntry> timings_;
    mutable std::mutex mutex_;
};

/**
 * RAII helper that calls Profiler::begin() on construction and
 * Profiler::end() on destruction, making it easy to profile an entire
 * scope without manual begin/end pairs.
 *
 * Example:
 *   void my_function() {
 *       OPTIMIZER_PROFILE("my_function");
 *       // ... body ...
 *   }  // timing stops here automatically
 */
class ScopedProfiler {
public:
    explicit ScopedProfiler(const std::string& name);
    ~ScopedProfiler();

    // Non-copyable, non-movable
    ScopedProfiler(const ScopedProfiler&) = delete;
    ScopedProfiler& operator=(const ScopedProfiler&) = delete;

private:
    std::string name_;
};

} // namespace optimizer

/**
 * Profile the current scope using the given label.
 * Uses __COUNTER__ to guarantee a unique variable name even when the macro
 * appears multiple times within a single translation unit or on the same line.
 */
#define OPTIMIZER_PROFILE(name) \
    ::optimizer::ScopedProfiler _optimizer_scoped_##__COUNTER__(name)

/**
 * Write accumulated statistics to @p filename.
 * Returns true on success, false on I/O error.
 */
#define OPTIMIZER_WRITE_STATS(filename) \
    ::optimizer::Profiler::instance().write_stats(filename)

/**
 * Reset all accumulated statistics.
 */
#define OPTIMIZER_RESET() \
    ::optimizer::Profiler::instance().reset()
