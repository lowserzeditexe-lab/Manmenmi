cmake_minimum_required(VERSION 3.25)
if(NOT DEFINED BUILD_DIR OR NOT DEFINED EXPECTED_TESTS)
  message(FATAL_ERROR "BUILD_DIR and EXPECTED_TESTS are required")
endif()
# Use CTest from the SAME CMake distribution, never a second PATH candidate.
get_filename_component(cmake_bin "${CMAKE_COMMAND}" DIRECTORY)
find_program(ctest NAMES ctest HINTS "${cmake_bin}" NO_DEFAULT_PATH REQUIRED)
execute_process(COMMAND "${ctest}" --test-dir "${BUILD_DIR}" --show-only=json-v1
  OUTPUT_VARIABLE inventory COMMAND_ERROR_IS_FATAL ANY)
string(JSON count LENGTH "${inventory}" tests)
if(NOT count EQUAL EXPECTED_TESTS)
  message(FATAL_ERROR "M0 test inventory mismatch: expected ${EXPECTED_TESTS}, got ${count}")
endif()
execute_process(COMMAND "${ctest}" --test-dir "${BUILD_DIR}" --no-tests=error
  --output-on-failure --output-junit results.xml RESULT_VARIABLE result)
if(NOT "${result}" STREQUAL "0")
  message(FATAL_ERROR "M0 CTest failed; exit=${result}. Failure must propagate to the CI job.")
endif()
message(STATUS "M0 CTest PASS: ${count}/${EXPECTED_TESTS}; this alone is not native CI evidence")