cmake_minimum_required(VERSION 3.25)
if(NOT DEFINED DEST)
  message(FATAL_ERROR "Pass DEST for the isolated negative fixture")
endif()
file(MAKE_DIRECTORY "${DEST}")
file(WRITE "${DEST}/CTestTestfile.cmake"
  "add_test(sentinel.forced_failure \"${CMAKE_COMMAND}\" -E false)\n")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DBUILD_DIR=${DEST}" -DEXPECTED_TESTS=1
  -P "${CMAKE_CURRENT_LIST_DIR}/RunTests.cmake" RESULT_VARIABLE result
  OUTPUT_VARIABLE out ERROR_VARIABLE err)
file(WRITE "${DEST}/failure-propagation.txt" "wrapper_exit=${result}\n${out}\n${err}")
if("${result}" STREQUAL "0" OR NOT err MATCHES "M0 CTest failed" OR
    NOT out MATCHES "sentinel.forced_failure" OR NOT out MATCHES "1 tests failed")
  message(FATAL_ERROR "Negative control did not demonstrate propagation:\n${out}\n${err}")
endif()
message(STATUS "VERIFIED: failed CTest propagates nonzero through the production wrapper; not a remote job result")