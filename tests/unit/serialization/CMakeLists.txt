# Copyright (c) 2014 Bryce Adelstein-Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    serialization
    serialization_array
    serialization_builtins
    serialization_complex
    serialization_map
    serialization_smart_ptr
    serialization_vector
    serialization_variant
    serialize_buffer
    zero_copy_serialization
)

add_subdirectory(polymorphic)

foreach(test ${tests})
  set(sources
      ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add example executable
  add_hpx_executable(${test}_test
                     SOURCES ${sources}
                     ${${test}_FLAGS}
                     EXCLUDE_FROM_ALL
                     HPX_PREFIX ${HPX_BUILD_PREFIX}
                     FOLDER "Tests/Unit/Serialization/")

  add_hpx_unit_test("serialization" ${test} ${${test}_PARAMETERS})

  # add a custom target for this example
  add_hpx_pseudo_target(tests.unit.serialization.${test})

  # make pseudo-targets depend on master pseudo-target
  add_hpx_pseudo_dependencies(tests.unit.serialization
                              tests.unit.serialization.${test})

  # add dependencies to pseudo-target
  add_hpx_pseudo_dependencies(tests.unit.serialization.${test}
                              ${test}_test_exe)
endforeach()
