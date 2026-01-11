# Copyright (c) 2026 CNES.
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
if(MSVC)
  target_compile_options(pyinterp PRIVATE /MD)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(pyinterp PRIVATE _ITERATOR_DEBUG_LEVEL=0)
  endif()
endif()

file(GLOB_RECURSE NB_SRC "src/pybind/*.cpp")

# Create an OBJECT library to enable parallel compilation on all platforms This
# is especially important for Windows where MSVC can better parallelize
# compilation when sources are in an OBJECT library
add_library(core_objects OBJECT ${NB_SRC})
target_include_directories(core_objects
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(core_objects PRIVATE BLAS::BLAS pyinterp nanobind-static)
if(MSVC)
  target_compile_options(core_objects PRIVATE /MD)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(core_objects PRIVATE _ITERATOR_DEBUG_LEVEL=0)
  endif()
endif()

# Now create the nanobind module using the precompiled object files
nanobind_add_module(core $<TARGET_OBJECTS:core_objects>)
target_link_libraries(
  core
  PRIVATE BLAS::BLAS pyinterp
  PUBLIC cpp_coverage)
if(MSVC)
  target_compile_options(core PRIVATE /MD)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(core PRIVATE _ITERATOR_DEBUG_LEVEL=0)
  endif()
endif()

if(GTest_FOUND)
  add_subdirectory(tests)
endif()
