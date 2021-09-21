/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include "fwd.hpp"

#include "net/error.hpp"
#include "net/event_result.hpp"
#include "net/receive_policy.hpp"

namespace benchmark::application {

struct mirror {
  mirror() {
    // nop
  }

  template <class Parent>
  net::error init(Parent& parent) {
    parent.configure_next_read(net::receive_policy::up_to(8096));
    return net::none;
  }

  template <class Parent>
  net::event_result produce(Parent& parent) {
    auto& buf = parent.write_buffer();
    {
      std::lock_guard<std::mutex> guard(lock_);
      buf.insert(buf.end(), received_.begin(), received_.end());
      received_.clear();
    }
    return net::event_result::ok;
  }

  bool has_more_data() {
    return !received_.empty();
  }

  template <class Parent>
  net::event_result consume(Parent& parent, util::const_byte_span data) {
    {
      std::lock_guard<std::mutex> guard(lock_);
      received_.insert(received_.end(), data.begin(), data.end());
    }
    parent.configure_next_read(net::receive_policy::up_to(8096));
    parent.register_writing();
    return net::event_result::ok;
  }

  template <class Parent>
  net::event_result handle_timeout(Parent&, uint64_t) {
    return net::event_result::ok;
  }

private:
  util::byte_buffer received_;
  std::mutex lock_;
};

} // namespace benchmark::application
