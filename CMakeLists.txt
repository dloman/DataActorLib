cmake_minimum_required(VERSION 2.6.2)

SET(CMAKE_C_COMPILER /usr/local/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/local/bin/g++)

project(DataCopier)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

find_package(wxWidgets REQUIRED)

include("${wxWidgets_USE_FILE}")

find_package(Boost COMPONENTS filesystem system REQUIRED)

include(${Boost_INCLUDE_DIR})

include_directories(.)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

message("wxdwidsfdgrt ${wxWidgets_LIBRARIES}")
add_executable(
  GridDisplayerTest
  WIN32
  Tests/GridDisplayTest.cpp
  )

target_link_libraries(
  GridDisplayerTest
  ${wxWidgets_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  )

