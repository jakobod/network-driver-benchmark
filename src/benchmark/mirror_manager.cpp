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

bool mirror_manager::handle_read_event() {
  for (int i = 0; i < 20; ++i) {
    auto read_res = read(handle<stream_socket>(), buf_);
    if (read_res == 0)
      return false;
    if (read_res < 0) {
      if (last_socket_error_is_temporary()) {
        return true;
      } else {
        mpx()->handle_error(
          error(socket_operation_failed,
                "[mirror_manager.read()] " + last_socket_error_as_string()));
        return false;
      }
    }
    byte_diff_ += read_res;
    register_writing();
  }
  return true;
}

bool mirror_manager::handle_write_event() {
  auto done_writing = [&]() { return byte_diff_ == 0; };
  for (int i = 0; i < 20; ++i) {
    auto num_bytes = std::min(byte_diff_, buf_.size());
    auto write_res = write(handle<stream_socket>(),
                           std::span(buf_.data(), num_bytes));
    if (write_res > 0) {
      byte_diff_ -= write_res;
      if (done_writing())
        return false;
    } else if (write_res < 0) {
      if (last_socket_error_is_temporary())
        return true;
      else
        mpx()->handle_error(
          error(socket_operation_failed,
                "[mirror_manager.write()] " + last_socket_error_as_string()));
    }
  }
  return !done_writing();
}

bool mirror_manager::handle_timeout(uint64_t) {
  return false;
}

} // namespace benchmark
