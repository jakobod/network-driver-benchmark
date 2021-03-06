/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 09.04.2021
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

namespace benchmark {

struct result {
  explicit result(size_t num_managers) : num_managers_(num_managers) {
    // nop
  }

  void update(size_t byte_written, size_t byte_received) {
    std::lock_guard<std::mutex> guard(lock_);
    thread_ids_.emplace(std::this_thread::get_id());
    byte_received_ += byte_received;
    byte_written_ += byte_written;
    if (++num_invocations_ == num_managers_) {
      print();
      clear();
    }
  }

private:
  void print() const {
    std::cout << "[result] sent = " << byte_written_
              << ", received = " << byte_received_ << ", ids = [";
    for (auto id : thread_ids_)
      std::cout << id;
    std::cout << "]" << std::endl;
  }

  void clear() {
    byte_received_ = 0;
    byte_written_ = 0;
    num_invocations_ = 0;
    thread_ids_.clear();
  }

  std::set<std::thread::id> thread_ids_;
  std::mutex lock_;
  const size_t num_managers_;
  size_t num_invocations_ = 0;
  size_t byte_received_ = 0;
  size_t byte_written_ = 0;
};

using result_ptr = std::shared_ptr<result>;

result_ptr make_result(size_t num_managers) {
  return std::make_shared<result>(num_managers);
}

} // namespace benchmark
