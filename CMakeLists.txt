cmake_minimum_required(VERSION 3.14...3.16 FATAL_ERROR)
project(server VERSION 0.1.0 LANGUAGES CXX)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.[hc]pp)

add_executable(server ${sources})

target_compile_features(server PRIVATE cxx_std_20)
target_compile_options(server PRIVATE /await)

target_include_directories(server PRIVATE src)
target_compile_definitions(server PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
  target_precompile_headers(server PRIVATE src/asio.hpp)
endif()

target_compile_definitions(server PRIVATE
  ASIO_HAS_CO_AWAIT
  ASIO_DISABLE_CONCEPTS
  ASIO_SEPARATE_COMPILATION
  _WIN32_WINNT=0x0601)

set(SILENCE_VS_WARNINGS ${CMAKE_C_COMPILER})
