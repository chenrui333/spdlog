#pragma once

#include <cstdint>
#include <thread>
#include <atomic>
#include <cassert>
#include <functional>

#include "../details/async_log_msg.h"
#include "../details/mpmc_blocking_q.h"
#include "dist_sink.h"

// async_sink is a sink that sends log messages to a dist_sink in a separate thread using a queue.
// The worker thread dequeues the messages and sends them to the dist_sink to perform the actual logging.
// The worker thread is terminated when the async_sink is destroyed.

namespace spdlog {
namespace sinks {

template <typename Mutex>
class async_sink final : public dist_sink<Mutex> {
public:
    using base_t = dist_sink<Mutex>;
    using async_log_msg = details::async_log_msg;
    using queue_t = details::mpmc_blocking_queue<async_log_msg>;
    enum { default_queue_size = 8192, max_queue_size = 1024 * 1024 * 10 };

    // Async overflow policy - block by default.
    enum class overflow_policy : std::uint8_t {
        block,           // Block until the log message can be enqueued (default).
        overrun_oldest,  // Overrun the oldest message in the queue if full.
        discard_new      // Discard the log message if the queue is full
    };

    async_sink(size_t queue_size, std::function<void()> on_thread_start, std::function<void()> on_thread_stop) {
        if (queue_size == 0 || queue_size > max_queue_size) {
            throw spdlog_ex("async_sink: invalid queue size");
        }
        // printf("........... Allocating queue: slot: %zu X %zu bytes ====> %lld KB ..............\n",
        //   queue_size, sizeof(details::async_log_msg), (sizeof(details::async_log_msg) * queue_size)/1024);
        q_ = std::make_unique<queue_t>(queue_size);

        worker_thread_ = std::thread([this, on_thread_start, on_thread_stop] {
            if (on_thread_start) on_thread_start();
            this->worker_loop();
            if (on_thread_stop) on_thread_stop();
        });
    }
    ~async_sink() override {
        try {
            q_->enqueue(async_log_msg(async_log_msg::type::terminate));
            worker_thread_.join();
        } catch (...) {
        }
    };

    async_sink(): async_sink(default_queue_size, nullptr, nullptr) {}
    explicit async_sink(size_t queue_size): async_sink(queue_size, nullptr, nullptr) {}
    async_sink(std::function<void()> on_thread_start, std::function<void()> on_thread_stop):
        async_sink(default_queue_size, on_thread_start, on_thread_stop) {}

    async_sink(const async_sink &) = delete;
    async_sink &operator=(const async_sink &) = delete;
    async_sink(async_sink &&) = default;
    async_sink &operator=(async_sink &&) = default;

    void set_overflow_policy(overflow_policy policy) { overflow_policy_ = policy; }
    [[nodiscard]] overflow_policy get_overflow_policy() const { return overflow_policy_; }

    [[nodiscard]] size_t get_overrun_counter() const { return q_->overrun_counter(); }
    void reset_overrun_counter() { q_->reset_overrun_counter(); }

    [[nodiscard]] size_t get_discard_counter() const { return q_->discard_counter(); }
    void reset_discard_counter() { q_->reset_discard_counter(); }

private:
    void sink_it_(const details::log_msg &msg) override {
        send_message_(async_log_msg::type::log, msg);
    }

    void flush_() override {
        send_message_(async_log_msg::type::flush, details::log_msg());
    }

    // asynchronously send the log message to the worker thread using the queue.
    // take into account the configured overflow policy.
    void send_message_(const async_log_msg::type msg_type, const details::log_msg &msg) {
        switch (overflow_policy_) {
            case overflow_policy::block:
                q_->enqueue(async_log_msg(msg_type, msg));
                break;
            case overflow_policy::overrun_oldest:
                q_->enqueue_nowait(async_log_msg(msg_type, msg));
                break;
            case overflow_policy::discard_new:
                q_->enqueue_if_have_room(async_log_msg(msg_type, msg));
                break;
            default:
                assert(false);
                throw spdlog_ex("async_sink: invalid overflow policy");
        }
    }

    void worker_loop () {
        details::async_log_msg incoming_msg;
        for (;;) {
            q_->dequeue(incoming_msg);
            switch (incoming_msg.message_type()) {
                case async_log_msg::type::log:
                    base_t::sink_it_(incoming_msg);
                break;
                case async_log_msg::type::flush:
                    base_t::flush_();
                break;
                case async_log_msg::type::terminate:
                    return;
                default:
                    assert(false);
            }
        }
    }

    std::atomic<overflow_policy> overflow_policy_ = overflow_policy::block;
    std::unique_ptr<queue_t> q_;
    std::thread worker_thread_;
};

using async_sink_mt = async_sink<std::mutex>;
using async_sink_st = async_sink<details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog
