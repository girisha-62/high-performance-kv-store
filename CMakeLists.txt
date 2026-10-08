cmake_minimum_required(VERSION 3.16)
project(high_performance_kv_store VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(kv_core
    src/key_value_store.cpp
    src/thread_pool.cpp
    src/protocol.cpp
    src/persistence.cpp
)
target_include_directories(kv_core PUBLIC include)
target_compile_options(kv_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(kv_server src/server.cpp)
target_link_libraries(kv_server PRIVATE kv_core)
target_compile_options(kv_server PRIVATE -Wall -Wextra -Wpedantic)

add_executable(kv_client src/client.cpp)
target_link_libraries(kv_client PRIVATE kv_core)
target_compile_options(kv_client PRIVATE -Wall -Wextra -Wpedantic)

if(WIN32)
  target_link_libraries(kv_server PRIVATE ws2_32)
  target_link_libraries(kv_client PRIVATE ws2_32)
endif()

if(BUILD_TESTS)
  enable_testing()
  add_executable(kv_tests tests/test_store.cpp)
  target_link_libraries(kv_tests PRIVATE kv_core)
  add_test(NAME kv_tests COMMAND kv_tests)
endif()
