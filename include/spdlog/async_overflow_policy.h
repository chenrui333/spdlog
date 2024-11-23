#pragma once

namespace spdlog {
// Async overflow policy - block by default.
enum class async_overflow_policy {
  block,           // Block until message can be enqueued
  overrun_oldest,  // Discard oldest message in the queue if full when trying to
                   // add new item.
  discard_new      // Discard new message if the queue is full when trying to add new item.
};

}


