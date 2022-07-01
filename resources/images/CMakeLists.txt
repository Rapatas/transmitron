install(
  FILES
    ${CMAKE_SOURCE_DIR}/resources/images/transmitron.ico
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}
)

file(
  COPY ${CMAKE_SOURCE_DIR}/resources/images/transmitron.ico
  DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}
)

