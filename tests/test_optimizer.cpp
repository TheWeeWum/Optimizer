#include "optimizer/optimizer.h"

#include <gtest/gtest.h>

#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------

static void reset() {
    optimizer::Profiler::instance().reset();
}

// ---------------------------------------------------------------------------
// Profiler – basic recording
// ---------------------------------------------------------------------------

TEST(ProfilerTest, SingleCallIsRecorded) {
    reset();

    optimizer::Profiler::instance().begin("work");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    optimizer::Profiler::instance().end("work");

    auto stats = optimizer::Profiler::instance().get_stats();
    ASSERT_EQ(stats.size(), 1u);
    EXPECT_EQ(stats[0].name, "work");
    EXPECT_EQ(stats[0].call_count, 1u);
    EXPECT_GE(stats[0].total_time_ms, 0.0);
    EXPECT_GE(stats[0].min_time_ms, 0.0);
    EXPECT_GE(stats[0].max_time_ms, 0.0);
    EXPECT_GE(stats[0].avg_time_ms, 0.0);
}

TEST(ProfilerTest, MultipleCallsAccumulate) {
    reset();

    constexpr int iterations = 3;
    for (int i = 0; i < iterations; ++i) {
        optimizer::Profiler::instance().begin("loop");
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        optimizer::Profiler::instance().end("loop");
    }

    auto stats = optimizer::Profiler::instance().get_stats();
    ASSERT_EQ(stats.size(), 1u);
    EXPECT_EQ(stats[0].call_count, static_cast<uint64_t>(iterations));
    EXPECT_GE(stats[0].total_time_ms, stats[0].min_time_ms);
    EXPECT_LE(stats[0].min_time_ms, stats[0].max_time_ms);
    EXPECT_GE(stats[0].avg_time_ms, stats[0].min_time_ms);
    EXPECT_LE(stats[0].avg_time_ms, stats[0].max_time_ms);
}

TEST(ProfilerTest, MultipleSectionsTrackedSeparately) {
    reset();

    optimizer::Profiler::instance().begin("sectionA");
    optimizer::Profiler::instance().end("sectionA");

    optimizer::Profiler::instance().begin("sectionB");
    optimizer::Profiler::instance().end("sectionB");

    auto stats = optimizer::Profiler::instance().get_stats();
    EXPECT_EQ(stats.size(), 2u);

    bool found_a = false;
    bool found_b = false;
    for (const auto& s : stats) {
        if (s.name == "sectionA") found_a = true;
        if (s.name == "sectionB") found_b = true;
    }
    EXPECT_TRUE(found_a);
    EXPECT_TRUE(found_b);
}

TEST(ProfilerTest, EndWithoutBeginIsIgnored) {
    reset();

    optimizer::Profiler::instance().end("never_started");

    auto stats = optimizer::Profiler::instance().get_stats();
    EXPECT_TRUE(stats.empty());
}

// ---------------------------------------------------------------------------
// Profiler – reset
// ---------------------------------------------------------------------------

TEST(ProfilerTest, ResetClearsAllStats) {
    reset();

    optimizer::Profiler::instance().begin("x");
    optimizer::Profiler::instance().end("x");

    optimizer::Profiler::instance().reset();

    auto stats = optimizer::Profiler::instance().get_stats();
    EXPECT_TRUE(stats.empty());
}

// ---------------------------------------------------------------------------
// Profiler – write_stats
// ---------------------------------------------------------------------------

TEST(ProfilerTest, WriteStatsCreatesValidCsvFile) {
    reset();

    optimizer::Profiler::instance().begin("alpha");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    optimizer::Profiler::instance().end("alpha");

    const std::string path = "/tmp/optimizer_test_stats.csv";
    ASSERT_TRUE(optimizer::Profiler::instance().write_stats(path));

    std::ifstream in(path);
    ASSERT_TRUE(in.is_open());

    // First line must be the CSV header.
    std::string header;
    std::getline(in, header);
    EXPECT_EQ(header, "name,call_count,total_ms,min_ms,max_ms,avg_ms");

    // There must be at least one data row.
    std::string data_row;
    ASSERT_TRUE(static_cast<bool>(std::getline(in, data_row)));
    EXPECT_FALSE(data_row.empty());
}

TEST(ProfilerTest, WriteStatsReturnsFalseForInvalidPath) {
    reset();

    optimizer::Profiler::instance().begin("b");
    optimizer::Profiler::instance().end("b");

    EXPECT_FALSE(optimizer::Profiler::instance().write_stats(
        "/nonexistent_dir/stats.csv"));
}

// ---------------------------------------------------------------------------
// ScopedProfiler
// ---------------------------------------------------------------------------

TEST(ScopedProfilerTest, RecordsTimingOnScopeExit) {
    reset();

    {
        optimizer::ScopedProfiler sp("scoped_section");
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    auto stats = optimizer::Profiler::instance().get_stats();
    ASSERT_EQ(stats.size(), 1u);
    EXPECT_EQ(stats[0].name, "scoped_section");
    EXPECT_EQ(stats[0].call_count, 1u);
}

// ---------------------------------------------------------------------------
// Macros
// ---------------------------------------------------------------------------

TEST(MacroTest, OptimizerProfileMacro) {
    reset();

    {
        OPTIMIZER_PROFILE("macro_section");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    auto stats = optimizer::Profiler::instance().get_stats();
    ASSERT_EQ(stats.size(), 1u);
    EXPECT_EQ(stats[0].name, "macro_section");
}

TEST(MacroTest, OptimizerWriteStatsMacro) {
    reset();

    optimizer::Profiler::instance().begin("m");
    optimizer::Profiler::instance().end("m");

    EXPECT_TRUE(OPTIMIZER_WRITE_STATS("/tmp/optimizer_macro_stats.csv"));
}

TEST(MacroTest, OptimizerResetMacro) {
    reset();

    optimizer::Profiler::instance().begin("r");
    optimizer::Profiler::instance().end("r");

    OPTIMIZER_RESET();

    auto stats = optimizer::Profiler::instance().get_stats();
    EXPECT_TRUE(stats.empty());
}
