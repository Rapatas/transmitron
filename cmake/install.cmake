include(GNUInstallDirs)

install(
  TARGETS
    ${PROJECT_NAME}
  RUNTIME
  DESTINATION bin
)

add_subdirectory(${CMAKE_SOURCE_DIR}/resources/debian)
add_subdirectory(${CMAKE_SOURCE_DIR}/resources/images)
