#include <chrono>
#include <iostream>
#include <limits>

#include "benchmark/application/mirror.hpp"
#include "benchmark/application/output.hpp"

#include "net/fwd.hpp"
#include "net/multiplexer.hpp"
#include "net/multiplexer_impl.hpp"
#include "net/socket_manager_factory.hpp"
#include "net/stream_transport.hpp"
#include "net/transport_adaptor.hpp"

#include "net/ip/v4_address.hpp"
#include "net/ip/v4_endpoint.hpp"

#include "util/config.hpp"
#include "util/error.hpp"

using namespace benchmark;
using namespace std::chrono_literals;

using benchmark::application::mirror;
using benchmark::application::output;

namespace {

[[maybe_unused]] constexpr const size_t unlimited
  = std::numeric_limits<size_t>::max();

[[noreturn]] void handle_error(const std::string& msg) {
  std::cerr << "ERROR: " << msg << std::endl;
  abort();
}

[[noreturn]] void handle_error(const util::error& err) {
  handle_error(to_string(err));
}

struct mirror_manager_factory : public net::socket_manager_factory {
  net::socket_manager_ptr make(net::socket handle,
                               net::multiplexer* mpx) override {
    using mirror_manager
      = net::stream_transport<net::transport_adaptor<mirror>>;
    return util::make_intrusive<mirror_manager>(
      socket_cast<net::stream_socket>(handle), mpx);
  }
};

net::multiplexer_ptr make_client(const std::uint16_t port,
                                 const util::config& cfg) {
  using output_manager = net::stream_transport<net::transport_adaptor<output>>;
  auto mpx_res = net::make_multiplexer(nullptr, cfg);
  if (auto err = get_error(mpx_res))
    handle_error(*err);
  auto mpx = std::get<net::multiplexer_ptr>(mpx_res);
  net::ip::v4_endpoint ep{net::ip::v4_address::localhost, port};
  if (auto err = mpx->tcp_connect<output_manager>(ep,
                                                  net::operation::read_write))
    handle_error(err);
  mpx->start();
  return mpx;
}

net::multiplexer_ptr make_server(const util::config& cfg) {
  auto factory = std::make_shared<mirror_manager_factory>();
  auto mpx_res = net::make_multiplexer(std::move(factory), cfg);
  if (auto err = util::get_error(mpx_res))
    handle_error(*err);
  auto mpx = std::get<net::multiplexer_ptr>(mpx_res);
  mpx->start();
  return mpx;
}

} // namespace

int main(int, char**) {
  util::config cfg;
  // cfg.add_config_entry("logger.terminal-output", true);
  LOG_INIT(cfg);
  auto server_mpx = make_server(cfg);
  std::cout << "Server port is = " << server_mpx->port() << std::endl;
  auto client_mpx = make_client(server_mpx->port(), util::config{});
  net::make_connected_tcp_stream_socket(
    {net::ip::v4_address::localhost, server_mpx->port()});
  std::string dummy;
  std::getline(std::cin, dummy);
  std::cout << "[main] shutting down..." << std::endl;
  server_mpx->shutdown();
  // client_mpx->shutdown();
  std::cout << "[main] joining..." << std::endl;
  server_mpx->join();
  // client_mpx->join();
  std::cout << "[main] done. BYE!" << std::endl;
}
