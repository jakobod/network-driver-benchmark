#include <chrono>
#include <iostream>

#include "benchmark/output_manager.hpp"
#include "fwd.hpp"
#include "net/epoll_multiplexer.hpp"
#include "net/tcp_stream_socket.hpp"

using namespace net;
using namespace benchmark;
using namespace std::chrono_literals;

[[noreturn]] void exit(std::string msg) {
  std::cerr << "ERROR: " << msg << std::endl;
  abort();
}

multiplexer_ptr make_client(size_t num_clients, uint16_t port, size_t bps) {
  // This can only be done since the client side does not accept any connections
  auto mpx_res = make_epoll_multiplexer(nullptr);
  if (auto err = get_error(mpx_res))
    exit(to_string(*err));
  auto mpx = std::get<multiplexer_ptr>(mpx_res);
  for (size_t num = 0; num < num_clients; ++num) {
    auto sock = net::make_connected_tcp_stream_socket("127.0.0.1", port);
    if (auto err = get_error(sock))
      exit(to_string(*err));
    auto mgr = std::make_shared<output_manager>(
      std::get<tcp_stream_socket>(sock), mpx.get(), bps);
    if (auto err = mgr->init())
      exit(to_string(err));
    mpx->add(std::move(mgr), operation::read_write);
  }
  mpx->start();
  return mpx;
}

multiplexer_ptr make_server() {
  auto factory = std::make_shared<mirror_manager_factory>();
  auto mpx_res = make_epoll_multiplexer(std::move(factory));
  if (auto err = get_error(mpx_res))
    exit(to_string(*err));
  auto mpx = std::get<net::multiplexer_ptr>(mpx_res);
  mpx->start();
  return mpx;
}

int main(int, char**) {
  auto server_mpx = make_server();
  auto client_mpx = make_client(1, server_mpx->port(), 10'000'000'000);
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
