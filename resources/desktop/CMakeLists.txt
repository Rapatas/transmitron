if (UNIX AND NOT APPLE)

  set(DESKTOP_PRE_CONF "${CMAKE_CURRENT_SOURCE_DIR}/transmitron.in.desktop")
  set(DESKTOP_POST_CONF "${CMAKE_BINARY_DIR}/transmitron.desktop")
  configure_file(${DESKTOP_PRE_CONF} ${DESKTOP_POST_CONF} @ONLY)

	install(
    FILES
      ${DESKTOP_POST_CONF}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
  )

	install(
    FILES
      transmitron.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
  )

endif()
