cmake_minimum_required(VERSION 2.6.2)

SET(CMAKE_C_COMPILER /usr/local/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/local/bin/g++)

project(GuiStuff)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

set(MODULE_NAME GuiStuff)
add_definitions(-DMODULE_NAME=\"GuiStuff\")

add_definitions(-Wall)
add_definitions(-fPIC)


set(GuiStuff_VERSION_MAJOR 0)
set(GuiStuff_VERSION_MINOR 1)

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

################################################################################
add_executable(
  GridDisplayerTest
  WIN32
  Tests/GridDisplayTest.cpp
  )

target_link_libraries(
  GridDisplayerTest
  ${wxWidgets_LIBRARIES}
  )

################################################################################
add_executable(
  DoOnGuiThread
  WIN32
  Tests/DoOnGuiThread.cpp
  )

target_link_libraries(
  DoOnGuiThread
  ${wxWidgets_LIBRARIES}
  )

################################################################################
add_library(
  GuiStuffLib
  GuiStuff/ScrollWindow.cpp
  GuiStuff/PictureInPictureWindow.cpp
  )

target_link_libraries(
  GuiStuffLib
  ${wxWidgets_LIBRARIES}
  )

################################################################################
add_executable(
  ScrollWindowTest
  Tests/ScrollWindowTest.cpp
  )

target_link_libraries(
  ScrollWindowTest
  GuiStuffLib
  ${wxWidgets_LIBRARIES}
  )

################################################################################
add_executable(
  PictureInPictureWindowTest
  Tests/PictureInPictureWindowTest.cpp
  )

target_link_libraries(
  PictureInPictureWindowTest
  GuiStuffLib
  ${wxWidgets_LIBRARIES}
  )

################################################################################
# Install
################################################################################
set(GuiStuff_VERSION_STRING "${GuiStuff_VERSION_MAJOR}.${GuiStuff_VERSION_MINOR}")
set(GuiStuff_DIRNAME        "GuiStuff-${GuiStuff_VERSION_STRING}")

if (MSVC AND CMAKE_CL_64)
  set(GuiStuff_DIRNAME_lib "lib/${GuiStuff_DIRNAME}/x64")
elseif (MSVC)
  set(GuiStuff_DIRNAME_lib "lib/${GuiStuff_DIRNAME}/x86")
else ()
  set(GuiStuff_DIRNAME_lib "lib/${GuiStuff_DIRNAME}")
endif ()

set(GuiStuff_DIRNAME_include "include/${GuiStuff_DIRNAME}")

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GuiStuffConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/GuiStuffConfig.cmake"
  INSTALL_DESTINATION
    ${GuiStuff_DIRNAME_lib}
  PATH_VARS
    GuiStuff_DIRNAME_lib
    GuiStuff_DIRNAME_include
  )

WRITE_BASIC_PACKAGE_VERSION_FILE(
  "${CMAKE_CURRENT_BINARY_DIR}/GuiStuffConfigVersion.cmake"
  VERSION ${GuiStuff_VERSION_STRING}
  COMPATIBILITY AnyNewerVersion
  )

install(
TARGETS
  GuiStuffLib
DESTINATION
  ${GuiStuff_DIRNAME_lib}
EXPORT
  GuiStuffExports
  )

install(
  EXPORT
    GuiStuffExports
  DESTINATION
    ${GuiStuff_DIRNAME_lib}
  )

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/GuiStuffConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/GuiStuffConfigVersion.cmake
  DESTINATION
    ${GuiStuff_DIRNAME_lib}
  )

install(
  FILES
    GuiStuff/GridDisplayer.hpp
    GuiStuff/ScrollWindow.hpp
    GuiStuff/PictureInPictureWindow.hpp
    GuiStuff/Helpers.hpp
  DESTINATION
    ${GuiStuff_DIRNAME_include}/GuiStuff
  )
