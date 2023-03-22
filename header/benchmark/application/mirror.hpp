/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include "net/fwd.hpp"
#include "util/fwd.hpp"

#include "net/application.hpp"
#include "net/event_result.hpp"
#include "net/layer.hpp"
#include "net/receive_policy.hpp"

#include "util/byte_span.hpp"
#include "util/config.hpp"
#include "util/error.hpp"

namespace benchmark::application {

class mirror : public net::application {
  static constexpr const std::size_t max_read_amount = 8096;

public:
  mirror(net::layer& parent) : parent_{parent} {
    // nop
  }

  util::error init(const util::config&) override {
    parent_.configure_next_read(net::receive_policy::up_to(max_read_amount));
    return util::none;
  }

  bool has_more_data() override {
    return !received_.empty();
  }

  net::event_result produce() override {
    auto& buf = parent_.write_buffer();
    buf.insert(buf.end(), received_.begin(), received_.end());
    received_.clear();
    return net::event_result::ok;
  }

  net::event_result consume(util::const_byte_span data) override {
    received_.insert(received_.end(), data.begin(), data.end());
    parent_.configure_next_read(net::receive_policy::up_to(max_read_amount));
    parent_.register_writing();
    return net::event_result::ok;
  }

  net::event_result handle_timeout(uint64_t) override {
    return net::event_result::ok;
  }

private:
  net::layer& parent_;
  util::byte_buffer received_;
};

} // namespace benchmark::application
