// #include <benchmark/benchmark.h>

// #include "benchmark/application/mirror.hpp"
// #include "net/stream_transport.hpp"
// #include "net/tls.hpp"
// #include "net/transport_adaptor.hpp"

// using namespace net;
// using namespace benchmark::application;

// namespace {

// struct tcp_communication : public benchmark::Fixture {};
// using server_stack_type = stream_transport<transport_adaptor<tls<mirror>>>;
// using client_stack_type = stream_transport<transport_adaptor<tls<counter>>>;
// } // namespace

// BENCHMARK_F(tcp_communication, make_message)(benchmark::State& state) {
//   for (auto _ : state) {
//   }
// }
