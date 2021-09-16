/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include "fwd.hpp"
#include "net/socket_manager.hpp"
#include "net/socket_manager_factory.hpp"

namespace benchmark {

class output_manager : public net::socket_manager {
public:
  // -- constructors -----------------------------------------------------------

  output_manager(net::socket handle, net::multiplexer* mpx,
                 size_t byte_per_second);

  net::error init() override;

  // -- event handling ---------------------------------------------------------

  bool handle_read_event() override;

  bool handle_write_event() override;

  bool handle_timeout(uint64_t timeout_id) override;

private:
  const size_t byte_per_second_;

  size_t bytes_received_ = 0;
  util::byte_array<256> send_buf_;

  size_t bytes_written_ = 0;
  util::byte_array<256> receive_buf_;
};

struct output_manager_factory : public net::socket_manager_factory {
  output_manager_factory(size_t byte_per_second)
    : byte_per_second_(byte_per_second) {
    // nop
  }

  net::socket_manager_ptr make(net::socket handle,
                               net::multiplexer* mpx) override {
    return std::make_shared<output_manager>(handle, mpx, byte_per_second_);
  }

private:
  size_t byte_per_second_ = 0;
};

} // namespace benchmark
