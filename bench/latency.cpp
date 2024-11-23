//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// latency.cpp : spdlog latency benchmarks
//

#include "benchmark/benchmark.h"

#include "spdlog/spdlog.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

void bench_c_string(benchmark::State &state, std::shared_ptr<spdlog::logger> logger) {
    const char *msg =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum pharetra metus cursus "
        "lacus placerat congue. Nulla egestas, mauris a tincidunt tempus, enim lectus volutpat mi, "
        "eu consequat sem "
        "libero nec massa. In dapibus ipsum a diam rhoncus gravida. Etiam non dapibus eros. Donec "
        "fringilla dui sed "
        "augue pretium, nec scelerisque est maximus. Nullam convallis, sem nec blandit maximus, "
        "nisi turpis ornare "
        "nisl, sit amet volutpat neque massa eu odio. Maecenas malesuada quam ex, posuere congue "
        "nibh turpis duis.";

    for (auto _ : state) {
        logger->info(msg);
    }
}

void bench_logger(benchmark::State &state, std::shared_ptr<spdlog::logger> logger) {
    int i = 0;
    for (auto _ : state) {
        logger->info("Hello logger: msg number {}...............", ++i);
    }
}
void bench_global_logger(benchmark::State &state, std::shared_ptr<spdlog::logger> logger) {
    spdlog::set_default_logger(std::move(logger));
    int i = 0;
    for (auto _ : state) {
        spdlog::info("Hello logger: msg number {}...............", ++i);
    }
}

void bench_disabled_macro(benchmark::State &state, std::shared_ptr<spdlog::logger> logger) {
    int i = 0;
    benchmark::DoNotOptimize(i);       // prevent unused warnings
    benchmark::DoNotOptimize(logger);  // prevent unused warnings
    for (auto _ : state) {
        SPDLOG_LOGGER_DEBUG(logger, "Hello logger: msg number {}...............", i++);
    }
}

void bench_disabled_macro_global_logger(benchmark::State &state,
                                        std::shared_ptr<spdlog::logger> logger) {
    spdlog::set_default_logger(std::move(logger));
    int i = 0;
    benchmark::DoNotOptimize(i);       // prevent unused warnings
    benchmark::DoNotOptimize(logger);  // prevent unused warnings
    for (auto _ : state) {
        SPDLOG_DEBUG("Hello logger: msg number {}...............", i++);
    }
}

#ifdef __linux__
void bench_dev_null() {
    auto sink_st = std::make_shared<spdlog::sinks::basic_file_sink_st>("/dev/null");
    auto dev_null_st = std::make_shared<spdlog::logger>("/dev/null_st", std::move(sink_st));
    RegisterBenchmark("/dev/null_st", bench_logger, std::move(dev_null_st))
        ->UseRealTime();

    auto sink_mt = std::make_shared<spdlog::sinks::basic_file_sink_st>("/dev/null");
    auto dev_null_mt = std::make_shared<spdlog::logger>("/dev/null_mt", std::move(sink_mt));
    RegisterBenchmark("/dev/null_mt", bench_logger, std::move(dev_null_mt))
        ->UseRealTime();
}
#endif  // __linux__

int main(int argc, char *argv[]) {
    using spdlog::sinks::null_sink_mt;
    using spdlog::sinks::null_sink_st;

    size_t file_size = 30 * 1024 * 1024;
    size_t rotating_files = 5;
    int n_threads = benchmark::CPUInfo::Get().num_cpus;

    auto full_bench = argc > 1 && std::string(argv[1]) == "full";

    // disabled loggers
    auto disabled_logger =
        std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_mt>());
    disabled_logger->set_level(spdlog::level::off);
    RegisterBenchmark("disabled-at-compile-time", bench_disabled_macro, disabled_logger);
    RegisterBenchmark("disabled-at-compile-time (global logger)",
                                 bench_disabled_macro_global_logger, disabled_logger);
    RegisterBenchmark("disabled-at-runtime", bench_logger, disabled_logger);
    RegisterBenchmark("disabled-at-runtime (global logger)", bench_global_logger,
                                 disabled_logger);
    // with backtrace of 64
    auto tracing_disabled_logger =
        std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_mt>());
    tracing_disabled_logger->enable_backtrace(64);
    RegisterBenchmark("disabled-at-runtime/backtrace", bench_logger,
                                 tracing_disabled_logger);

    auto null_logger_st =
        std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_st>());
    RegisterBenchmark("null_sink_st (500_bytes c_str)", bench_c_string,
                                 std::move(null_logger_st));
    RegisterBenchmark("null_sink_st", bench_logger, null_logger_st);
    RegisterBenchmark("null_sink_st (global logger)", bench_global_logger,
                                 null_logger_st);
    // with backtrace of 64
    auto tracing_null_logger_st =
        std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_st>());
    tracing_null_logger_st->enable_backtrace(64);
    RegisterBenchmark("null_sink_st/backtrace", bench_logger, tracing_null_logger_st);

