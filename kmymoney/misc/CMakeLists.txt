install(PROGRAMS financequote.pl
        DESTINATION ${DATA_INSTALL_DIR}/kmymoney/misc)

set( kmm_utils_validators_SRCS
  charvalidator.cpp
  validators.cpp
)

set( kmm_webconnect_SRCS
  webpricequote.cpp
  webconnect.cpp
)

if( "${HAVE_UNISTD_H}" AND "${HAVE_PWD_H}" )
  set( kmm_utils_platformtools_SRCS
    platformtools_gnu.cpp
  )
elseif( "${HAVE_WINDOWS_H}" AND "${HAVE_LMCONS_H}" AND "${HAVE_PROCESS_H}" )
  set( kmm_utils_platformtools_SRCS
    platformtools_nognu.cpp
  )
else()
  message(FATAL_ERROR "make sure either windows.h and lmcons.h (on msvc platforms) or unistd.h and pwd.h (on other platforms) are present.")
endif()

set( kmm_utils_validators_HEADER
  charvalidator.h
  validators.h
)

set( kmm_webconnect_HEADER
  webpricequote.h
  webconnect.h
)

set( kmm_utils_platformtools_HEADER
  platformtools.h
)

set( kmm_printer_SRCS
  kmm_printer.cpp
)


add_library(kmm_utils_validators STATIC ${kmm_utils_validators_SRCS})
add_library(kmm_utils_platformtools STATIC ${kmm_utils_platformtools_SRCS})
add_library(kmm_webconnect SHARED ${kmm_webconnect_SRCS})
add_library(kmm_printer SHARED ${kmm_printer_SRCS})

target_link_libraries( kmm_utils_validators
  PUBLIC
  Qt5::Core
  Qt5::Gui
)

target_link_libraries( kmm_webconnect
  PUBLIC
    Qt5::Core
    Qt5::Network
    KF5::ConfigCore
    KF5::Service
    KF5::Codecs
    KF5::KIOCore
    KF5::KIOWidgets
    Alkimia::alkimia
    kmm_mymoney
    kmm_csvimportercore
  PRIVATE
    KF5::I18n
)

target_link_libraries( kmm_utils_platformtools
  PUBLIC
  Qt5::Core
)

target_link_libraries(kmm_printer
  PUBLIC
  Qt5::Core
  Qt5::PrintSupport
)

set_target_properties(kmm_printer PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)

generate_export_header(kmm_printer)

set_target_properties(kmm_webconnect PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)

generate_export_header(kmm_webconnect)

install(TARGETS kmm_printer kmm_webconnect ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmm_printer_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

