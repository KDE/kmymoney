# Try to find qsql_sqlite.cpp and qsql_sqlite.h
# Once done this will define
#
#  QSQLITESOURCE_FOUND - system has qt sources with qslqlite
#  QSQLITESOURCE_SRCS_DIR - full path to qsql_sqlite.cpp
#  QSQLITESOURCE_INCLUDE_DIR - header files

# Copyright (c) 2014, Christian Dávid, <christian-david@web.de>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This file is usually in the source code you have to get from git
find_path(QSQLITESOURCE_SRCS_DIR qsql_sqlite.cpp
  DOC "Path to source dir of QSQLiteDriver"
  PATH_SUFFIXES
  src/sql/drivers/sqlite/
  PATHS
  ${CMAKE_SOURCE_DIR}/3rdparty/Qt
  ${QT_INCLUDE_DIR}
)

# To find this file you usually need to make the sources from git (this will create the necessary folder hierarchy)
find_path(QSQLITESOURCE_INCLUDE_DIR QtSql/private/qsqlcachedresult_p.h
  DOC "Path to qt include files with private headers"
  PATH_SUFFIXES
  include
  PATHS
  ${CMAKE_SOURCE_DIR}/3rdparty/Qt
  ${QT_INCLUDE_DIR}
)

set(QSQLITESOURCE_SRCS_DIRS ${QSQLITESOURCE_SRCS_DIR})
set(QSQLITESOURCE_INCLUDE_DIRS ${QSQLITESOURCE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QSQLiteSource "Could not find QSQLite source." QSQLITESOURCE_SRCS_DIR QSQLITESOURCE_INCLUDE_DIR)

mark_as_advanced(QSQLITESOURCE_SRCS_DIR QSQLITESOURCE_INCLUDE_DIR)
