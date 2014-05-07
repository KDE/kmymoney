# - Try to find SqlCipher
# Once done this will define
#
#  SQLCIPHER_FOUND - system has SqlCipher
#  SQLCIPHER_INCLUDE_DIR - the SqlCipher include directory
#  SQLCIPHER_LIBRARIES - Link these to use SqlCipher
#  SQLCIPHER_DEFINITIONS - Compiler switches required for using SqlCipher
# Redistribution and use is allowed according to the terms of the BSD license.

# Copyright (c) 2008, Gilles Caulier, <caulier.gilles@gmail.com>
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
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
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

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if( NOT WIN32 )
  find_package(PkgConfig)

  pkg_check_modules(PC_SQLCIPHER QUIET sqlcipher)

  set(SQLCIPHER_DEFINITIONS ${PC_SQLCIPHER_CFLAGS_OTHER})
endif( NOT WIN32 )

find_path(SQLCIPHER_INCLUDE_DIR NAMES sqlcipher/sqlite3.h
  PATHS
  ${PC_SQLCIPHER_INCLUDEDIR}
  ${PC_SQLCIPHER_INCLUDE_DIRS}
  ${CMAKE_INCLUDE_PATH}
)

find_library(SQLCIPHER_LIBRARIES NAMES sqlcipher
  PATHS
  ${PC_SQLCIPHER_LIBDIR}
  ${PC_SQLCIPHER_LIBRARY_DIRS}
)

add_definitions(-DSQLITE_HAS_CODEC)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SqlCipher DEFAULT_MSG SQLCIPHER_INCLUDE_DIR SQLCIPHER_LIBRARIES )

# show the SQLCIPHER_INCLUDE_DIR and SQLCIPHER_LIBRARIES variables only in the advanced view
mark_as_advanced(SQLCIPHER_INCLUDE_DIR SQLCIPHER_LIBRARIES )

