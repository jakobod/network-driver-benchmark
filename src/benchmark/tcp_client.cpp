/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#include "benchmark/tcp_client.hpp"

#include <chrono>
#include <iostream>
#include <variant>

#include "net/operation.hpp"

namespace benchmark {

using namespace net;
using namespace std::chrono;

tcp_client::tcp_client(size_t byte_per_second)
  : epoll_fd_(invalid_socket_id),
    byte_per_second_(byte_per_second),
    written_(0),
    received_(0),
    event_mask_(operation::none),
    running_(false) {
  // nop
}

tcp_client::~tcp_client() {
  close(handle_);
}

error tcp_client::init(const std::string ip, const uint16_t port) {
  epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd_ < 0)
    return error(runtime_error, "creating epoll fd failed");
  return connect(std::move(ip), port);
}

// -- thread functions ---------------------------------------------------------

void tcp_client::run() {
  using namespace std::chrono;
  auto block_until = steady_clock::now() + 1s;
  while (running_) {
    auto next_wakeup
      = duration_cast<milliseconds>(block_until - steady_clock::now());
    auto num_events = epoll_wait(epoll_fd_, pollset_.data(),
                                 static_cast<int>(pollset_.size()),
                                 next_wakeup.count());
    if (num_events < 0) {
      switch (errno) {
        case EINTR:
          continue;
        default:
          std::cerr << "epoll_wait failed: " << last_socket_error_as_string()
                    << std::endl;
          running_ = false;
          break;
      }
    } else {
      for (int i = 0; i < num_events; ++i) {
        if (pollset_[i].events == (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
          std::cerr << "epoll_wait failed: socket = " << i
                    << last_socket_error_as_string() << std::endl;
          stop();
        } else {
          if ((pollset_[i].events & operation::read) == operation::read) {
            auto read_res = read();
            if (read_res == state::error || read_res == state::disconnected) {
              stop();
              break;
            } else if (read_res == state::done) {
              disable(handle_, operation::read);
            }
          }
          if ((pollset_[i].events & operation::write) == operation::write) {
            auto write_res = write();
            if (write_res == state::error || write_res == state::disconnected) {
              stop();
              break;
            } else if (write_res == state::done) {
              disable(handle_, operation::write);
            }
          }
        }
      }
      if (block_until <= steady_clock::now()) {
        block_until += 1s;
        reset();
      }
    }
  }
  std::cerr << "tcp_client done" << std::endl;
}

void tcp_client::start() {
  running_ = true;
  writer_thread_ = std::thread(&tcp_client::run, this);
}

void tcp_client::stop() {
  running_ = false;
}

void tcp_client::join() {
  if (writer_thread_.joinable())
    writer_thread_.join();
}

// -- Private member functions -------------------------------------------------

error tcp_client::connect(const std::string ip, const uint16_t port) {
  auto connection_res = make_connected_tcp_stream_socket(std::move(ip), port);
  if (auto err = get_error(connection_res))
    return *err;
  handle_ = std::get<tcp_stream_socket>(connection_res);
  add(handle_);
  return none;
}

tcp_client::state tcp_client::read() {
  for (int i = 0; i < 20; ++i) {
    auto read_result = net::read(handle_, data_);
    if (read_result > 0) {
      received_ += read_result;
      if (received_ >= byte_per_second_)
        return state::done;
    } else if (read_result == 0) {
      std::cerr << "server disconnected" << std::endl;
      return state::disconnected;
    } else {
      if (!last_socket_error_is_temporary()) {
        std::cerr << "ERROR read failed: " << last_socket_error_as_string()
                  << std::endl;
        return state::error;
      }
    }
  }
  return state::go_on;
}

tcp_client::state tcp_client::write() {
  auto num_bytes = std::min(data_.size(), byte_per_second_ - written_);
  auto write_res = net::write(handle_, std::span(data_.data(), num_bytes));
  if (write_res >= 0) {
    written_ += write_res;
    if (written_ >= byte_per_second_)
      return state::done;
  } else if (write_res < 0) {
    if (!last_socket_error_is_temporary()) {
      std::cerr << "ERROR write failed: " << last_socket_error_as_string()
                << std::endl;
      return state::error;
    }
  }
  return state::go_on;
}

void tcp_client::reset() {
  written_ = 0;
  received_ = 0;
  enable(handle_, operation::read_write);
}

// -- epoll management ---------------------------------------------------------

bool tcp_client::mask_add(operation flag) {
  if ((event_mask_ & flag) == flag)
    return false;
  event_mask_ = event_mask_ | flag;
  return true;
}

bool tcp_client::mask_del(operation flag) {
  if ((event_mask_ & flag) == operation::none)
    return false;
  event_mask_ = event_mask_ & ~flag;
  return true;
}

void tcp_client::add(net::socket handle) {
  nonblocking(handle, true);
  event_mask_ = operation::read_write;
  mod(handle.id, EPOLL_CTL_ADD, event_mask_);
}

void tcp_client::del(net::socket handle) {
  event_mask_ = operation::none;
  mod(handle.id, EPOLL_CTL_DEL, event_mask_);
}

void tcp_client::enable(net::socket handle, operation op) {
  if (mask_add(op))
    mod(handle.id, EPOLL_CTL_MOD, event_mask_);
}

void tcp_client::disable(net::socket handle, operation op) {
  if (!mask_del(op))
    return;
  mod(handle.id, EPOLL_CTL_MOD, event_mask_);
}

void tcp_client::mod(int fd, int op, operation events) {
  epoll_event event = {};
  event.events = static_cast<uint32_t>(events);
  event.data.fd = fd;
  if (epoll_ctl(epoll_fd_, op, fd, &event) < 0) {
    std::cerr << "ERROR epoll_ctl: " << last_socket_error_as_string()
              << std::endl;
    stop();
  }
}

} // namespace benchmark
