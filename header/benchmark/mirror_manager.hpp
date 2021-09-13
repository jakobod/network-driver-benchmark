/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include "fwd.hpp"
#include "net/socket_manager.hpp"

namespace benchmark {

class mirror_manager : public net::socket_manager {
public:
  // -- constructors -----------------------------------------------------------

  mirror_manager(net::socket handle, net::multiplexer* mpx);

  ~mirror_manager() = default;

  // -- event handling ---------------------------------------------------------

  bool handle_read_event() override;

  bool handle_write_event() override;

  virtual std::string me() const {
    return "mirror_manager";
  }

private:
  size_t byte_diff_ = 0;
  detail::byte_array<8096> buf_;
};

} // namespace benchmark
