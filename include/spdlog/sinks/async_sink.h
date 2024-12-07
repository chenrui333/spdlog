#pragma once

#include <cstdint>
#include <thread>
#include <atomic>
#include <cassert>

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
    using async_message_t = details::async_log_msg;
    using queue_t = details::mpmc_blocking_queue<async_message_t>;
    enum { max_queue_size = 1024 * 1024 * 10 };

    // Async overflow policy - block by default.
    enum class overflow_policy : std::uint8_t {
        block,           // Block until message can be enqueued (default
        overrun_oldest,  // Discard oldest message in the queue if full when trying to
                         // add new item.
        discard_new      // Discard new message if the queue is full when trying to add new item.
    };

    explicit async_sink(size_t queue_size) {
        if (queue_size == 0 || queue_size > max_queue_size) {
            throw spdlog_ex("async_sink: invalid queue size");
        }
        // printf("........... Allocating queue: slot: %zu X %zu bytes ====> %lld KB ..............\n",
        //   queue_size, sizeof(details::async_log_msg), (sizeof(details::async_log_msg) * queue_size)/1024);
        q_ = std::make_unique<queue_t>(queue_size);

        worker_thread_ = std::thread([this] {
            details::async_log_msg incoming_msg;
            for (;;) {
                q_->dequeue(incoming_msg);
                if (incoming_msg.message_type() == async_message_t::type::terminate) {
                    break;
                }
                base_t::sink_it_(incoming_msg);
            }
        });
    }
    ~async_sink() override {
        try {
            q_->enqueue(async_message_t(async_message_t::type::terminate));
            worker_thread_.join();
        } catch (...) {
        }
    };

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
        send_message_(async_message_t::type::log, msg);
    }

    void flush_() override {
        send_message_(async_message_t::type::flush, details::log_msg());
    }

    // asynchronously send the log message to the worker thread using the queue.
    // take into account the configured overflow policy.
    void send_message_(const async_message_t::type msg_type, const details::log_msg &msg) {
        switch (overflow_policy_) {
            case overflow_policy::block:
                q_->enqueue(async_message_t(msg_type, msg));
                break;
            case overflow_policy::overrun_oldest:
                q_->enqueue_nowait(async_message_t(msg_type, msg));
                break;
            case overflow_policy::discard_new:
                q_->enqueue_if_have_room(async_message_t(msg_type, msg));
                break;
            default:
                assert(false);
                throw spdlog_ex("async_sink: invalid overflow policy");
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
