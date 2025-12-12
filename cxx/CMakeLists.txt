# Copyright (c) 2025 CNES
#
# All rights reserved. Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
include(ProcessorCount)
include_directories(include)

ProcessorCount(NUM_CORES)

# Create the pure C++ library used by pybind
file(GLOB_RECURSE LIB_SRC "src/library/*.cpp")
add_library(pyinterp ${LIB_SRC})
target_include_directories(pyinterp
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE NB_SRC "src/pybind/*.cpp")
nanobind_add_module(core ${NB_SRC})
target_link_libraries(
  core
  PRIVATE BLAS::BLAS pyinterp
  PUBLIC cpp_coverage)

if(GTest_FOUND)
  add_subdirectory(tests)
endif()
