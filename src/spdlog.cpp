// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/spdlog.h"

#include <memory>

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace spdlog {



void set_formatter(std::unique_ptr<formatter> formatter) {
    default_logger_raw()->set_formatter(std::move(formatter));
}

void set_pattern(std::string pattern, pattern_time_type time_type) {
    set_formatter(std::make_unique<pattern_formatter>(std::move(pattern), time_type));
}

level get_level() { return default_logger_raw()->log_level(); }

bool should_log(level level) { return default_logger_raw()->should_log(level); }

void set_level(level level) { default_logger_raw()->set_level(level); }

void flush_on(level level) { default_logger_raw()->flush_on(level); }

void set_error_handler(void (*handler)(const std::string &msg)) { default_logger_raw()->set_error_handler(handler); }


void shutdown() { details::context::instance().shutdown(); }

std::shared_ptr<logger> default_logger() { return details::context::instance().default_logger(); }

logger *default_logger_raw() { return details::context::instance().get_default_raw(); }

void set_default_logger(std::shared_ptr<logger> default_logger) {
    details::context::instance().set_default_logger(std::move(default_logger));
}

}  // namespace spdlog
