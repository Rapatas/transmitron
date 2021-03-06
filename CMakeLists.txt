cmake_minimum_required(VERSION 3.5)

project(
  transmitron 
  VERSION 0.0.0
)

set(INFO_PRE_CONF ./src/Transmitron/Info.in.cpp)
set(INFO_POST_CONF ${CMAKE_CURRENT_BINARY_DIR}/Transmitron/Info.cpp)

add_executable(
  ${PROJECT_NAME}
  ${INFO_POST_CONF}
  src/Helpers/Helpers.cpp
  src/MQTT/BrokerOptions.cpp
  src/MQTT/Client.cpp
  src/MQTT/Message.cpp
  src/MQTT/Subscription.cpp
  src/Transmitron/App.cpp
  src/Transmitron/Models/History.cpp
  src/Transmitron/Models/Layouts.cpp
  src/Transmitron/Models/Profiles.cpp
  src/Transmitron/Models/Snippets.cpp
  src/Transmitron/Models/Subscriptions.cpp
  src/Transmitron/Notifiers/Layouts.cpp
  src/Transmitron/Resources/history/history-18x14.cpp
  src/Transmitron/Resources/history/history-18x18.cpp
  src/Transmitron/Resources/pin/not-pinned-18x18.cpp
  src/Transmitron/Resources/pin/pinned-18x18.cpp
  src/Transmitron/Resources/plus/plus-18x18.cpp
  src/Transmitron/Resources/preview/preview-18x14.cpp
  src/Transmitron/Resources/preview/preview-18x18.cpp
  src/Transmitron/Resources/qos/qos-0.cpp
  src/Transmitron/Resources/qos/qos-1.cpp
  src/Transmitron/Resources/qos/qos-2.cpp
  src/Transmitron/Resources/send/send-18x14.cpp
  src/Transmitron/Resources/send/send-18x18.cpp
  src/Transmitron/Resources/snippets/snippets-18x14.cpp
  src/Transmitron/Resources/snippets/snippets-18x18.cpp
  src/Transmitron/Resources/subscription/subscription-18x14.cpp
  src/Transmitron/Resources/subscription/subscription-18x18.cpp
  src/Transmitron/Tabs/Client.cpp
  src/Transmitron/Tabs/Homepage.cpp
  src/Transmitron/Tabs/Settings.cpp
  src/Transmitron/Types/Subscription.cpp
  src/Transmitron/Widgets/Edit.cpp
  src/Transmitron/Widgets/Layouts.cpp
  src/Transmitron/Widgets/TopicCtrl.cpp
  src/main.cpp
)

configure_file(
  ${INFO_PRE_CONF}
  ${INFO_POST_CONF}
)

target_compile_features(
  ${PROJECT_NAME}
  PRIVATE
    cxx_std_17
)

target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    ./src
)

set(gcc_warnings
  -Werror
  -Wall
  -Wextra
  -Wconversion
  -Wsign-conversion
  -Wfloat-conversion
  -Wpedantic
)

target_compile_options(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>: ${gcc_warnings}>
    $<$<CXX_COMPILER_ID:Clang>: ${gcc_warnings}>
)

target_compile_definitions(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    wxUSE_THREADS=1
)

find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(wxwidgets REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(cppcodec REQUIRED)
find_package(fmt REQUIRED)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    Threads::Threads
    PahoMqttCpp::PahoMqttCpp
    wxwidgets::wxwidgets
    nlohmann_json::nlohmann_json
    tinyxml2::tinyxml2
    cppcodec::cppcodec
    fmt::fmt
    stdc++fs
)
