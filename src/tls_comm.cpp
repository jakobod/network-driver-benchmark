#include <chrono>
#include <iostream>
#include <limits>

#include "benchmark/application/mirror.hpp"
#include "benchmark/application/output.hpp"

#include "net/epoll_multiplexer.hpp"
#include "net/fwd.hpp"
#include "net/socket_manager_factory.hpp"
#include "net/stream_transport.hpp"
#include "net/tls.hpp"
#include "net/transport_adaptor.hpp"

#include "net/ip/v4_address.hpp"
#include "net/ip/v4_endpoint.hpp"

#include "util/error.hpp"

using namespace net;
using namespace net::ip;
using namespace benchmark;
using namespace std::chrono_literals;

using benchmark::application::mirror;
using benchmark::application::output;

static constexpr size_t unlimited = std::numeric_limits<size_t>::max();

[[noreturn]] void handle_error(const std::string& msg) {
  std::cerr << "ERROR: " << msg << std::endl;
  abort();
}

[[noreturn]] void handle_error(const util::error& err) {
  handle_error(to_string(err));
}

struct mirror_manager_factory : public socket_manager_factory {
  mirror_manager_factory(openssl::tls_context& ctx, bool is_client)
    : ctx_{ctx}, is_client_{is_client} {
    // nop
  }

  socket_manager_ptr make(net::socket handle, multiplexer* mpx) override {
    using mirror_manager = stream_transport<transport_adaptor<tls<mirror>>>;
    return std::make_shared<mirror_manager>(socket_cast<stream_socket>(handle),
                                            mpx, ctx_, is_client_);
  }

private:
  openssl::tls_context& ctx_;
  bool is_client_;
};

multiplexer_ptr make_client(uint16_t port, openssl::tls_context& ctx) {
  using output_manager = stream_transport<transport_adaptor<tls<output>>>;
  auto mpx_res = make_epoll_multiplexer(nullptr);
  if (auto err = get_error(mpx_res))
    handle_error(*err);
  auto mpx = std::get<multiplexer_ptr>(mpx_res);
  v4_endpoint ep{v4_address::localhost, port};
  if (auto err = mpx->tcp_connect<output_manager>(ep, operation::read_write,
                                                  ctx, true))
    handle_error(err);
  mpx->start();
  return mpx;
}

multiplexer_ptr make_server(openssl::tls_context& ctx) {
  auto factory = std::make_shared<mirror_manager_factory>(ctx, false);
  auto mpx_res = make_epoll_multiplexer(std::move(factory));
  if (auto err = get_error(mpx_res))
    handle_error(*err);
  auto mpx = std::get<multiplexer_ptr>(mpx_res);
  mpx->start();
  return mpx;
}

int main(int, char**) {
  openssl::tls_context ctx;
  if (auto err = ctx.init(CERT_DIRECTORY "/server.crt",
                          CERT_DIRECTORY "/server.key"))
    handle_error(err);
  auto server_mpx = make_server(ctx);
  auto client_mpx = make_client(server_mpx->port(), ctx);
  std::string dummy;
  std::getline(std::cin, dummy);
  std::cout << "[main] shutting down..." << std::endl;
  server_mpx->shutdown();
  client_mpx->shutdown();
  std::cout << "[main] joining..." << std::endl;
  server_mpx->join();
  client_mpx->join();
  std::cout << "[main] done. BYE!" << std::endl;
}
