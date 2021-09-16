/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include <array>
#include <sys/epoll.h>
#include <thread>

#include "fwd.hpp"
#include "net/tcp_stream_socket.hpp"

namespace benchmark {

class tcp_client {
  using epoll_fd = int;
  static constexpr size_t max_epoll_events = 1;
  using pollset = std::array<epoll_event, max_epoll_events>;

public:
  tcp_client() = default;

  tcp_client(size_t byte_per_second);

  ~tcp_client();

  net::error init(const std::string ip, uint16_t port);

  // -- thread functions -------------------------------------------------------

  void run();

  void start();

  void stop();

  void join();

private:
  // -- read/write results -----------------------------------------------------

  enum class state {
    ok = 0,
    done,
    error,
    disconnected,
  };

  // -- Connection management --------------------------------------------------

  net::error connect(const std::string ip, const uint16_t port);

  state read();

  state write();

  void reset();

  // -- epoll management -------------------------------------------------------

  bool mask_add(net::operation flag);

  bool mask_del(net::operation flag);

  void add(net::socket handle);

  void del(net::socket handle);

  void enable(net::socket handle, net::operation op);

  void disable(net::socket handle, net::operation op);

  void mod(int fd, int op, net::operation events);

  // -- members ----------------------------------------------------------------

  // Remote node identifiers
  net::tcp_stream_socket handle_;

  // epoll variables
  epoll_fd epoll_fd_;
  pollset pollset_;

  // read and write state
  util::byte_array<8096> data_;
  const size_t byte_per_second_;
  size_t written_;
  size_t received_;
  net::operation event_mask_;

  // thread state
  bool running_;
  std::thread writer_thread_;
};

} // namespace benchmark
