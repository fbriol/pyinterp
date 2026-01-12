# Copyright (c) 2026 CNES.
#
# All rights reserved. Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
include(ProcessorCount)
include_directories(include)

ProcessorCount(NUM_CORES)

# Create the pure C++ library used by pybind
file(GLOB_RECURSE LIB_SRC "src/library/*.cpp")
add_library(pyinterp STATIC ${LIB_SRC})
target_include_directories(pyinterp
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(MSVC)
  target_compile_options(pyinterp PRIVATE /MD)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(pyinterp PRIVATE _ITERATOR_DEBUG_LEVEL=0)
  endif()
endif()

# Windows requires separate source file variables to avoid name conflicts
# between files in different directories
file(GLOB NB_SRC "src/pybind/*.cpp")
file(GLOB NB_SRC_GEOMETRY "src/pybind/geometry/*.cpp")
file(GLOB_RECURSE NB_SRC_FILL "src/pybind/fill/*.cpp")
file(GLOB_RECURSE NB_SRC_GEOHASH "src/pybind/geohash/*.cpp")
file(GLOB_RECURSE NB_SRC_GEOMETRIC "src/pybind/geometric/*.cpp")
file(GLOB_RECURSE NB_SRC_GEOMETRY_CARTESIAN
     "src/pybind/geometry/cartesian/*.cpp")
file(GLOB_RECURSE NB_SRC_GEOMETRY_GEOGRAPHIC
     "src/pybind/geometry/geographic/*.cpp")
file(GLOB_RECURSE NB_SRC_WINDOWED "src/pybind/windowed/*.cpp")

# Generate library for each folder
add_library(fill STATIC ${NB_SRC_FILL})
add_library(geohash STATIC ${NB_SRC_GEOHASH})
target_link_libraries(geohash PRIVATE pyinterp)
add_library(geometric STATIC ${NB_SRC_GEOMETRIC})
add_library(geometry STATIC ${NB_SRC_GEOMETRY})
add_library(geometry_cartesian STATIC ${NB_SRC_GEOMETRY_CARTESIAN})
add_library(geometry_geographic STATIC ${NB_SRC_GEOMETRY_GEOGRAPHIC})
add_library(windowed STATIC ${NB_SRC_WINDOWED})

file(GLOB_RECURSE NB_SRC "src/pybind/*.cpp")
nanobind_add_module(core ${NB_SRC})
target_link_libraries(
  core
  PRIVATE BLAS::BLAS
          fill
          geohash
          geometric
          geometry
          geometry_cartesian
          geometry_geographic
          windowed
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
