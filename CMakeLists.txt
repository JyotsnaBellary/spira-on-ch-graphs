cmake_minimum_required(VERSION 3.20)
project(spira_on_ch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable
add_executable(spira
    src/main.cpp
    algorithms/bfs.cpp
    algorithms/pre-processing/witness-dijkstra.cpp
    algorithms/pre-processing/contraction-hierarchy.cpp
    algorithms/shortest-path/ch-dijkstra.cpp
    algorithms/shortest-path/dijkstra.cpp
    algorithms/shortest-path/spira.cpp
    algorithms/shortest-path/new-variant.cpp
    evaluations/benchmark_tests.cpp
    # data-structures/graph.cpp
    # data-structures/ch-graph.cpp
    utils/file-handler.cpp
    utils/utilities.cpp
    utils/kahip-runner.cpp
)

target_include_directories(spira PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/algorithms
                                         ${CMAKE_CURRENT_SOURCE_DIR}/data-structures
                                         ${CMAKE_CURRENT_SOURCE_DIR}/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/evaluations
                                         ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# KaHIP configuration
set(KAHIP_HOME "" CACHE PATH "Root of KaHIP (contains deploy/ or build/)")
set(KAHIP_NODE_ORDERING "" CACHE FILEPATH "Full path to KaHIP node_ordering (optional override)")

if(NOT KAHIP_HOME AND DEFINED ENV{KAHIP_HOME})
  set(KAHIP_HOME "$ENV{KAHIP_HOME}")
endif()
if(NOT KAHIP_NODE_ORDERING AND DEFINED ENV{KAHIP_NODE_ORDERING})
  set(KAHIP_NODE_ORDERING "$ENV{KAHIP_NODE_ORDERING}")
endif()

if(NOT KAHIP_NODE_ORDERING)
  find_program(
    KAHIP_NODE_ORDERING
    NAMES node_ordering
    HINTS ${KAHIP_HOME}
    PATH_SUFFIXES deploy bin build build/bin build/parallel/parallel_src
    REQUIRED
  )
endif()

message(STATUS "KaHIP node_ordering: ${KAHIP_NODE_ORDERING}")

target_compile_definitions(spira PRIVATE
  KAHIP_NODE_ORDERING_PATH=\"${KAHIP_NODE_ORDERING}\"
  ERP_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

# # Library target for your code
# add_library(sssp_lib
#     Algorithms/dijkstra.cpp
#     Algorithms/spira.cpp
#     Algorithms/new_variant.cpp
#     Graph/graph.cpp
#     Utils/file_handler.cpp
#     Evaluations/basic_test.cpp
#     Evaluations/benchmark_tests.cpp
#     shortest-path-visualizer/visualizer.cpp
#     shortest-path-visualizer/interactive_cli.cpp
# )

# # Use C++20; set per-target (best practice)
# target_compile_features(sssp_lib PUBLIC cxx_std_20)

# # target_include_directories(sssp_lib PUBLIC ${CMAKE_SOURCE_DIR}/Include)
# target_include_directories(sssp_lib PUBLIC
#     ${CMAKE_SOURCE_DIR}/Graph
#     ${CMAKE_SOURCE_DIR}/Algorithms
#     ${CMAKE_SOURCE_DIR}/Include  
#     ${CMAKE_SOURCE_DIR}/Utils   
#     ${CMAKE_SOURCE_DIR}/Evaluations   
#     ${CMAKE_SOURCE_DIR}/shortest-path-visualizer   
#     # ${CMAKE_SOURCE_DIR}/src   
# )

# # CLI app
# add_executable(sssp_cli src/main.cpp)
# target_compile_features(sssp_cli PRIVATE cxx_std_20)
# target_link_libraries(sssp_cli PRIVATE sssp_lib)

# # ---- Warnings (both targets) ----
# if (SSSP_ENABLE_WARNINGS)
#     if (MSVC)
#         target_compile_options(sssp_lib PRIVATE /W4 /permissive-)
#         target_compile_options(sssp_cli PRIVATE /W4 /permissive-)
#     else()
#         target_compile_options(sssp_lib PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
#         target_compile_options(sssp_cli PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
#     endif()
# endif()

# # ---- Debug flags & Sanitizers (GCC/Clang only, Debug only) ----
# if (SSSP_ENABLE_SANITIZERS AND NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
#   foreach(tgt sssp_lib sssp_cli)
#     # Nice debug experience
#     target_compile_options(${tgt} PRIVATE
#       $<$<CONFIG:Debug>:-g -O0 -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS>
#     )
#     target_link_options(${tgt} PRIVATE
#       $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
#     )
#     # Sanitizers
#     target_compile_options(${tgt} PRIVATE
#       $<$<CONFIG:Debug>:-fsanitize=address,undefined>
#     )
#     target_link_options(${tgt} PRIVATE
#       $<$<CONFIG:Debug>:-fsanitize=address,undefined>
#     )
#   endforeach()
# endif()