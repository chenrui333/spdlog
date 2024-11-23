// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
    #error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <mutex>

#include <spdlog/async.h>
#include <spdlog/details/null_mutex.h>
//
// color sinks
//
#ifdef _WIN32
    #include <spdlog/sinks/wincolor_sink-inl.h>
    template class SPDLOG_API spdlog::sinks::wincolor_sink<std::mutex>;
    template class SPDLOG_API spdlog::sinks::wincolor_sink<spdlog::details::null_mutex>;
    template class SPDLOG_API spdlog::sinks::wincolor_stdout_sink<std::mutex>;
    template class SPDLOG_API spdlog::sinks::wincolor_stdout_sink<spdlog::details::null_mutex>;
    template class SPDLOG_API spdlog::sinks::wincolor_stderr_sink<std::mutex>;
    template class SPDLOG_API spdlog::sinks::wincolor_stderr_sink<spdlog::details::null_mutex>;
#else
    #include "spdlog/sinks/ansicolor_sink-inl.h"
    template class SPDLOG_API spdlog::sinks::ansicolor_sink<std::mutex>;
    template class SPDLOG_API spdlog::sinks::ansicolor_sink<spdlog::details::null_mutex>;
    template class SPDLOG_API spdlog::sinks::ansicolor_stdout_sink<std::mutex>;
    template class SPDLOG_API spdlog::sinks::ansicolor_stdout_sink<spdlog::details::null_mutex>;
    template class SPDLOG_API spdlog::sinks::ansicolor_stderr_sink<std::mutex>;
    template class SPDLOG_API spdlog::sinks::ansicolor_stderr_sink<spdlog::details::null_mutex>;
#endif
