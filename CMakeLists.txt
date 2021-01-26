cmake_minimum_required(VERSION 3.5)

project(
  transmitron 
  VERSION 0.0.0
)

set(INFO_PRE_CONF ./src/Info.in.cpp)
set(INFO_POST_CONF ${CMAKE_CURRENT_BINARY_DIR}/Info.cpp)

add_executable(
  ${PROJECT_NAME}
  ${INFO_POST_CONF}
  ./src/Client.cpp
  ./src/Connection.cpp
  ./src/Homepage.cpp
  ./src/MQTT/Client.cpp
  ./src/MQTT/Subscription.cpp
  ./src/Models/Connections.cpp
  ./src/Models/History.cpp
  ./src/Models/Subscriptions.cpp
  ./src/Models/SubscriptionData.cpp
  ./src/Transmitron.cpp
  ./src/Widgets/Edit.cpp
  ./src/images/pin/not-pinned-18x18.cpp
  ./src/images/pin/pinned-18x18.cpp
  ./src/images/qos/qos-0.cpp
  ./src/images/qos/qos-1.cpp
  ./src/images/qos/qos-2.cpp
  ./src/images/plus/plus-18x18.cpp
  ./src/images/send/send-18x18.cpp
  ./src/main.cpp
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
