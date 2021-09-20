#include <chrono>
#include <iostream>
#include <limits>

#include "benchmark/application/mirror.hpp"
#include "benchmark/application/output.hpp"
#include "benchmark/result.hpp"
#include "fwd.hpp"
#include "net/manager/stream.hpp"
#include "net/multithreaded_epoll_multiplexer.hpp"
#include "net/socket_manager_factory.hpp"

using namespace net;
using namespace benchmark;
using namespace std::chrono_literals;

static constexpr size_t unlimited = std::numeric_limits<size_t>::max();

[[noreturn]] void exit(std::string msg) {
  std::cerr << "ERROR: " << msg << std::endl;
  abort();
}

[[noreturn]] void exit(error err) {
  exit(to_string(err));
}

struct mirror_manager_factory : public socket_manager_factory {
  socket_manager_ptr make(net::socket handle, multiplexer* mpx) override {
    using mirror_manager = manager::stream<application::mirror>;
    return std::make_shared<mirror_manager>(socket_cast<stream_socket>(handle),
                                            mpx);
  }
};

multiplexer_ptr make_client(size_t num_clients, size_t num_threads,
                            uint16_t port, size_t bps) {
  using output_manager = manager::stream<application::output>;
  auto mpx_res = make_multithreaded_epoll_multiplexer(nullptr, num_threads);
  if (auto err = get_error(mpx_res))
    exit(*err);
  auto mpx = std::get<multiplexer_ptr>(mpx_res);
  auto results = make_result(num_clients);
  for (size_t num = 0; num < num_clients; ++num)
    if (auto err = mpx->tcp_connect<output_manager>(
          "127.0.0.1", port, operation::read_write, results, bps))
      exit(err);
  mpx->start();
  return mpx;
}

multiplexer_ptr make_server(size_t num_threads) {
  auto factory = std::make_shared<mirror_manager_factory>();
  auto mpx_res = make_multithreaded_epoll_multiplexer(std::move(factory),
                                                      num_threads);
  if (auto err = get_error(mpx_res))
    exit(*err);
  auto mpx = std::get<net::multiplexer_ptr>(mpx_res);
  mpx->start();
  return mpx;
}

int main(int, char**) {
  size_t num_clients = 100;
  size_t num_threads = 2;
  auto server_mpx = make_server(num_threads);
  auto client_mpx = make_client(num_clients, num_threads, server_mpx->port(),
                                unlimited);
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
