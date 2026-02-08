cmake_minimum_required(VERSION 3.15)
project(OrderBookEngine)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: vcpkg toolchain check (if you use vcpkg)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "No CMAKE_TOOLCHAIN_FILE defined, skipping vcpkg integration")
endif()

# -------------------------------------------------------------------
# Fetch GoogleTest
# -------------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

find_package(GTest CONFIG REQUIRED)

# -------------------------------------------------------------------
# Core library: orderbook, dispatcher, matching engine
# -------------------------------------------------------------------

add_library(orderbook_core
    Sources/OrderBook.cpp
    Sources/MatchingEngine.cpp
    Sources/OrderDispatcher.cpp
)

target_include_directories(orderbook_core PUBLIC Headers)

# -------------------------------------------------------------------
# ConcurrentQueue dependency (moodycamel)
# -------------------------------------------------------------------

FetchContent_Declare(
  moodycamel_concurrentqueue
  GIT_REPOSITORY https://github.com/cameron314/concurrentqueue.git
  GIT_TAG        v1.0.3
)

FetchContent_MakeAvailable(moodycamel_concurrentqueue)

# Link concurrentqueue to core
target_link_libraries(orderbook_core PRIVATE concurrentqueue)

# -------------------------------------------------------------------
# Main executable
# -------------------------------------------------------------------

add_executable(OrderBookEngine
    Sources/main.cpp
)

target_link_libraries(OrderBookEngine PRIVATE orderbook_core concurrentqueue)

# -------------------------------------------------------------------
# Benchmarks
# -------------------------------------------------------------------

add_executable(benchmark_orders
    benchmarks/benchmark_orders.cpp
)
add_executable(benchmark_orders_mt_v2
    benchmarks/benchmark_orders_mt_v2.cpp
)
add_executable(benchmark_latency
    benchmarks/benchmark_latency.cpp
)

target_link_libraries(benchmark_orders PRIVATE orderbook_core concurrentqueue)
target_link_libraries(benchmark_orders_mt_v2 PRIVATE orderbook_core concurrentqueue)
target_link_libraries(benchmark_latency PRIVATE orderbook_core concurrentqueue)

# -------------------------------------------------------------------
# Tests
# -------------------------------------------------------------------

enable_testing()

add_executable(OrderBookEngine_tests
    Tests/orderbook_market_test.cpp
    Tests/orderbook_limit_test.cpp
)

target_link_libraries(OrderBookEngine_tests
    PRIVATE
      orderbook_core
      concurrentqueue
      GTest::gtest_main
)

add_test(NAME AllTests COMMAND OrderBookEngine_tests)
