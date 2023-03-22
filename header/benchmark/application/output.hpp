/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include "net/application.hpp"
#include "net/event_result.hpp"
#include "net/layer.hpp"
#include "net/receive_policy.hpp"

#include "util/byte_array.hpp"
#include "util/config.hpp"
#include "util/error.hpp"
#include "util/format.hpp"

#include <chrono>
#include <iostream>

namespace benchmark::application {

class output : public net::application {
  static constexpr const std::size_t max_read_size = 8096;

public:
  output(net::layer& parent) : parent_{parent} {
    uint8_t b = 0;
    for (auto& v : data_)
      v = std::byte(b++);
  }

  util::error init(const util::config&) override {
    parent_.configure_next_read(net::receive_policy::up_to(max_read_size));
    parent_.set_timeout_in(std::chrono::seconds(1));
    return util::none;
  }

  bool has_more_data() override {
    return true;
  }

  net::event_result produce() override {
    auto& buf = parent_.write_buffer();
    buf.insert(buf.end(), data_.begin(), data_.end());
    byte_produced_ += data_.size();
    return net::event_result::ok;
  }

  net::event_result consume(util::const_byte_span data) override {
    parent_.configure_next_read(net::receive_policy::up_to(max_read_size));
    byte_received_ += data.size();
    return net::event_result::ok;
  }

  net::event_result handle_timeout(uint64_t) override {
    parent_.set_timeout_in(std::chrono::seconds(1));
    std::cout << util::format("Received = {0}, Produced = {1}", byte_received_,
                              byte_produced_)
              << std::endl;
    byte_received_ = 0;
    byte_produced_ = 0;
    parent_.register_writing();
    return net::event_result::ok;
  }

private:
  net::layer& parent_;
  util::byte_array<256> data_;
  size_t byte_produced_ = 0;
  size_t byte_received_ = 0;
};

} // namespace benchmark::application
