
include(GNUInstallDirs)

install(
  FILES
    ${CMAKE_SOURCE_DIR}/resources/images/transmitron.ico
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}
)


install(
  TARGETS
    ${PROJECT_NAME}
  RUNTIME
  DESTINATION bin
)

add_subdirectory(${CMAKE_SOURCE_DIR}/resources/desktop)
