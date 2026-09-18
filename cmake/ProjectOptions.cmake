add_library(manmenmi_options INTERFACE)
# Stable source/build paths permit comparisons of independent clean builds.
# This does not pin the host OS, standard library or linker distribution.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT MSVC)
  target_compile_options(manmenmi_options INTERFACE
    "-ffile-prefix-map=${PROJECT_SOURCE_DIR}=/manmenmi"
    "-ffile-prefix-map=${PROJECT_BINARY_DIR}=/manmenmi/build"
    "-fdebug-prefix-map=${PROJECT_SOURCE_DIR}=/manmenmi"
    "-fdebug-prefix-map=${PROJECT_BINARY_DIR}=/manmenmi/build"
    -fdebug-compilation-dir=/manmenmi/build)
  if(MINGW)
    target_link_options(manmenmi_options INTERFACE -Wl,--no-insert-timestamp)
  endif()
endif()
if(MSVC)
  target_compile_options(manmenmi_options INTERFACE /W4 /permissive- /EHsc /utf-8)
  if(MANMENMI_WARNINGS_AS_ERRORS)
    target_compile_options(manmenmi_options INTERFACE /WX)
  endif()
else()
  target_compile_options(manmenmi_options INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
  if(MANMENMI_WARNINGS_AS_ERRORS)
    target_compile_options(manmenmi_options INTERFACE -Werror)
  endif()
endif()
if(MANMENMI_COVERAGE OR MANMENMI_SANITIZERS)
  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "Coverage/sanitizers currently require Clang on Linux.")
  endif()
endif()
if(MANMENMI_COVERAGE)
  target_compile_options(manmenmi_options INTERFACE -fprofile-instr-generate -fcoverage-mapping)
  target_link_options(manmenmi_options INTERFACE -fprofile-instr-generate)
endif()
if(MANMENMI_SANITIZERS)
  target_compile_options(manmenmi_options INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(manmenmi_options INTERFACE -fsanitize=address,undefined)
endif()