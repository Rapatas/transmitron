cmake_minimum_required(VERSION 3.17)

project(transmitron)

set(PROGRAM_DESCRIPTION "Transmitron MQTT desktop client")

set(TRANSMITRON_VERSION_MAJOR 0)
set(TRANSMITRON_VERSION_MINOR 0)
set(TRANSMITRON_VERSION_PATCH 5)
set(TRANSMITRON_VERSION_POSTFIX "")

string(CONCAT TRANSMITRON_VERSION
  ${TRANSMITRON_VERSION_MAJOR}
  "."
  ${TRANSMITRON_VERSION_MINOR}
  "."
  ${TRANSMITRON_VERSION_PATCH}
  ${TRANSMITRON_VERSION_POSTFIX}
)
set(PROJECT_VERSION ${TRANSMITRON_VERSION})

set(TRANSMITRON_NAME "Transmitron")
set(TRANSMITRON_BIN_NAME "transmitron")

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
  set(TRANSMITRON_ARCH "amd64")
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
  set(TRANSMITRON_ARCH "i686")
else()
  set(TRANSMITRON_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(fmt REQUIRED)
find_package(CLI11 REQUIRED)
find_package(spdlog REQUIRED)
find_package(date REQUIRED)
find_package(wxWidgets REQUIRED
  COMPONENTS
    aui
    core
    propgrid
    richtext
    stc
)

include(${CMAKE_SOURCE_DIR}/cmake/clang-tidy.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/git-version.cmake)

add_subdirectory(src)

include(${CMAKE_SOURCE_DIR}/cmake/install.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/cpack.cmake)
