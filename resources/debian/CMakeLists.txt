if (UNIX AND NOT APPLE)

  set(DEB_POSTINST_PRE_CONF "${CMAKE_CURRENT_SOURCE_DIR}/postinst.in.sh")
  set(DEB_POSTINST_POST_CONF "${CMAKE_BINARY_DIR}/postinst")
  set(DEB_POSTINST_POST_CONF ${DEB_POSTINST_POST_CONF} PARENT_SCOPE)
  configure_file(${DEB_POSTINST_PRE_CONF} ${DEB_POSTINST_POST_CONF} @ONLY)

  install(CODE "
    include(CPack)
    configure_file(${DEB_POSTINST_PRE_CONF} ${DEB_POSTINST_POST_CONF} @ONLY)
  ")

  install(
    FILES
      mimetypes-transmitron.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages/
  )

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

  install(
    FILES
      transmitron.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/mimetypes
    RENAME text-transmitron.svg
  )

endif ()
