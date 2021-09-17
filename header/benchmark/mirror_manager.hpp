/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include "fwd.hpp"
#include "net/socket_manager.hpp"
#include "net/socket_manager_factory.hpp"

namespace benchmark {

class mirror_manager : public net::socket_manager {
public:
  // -- constructors -----------------------------------------------------------

  mirror_manager(net::socket handle, net::multiplexer* mpx);

  ~mirror_manager() = default;

  net::error init();

  // -- event handling ---------------------------------------------------------

  net::event_result handle_read_event() override;

  net::event_result handle_write_event() override;

  net::event_result handle_timeout(uint64_t timeout_id) override;

  virtual std::string me() const {
    return "mirror_manager";
  }

private:
  size_t byte_diff_ = 0;
  util::byte_array<8096> buf_;
};

struct mirror_manager_factory : public net::socket_manager_factory {
  net::socket_manager_ptr make(net::socket handle,
                               net::multiplexer* mpx) override {
    return std::make_shared<mirror_manager>(handle, mpx);
  }
};

} // namespace benchmark
