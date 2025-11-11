cmake_minimum_required(VERSION 3.20)
project(sssp LANGUAGES CXX)

# Options
option(SSSP_ENABLE_WARNINGS "Enable warnings" ON)
option(SSSP_ENABLE_SANITIZERS "Enable Address/UB sanitizers in Debug" ON)

# For tooling (clangd, VSCode)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Library target for your code
add_library(sssp_lib
    Algorithms/dijkstra.cpp
    Algorithms/spira.cpp
    Algorithms/new_variant.cpp
    Graph/graph.cpp
    Utils/file_handler.cpp
    Evaluations/basic_test.cpp
    Evaluations/benchmark_tests.cpp

)

# Use C++20; set per-target (best practice)
target_compile_features(sssp_lib PUBLIC cxx_std_20)

# target_include_directories(sssp_lib PUBLIC ${CMAKE_SOURCE_DIR}/Include)
target_include_directories(sssp_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/Graph
    ${CMAKE_SOURCE_DIR}/Algorithms
    ${CMAKE_SOURCE_DIR}/Include  
    ${CMAKE_SOURCE_DIR}/Utils   
    ${CMAKE_SOURCE_DIR}/Evaluations   
)

# CLI app
add_executable(sssp_cli src/main.cpp)
target_compile_features(sssp_cli PRIVATE cxx_std_20)
target_link_libraries(sssp_cli PRIVATE sssp_lib)

# ---- Warnings (both targets) ----
if (SSSP_ENABLE_WARNINGS)
    if (MSVC)
        target_compile_options(sssp_lib PRIVATE /W4 /permissive-)
        target_compile_options(sssp_cli PRIVATE /W4 /permissive-)
    else()
        target_compile_options(sssp_lib PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
        target_compile_options(sssp_cli PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endif()
endif()

# ---- Debug flags & Sanitizers (GCC/Clang only, Debug only) ----
if (SSSP_ENABLE_SANITIZERS AND NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  foreach(tgt sssp_lib sssp_cli)
    # Nice debug experience
    target_compile_options(${tgt} PRIVATE
      $<$<CONFIG:Debug>:-g -O0 -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS>
    )
    target_link_options(${tgt} PRIVATE
      $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    )
    # Sanitizers
    target_compile_options(${tgt} PRIVATE
      $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
    target_link_options(${tgt} PRIVATE
      $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
  endforeach()
endif()