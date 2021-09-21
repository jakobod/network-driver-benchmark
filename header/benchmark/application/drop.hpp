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

struct drop {
  drop() {
    // nop
  }

  template <class Parent>
  net::error init(Parent& parent) {
    parent.configure_next_read(net::receive_policy::up_to(8096));
    return net::none;
  }

  template <class Parent>
  net::event_result produce(Parent&) {
    return net::event_result::ok;
  }

  bool has_more_data() {
    return true;
  }

  template <class Parent>
  net::event_result consume(Parent& parent, util::const_byte_span) {
    parent.configure_next_read(net::receive_policy::up_to(8096));
    return net::event_result::ok;
  }

  template <class Parent>
  net::event_result handle_timeout(Parent&, uint64_t) {
    return net::event_result::ok;
  }
};

} // namespace benchmark::application
