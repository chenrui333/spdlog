// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/spdlog.h"

#include <memory>

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace spdlog {

void initialize_logger(std::shared_ptr<logger> logger) { details::context::instance().initialize_logger(std::move(logger)); }

void set_formatter(std::unique_ptr<spdlog::formatter> formatter) {
    details::context::instance().set_formatter(std::move(formatter));
}

void set_pattern(std::string pattern, pattern_time_type time_type) {
    set_formatter(std::make_unique<spdlog::pattern_formatter>(std::move(pattern), time_type));
}

level get_level() { return default_logger_raw()->log_level(); }

bool should_log(level level) { return default_logger_raw()->should_log(level); }

void set_level(level level) { details::context::instance().set_level(level); }

void flush_on(level level) { details::context::instance().flush_on(level); }

void set_error_handler(void (*handler)(const std::string &msg)) { details::context::instance().set_error_handler(handler); }

void apply_all(const std::function<void(std::shared_ptr<logger>)> &fun) { details::context::instance().apply_all(fun); }

void drop(const std::string &name) { details::context::instance().drop(name); }

void drop_all() { details::context::instance().drop_all(); }

void shutdown() { details::context::instance().shutdown(); }

void set_automatic_registration(bool automatic_registration) {
    details::context::instance().set_automatic_registration(automatic_registration);
}

std::shared_ptr<spdlog::logger> default_logger() { return details::context::instance().default_logger(); }

spdlog::logger *default_logger_raw() { return details::context::instance().get_default_raw(); }

void set_default_logger(std::shared_ptr<spdlog::logger> default_logger) {
    details::context::instance().set_default_logger(std::move(default_logger));
}

void apply_logger_env_levels(std::shared_ptr<logger> logger) {
    details::context::instance().apply_logger_env_levels(std::move(logger));
}
}  // namespace spdlog
