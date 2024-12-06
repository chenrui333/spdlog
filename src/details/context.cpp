// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/common.h"
#include "spdlog/details/context.h"
#include "spdlog/logger.h"
#include "spdlog/pattern_formatter.h"

#ifndef SPDLOG_DISABLE_DEFAULT_LOGGER
    // support for the default stdout color logger
    #ifdef _WIN32
        #include "spdlog/sinks/wincolor_sink.h"
    #else

        #include "spdlog/sinks/ansicolor_sink.h"

    #endif
#endif  // SPDLOG_DISABLE_DEFAULT_LOGGER

#include <memory>
#include <string>
#include <unordered_map>

static constexpr size_t small_map_threshold = 10;

namespace spdlog {
namespace details {

context::context() {
#ifndef SPDLOG_DISABLE_DEFAULT_LOGGER
    // create default logger (ansicolor_stdout_sink_mt or wincolor_stdout_sink_mt in windows).
    #ifdef _WIN32
    auto color_sink = std::make_shared<sinks::wincolor_stdout_sink_mt>();
    #else
    auto color_sink = std::make_shared<sinks::ansicolor_stdout_sink_mt>();
    #endif
    const char *default_logger_name = "";
    default_logger_ = std::make_shared<logger>(default_logger_name, std::move(color_sink));

#endif  // SPDLOG_DISABLE_DEFAULT_LOGGER
}

context::~context() = default;

std::shared_ptr<logger> context::default_logger() {
    return default_logger_;
}

// Return raw ptr to the default logger.
// To be used directly by the spdlog default api (e.g. spdlog::info)
// This make the default API faster, but cannot be used concurrently with set_default_logger().
// e.g do not call set_default_logger() from one thread while calling spdlog::info() from another.
logger *context::get_default_raw() const { return default_logger_.get(); }

// set default logger.
// default logger is stored in default_logger_ (for faster retrieval) and in the loggers_ map.
void context::set_default_logger(std::shared_ptr<logger> new_default_logger) {
    default_logger_ = std::move(new_default_logger);
}

void context::set_tp(std::shared_ptr<thread_pool> tp) {
    std::lock_guard lock(tp_mutex_);
    tp_ = std::move(tp);
}

std::shared_ptr<thread_pool> context::get_tp() {
    std::lock_guard lock(tp_mutex_);
    return tp_;
}

// clean all resources and threads started by the registry
void context::shutdown() {
    std::lock_guard lock(tp_mutex_);
    tp_.reset();
}

std::recursive_mutex &context::tp_mutex() { return tp_mutex_; }

context &context::instance() {
    static context s_instance;
    return s_instance;
}

}  // namespace details
}  // namespace spdlog
