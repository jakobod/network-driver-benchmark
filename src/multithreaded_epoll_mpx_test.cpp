#include <chrono>
#include <cstdlib>
#include <iostream>
#include <limits>

#include "benchmark/application/drop.hpp"
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

struct drop_manager_factory : public socket_manager_factory {
  socket_manager_ptr make(net::socket handle, multiplexer* mpx) override {
    using mirror_manager = manager::stream<application::drop>;
    return std::make_shared<mirror_manager>(socket_cast<stream_socket>(handle),
                                            mpx);
  }
};

multiplexer_ptr make_server(size_t num_threads) {
  auto factory = std::make_shared<drop_manager_factory>();
  auto mpx_res = make_multithreaded_epoll_multiplexer(std::move(factory),
                                                      num_threads);
  if (auto err = get_error(mpx_res))
    exit(*err);
  auto mpx = std::get<net::multiplexer_ptr>(mpx_res);
  mpx->start();
  return mpx;
}

int main(int num_args, char** args) {
  size_t num_clients = 1;
  size_t num_threads = 1;

  for (int i = 1; i < num_args; ++i) {
    auto flag = std::string(args[i]);
    if (flag == "-t" || flag == "--threads")
      num_threads = atoi(args[++i]);
    else if (flag == "-c" || flag == "--clients")
      num_clients = atoi(args[++i]);
    else
      exit("unrecognized argument " + flag);
  }
  std::cout << "running multithreaded benchmark using " << num_clients
            << " clients and " << num_threads << " num_threads = " << std::endl;

  auto server_mpx = make_server(num_threads);
  std::this_thread::sleep_for(500ms);
  std::cerr << "multiplexer should run now" << std::endl;
  std::cerr << "[ctrl-D] quits, anything else adds more managers" << std::endl;
  std::vector<tcp_stream_socket> sockets;
  sockets.emplace_back(std::get<tcp_stream_socket>(
    net::make_connected_tcp_stream_socket("127.0.0.1", server_mpx->port())));

  std::cerr << "writing data now" << std::endl;
  std::string dummy;
  util::byte_array<256> data;
  while (std::getline(std::cin, dummy))
    write(sockets.back(), data);

  std::cout << "[main] shutting down..." << std::endl;
  server_mpx->shutdown();
  std::cout << "[main] joining..." << std::endl;
  server_mpx->join();
  std::cout << "[main] done. BYE!" << std::endl;
}
