#include <benchmark/benchmark.h>

#include "util/format.hpp"

using namespace std::string_literals;

static void BM_format_one(benchmark::State& state) {
  for (auto _ : state)
    util::format("Hello {0}", "World"s);
}
BENCHMARK(BM_format_one);

static void BM_format_two(benchmark::State& state) {
  for (auto _ : state)
    util::format("{0} {1}", "Hello"s, "World"s);
}
BENCHMARK(BM_format_two);

static void BM_format_three(benchmark::State& state) {
  for (auto _ : state)
    util::format("{0} {1} {2}", "Hello"s, "World"s, "yo"s);
}
BENCHMARK(BM_format_three);

static void BM_format_four_short(benchmark::State& state) {
  for (auto _ : state)
    util::format("{0} {1} {2} {3}", "1"s, "2"s, "3"s, "4"s);
}
BENCHMARK(BM_format_four_short);

static void BM_format_four_long(benchmark::State& state) {
  for (auto _ : state)
    util::format("{0} {1} {2} {3}", "ojferiogjhoeijger"s,
                 "guihetnilghnetiughetg"s, "ifuhberifuheifheifr"s,
                 "fiurehfiuehfiuhfiuerhfi"s);
}
BENCHMARK(BM_format_four_long);
