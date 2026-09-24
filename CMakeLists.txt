cmake_minimum_required(VERSION 3.25)
project(MANMENMI VERSION 0.1.0 LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME MATCHES "^(Windows|Linux)$")
  message(FATAL_ERROR "MANMENMI targets Windows/Linux only. macOS is not supported.")
endif()
option(MANMENMI_ALLOW_UNSUPPORTED_HOST "Allow non-x64 foundation tests; NOT platform support" OFF)
if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(AMD64|amd64|x86_64|X86_64)$" AND NOT MANMENMI_ALLOW_UNSUPPORTED_HOST)
  message(FATAL_ERROR "x64 required. Non-x64 foundation experiments require MANMENMI_ALLOW_UNSUPPORTED_HOST=ON.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "MANMENMI requires a 64-bit toolchain.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(MANMENMI_WARNINGS_AS_ERRORS "Treat project warnings as errors" ON)
option(MANMENMI_COVERAGE "Instrument project for LLVM source coverage" OFF)
option(MANMENMI_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
include(CTest)
include(FetchContent)
include(cmake/ProjectOptions.cmake)
find_package(Threads REQUIRED)

find_package(SDL3 CONFIG QUIET)
if(NOT TARGET SDL3::SDL3)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  set(SDL_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    sdl3
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG "release-3.2.14"
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(sdl3)
endif()

add_library(manmenmi_core STATIC
  core/src/backend.cpp core/src/config.cpp core/src/options.cpp core/src/log.cpp core/src/result.cpp core/src/runtime.cpp)
add_library(manmenmi::core ALIAS manmenmi_core)
target_include_directories(manmenmi_core PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/core/include>")
target_compile_features(manmenmi_core PUBLIC cxx_std_20)
target_link_libraries(manmenmi_core PUBLIC Threads::Threads PRIVATE manmenmi_options)

add_library(manmenmi_window INTERFACE)
add_library(manmenmi::window ALIAS manmenmi_window)
target_include_directories(manmenmi_window INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/window/include>")
add_library(manmenmi_window_native STATIC
  window/src/sdl_window.cpp)
target_include_directories(manmenmi_window_native
  PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/window/include>"
  PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/window/src>")
target_link_libraries(manmenmi_window_native PUBLIC manmenmi::core PRIVATE SDL3::SDL3)
target_link_libraries(manmenmi_window INTERFACE manmenmi_window_native)
add_library(manmenmi_graphics STATIC
  graphics/src/graphics.cpp)
add_library(manmenmi::graphics ALIAS manmenmi_graphics)
target_include_directories(manmenmi_graphics PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/graphics/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/window/include>")
target_link_libraries(manmenmi_graphics PUBLIC manmenmi::core PRIVATE SDL3::SDL3)
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
  target_compile_definitions(manmenmi_graphics PRIVATE MANMENMI_HAS_VULKAN=1)
  target_link_libraries(manmenmi_graphics PRIVATE Vulkan::Vulkan)
endif()
if(WIN32)
  target_compile_definitions(manmenmi_graphics PRIVATE MANMENMI_HAS_DX12=1)
  target_link_libraries(manmenmi_graphics PRIVATE d3d12 dxgi d3dcompiler)
endif()

add_library(manmenmi_gx2 STATIC
  gx2/src/gx2.cpp)
add_library(manmenmi::gx2 ALIAS manmenmi_gx2)
target_include_directories(manmenmi_gx2 PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/gx2/include>")
target_link_libraries(manmenmi_gx2 PUBLIC manmenmi::graphics)

add_executable(manmenmi_probe tools/probe/main.cpp)
target_link_libraries(manmenmi_probe PRIVATE manmenmi::core manmenmi_options)
target_compile_definitions(manmenmi_probe PRIVATE MANMENMI_VERSION="${PROJECT_VERSION}")
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
message(STATUS "M1.1 native window backend enabled privately via SDL3; no public SDL3 API leak")