include(KaTestrophe)
include(GoogleTest)

function (xxxxx-1ing_set_kassert_flags xxxxx-1_TARGET_NAME)
    cmake_parse_arguments("xxxxx-1" "NO_EXCEPTION_MODE" "" "" ${ARGN})

    # Use global assertion level
    target_compile_definitions(${xxxxx-1_TARGET_NAME} PRIVATE -DKASSERT_ASSERTION_LEVEL=${KASSERT_ASSERTION_LEVEL})

    # Explicitly specify exception mode for tests, default to no exception mode
    if (NOT xxxxx-1_NO_EXCEPTION_MODE)
        target_compile_definitions(${xxxxx-1_TARGET_NAME} PRIVATE -DKASSERT_EXCEPTION_MODE)
    endif ()
endfunction ()

# Convenience wrapper for adding tests for xxxxx-1 this creates the target, links googletest and xxxxx-1ing, enables
# warnings and registers the test
#
# TARGET_NAME the target name FILES the files of the target
#
# example: xxxxx-1ing_register_test(mytarget FILES mytarget.cpp)
function (xxxxx-1ing_register_test xxxxx-1_TARGET_NAME)
    cmake_parse_arguments("xxxxx-1" "NO_GLIBCXX_DEBUG_CONTAINERS" "" "FILES" ${ARGN})
    add_executable(${xxxxx-1_TARGET_NAME} ${xxxxx-1_FILES})
    target_link_libraries(${xxxxx-1_TARGET_NAME} PRIVATE gtest gtest_main gmock xxxxx-1ing_base)
    target_compile_options(${xxxxx-1_TARGET_NAME} PRIVATE ${xxxxx-1_WARNING_FLAGS})
    gtest_discover_tests(${xxxxx-1_TARGET_NAME} WORKING_DIRECTORY ${PROJECT_DIR})
    xxxxx-1ing_set_kassert_flags(${xxxxx-1_TARGET_NAME} ${ARGN})
    if (NOT ${xxxxx-1_NO_GLIBCXX_DEBUG_CONTAINERS})
        target_compile_definitions(${xxxxx-1_TARGET_NAME} PRIVATE -D_GLIBCXX_DEBUG)
        target_compile_definitions(${xxxxx-1_TARGET_NAME} PRIVATE -D_GLIBCXX_DEBUG_PEDANTIC)
    endif ()

    if (xxxxx-1_TEST_ENABLE_SANITIZERS)
        target_compile_options(${xxxxx-1_TARGET_NAME} PRIVATE -fsanitize=address)
        target_link_options(${xxxxx-1_TARGET_NAME} PRIVATE -fsanitize=address)
        target_compile_options(${xxxxx-1_TARGET_NAME} PRIVATE -fsanitize=undefined)
        target_link_options(${xxxxx-1_TARGET_NAME} PRIVATE -fsanitize=undefined)
        target_compile_options(${xxxxx-1_TARGET_NAME} PRIVATE -fno-sanitize-recover=all)
        target_link_options(${xxxxx-1_TARGET_NAME} PRIVATE -fno-sanitize-recover=all)
    endif ()
endfunction ()

# Convenience wrapper for adding tests for xxxxx-1 which rely on MPI this creates the target, links googletest, xxxxx-1ing
# and MPI, enables warnings and registers the tests
#
# TARGET_NAME the target name FILES the files of the target CORES the number of MPI ranks to run the test for
#
# example: xxxxx-1ing_register_mpi_test(mytarget FILES mytarget.cpp CORES 1 2 4 8)
function (xxxxx-1ing_register_mpi_test xxxxx-1_TARGET_NAME)
    cmake_parse_arguments("xxxxx-1" "NO_GLIBCXX_DEBUG_CONTAINERS" "" "FILES;CORES" ${ARGN})
    katestrophe_add_test_executable(${xxxxx-1_TARGET_NAME} FILES ${xxxxx-1_FILES})
    target_link_libraries(${xxxxx-1_TARGET_NAME} PRIVATE xxxxx-1ing_base)
    if (xxxxx-1_TESTS_DISCOVER)
        katestrophe_add_mpi_test(
            ${xxxxx-1_TARGET_NAME}
            CORES ${xxxxx-1_CORES}
            DISCOVER_TESTS
        )
    else ()
        katestrophe_add_mpi_test(${xxxxx-1_TARGET_NAME} CORES ${xxxxx-1_CORES})
    endif ()
    xxxxx-1ing_set_kassert_flags(${xxxxx-1_TARGET_NAME} ${ARGN})
    if (NOT ${xxxxx-1_NO_GLIBCXX_DEBUG_CONTAINERS})
        target_compile_definitions(${xxxxx-1_TARGET_NAME} PRIVATE -D_GLIBCXX_DEBUG)
        target_compile_definitions(${xxxxx-1_TARGET_NAME} PRIVATE -D_GLIBCXX_DEBUG_PEDANTIC)
    endif ()
    if (xxxxx-1_TEST_ENABLE_SANITIZERS)
        target_compile_options(${xxxxx-1_TARGET_NAME} PRIVATE -fsanitize=undefined)
        target_link_options(${xxxxx-1_TARGET_NAME} PRIVATE -fsanitize=undefined)
        target_compile_options(${xxxxx-1_TARGET_NAME} PRIVATE -fsanitize=address)
        target_link_options(${xxxxx-1_TARGET_NAME} PRIVATE -fsanitize=address)
        target_compile_options(${xxxxx-1_TARGET_NAME} PRIVATE -fno-sanitize-recover=all)
        target_link_options(${xxxxx-1_TARGET_NAME} PRIVATE -fno-sanitize-recover=all)
    endif ()
endfunction ()

# Convenience wrapper for registering a set of tests that should fail to compile and require xxxxx-1 to be linked.
#
# TARGET prefix for the targets to be built FILES the list of files to include in the target SECTIONS sections of the
# compilation test to build
#
function (xxxxx-1ing_register_compilation_failure_test xxxxx-1_TARGET_NAME)
    cmake_parse_arguments("xxxxx-1" "NO_EXCEPTION_MODE" "" "FILES;SECTIONS" ${ARGN})
    katestrophe_add_compilation_failure_test(
        TARGET ${xxxxx-1_TARGET_NAME}
        FILES ${xxxxx-1_FILES}
        SECTIONS ${xxxxx-1_SECTIONS}
        LIBRARIES xxxxx-1ing_base
    )
    xxxxx-1ing_set_kassert_flags(${xxxxx-1_TARGET_NAME} ${ARGN})
endfunction ()
