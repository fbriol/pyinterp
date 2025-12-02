# Copyright (c) 2025 CNES
#
# All rights reserved. Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
include(ProcessorCount)
include_directories(include)

ProcessorCount(NUM_CORES)

# Create a header-only interface library for pyinterp
add_library(pyinterp INTERFACE)
target_include_directories(pyinterp
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES "pybind/*.cpp")
nanobind_add_module(core ${SOURCES})
target_link_libraries(
  core
  PRIVATE BLAS::BLAS
  PUBLIC cpp_coverage)

if(GTest_FOUND)
  add_subdirectory(tests)
endif()
