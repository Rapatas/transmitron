# TODO:
# - No readme as welcome

if (WIN32)
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_SOURCE_GENERATOR "ZIP")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CPACK_GENERATOR "PackageMake")
  set(CPACK_SOURCE_GENERATOR "TGZ")
else ()
  set(CPACK_GENERATOR "DEB")
  set(CPACK_SOURCE_GENERATOR "TGZ")
endif ()

if(WIN32)
  set(CPACK_PACKAGE_INSTALL_DIRECTORY ${TRANSMITRON_NAME})
else()
  set(CPACK_PACKAGE_INSTALL_DIRECTORY ${TRANSMITRON_BIN_NAME})
endif()

set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})
set(CPACK_STRIP_FILES TRUE)
set(CPACK_WARN_ON_ABSOLUTE_INSTALL_DESTINATION TRUE)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROGRAM_DESCRIPTION})
set(CPACK_PACKAGE_EXECUTABLES ${TRANSMITRON_NAME} ${TRANSMITRON_BIN_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${TRANSMITRON_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TRANSMITRON_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${TRANSMITRON_VERSION_PATCH})
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_PACKAGE_CONTACT "Andy Rapatas <https://github.com/rapatas>")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/rapatas/transmitron")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION "${PROGRAM_DESCRIPTION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README  "${CMAKE_SOURCE_DIR}/README.md")

if(APPLE)
  set(CPACK_PACKAGE_VENDOR "Organisation")
else()
  set(CPACK_PACKAGE_VENDOR ${CPACK_PACKAGE_HOMEPAGE_URL})
endif()

string(CONCAT CPACK_PACKAGE_FILE_NAME
  ${TRANSMITRON_BIN_NAME}
  "_"
  ${TRANSMITRON_VERSION}
  "_"
  ${TRANSMITRON_ARCH}
)

# Source
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
set(
  CPACK_SOURCE_IGNORE_FILES
  ".drone.yml"
  ".cache"
  ".gitignore"
  "compile_commands.json"
  ".*.swp"
  "*.md"
  "build-*"
  "/[.]git/"
)

# NSIS (Windows .exe installer)
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/images/transmitron.ico")
set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_SOURCE_DIR}/resources/images/nsis/header.bmp")
set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}/resources/images/nsis/welcomefinish.bmp")
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/transmitron.exe")
set(CPACK_NSIS_COMPRESSOR "/SOLID lzma")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
set(CPACK_NSIS_MODIFY_PATH "ON")
set(CPACK_NSIS_HELP_LINK ${CPACK_PACKAGE_HOMEPAGE_URL})
set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_PACKAGE_HOMEPAGE_URL})
set(CPACK_NSIS_MENU_LINKS "${CPACK_PACKAGE_HOMEPAGE_URL}" "Transmitron Homepage")
set(CPACK_NSIS_PACKAGE_NAME ${TRANSMITRON_NAME})
set(CPACK_NSIS_DISPLAY_NAME ${TRANSMITRON_NAME})
set(CPACK_NSIS_CONTACT ${CPACK_PACKAGE_CONTACT})
set(CPACK_NSIS_COMPRESSOR "${CPACK_NSIS_COMPRESSOR}\n  SetCompressorDictSize 64")
set(CPACK_NSIS_COMPRESSOR "${CPACK_NSIS_COMPRESSOR}\n  BrandingText '${CPACK_PACKAGE_DESCRIPTION_SUMMARY}'")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
  WriteRegStr SHCTX 'SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\App Paths\\\\transmitron.exe' '' '$INSTDIR\\\\bin\\\\transmitron.exe'\n\
  WriteRegStr SHCTX 'SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\App Paths\\\\transmitron.exe' 'Path' '$INSTDIR\\\\bin'
")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
  DeleteRegKey SHCTX 'SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\App Paths\\\\transmitron.exe'
")
set(CPACK_NSIS_CREATE_ICONS_EXTRA "
  CreateShortCut \\\"$DESKTOP\\\\Transmitron.lnk\\\" \\\"$INSTDIR\\\\bin\\\\transmitron.exe\\\"
")
set(CPACK_NSIS_DELETE_ICONS_EXTRA "
  Delete \\\"$DESKTOP\\\\Transmitron.lnk\\\"
")

# DEB (Linux .deb bundle)
add_subdirectory(${CMAKE_SOURCE_DIR}/resources/debian)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk2.0-dev")
set(CPACK_DEBIAN_PACKAGE_SECTION "internet")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${DEB_POSTINST_POST_CONF})
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "## Features:

 Profiles: Store connections to brokers.
 Multiple Connections: Connect to multiple Profiles at the same time using tabs.
 Snippets: Store messages in a nested folder structure, ready to publish.
 Folding: For messages with nested data.
 Syntax highlight, detection & formatting: Supports JSON and XML.
 Flexible: Resize, drag, detach or hide each sidebar separately.
 Layouts: Store sidebar locations and sizes.
 XDG BaseDir: Respects the XDG Base Directory specification.
 Native UI: Using wxWidgets to integrate seamlessly with your desktop theme.
 Mute / Solo: Hide or isolate messages for each subscription.
 History Filter: Limit history using search terms.
 Cross-Platform: Built for Windows and Linux.
 "
)

include(CPack)
