/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include <chrono>

#include "fwd.hpp"
#include "net/error.hpp"
#include "net/event_result.hpp"

namespace benchmark::application {

struct output {
  output(size_t byte_per_second) : byte_per_second_(byte_per_second) {
    uint8_t b = 0;
    for (size_t i = 0; i < 256; ++i)
      data_.emplace_back(std::byte(b++));
  }

  template <class Parent>
  net::error init(Parent& parent) {
    using namespace std::chrono_literals;
    std::cerr << "output: init called" << std::endl;
    parent.configure_next_read(net::receive_policy::up_to(8096));
    parent.set_timeout_in(1s, 0);
    return net::none;
  }

  template <class Parent>
  net::event_result produce(Parent& parent) {
    auto& buf = parent.write_buffer();
    buf.insert(buf.end(), data_.begin(), data_.end());
    byte_produced_ += data_.size();
    return net::event_result::ok;
  }

  bool has_more_data() {
    return byte_produced_ < byte_per_second_;
  }

  template <class Parent>
  net::event_result consume(Parent& parent, util::const_byte_span data) {
    parent.configure_next_read(net::receive_policy::up_to(8096));
    byte_received_ += data.size();
    return net::event_result::ok;
  }

  template <class Parent>
  net::event_result handle_timeout(Parent& parent, uint64_t) {
    using namespace std::chrono_literals;
    parent.set_timeout_in(1s, 0);
    std::cerr << "[output] created = " << byte_produced_
              << ", received = " << byte_received_ << " byte" << std::endl;
    byte_received_ = 0;
    byte_produced_ = 0;
    parent.register_writing();
    return net::event_result::ok;
  }

private:
  util::byte_buffer data_;
  const size_t byte_per_second_;
  size_t byte_produced_ = 0;
  size_t byte_received_ = 0;
};

} // namespace benchmark::application
