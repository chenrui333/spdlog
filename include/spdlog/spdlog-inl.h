// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
    #include <spdlog/spdlog.h>
#endif

#include <spdlog/common.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace spdlog {

#ifndef SPDLOG_DISABLE_DEFAULT_LOGGER
using default_sink_t = sinks::stdout_color_sink_mt;
#else
using default_sink_t = sinks::null_sink_mt;
#endif

static auto s_logger = std::make_shared<logger>("", std::make_shared<default_sink_t>());

SPDLOG_INLINE void set_formatter(std::unique_ptr<spdlog::formatter> formatter) {
    s_logger->set_formatter(std::move(formatter));
}

SPDLOG_INLINE void set_pattern(std::string pattern, pattern_time_type time_type) {
    set_formatter(
        std::unique_ptr<spdlog::formatter>(new pattern_formatter(std::move(pattern), time_type)));
}

SPDLOG_INLINE void enable_backtrace(size_t n_messages) {
    s_logger->enable_backtrace(n_messages);
}

SPDLOG_INLINE void disable_backtrace() { s_logger->disable_backtrace(); }

SPDLOG_INLINE void dump_backtrace() {  s_logger->dump_backtrace(); }

SPDLOG_INLINE level::level_enum get_level() { return  s_logger->level(); }

SPDLOG_INLINE bool should_log(level::level_enum log_level) {
    return default_logger()->should_log(log_level);
}

SPDLOG_INLINE void set_level(level::level_enum log_level) {
    s_logger->set_level(log_level);
}

SPDLOG_INLINE void flush_on(level::level_enum log_level) {
    s_logger->flush_on(log_level);
}

SPDLOG_INLINE void set_error_handler(void (*handler)(const std::string &msg)) {
    s_logger->set_error_handler(handler);
}


SPDLOG_INLINE void shutdown() { s_logger.reset(); }

SPDLOG_INLINE std::shared_ptr<spdlog::logger> default_logger() {
    return s_logger;
}


SPDLOG_INLINE void set_default_logger(std::shared_ptr<spdlog::logger> default_logger) {
    s_logger = std::move(default_logger);
}

}  // namespace spdlog
