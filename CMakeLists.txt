# -- project setup -------------------------------------------------------------

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(lib_net_benchmark CXX)

include(FetchContent)

# -- set useful CMake options --------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# -- add executables -----------------------------------------------------------

set(LIB_NET_BENCHMARK_SOURCES
  src/benchmark/mirror_manager.cpp
  src/benchmark/output_manager.cpp
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

macro(add_target name)
  add_executable(${name} 
                 "src/${name}.cpp"
                 ${LIB_NET_BENCHMARK_SOURCES})
  target_include_directories(${name} PRIVATE 
                             "${network-driver_SOURCE_DIR}/header"
                             "${CMAKE_CURRENT_SOURCE_DIR}/header")
  target_link_libraries(${name} PRIVATE net)
endmacro()

add_target(main)
