#
# - Find tools needed for building RPM Packages
#   on Linux systems and defines macro that helps to
#   build source or binary RPM, the MACRO assumes
#   CMake 2.4.x which includes CPack support.
#   CPack is used to build tar.bz2 source tarball
#   which may be used by a custom user-made spec file.
#
# - Define RPMTools_ADD_RPM_TARGETS which defines
#   two (top-level) CUSTOM targets for building
#   source and binary RPMs
#
# Those CMake macros are provided by the TSP Developer Team
# https://savannah.nongnu.org/projects/tsp
#

IF(RPMTools_RPMBUILD_EXECUTABLE)
  SET(RPMTools_FIND_QUIETLY TRUE)
ENDIF(RPMTools_RPMBUILD_EXECUTABLE)

IF (WIN32)
  MESSAGE(STATUS "RPM tools not available on Win32 systems")
ENDIF(WIN32)

IF (UNIX)
  # Look for RPM builder executable
  FIND_PROGRAM(RPMTools_RPMBUILD_EXECUTABLE
    NAMES rpmbuild
    PATHS "/usr/bin;/usr/lib/rpm"
    PATH_SUFFIXES bin
    DOC "The RPM builder tool")
  MARK_AS_ADVANCED(RPMTools_RPMBUILD_EXECUTABLE)

  IF (RPMTools_RPMBUILD_EXECUTABLE)
    IF(NOT RPMTools_FIND_QUIETLY)
      MESSAGE(STATUS "Looking for RPMTools... - found rpmuild is ${RPMTools_RPMBUILD_EXECUTABLE}")
    ENDIF(NOT RPMTools_FIND_QUIETLY)
    SET(RPMTools_RPMBUILD_FOUND "YES")
    GET_FILENAME_COMPONENT(RPMTools_BINARY_DIRS ${RPMTools_RPMBUILD_EXECUTABLE} PATH)
  ELSE (RPMTools_RPMBUILD_EXECUTABLE)
    SET(RPMTools_RPMBUILD_FOUND "NO")
    IF(NOT RPMTools_FIND_QUIETLY)
      MESSAGE(STATUS "Looking for RPMTools... - rpmbuild NOT FOUND")
    ENDIF(NOT RPMTools_FIND_QUIETLY)
  ENDIF (RPMTools_RPMBUILD_EXECUTABLE)

  # Detect if CPack was included or not
  IF (NOT DEFINED "CPACK_PACKAGE_NAME")
    MESSAGE(FATAL_ERROR "CPack was not included, you should include CPack before Using RPMTools")
  ENDIF (NOT DEFINED "CPACK_PACKAGE_NAME")

  IF (RPMTools_RPMBUILD_FOUND)
    SET(RPMTools_FOUND TRUE)
    #
    # - first arg  (ARGV0) is RPM name
    # - second arg (ARGV1) is the RPM spec file path
    # - third arg  (ARGV2) is the RPM ROOT DIRECTORY used to build RPMs [optional]
    #
    MACRO(RPMTools_ADD_RPM_TARGETS RPMNAME SPECFILE_PATH)

      # Verify whether if RPM_ROOTDIR was provided or not
      IF("${ARGV2}" STREQUAL "")
        SET(RPM_ROOTDIR "${CMAKE_BINARY_DIR}/RPM")
      ELSE ("${ARGV2}" STREQUAL "")
        SET(RPM_ROOTDIR "${ARGV2}")
      ENDIF("${ARGV2}" STREQUAL "")
      IF(NOT RPMTools_FIND_QUIETLY)
        MESSAGE(STATUS "RPMTools:: Using RPM_ROOTDIR=${RPM_ROOTDIR}")
      ENDIF(NOT RPMTools_FIND_QUIETLY)

      # Prepare RPM build tree
      SET(RPMTools_RPM_BUILD_TREE "${RPM_ROOTDIR}"
        "${RPM_ROOTDIR}/tmp" "${RPM_ROOTDIR}/BUILD"
        "${RPM_ROOTDIR}/RPMS" "${RPM_ROOTDIR}/SOURCES"
        "${RPM_ROOTDIR}/SPECS" "${RPM_ROOTDIR}/SRPMS")
      FOREACH(_dir ${RPMTools_RPM_BUILD_TREE})
        ADD_CUSTOM_COMMAND(OUTPUT ${_dir}
          COMMAND ${CMAKE_COMMAND} -E make_directory "${_dir}")
      ENDFOREACH(_dir)

      #
      # We check whether if the provided spec file is
      # to be configured or not.
      #
      GET_FILENAME_COMPONENT(SPECFILE_EXT ${SPECFILE_PATH} EXT)
      IF ("${SPECFILE_EXT}" STREQUAL ".spec")
        # This is a 'ready-to-use' spec file which does not need to be CONFIGURED
        SET(SPECFILE_NAME "${RPMNAME}.spec")
        MESSAGE(STATUS "Simple copy spec file <${SPECFILE_PATH}> --> <${RPM_ROOTDIR}/SPECS/${SPECFILE_NAME}>")
        CONFIGURE_FILE(
          ${SPECFILE_PATH}
          ${RPM_ROOTDIR}/SPECS/${SPECFILE_NAME}
          COPYONLY)
      ELSE ("${SPECFILE_EXT}" STREQUAL ".spec")
        # This is a to-be-configured spec file
        SET(SPECFILE_NAME "${RPMNAME}.spec")
        IF(NOT RPMTools_FIND_QUIETLY)
          MESSAGE(STATUS "Configuring spec file <RPM/SPECS/${RPMNAME}.spec>")
        ENDIF(NOT RPMTools_FIND_QUIETLY)
        #SET(CPACK_RPM_DIRECTORY RPM_ROOTDIR)
        SET(CPACK_RPM_FILE_NAME ${CMAKE_BINARY_DIR}/${CPACK_SOURCE_PACKAGE_FILE_NAME}.src.rpm)
        SET(RPM_NAME ${RPMNAME})# provided to the spec.in file
        CONFIGURE_FILE(
          ${SPECFILE_PATH}
          ${RPM_ROOTDIR}/SPECS/${SPECFILE_NAME}
          @ONLY)
      ENDIF ("${SPECFILE_EXT}" STREQUAL ".spec")

      # If we have to recreate the spec-file, we rerun cmake
      ADD_CUSTOM_COMMAND(OUTPUT ${RPM_ROOTDIR}/SPECS/${SPECFILE_NAME}
        COMMAND cmake .)

      ADD_CUSTOM_TARGET(${RPMNAME}_srpm
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${RPM_ROOTDIR}/SRPMS
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RPM_ROOTDIR}/SRPMS
        COMMAND cpack -G TBZ2 --config CPackSourceConfig.cmake
        COMMAND ${CMAKE_COMMAND} -E copy ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2 ${RPM_ROOTDIR}/SOURCES
        COMMAND "${RPMTools_RPMBUILD_EXECUTABLE}" -bs  --define=\"_topdir ${RPM_ROOTDIR}\" --buildroot=${RPM_ROOTDIR}/tmp "${RPM_ROOTDIR}/SPECS/${SPECFILE_NAME}"
        DEPENDS
        ${RPMTools_RPM_BUILD_TREE}
        "${RPM_ROOTDIR}/SPECS/${SPECFILE_NAME}"
        )

      ADD_CUSTOM_TARGET(${RPMNAME}_rpm
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${RPM_ROOTDIR}/RPMS
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RPM_ROOTDIR}/RPMS
        COMMAND cpack -G TBZ2 --config CPackSourceConfig.cmake
        COMMAND ${CMAKE_COMMAND} -E copy ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2 ${RPM_ROOTDIR}/SOURCES
        #
        # If we add the following
        # and remove the %prep, %build and %install phase of the spec
        # file, the packager might save some time
        #COMMAND make "DESTDIR=${RPM_ROOTDIR}/tmp" install
        COMMAND "${RPMTools_RPMBUILD_EXECUTABLE}" -bb  --define=\"_topdir ${RPM_ROOTDIR}\" --buildroot=${RPM_ROOTDIR}/tmp "${RPM_ROOTDIR}/SPECS/${SPECFILE_NAME}"
        DEPENDS
        ${RPMTools_RPM_BUILD_TREE}
        "${RPM_ROOTDIR}/SPECS/${SPECFILE_NAME}"
        )
    ENDMACRO(RPMTools_ADD_RPM_TARGETS)

  ELSE (RPMTools_RPMBUILD_FOUND)
    SET(RPMTools FALSE)
  ENDIF (RPMTools_RPMBUILD_FOUND)

ENDIF (UNIX)
