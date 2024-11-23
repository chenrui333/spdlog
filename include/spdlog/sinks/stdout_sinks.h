// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <cstdio>
#include <spdlog/sinks/base_sink.h>

#ifdef _WIN32
    #include <spdlog/details/windows_include.h>
#endif

namespace spdlog {

namespace sinks {

template <typename Mutex>
class stdout_sink_base : public base_sink<Mutex> {
public:
    explicit stdout_sink_base(FILE *file);
    ~stdout_sink_base() override = default;

    stdout_sink_base(const stdout_sink_base &other) = delete;
    stdout_sink_base(stdout_sink_base &&other) = delete;

    stdout_sink_base &operator=(const stdout_sink_base &other) = delete;
    stdout_sink_base &operator=(stdout_sink_base &&other) = delete;
protected:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;
    FILE *file_;
#ifdef _WIN32
    HANDLE handle_;
#endif  // WIN32
};

template <typename Mutex>
class stdout_sink : public stdout_sink_base<Mutex> {
public:
    stdout_sink();
};

template <typename Mutex>
class stderr_sink : public stdout_sink_base<Mutex> {
public:
    stderr_sink();
};

using stdout_sink_mt = stdout_sink<std::mutex>;
using stdout_sink_st = stdout_sink<details::null_mutex>;

using stderr_sink_mt = stderr_sink<std::mutex>;
using stderr_sink_st = stderr_sink<details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
    #include "stdout_sinks-inl.h"
#endif
