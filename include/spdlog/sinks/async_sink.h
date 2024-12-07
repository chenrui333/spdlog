#pragma once

#include <thread>

#include "../details/async_log_msg.h"
#include "../details/mpmc_blocking_q.h"
#include "./dist_sink.h"

namespace spdlog {
namespace sinks {

template <typename Mutex>
class async_sink final : public dist_sink<Mutex> {
public:
  using base_t = dist_sink<Mutex>;
  using queue_t = details::mpmc_blocking_queue<details::async_log_msg>;
  async_sink() {
    q_ = std::make_unique<queue_t>(8192);
    worker_thread_ = std::thread([this] {
      details::async_log_msg incoming_msg;
      for (;;) {
        q_->dequeue(incoming_msg);
        if (incoming_msg.message_type() == details::async_log_msg::msg_type::terminate) {
          break;
        }
        base_t::sink_it_(incoming_msg);
      }
    });
  }
  ~async_sink() override {
    try {
      q_->enqueue(details::async_log_msg(details::async_log_msg::msg_type::terminate));
      worker_thread_.join();
    } catch (...) {}
  };

  async_sink(const async_sink &) = delete;
  async_sink &operator=(const async_sink &) = delete;
  async_sink(async_sink &&) = default;
  async_sink &operator=(async_sink &&) = default;




private:
  void sink_it_(const details::log_msg &msg) override {
    // Asynchronously send the log message to the base sink
    q_->enqueue(details::async_log_msg(details::async_log_msg::msg_type::log, msg));
  }
  void flush_() override {
    // Asynchronously flush the base sink
    q_->enqueue(details::async_log_msg(details::async_log_msg::msg_type::flush));
  }

  std::thread worker_thread_;
  std::unique_ptr<queue_t> q_;
};

using async_sink_mt = async_sink<std::mutex>;
using async_sink_st = async_sink<details::null_mutex>;

} // namespace sinks
}  // namespace spdlog
