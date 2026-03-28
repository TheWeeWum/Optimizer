#include "optimizer/optimizer.h"

#include <fstream>
#include <mutex>

namespace optimizer {

// ---------------------------------------------------------------------------
// Profiler
// ---------------------------------------------------------------------------

Profiler& Profiler::instance() {
    static Profiler profiler;
    return profiler;
}

void Profiler::begin(const std::string& name) {
    // Capture the timestamp before acquiring the lock to minimise overhead.
    auto now = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);
    TimingEntry& entry = timings_[name];
    entry.start = now;
    entry.active = true;
}

void Profiler::end(const std::string& name) {
    // Capture the timestamp before acquiring the lock to minimise overhead.
    auto now = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = timings_.find(name);
    if (it == timings_.end() || !it->second.active) {
        return;
    }

    TimingEntry& entry = it->second;
    entry.active = false;

    double elapsed_ms =
        std::chrono::duration<double, std::milli>(now - entry.start).count();

    entry.call_count++;
    entry.total_time_ms += elapsed_ms;

    if (elapsed_ms < entry.min_time_ms) {
        entry.min_time_ms = elapsed_ms;
    }
    if (elapsed_ms > entry.max_time_ms) {
        entry.max_time_ms = elapsed_ms;
    }
}

bool Profiler::write_stats(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ofstream out(filename);
    if (!out.is_open()) {
        return false;
    }

    out << "name,call_count,total_ms,min_ms,max_ms,avg_ms\n";

    for (const auto& kv : timings_) {
        const TimingEntry& entry = kv.second;
        if (entry.call_count == 0) {
            continue;
        }

        double avg_ms = entry.total_time_ms / static_cast<double>(entry.call_count);
        double min_ms = (entry.min_time_ms == std::numeric_limits<double>::max())
                            ? 0.0
                            : entry.min_time_ms;

        out << kv.first << ","
            << entry.call_count << ","
            << entry.total_time_ms << ","
            << min_ms << ","
            << entry.max_time_ms << ","
            << avg_ms << "\n";
    }

    return out.good();
}

void Profiler::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    timings_.clear();
}

std::vector<FunctionStats> Profiler::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<FunctionStats> stats;
    stats.reserve(timings_.size());

    for (const auto& kv : timings_) {
        const TimingEntry& entry = kv.second;
        if (entry.call_count == 0) {
            continue;
        }

        double avg_ms = entry.total_time_ms / static_cast<double>(entry.call_count);
        double min_ms = (entry.min_time_ms == std::numeric_limits<double>::max())
                            ? 0.0
                            : entry.min_time_ms;

        stats.push_back({kv.first,
                         entry.call_count,
                         entry.total_time_ms,
                         min_ms,
                         entry.max_time_ms,
                         avg_ms});
    }

    return stats;
}

// ---------------------------------------------------------------------------
// ScopedProfiler
// ---------------------------------------------------------------------------

ScopedProfiler::ScopedProfiler(const std::string& name) : name_(name) {
    Profiler::instance().begin(name_);
}

ScopedProfiler::~ScopedProfiler() {
    Profiler::instance().end(name_);
}

} // namespace optimizer
