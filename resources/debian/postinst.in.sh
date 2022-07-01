#!/bin/sh

update-mime-database @CMAKE_INSTALL_PREFIX@/share/mime
gtk-update-icon-cache @CMAKE_INSTALL_PREFIX@/share/icons/hicolor
