//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog usage example

#include <chrono>
#include <clocale>
#include <cstdio>

#include "/home/gabi//spdlog2/tests/test_sink.h"
#include "spdlog/sinks/async_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "spdlog/stopwatch.h"

using namespace spdlog::sinks;

void worker_thread(spdlog::logger &logger, int n_messages) {
    for (int i = 0; i < n_messages; ++i) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        logger.info("Worker thread message #{}", i);
    }
}
#include <iostream>
bool async_example2(size_t n_threads, size_t n_messages) {
    auto test_sink = std::make_shared<test_sink_st>();
    auto n_total_messages = n_messages * n_threads;
    {
        std::vector<std::thread> threads;
        auto async_sink = std::make_shared<async_sink_mt>(8192);
        async_sink->add_sink(test_sink);
        //auto logger = std::make_shared<spdlog::logger>("async_logger", async_sink);
        spdlog::logger logger("async_logger", async_sink);
        spdlog::stopwatch sw;
        for (int i = 0; i < n_threads; ++i) {
            threads.emplace_back(worker_thread, std::ref(logger), n_messages);
        }

        // wait for worker threads to finish
        for (auto &t : threads) {
            t.join();
        }


        auto millis = sw.elapsed_ms().count();
        if (millis == 0) {
            millis = 1;
        }
        spdlog::info("+++ {:L}/sec +++  ({} ms)", n_total_messages * 1000 / millis, millis);
    }

    // wait for the worker thread to finish
    // check that all messages were processed
    if (test_sink->msg_counter() != n_messages * n_threads) {
        spdlog::error("Expected: {:L}, Counted: {:L}", n_total_messages,
                      test_sink->msg_counter());
        return false;
    }
    spdlog::info("OK: {:L}", test_sink->msg_counter());
    return true;
}

#include <random>
int main(int, char *[]) {
    std::locale::global(std::locale("en_US.UTF-8"));
    try {
        // random n_messages and n_threads
        std::mt19937 gen(std::random_device{}());
        for (int i = 0; i < 100; i++) {
            // auto n_threads = std::uniform_int_distribution<size_t>(1, 10)(gen);
            // auto n_messages = std::uniform_int_distribution<size_t>(1, 1'000'000)(gen);
            auto n_threads = 4;
            auto n_messages = 1'000'000 / n_threads;

            spdlog::info("***********************************");
            spdlog::info("Test #{}.  {} x {:L} = {:L} msgs", i, n_threads, n_messages, n_threads * n_messages);
            if (!async_example2(n_threads, n_messages)) {
                spdlog::error("Stopped");
                break;
            }
        }

        spdlog::shutdown();
    }

    // Exceptions will only be thrown upon failed logger or sink construction (not during logging).
    catch (const spdlog::spdlog_ex &ex) {
        std::printf("Log initialization failed: %s\n", ex.what());
        return 1;
    }
}
