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

if(LIB_NET_LOG_LEVEL)
  message(STATUS "Logging enabled: ${LIB_NET_LOG_LEVEL}")
  add_compile_definitions(NET_LOG_LEVEL=${LIB_NET_LOG_LEVEL})
endif()

FetchContent_Declare(
  network-driver
  GIT_REPOSITORY https://github.com/jakobod/network-driver.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(network-driver)

# Set preprocessor definition to the dummy cert folder
add_compile_definitions(CERT_DIRECTORY="${network-driver_BINARY_DIR}/certs")

# -- create obj ----------------------------------------------------------------

file(GLOB_RECURSE LIB_NET_BENCHMARK_HEADERS 
     "${CMAKE_CURRENT_SOURCE_DIR}/header/*.hpp")

set(LIB_NET_BENCHMARK_SOURCES)

add_library(lib_net_benchmark_obj OBJECT
            ${LIB_NET_BENCHMARK_HEADERS} ${LIB_NET_BENCHMARK_SOURCES})
set_property(TARGET lib_net_benchmark_obj PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(lib_net_benchmark_obj PRIVATE
                           "${network-driver_SOURCE_DIR}/libnet"
                           "${CMAKE_CURRENT_SOURCE_DIR}/header")

# -- add benchmark -------------------------------------------------------------

if (LIB_NET_USE_GOOGLEBENCHMARK)
  # -- fetch network-driver repo -----------------------------------------------
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG master)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
  FetchContent_MakeAvailable(googlebenchmark)

  add_executable(
    lib_net_benchmark
    src/tcp_communication.cpp
    src/formatting.cpp
  )

  target_include_directories(lib_net_benchmark PRIVATE                              
    "${network-driver_SOURCE_DIR}/libnet"
    "${CMAKE_CURRENT_SOURCE_DIR}/header")

  target_link_libraries(lib_net_benchmark PRIVATE 
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
endif()

# -- add executables -----------------------------------------------------------

macro(add_target name)
  add_executable(${name} "src/${name}.cpp")
  target_include_directories(${name} PRIVATE 
                             "${network-driver_SOURCE_DIR}/libnet"
                             "${CMAKE_CURRENT_SOURCE_DIR}/header")
  target_link_libraries(${name} PRIVATE net lib_net_benchmark_obj)
endmacro()

add_target(tcp_comm)
# add_target(tls_comm)
# add_target(multithreaded_epoll_mpx)
# add_target(multithreaded_epoll_mpx_test)
