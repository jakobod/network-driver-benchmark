/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#include "benchmark/output_manager.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <span>

#include "net/multiplexer.hpp"
#include "net/stream_socket.hpp"

namespace benchmark {

using namespace net;
using namespace std::chrono_literals;

static constexpr size_t max_consecutive_reads = 20;
static constexpr size_t max_consecutive_writes = 20;

output_manager::output_manager(net::socket handle, multiplexer* mpx,
                               size_t byte_per_second)
  : socket_manager(handle, mpx), byte_per_second_(byte_per_second) {
  nonblocking(handle, true);
  uint8_t b = 0;
  for (auto& val : send_buf_)
    val = std::byte(b++);
}

net::error output_manager::init() {
  set_timeout_in(1s, 0);
  return none;
}

event_result output_manager::handle_read_event() {
  for (size_t i = 0; i < max_consecutive_reads; ++i) {
    auto read_res = read(handle<stream_socket>(), receive_buf_);
    if (read_res == 0)
      return event_result::error;
    if (read_res < 0) {
      if (last_socket_error_is_temporary()) {
        return event_result::ok;
      } else {
        mpx()->handle_error(
          error(socket_operation_failed,
                "[output_manager.read()] " + last_socket_error_as_string()));
        return event_result::error;
      }
    }
    bytes_received_ += read_res;
  }
  return event_result::ok;
}

event_result output_manager::handle_write_event() {
  auto done_writing = [&]() { return bytes_written_ == byte_per_second_; };
  for (size_t i = 0; i < max_consecutive_writes; ++i) {
    auto missing_bytes = byte_per_second_ - bytes_written_;
    auto offset = missing_bytes % send_buf_.size();
    auto data = send_buf_.data() + offset;
    auto size = send_buf_.size() - offset;
    auto write_res = write(handle<stream_socket>(),
                           std::span(data, std::min(missing_bytes, size)));
    if (write_res > 0) {
      bytes_written_ += write_res;
      if (done_writing())
        return event_result::done;
    } else if (write_res < 0) {
      if (last_socket_error_is_temporary()) {
        return event_result::ok;
      } else {
        mpx()->handle_error(
          error(socket_operation_failed,
                "[output_manager.write()] " + last_socket_error_as_string()));
        return event_result::error;
      }
    }
  }
  return done_writing() ? event_result::done : event_result::ok;
}

event_result output_manager::handle_timeout(uint64_t) {
  set_timeout_in(1s, 0);
  std::cerr << "[output_manager] sent = " << bytes_written_
            << " byte, received = " << bytes_received_ << " byte" << std::endl;
  bytes_received_ = 0;
  bytes_written_ = 0;
  register_writing();
  return event_result::ok;
}

} // namespace benchmark
