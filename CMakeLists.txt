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
  src/MQTT/Client.cpp
  src/MQTT/Subscription.cpp
  src/Transmitron/Models/Connections.cpp
  src/Transmitron/Models/History.cpp
  src/Transmitron/Models/Subscriptions.cpp
  src/Transmitron/Models/Snippets.cpp
  src/Transmitron/Resources/pin/not-pinned-18x18.cpp
  src/Transmitron/Resources/pin/pinned-18x18.cpp
  src/Transmitron/Resources/plus/plus-18x18.cpp
  src/Transmitron/Resources/qos/qos-0.cpp
  src/Transmitron/Resources/qos/qos-1.cpp
  src/Transmitron/Resources/qos/qos-2.cpp
  src/Transmitron/Resources/send/send-18x18.cpp
  src/Transmitron/Tabs/Client.cpp
  src/Transmitron/Tabs/Homepage.cpp
  src/Transmitron/App.cpp
  src/Transmitron/Types/Connection.cpp
  src/Transmitron/Types/SubscriptionData.cpp
  src/Transmitron/Widgets/Edit.cpp
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
