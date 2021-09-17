/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#include "benchmark/mirror_manager.hpp"

#include <iostream>
#include <span>

#include "net/multiplexer.hpp"
#include "net/stream_socket.hpp"

namespace benchmark {

using namespace net;

mirror_manager::mirror_manager(net::socket handle, multiplexer* mpx)
  : socket_manager(handle, mpx) {
  nonblocking(handle, true);
}

net::error mirror_manager::init() {
  return none;
}

event_result mirror_manager::handle_read_event() {
  for (int i = 0; i < 20; ++i) {
    auto read_res = read(handle<stream_socket>(), buf_);
    if (read_res == 0)
      return event_result::error;
    if (read_res < 0) {
      if (last_socket_error_is_temporary()) {
        return event_result::ok;
      } else {
        mpx()->handle_error(
          error(socket_operation_failed,
                "[mirror_manager.read()] " + last_socket_error_as_string()));
        return event_result::error;
      }
    }
    byte_diff_ += read_res;
    register_writing();
  }
  return event_result::ok;
}

event_result mirror_manager::handle_write_event() {
  auto done_writing = [&]() { return byte_diff_ == 0; };
  for (int i = 0; i < 20; ++i) {
    auto num_bytes = std::min(byte_diff_, buf_.size());
    auto write_res = write(handle<stream_socket>(),
                           std::span(buf_.data(), num_bytes));
    if (write_res > 0) {
      byte_diff_ -= write_res;
      if (done_writing())
        return event_result::done;
    } else if (write_res < 0) {
      if (last_socket_error_is_temporary())
        return event_result::ok;
      else
        mpx()->handle_error(
          error(socket_operation_failed,
                "[mirror_manager.write()] " + last_socket_error_as_string()));
    }
  }
  return done_writing() ? event_result::done : event_result::ok;
}

event_result mirror_manager::handle_timeout(uint64_t) {
  return event_result::ok;
}

} // namespace benchmark
