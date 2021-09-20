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

# -- fetch network-driver repo -------------------------------------------------

FetchContent_Declare(
  network-driver
  GIT_REPOSITORY https://github.com/jakobod/network-driver.git
  GIT_TAG        master
)
FetchContent_Populate(network-driver)
add_subdirectory(${network-driver_SOURCE_DIR} ${network-driver_BINARY_DIR})

# -- create obj ----------------------------------------------------------------

file(GLOB_RECURSE LIB_NET_BENCHMARK_HEADERS 
     "${CMAKE_CURRENT_SOURCE_DIR}/header/*.hpp")

set(LIB_NET_BENCHMARK_SOURCES)

add_library(benchmark_obj OBJECT
            ${LIB_NET_BENCHMARK_HEADERS} ${LIB_NET_BENCHMARK_SOURCES})
set_property(TARGET benchmark_obj PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(benchmark_obj PRIVATE
                           "${network-driver_SOURCE_DIR}/header"
                           "${CMAKE_CURRENT_SOURCE_DIR}/header")

# -- add executables -----------------------------------------------------------

macro(add_target name)
  add_executable(${name} "src/${name}.cpp")
  target_include_directories(${name} PRIVATE 
                             "${network-driver_SOURCE_DIR}/header"
                             "${CMAKE_CURRENT_SOURCE_DIR}/header")
  target_link_libraries(${name} PRIVATE net benchmark_obj)
endmacro()

add_target(epoll_mpx)
add_target(multithreaded_epoll_mpx)
