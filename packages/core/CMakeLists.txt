cmake_minimum_required(VERSION 3.20)

project(lightgraph_core LANGUAGES CXX)

option(LIGHTGRAPH_CORE_BUILD_TESTS "Build Lightgraph core tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(
  GLOB_RECURSE LIGHTGRAPH_CORE_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/ofxColorTheory/src/*.cpp"
)

add_library(lightgraph_core STATIC ${LIGHTGRAPH_CORE_SOURCES})

target_include_directories(
  lightgraph_core
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/host/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(LIGHTGRAPH_CORE_BUILD_TESTS)
  enable_testing()

  add_executable(
    lightgraph_core_smoke
    tests/core_smoke_test.cpp
  )

  target_link_libraries(lightgraph_core_smoke PRIVATE lightgraph_core)

  add_test(NAME lightgraph_core_smoke COMMAND lightgraph_core_smoke)
endif()
