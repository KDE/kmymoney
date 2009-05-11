# This file has been taken from 'Rosegarden'
# A MIDI and audio sequencer and musical notation editor.
#
# This program is Copyright 2000-2008
#     Guillaume Laurent   <glaurent@telegraph-road.org>,
#     Chris Cannam        <cannam@all-day-breakfast.com>,
#     Richard Bown        <richard.bown@ferventsoftware.com>
#
# The moral rights of Guillaume Laurent, Chris Cannam, and Richard
# Bown to claim authorship of this work have been asserted.
#
# This file is Copyright 2006-2008
#     Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
#
# Other copyrights also apply to some parts of this work.  Please
# see the AUTHORS file and individual file headers for details.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.  See the file
# COPYING included with this distribution for more information.
# Find the kde-config program and retrieve the install dirs

# Variables:
#       HAVE_KDECONFIG
#       KDECONFIG_EXECUTABLE
#       KDE3PREFIX
#       KDE3HTMLDIR
#       KDE3DATADIR
#       KDE3ICONDIR
#       KDE3MIMEDIR
#       KDE3MENUDIR
#       KDE3EXECDIR
#       KDE3L18NDIR

IF(KDECONFIG_EXECUTABLE)
    SET(HAVE_KDECONFIG TRUE)
ELSE(KDECONFIG_EXECUTABLE)
    FIND_PROGRAM(KDECONFIG_EXECUTABLE NAMES kde-config PATHS
        $ENV{KDEDIR}/bin
        /opt/kde3/bin
        /opt/kde/bin
        NO_DEFAULT_PATH
    )
    FIND_PROGRAM(KDECONFIG_EXECUTABLE kde-config)
ENDIF(KDECONFIG_EXECUTABLE)

IF(NOT KDE3PREFIX)
    EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --version
                    OUTPUT_VARIABLE kde_config_version )
    STRING(REGEX MATCH "KDE: .\\." kde_version ${kde_config_version})
    IF (${kde_version} MATCHES "KDE: 3\\.")
        EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --prefix
                	OUTPUT_VARIABLE kdedir )
        STRING(REGEX REPLACE "\n" "" KDE3PREFIX "${kdedir}")
    ENDIF (${kde_version} MATCHES "KDE: 3\\.")
ENDIF(NOT KDE3PREFIX)

IF(NOT KDE3HTMLDIR)
    EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --expandvars --install html
        OUTPUT_VARIABLE _htmlinstalldir)
    STRING(REGEX REPLACE "\n" "" _htmlinstalldir "${_htmlinstalldir}")
    STRING(REPLACE "${KDE3PREFIX}/" "" KDE3HTMLDIR "${_htmlinstalldir}")
    MESSAGE(STATUS "KDE3HTMLDIR : ${KDE3HTMLDIR}")
ENDIF(NOT KDE3HTMLDIR)

IF(NOT KDE3DATADIR)
    EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --expandvars --install data
        OUTPUT_VARIABLE _datainstalldir)
    STRING(REGEX REPLACE "\n" "" _datainstalldir "${_datainstalldir}")
    STRING(REPLACE "${KDE3PREFIX}/" "" KDE3DATADIR "${_datainstalldir}")
    MESSAGE(STATUS "KDE3DATADIR : ${KDE3DATADIR}")
ENDIF(NOT KDE3DATADIR)

IF(NOT KDE3ICONDIR)
    EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --expandvars --install icon
        OUTPUT_VARIABLE _iconinstalldir)
    STRING(REGEX REPLACE "\n" "" _iconinstalldir "${_iconinstalldir}")
    STRING(REPLACE "${KDE3PREFIX}/" "" KDE3ICONDIR "${_iconinstalldir}")
    MESSAGE(STATUS "KDE3ICONDIR : ${KDE3ICONDIR}")
ENDIF(NOT KDE3ICONDIR)

IF(NOT KDE3MIMEDIR)
    EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --expandvars --install mime
        OUTPUT_VARIABLE _mimeinstalldir)
    STRING(REGEX REPLACE "\n" "" _mimeinstalldir "${_mimeinstalldir}")
    STRING(REPLACE "${KDE3PREFIX}/" "" KDE3MIMEDIR "${_mimeinstalldir}")
    MESSAGE(STATUS "KDE3MIMEDIR : ${KDE3MIMEDIR}")
ENDIF(NOT KDE3MIMEDIR)

IF(NOT KDE3MENUDIR)
    EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --expandvars --install xdgdata-apps
        OUTPUT_VARIABLE _menuinstalldir)
    STRING(REGEX REPLACE "\n" "" _menuinstalldir "${_menuinstalldir}")
    STRING(REPLACE "${KDE3PREFIX}/" "" KDE3MENUDIR "${_menuinstalldir}")
    MESSAGE(STATUS "KDE3MENUDIR : ${KDE3MENUDIR}")
ENDIF(NOT KDE3MENUDIR)

IF(NOT KDE3L18NDIR)
    EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --expandvars --install locale
        OUTPUT_VARIABLE _l18ninstalldir)
    STRING(REGEX REPLACE "\n" "" _l18ninstalldir "${_l18ninstalldir}")
    STRING(REPLACE "${KDE3PREFIX}/" "" KDE3L18NDIR "${_l18ninstalldir}")
    MESSAGE(STATUS "KDE3L18NDIR : ${KDE3L18NDIR}")
ENDIF(NOT KDE3L18NDIR)

IF(NOT KDE3EXECDIR)
    EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --expandvars --install exe
        OUTPUT_VARIABLE _execinstalldir)
    STRING(REGEX REPLACE "\n" "" _execinstalldir "${_execinstalldir}")
    STRING(REPLACE "${KDE3PREFIX}/" "" KDE3EXECDIR "${_execinstalldir}")
    MESSAGE(STATUS "KDE3EXECDIR : ${KDE3EXECDIR}")
ENDIF(NOT KDE3EXECDIR)
