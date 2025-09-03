cmake_minimum_required(VERSION 3.20)
project(erp LANGUAGES CXX)

# Defaults
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(ERP_ENABLE_WARNINGS "Enable warnings" ON)
option(ERP_ENABLE_SANITIZERS "Enable Address/UB sanitizers in Debug" ON)

# Library target for your code
add_library(erp_lib
    src/main_stub.cpp # placeholder; replaced later
    src/io/file_handler.cpp      
    src/algorithms/dijkstra.cpp
    src/algorithms/witness_dijkstra.cpp
    src/algorithms/ch_dijkstra.cpp
    src/algorithms/CH.cpp
    src/algorithms/CCH.cpp
    src/utils/utils.cpp
    src/utils/helpers.cpp
    # src/benchmarks.cpp
    src/utils/kahip_runner.cpp
)

target_include_directories(erp_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

if (ERP_ENABLE_WARNINGS)
    if (MSVC)
        target_compile_options(erp_lib PRIVATE /W4 /permissive-)
    else()
        target_compile_options(erp_lib PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endif()
endif()

if (ERP_ENABLE_SANITIZERS AND NOT MSVC)
    set(SANITIZERS "-fsanitize=address,undefined")
    target_compile_options(erp_lib PRIVATE $<$<CONFIG:Debug>:${SANITIZERS}>)
    target_link_options(erp_lib PRIVATE $<$<CONFIG:Debug>:${SANITIZERS}>)
    # explicitly link sanitizers too
    target_link_libraries(erp_lib PRIVATE $<$<CONFIG:Debug>:asan ubsan>)
endif()


# CLI app
add_executable(erp_cli src/main.cpp)
target_link_libraries(erp_cli PRIVATE erp_lib)

enable_testing()
find_package(GTest REQUIRED)

add_executable(runTests
    tests/test_graph.cpp
    # tests/test_dijkstra.cpp
)

target_link_libraries(runTests PRIVATE erp_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(runTests)


# To run Kahip program 
# set(KAHIP_HOME "" CACHE PATH "Root of KaHIP install/checkout (folder containing deploy/ or build/)")

# find_program(
#   KAHIP_NODE_SEPARATOR
#   NAMES node_separator
#   HINTS ${KAHIP_HOME}
#   PATH_SUFFIXES deploy build build/parallel/parallel_src
# )

# if(NOT KAHIP_NODE_SEPARATOR)
#   message(FATAL_ERROR "KaHIP node_separator not found. Set KAHIP_HOME or KAHIP_NODE_SEPARATOR.")
# endif()

# # After find_program(...) set KAHIP_NODE_SEPARATOR
# message(STATUS "KaHIP node_separator: ${KAHIP_NODE_SEPARATOR}")

# # Pass the absolute path into your C++ as a macro
# target_compile_definitions(erp_lib PUBLIC KAHIP_NODE_SEPARATOR_PATH="${KAHIP_NODE_SEPARATOR}")
# target_compile_definitions(erp_cli PRIVATE KAHIP_NODE_SEPARATOR_PATH="${KAHIP_NODE_SEPARATOR}")
# target_compile_definitions(runTests PRIVATE KAHIP_NODE_SEPARATOR_PATH="${KAHIP_NODE_SEPARATOR}")

# --- KaHIP setup (place this AFTER add_library/add_executable for your targets) ---

# User-settable cache entries
set(KAHIP_HOME "" CACHE PATH "Root of KaHIP (contains deploy/ or build/)")
set(KAHIP_NODE_ORDERING "" CACHE FILEPATH "Full path to KaHIP node_ordering (optional override)")

# Fallback to environment if cache not set (so build.sh can pass env vars)
if(NOT KAHIP_HOME AND DEFINED ENV{KAHIP_HOME})
  set(KAHIP_HOME "$ENV{KAHIP_HOME}")
endif()
if(NOT KAHIP_NODE_ORDERING AND DEFINED ENV{KAHIP_NODE_ORDERING})
  set(KAHIP_NODE_ORDERING "$ENV{KAHIP_NODE_ORDERING}")
endif()

# Locate the binary only if an explicit path wasn't provided
if(NOT KAHIP_NODE_ORDERING)
  find_program(
    KAHIP_NODE_ORDERING
    NAMES node_ordering
    HINTS ${KAHIP_HOME}
    PATH_SUFFIXES
      deploy
      bin
      build
      build/bin
      build/parallel/parallel_src
    REQUIRED
  )
endif()

message(STATUS "KaHIP node_ordering: ${KAHIP_NODE_ORDERING}")

# Expose values to your C++ as string literals (note the escaped quotes)
foreach(t IN ITEMS erp_lib erp_cli runTests)
  if(TARGET ${t})
    target_compile_definitions(${t} PRIVATE
      KAHIP_NODE_ORDERING_PATH=\"${KAHIP_NODE_ORDERING}\"
      ERP_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"
    )
  endif()
endforeach()