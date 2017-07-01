cmake_minimum_required(VERSION 2.6.2)

SET(CMAKE_C_COMPILER /usr/local/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/local/bin/g++)

project(GuiStuff)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

################################################################################
find_package(DanLib REQUIRED)

include_directories(${DanLib_INCLUDE_DIR})
################################################################################
find_package(wxWidgets REQUIRED)

include("${wxWidgets_USE_FILE}")

################################################################################
find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
################################################################################

include_directories(.)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

add_executable(
  GridDisplayerTest
  WIN32
  Tests/GridDisplayTest.cpp
  )

target_link_libraries(
  GridDisplayerTest
  ${wxWidgets_LIBRARIES}
  )

add_executable(
  DoOnGuiThread
  WIN32
  Tests/DoOnGuiThread.cpp
  )

target_link_libraries(
  DoOnGuiThread
  ${wxWidgets_LIBRARIES}
  )

add_library(
  GuiStuffLib
  GuiStuff/ScrollWindow.cpp
  )

target_link_libraries(
  DoOnGuiThread
  ${wxWidgets_LIBRARIES}
  )

add_executable(
  ScrollWindowTest
  Tests/ScrollWindowTest.cpp
  )

target_link_libraries(
  ScrollWindowTest
  GuiStuffLib
  ${wxWidgets_LIBRARIES}
  )

