// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/details/context.h"
#include "spdlog/logger.h"

#ifndef SPDLOG_DISABLE_GLOBAL_LOGGER
    // support for the global stdout color logger
    #ifdef _WIN32
        #include "spdlog/sinks/wincolor_sink.h"
    #else

        #include "spdlog/sinks/ansicolor_sink.h"

    #endif
#endif  // SPDLOG_DISABLE_GLOBAL_LOGGER

#include <memory>

namespace spdlog {
namespace details {

context::context() {
#ifndef SPDLOG_DISABLE_GLOBAL_LOGGER
    // create global logger (ansicolor_stdout_sink_mt or wincolor_stdout_sink_mt in windows).
    #ifdef _WIN32
    auto color_sink = std::make_shared<sinks::wincolor_stdout_sink_mt>();
    #else
    auto color_sink = std::make_shared<sinks::ansicolor_stdout_sink_mt>();
    #endif
    const char *global_logger_name = "";
    global_logger_ = std::make_shared<logger>(global_logger_name, std::move(color_sink));

#endif  // SPDLOG_DISABLE_GLOBAL_LOGGER
}

context::~context() = default;

std::shared_ptr<logger> context::global_logger() {
    return global_logger_;
}

// Return raw ptr to the global logger.
// To be used directly by the spdlog default api (e.g. spdlog::info)
// This make the default API faster, but cannot be used concurrently with set_global_logger().
// e.g do not call set_global_logger() from one thread while calling spdlog::info() from another.
logger *context::global_logger_raw() const noexcept{ return global_logger_.get(); }

// set global logger
void context::set_logger(std::shared_ptr<logger> new_global_logger) {
    global_logger_ = std::move(new_global_logger);
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

}  // namespace details
}  // namespace spdlog
