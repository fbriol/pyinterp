# Copyright (c) 2025 CNES
#
# All rights reserved. Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
enable_testing()

# Create a new test suite.
macro(ADD_TESTCASE testname sources)
  set(libraries ${ARGN})
  add_executable(test_${testname} ${sources})
  target_link_libraries(test_${testname} GTest::gtest_main ${libraries}
                        cpp_coverage)
  add_test(NAME test_${testname}
           COMMAND ${EXECUTABLE_OUTPUT_PATH}/test_${testname}
                   --gtest_output=xml:test_${testname}.xml)
  if(CODE_COVERAGE)
    add_coverage(test_${testname})
  endif()
endmacro()

# dateutils
add_testcase(dateutils "${CMAKE_CURRENT_SOURCE_DIR}/dateutils.cpp")

# serialization_buffer
add_testcase(serialization_buffer
             "${CMAKE_CURRENT_SOURCE_DIR}/serialization_buffer.cpp")
# axis
file(GLOB_RECURSE axis_src "${CMAKE_CURRENT_SOURCE_DIR}/math/axis/*.cpp")
add_testcase(axis "${axis_src}")

# interpolate
file(GLOB_RECURSE interpolate_src
     "${CMAKE_CURRENT_SOURCE_DIR}/math/interpolate/*.cpp")
add_testcase(interpolate "${interpolate_src}" BLAS::BLAS)

# statistics
file(GLOB_RECURSE statistics_src
     "${CMAKE_CURRENT_SOURCE_DIR}/math/statistics/*.cpp")
add_testcase(statistics "${statistics_src}")

# parallel_for
add_testcase(parallel_for "${CMAKE_CURRENT_SOURCE_DIR}/parallel_for.cpp")

# geometry
file(GLOB_RECURSE geometry_src "${CMAKE_CURRENT_SOURCE_DIR}/geometry/*.cpp")
add_testcase(geometry "${geometry_src}" BLAS::BLAS)
