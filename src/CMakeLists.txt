
set(
  ${TRANSMITRON_BIN_NAME}_SOURCE

  Arguments.cpp
  Common/Console.cpp
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
  GUI/App.cpp
  GUI/ArtProvider.cpp
  GUI/Events/Connection.cpp
  GUI/Events/Edit.cpp
  GUI/Events/Layout.cpp
  GUI/Events/Profile.cpp
  GUI/Events/Recording.cpp
  GUI/Events/Subscription.cpp
  GUI/Events/TopicCtrl.cpp
  GUI/Models/FsTree.cpp
  GUI/Models/History.cpp
  GUI/Models/KnownTopics.cpp
  GUI/Models/Layouts.cpp
  GUI/Models/Messages.cpp
  GUI/Models/Profiles.cpp
  GUI/Models/ProfilesWrapper.cpp
  GUI/Models/Subscriptions.cpp
  GUI/Notifiers/Layouts.cpp
  GUI/Resources/history/history-18x14.cpp
  GUI/Resources/messages/messages-18x14.cpp
  GUI/Resources/pin/not-pinned-18x18.cpp
  GUI/Resources/pin/pinned-18x18.cpp
  GUI/Resources/preview/preview-18x14.cpp
  GUI/Resources/qos/qos-0.cpp
  GUI/Resources/qos/qos-1.cpp
  GUI/Resources/qos/qos-2.cpp
  GUI/Resources/send/send-18x14.cpp
  GUI/Resources/subscription/subscription-18x14.cpp
  GUI/Tabs/Client.cpp
  GUI/Tabs/Homepage.cpp
  GUI/Tabs/Settings.cpp
  GUI/Types/ClientOptions.cpp
  GUI/Types/Subscription.cpp
  GUI/Widgets/Edit.cpp
  GUI/Widgets/Layouts.cpp
  GUI/Widgets/TopicCtrl.cpp
  MQTT/BrokerOptions.cpp
  MQTT/Client.cpp
  MQTT/Message.cpp
  MQTT/Subscription.cpp
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

  set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -static -static-libstdc++")
  set(CMAKE_C_LINK_FLAGS   "${CMAKE_C_LINK_FLAGS}   -static -static-libgcc")

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
    CLI11::CLI11
    PahoMqttCpp::paho-mqttpp3-static
    Threads::Threads
    date::date
    fmt::fmt
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    stdc++fs
    tinyxml2::tinyxml2
    wxWidgets::wxWidgets
)
