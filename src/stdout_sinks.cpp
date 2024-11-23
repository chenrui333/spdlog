// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
    #error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <mutex>

#include <spdlog/async.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/stdout_sinks-inl.h>

template class SPDLOG_API spdlog::sinks::stdout_sink_base<std::mutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink_base<spdlog::details::null_mutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink<spdlog::details::null_mutex>;
template class SPDLOG_API spdlog::sinks::stderr_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::stderr_sink<spdlog::details::null_mutex>;