#ifdef __linux__
    bench_dev_null();
#endif  // __linux__

    if (full_bench) {
        // basic_st
        auto sink =
            std::make_shared<spdlog::sinks::basic_file_sink_st>("latency_logs/basic_st.log", true);
        auto basic_st = std::make_shared<spdlog::logger>("basic_st", std::move(sink));
        RegisterBenchmark("basic_st", bench_logger, std::move(basic_st))->UseRealTime();
        // with backtrace of 64

        auto sink2 = std::make_shared<spdlog::sinks::basic_file_sink_st>(
            "latency_logs/tracing_basic_st.log", true);
        auto tracing_basic_st =
            std::make_shared<spdlog::logger>("tracing_basic_st", std::move(sink2));
        tracing_basic_st->enable_backtrace(64);
        RegisterBenchmark("basic_st/backtrace", bench_logger,
                                     std::move(tracing_basic_st))
            ->UseRealTime();

        // rotating st
        auto sink3 = std::make_shared<spdlog::sinks::rotating_file_sink_st>(
            "latency_logs/rotating_st.log", file_size, rotating_files);
        auto rotating_st = std::make_shared<spdlog::logger>("rotate_st", std::move(sink3));
        RegisterBenchmark("rotating_st", bench_logger, std::move(rotating_st))
            ->UseRealTime();
        // with backtrace of 64
        auto sink4 = std::make_shared<spdlog::sinks::rotating_file_sink_st>(
            "latency_logs/tracing_rotating_st.log", file_size, rotating_files);

        auto tracing_rotating_st =
            std::make_shared<spdlog::logger>("tracing_rotating_st", std::move(sink4));
        RegisterBenchmark("rotating_st/backtrace", bench_logger, std::move(tracing_rotating_st))->UseRealTime();

        // daily st
        auto sink5 =
            std::make_shared<spdlog::sinks::daily_file_sink_st>("latency_logs/daily_st.log", 1, 0);
        auto daily_st = std::make_shared<spdlog::logger>("daily_st", std::move(sink5));
        RegisterBenchmark("daily_st", bench_logger, std::move(daily_st))->UseRealTime();

        auto sink6 = std::make_shared<spdlog::sinks::daily_file_sink_st>("latency_logs/tracing_daily_st.log", 1, 0);
        auto tracing_daily_st =
            std::make_shared<spdlog::logger>("logger", std::move(sink6));
        RegisterBenchmark("daily_st/backtrace", bench_logger,
                                     std::move(tracing_daily_st))
            ->UseRealTime();

        //
        // Multi threaded bench, 10 loggers using same logger concurrently
        //
        auto null_logger_mt =
            std::make_shared<spdlog::logger>("logger", std::make_shared<null_sink_mt>());
        RegisterBenchmark("null_sink_mt", bench_logger, null_logger_mt)
            ->Threads(n_threads)
            ->UseRealTime();

        // basic_mt
        auto sink7 =
            std::make_shared<spdlog::sinks::basic_file_sink_mt>("latency_logs/basic_mt.log", true);
        auto basic_mt = std::make_shared<spdlog::logger>("logger", std::move(sink7));
        RegisterBenchmark("basic_mt", bench_logger, std::move(basic_mt))
            ->Threads(n_threads)
            ->UseRealTime();

        // rotating mt
        auto sink8 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "latency_logs/rotating_mt.log", file_size, rotating_files);

        auto rotating_mt = std::make_shared<spdlog::logger>("logger", std::move(sink8));

        RegisterBenchmark("rotating_mt", bench_logger, std::move(rotating_mt))
            ->Threads(n_threads)
            ->UseRealTime();

        // daily mt
        auto sink9 =
            std::make_shared<spdlog::sinks::daily_file_sink_mt>("latency_logs/daily_mt.log",1,0);
        auto daily_mt = std::make_shared<spdlog::logger>("logger", std::move(sink9));
        RegisterBenchmark("daily_mt", bench_logger, std::move(daily_mt))
            ->Threads(n_threads)
            ->UseRealTime();
    }

    // async
    auto queue_size = 1024 * 1024 * 3;
    auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, 1);
    auto async_logger = std::make_shared<spdlog::async_logger>(
        "async_logger", std::make_shared<null_sink_mt>(), std::move(tp),
        spdlog::async_overflow_policy::overrun_oldest);
    RegisterBenchmark("async_logger", bench_logger, async_logger)
        ->Threads(n_threads)
        ->UseRealTime();

    auto async_logger_tracing = std::make_shared<spdlog::async_logger>(
        "async_logger_tracing", std::make_shared<null_sink_mt>(), std::move(tp),
        spdlog::async_overflow_policy::overrun_oldest);
    async_logger_tracing->enable_backtrace(32);
    RegisterBenchmark("async_logger/tracing", bench_logger, async_logger_tracing)
        ->Threads(n_threads)
        ->UseRealTime();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
