// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// Loggers registry of unique name->logger pointer
// An attempt to create a logger with an already existing name will result with spdlog_ex exception.
// If user requests a non-existing logger, nullptr will be returned
// This class is thread safe

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../common.h"
#include "./periodic_worker.h"

namespace spdlog {
class logger;

namespace details {
class thread_pool;

class SPDLOG_API context {
public:
    using log_levels = std::unordered_map<std::string, level>;

    static context &instance();
    context(const context &) = delete;
    context &operator=(const context &) = delete;

    std::shared_ptr<logger> default_logger();

    // Return raw ptr to the default logger.
    // To be used directly by the spdlog default api (e.g. spdlog::info)
    // This make the default API faster, but cannot be used concurrently with set_default_logger().
    // e.g do not call set_default_logger() from one thread while calling spdlog::info() from
    // another.
    logger *get_default_raw() const;

    // set default logger.
    // default logger is stored in default_logger_ (for faster retrieval) and in the loggers_ map.
    void set_default_logger(std::shared_ptr<logger> new_default_logger);

    void set_tp(std::shared_ptr<thread_pool> tp);

    std::shared_ptr<thread_pool> get_tp();

    // clean all resources
    void shutdown();

    std::recursive_mutex &tp_mutex();

private:
    context();
    ~context();

    std::recursive_mutex tp_mutex_;
    std::shared_ptr<thread_pool> tp_;
    std::shared_ptr<logger> default_logger_;
};

}  // namespace details
}  // namespace spdlog
