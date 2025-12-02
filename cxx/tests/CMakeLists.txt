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
file(GLOB_RECURSE dateutils_src "${CMAKE_CURRENT_SOURCE_DIR}/dateutils.cpp")
add_testcase(dateutils "${dateutils_src}")

# serialization_buffer
file(GLOB_RECURSE serialization_buffer_src
     "${CMAKE_CURRENT_SOURCE_DIR}/serialization_buffer.cpp")
add_testcase(serialization_buffer "${serialization_buffer_src}")

# axis
file(GLOB_RECURSE axis_src "${CMAKE_CURRENT_SOURCE_DIR}/math/axis/*.cpp")
add_testcase(axis "${axis_src}")

# # test_math file(GLOB_RECURSE math_src
# "${CMAKE_CURRENT_SOURCE_DIR}/math/*.cpp") add_testcase(math "${math_src}"
# BLAS::BLAS)

# # test_geodetic file(GLOB_RECURSE geodetic_src
# "${CMAKE_CURRENT_SOURCE_DIR}/geodetic/*.cpp") add_testcase(geodetic
# "${geodetic_src}")

# # test_geometry file(GLOB_RECURSE geometry_src
# "${CMAKE_CURRENT_SOURCE_DIR}/geometry/*.cpp") add_testcase(geometry
# "${geometry_src}" BLAS::BLAS)

# add_testcase(axis_container) add_testcase(axis)
# add_testcase(geodetic_coordinates) add_testcase(geodetic_system)
# add_testcase(geometry_rtree ${BLAS_LIBRARIES})
# add_testcase(interpolation_akima) add_testcase(interpolation_bicubic
# ${BLAS_LIBRARIES}) add_testcase(interpolation_bilinear)
# add_testcase(interpolation_cspline ${BLAS_LIBRARIES})
# add_testcase(interpolation_linear) add_testcase(interpolation_polynomial)
# add_testcase(interpolation_search) add_testcase(interpolation_steffen)
# add_testcase(math_bicubic ${BLAS_LIBRARIES}) add_testcase(math_binning)
# add_testcase(math_bivariate ${BLAS_LIBRARIES}) add_testcase(math_dct2d
# ${FFT_LIBRARIES}) add_testcase(math_descriptive_statistics)
# add_testcase(math_fft2d ${FFT_LIBRARIES}) add_testcase(math_kriging
# ${BLAS_LIBRARIES}) add_testcase(math_linear) add_testcase(math_rbf
# ${BLAS_LIBRARIES}) add_testcase(math_spline ${BLAS_LIBRARIES})
# add_testcase(math_streaming_histogram) add_testcase(math_trivariate)
# add_testcase(math_window_function) add_testcase(math) add_testcase(thread)
