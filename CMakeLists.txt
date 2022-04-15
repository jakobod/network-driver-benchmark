# -- project setup -------------------------------------------------------------

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(lib_net_benchmark CXX)

include(FetchContent)

# -- set useful CMake options --------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
add_compile_options(-Wall -Wextra -pedantic)

message (STATUS "Build type ${CMAKE_BUILD_TYPE}")

# -- fetch network-driver repo -------------------------------------------------

FetchContent_Declare(
  network-driver
  GIT_REPOSITORY https://github.com/jakobod/network-driver.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(network-driver)

# -- fetch network-driver repo -------------------------------------------------

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG master)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
FetchContent_MakeAvailable(googlebenchmark)

# -- create obj ----------------------------------------------------------------

file(GLOB_RECURSE LIB_NET_BENCHMARK_HEADERS 
     "${CMAKE_CURRENT_SOURCE_DIR}/header/*.hpp")

set(LIB_NET_BENCHMARK_SOURCES)

add_library(lib_net_benchmark_obj OBJECT
            ${LIB_NET_BENCHMARK_HEADERS} ${LIB_NET_BENCHMARK_SOURCES})
set_property(TARGET lib_net_benchmark_obj PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(lib_net_benchmark_obj PRIVATE
                           "${network-driver_SOURCE_DIR}/header"
                           "${CMAKE_CURRENT_SOURCE_DIR}/header")

# -- add benchmark -------------------------------------------------------------

add_executable(
  lib_net_benchmark
  src/sending.cpp
)

target_include_directories(lib_net_benchmark PRIVATE                              
  "${network-driver_SOURCE_DIR}/header"
  "${CMAKE_CURRENT_SOURCE_DIR}/header")

target_link_libraries(lib_net_benchmark PRIVATE 
  benchmark::benchmark
  benchmark::benchmark_main
  net
  lib_net_benchmark_obj
)

# Allows issuing `make bench` to build and run the benchmarks in a single command
add_custom_target(
  bench
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/lib_net_benchmark"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/lib_net_benchmark"
  COMMENT "Run the benchmarks"
)

# -- add executables -----------------------------------------------------------

macro(add_target name)
  add_executable(${name} "src/${name}.cpp")
  target_include_directories(${name} PRIVATE 
                             "${network-driver_SOURCE_DIR}/header"
                             "${CMAKE_CURRENT_SOURCE_DIR}/header")
  target_link_libraries(${name} PRIVATE net lib_net_benchmark_obj)
endmacro()



add_target(epoll_mpx)
# add_target(multithreaded_epoll_mpx)
# add_target(multithreaded_epoll_mpx_test)
