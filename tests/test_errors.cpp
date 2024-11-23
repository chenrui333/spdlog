/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"
#include "spdlog/async_logger.h"

#include <iostream>

#define SIMPLE_LOG "test_logs/simple_log.txt"
#define SIMPLE_ASYNC_LOG "test_logs/simple_async_log.txt"

class failing_sink : public spdlog::sinks::base_sink<std::mutex> {
protected:
    void sink_it_(const spdlog::details::log_msg &) final {
        throw std::runtime_error("some error happened during log");
    }

    void flush_() final { throw std::runtime_error("some error happened during flush"); }
};
struct custom_ex {};

#if !defined(SPDLOG_USE_STD_FORMAT)  // std format doesn't fully support runtime strings
TEST_CASE("default_error_handler", "[errors]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    auto logger = std::make_unique<spdlog::logger>("test-error", std::move(sink));
    logger->set_pattern("%v");
    logger->info(SPDLOG_FMT_RUNTIME("Test message {} {}"), 1);
    logger->info("Test message {}", 2);
    logger->flush();
    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) == spdlog::fmt_lib::format("Test message 2{}", default_eol));
    REQUIRE(count_lines(SIMPLE_LOG) == 1);
}

TEST_CASE("custom_error_handler", "[errors]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    spdlog::logger logger ("logger", std::move(sink));
    logger.flush_on(spdlog::level::info);
    logger.set_error_handler([=](const std::string &) { throw custom_ex(); });
    logger.info("Good message #1");

    REQUIRE_THROWS_AS(logger.info(SPDLOG_FMT_RUNTIME("Bad format msg {} {}"), "xxx"), custom_ex);
    logger.info("Good message #2");
    require_message_count(SIMPLE_LOG, 2);
}
#endif

TEST_CASE("default_error_handler2", "[errors]") {
    spdlog::logger logger("failed_logger", std::make_shared<failing_sink>());
    logger.set_error_handler([=](const std::string &) { throw custom_ex(); });
    REQUIRE_THROWS_AS(logger.info("Some message"), custom_ex);
}

TEST_CASE("flush_error_handler", "[errors]") {
    spdlog::logger logger("failed_logger", std::make_shared<failing_sink>());
    logger.set_error_handler([=](const std::string &) { throw custom_ex(); });
    REQUIRE_THROWS_AS(logger.flush(), custom_ex);
}

#if !defined(SPDLOG_USE_STD_FORMAT)
TEST_CASE("async_error_handler", "[errors]") {
    prepare_logdir();
    std::string err_msg("log failed with some msg");

    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_ASYNC_LOG);
    {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
        auto tp = std::make_shared<spdlog::details::thread_pool>(128, 1);
        auto logger = std::make_shared<spdlog::async_logger>("logger", std::move(sink), std::move(tp));
        logger->set_error_handler([=](const std::string &) {
            std::ofstream ofs("test_logs/custom_err.txt");
            if (!ofs) {
                throw std::runtime_error("Failed open test_logs/custom_err.txt");
            }
            ofs << err_msg;
        });
        logger->info("Good message #1");
        logger->info(SPDLOG_FMT_RUNTIME("Bad format msg {} {}"), "xxx");
        logger->info("Good message #2");
    }
    require_message_count(SIMPLE_ASYNC_LOG, 2);
    REQUIRE(file_contents("test_logs/custom_err.txt") == err_msg);
}
#endif

// Make sure async error handler is executed
TEST_CASE("async_error_handler2", "[errors]") {
    prepare_logdir();
    std::string err_msg("This is async handler error message");
    {
        spdlog::details::os::create_dir(SPDLOG_FILENAME_T("test_logs"));
        auto tp = std::make_shared<spdlog::details::thread_pool>(128, 1);
        spdlog::async_logger logger("failed_logger", std::make_shared<failing_sink>(), tp);
        logger.set_error_handler([=](const std::string &) {
            std::ofstream ofs("test_logs/custom_err2.txt");
            if (!ofs) throw std::runtime_error("Failed open test_logs/custom_err2.txt");
            ofs << err_msg;
        });
        logger.info("Hello failure");
    }
    REQUIRE(file_contents("test_logs/custom_err2.txt") == err_msg);
}
