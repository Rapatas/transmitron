
set(
  ${TRANSMITRON_BIN_NAME}_SOURCE

  Arguments.cpp
  Common/Env.Linux.cpp
  Common/Env.Windows.cpp
  Common/Extract.cpp
  Common/Helpers.cpp
  Common/Log.cpp
  Common/String.cpp
  Common/Url.cpp
  Common/XdgBaseDir.Linux.cpp
  Common/XdgBaseDir.Windows.cpp
  Common/XdgBaseDir.cpp
  Common/Console.cpp
  MQTT/BrokerOptions.cpp
  MQTT/Client.cpp
  MQTT/Message.cpp
  MQTT/Subscription.cpp
  Transmitron/App.cpp
  Transmitron/Models/History.cpp
  Transmitron/Models/KnownTopics.cpp
  Transmitron/Models/Layouts.cpp
  Transmitron/Models/Messages.cpp
  Transmitron/Models/Profiles.cpp
  Transmitron/Models/Subscriptions.cpp
  Transmitron/Notifiers/Layouts.cpp
  Transmitron/Resources/gear/gear-18x18.cpp
  Transmitron/Resources/history/history-18x14.cpp
  Transmitron/Resources/history/history-18x18.cpp
  Transmitron/Resources/messages/messages-18x14.cpp
  Transmitron/Resources/messages/messages-18x18.cpp
  Transmitron/Resources/pin/not-pinned-18x18.cpp
  Transmitron/Resources/pin/pinned-18x18.cpp
  Transmitron/Resources/plus/plus-18x18.cpp
  Transmitron/Resources/preview/preview-18x14.cpp
  Transmitron/Resources/preview/preview-18x18.cpp
  Transmitron/Resources/qos/qos-0.cpp
  Transmitron/Resources/qos/qos-1.cpp
  Transmitron/Resources/qos/qos-2.cpp
  Transmitron/Resources/send/send-18x14.cpp
  Transmitron/Resources/send/send-18x18.cpp
  Transmitron/Resources/subscription/subscription-18x14.cpp
  Transmitron/Resources/subscription/subscription-18x18.cpp
  Transmitron/Tabs/Client.cpp
  Transmitron/Tabs/Homepage.cpp
  Transmitron/Tabs/Settings.cpp
  Transmitron/Types/ClientOptions.cpp
  Transmitron/Types/Subscription.cpp
  Transmitron/Widgets/Edit.cpp
  Transmitron/Widgets/Layouts.cpp
  Transmitron/Widgets/TopicCtrl.cpp
  main.cpp

)

set(INFO_PRE_CONF "${CMAKE_CURRENT_SOURCE_DIR}/Common/Info.in.cpp")
set(INFO_POST_CONF "${CMAKE_BINARY_DIR}/Common/Info.cpp")
list(APPEND ${TRANSMITRON_BIN_NAME}_SOURCE ${INFO_POST_CONF})

set(VERSION_PRE_CONF "${CMAKE_CURRENT_SOURCE_DIR}/Common/Version.in.cpp")
set(VERSION_POST_CONF "${CMAKE_BINARY_DIR}/Common/Version.cpp")
list(APPEND ${TRANSMITRON_BIN_NAME}_SOURCE ${VERSION_POST_CONF})

configure_file(${INFO_PRE_CONF} ${INFO_POST_CONF} @ONLY)
configure_file(${VERSION_PRE_CONF} ${VERSION_POST_CONF} @ONLY)

if (WIN32)

  set(RESOURCE_PRE_CONF "${CMAKE_CURRENT_SOURCE_DIR}/transmitron.in.rc")
  set(RESOURCE_POST_CONF "${CMAKE_BINARY_DIR}/transmitron.rc")
  list(APPEND ${PROJECT_NAME}_SOURCE ${RESOURCE_POST_CONF})

  configure_file(
    ${RESOURCE_PRE_CONF}
    ${RESOURCE_POST_CONF}
    @ONLY
  )

endif()

add_executable(${TRANSMITRON_BIN_NAME} ${${TRANSMITRON_BIN_NAME}_SOURCE})

target_compile_features(
  ${TRANSMITRON_BIN_NAME}
  PRIVATE
    cxx_std_17
)

set_target_properties(
  ${TRANSMITRON_BIN_NAME}
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)

target_include_directories(${TRANSMITRON_BIN_NAME} PRIVATE . )

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
  ${TRANSMITRON_BIN_NAME}
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>: ${gcc_warnings}>
    $<$<CXX_COMPILER_ID:Clang>: ${gcc_warnings}>
)

target_compile_definitions(
  ${TRANSMITRON_BIN_NAME}
  PRIVATE
    wxUSE_THREADS=1
)

target_link_options(
  ${TRANSMITRON_BIN_NAME}
  PRIVATE
    $<$<AND:$<BOOL:${WIN32}>,$<CONFIG:Release>>:-mwindows>
)

target_link_libraries(
  ${TRANSMITRON_BIN_NAME}
  PRIVATE
    Threads::Threads
    PahoMqttCpp::PahoMqttCpp
    wxwidgets::wxwidgets
    nlohmann_json::nlohmann_json
    tinyxml2::tinyxml2
    fmt::fmt
    spdlog::spdlog
    CLI11::CLI11
    date::date
    stdc++fs
)
