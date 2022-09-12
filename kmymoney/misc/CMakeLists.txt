install(PROGRAMS financequote.pl
        DESTINATION ${KDE_INSTALL_DATADIR}/kmymoney/misc)

set( kmm_utils_validators_SRCS
  charvalidator.cpp
  validators.cpp
)

set( kmm_url_SRCS
  kmmurl.cpp
)

set( kmm_textbrowser_SRCS
  kmmtextbrowser.cpp
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

set( kmm_url_HEADER
  kmmurl.h
)

set( kmm_textbrowser_HEADER
  kmmtextbrowser.h
)
set( kmm_printer_SRCS
  kmm_printer.cpp
)

set( kmm_selections_SRCS
  selectedobjects.cpp
)

add_library(kmm_utils_validators STATIC ${kmm_utils_validators_SRCS})
add_library(kmm_utils_platformtools STATIC ${kmm_utils_platformtools_SRCS})
add_library(kmm_url STATIC ${kmm_url_SRCS})
add_library(kmm_textbrowser STATIC ${kmm_textbrowser_SRCS})
add_library(kmm_webconnect ${kmm_webconnect_SRCS})
add_library(kmm_printer ${kmm_printer_SRCS})
add_library(kmm_selections ${kmm_selections_SRCS})

target_link_libraries( kmm_utils_validators
  PUBLIC
  Qt::Core
  Qt::Gui
)

target_link_libraries( kmm_webconnect
  PUBLIC
    Qt::Core
    Qt::Network
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
  Qt::Core
)

target_link_libraries( kmm_url
  PUBLIC
  Qt::Core
)

target_link_libraries( kmm_textbrowser
  PUBLIC
  Qt::Core
  Qt::Widgets
)

target_link_libraries(kmm_printer
  PUBLIC
  Qt::Core
  Qt::PrintSupport
)

target_link_libraries( kmm_selections
  PUBLIC
  Qt::Core
)


set_target_properties(kmm_selections PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_selections)

set_target_properties(kmm_printer PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_printer)

set_target_properties(kmm_webconnect PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_webconnect)


if(ENABLE_GPG)
  if(WIN32)
    find_package(KDEWin REQUIRED)
    target_include_directories(kmm_gpgfile PUBLIC ${KDEWIN_INCLUDES})
    target_compile_definitions(kgpgfile PUBLIC ${KDEWIN_DEFINITIONS})
  endif()
endif()

set( kmm_gpgfile_SRCS
  kgpgfile.cpp
)
add_library(kmm_gpgfile ${kmm_gpgfile_SRCS})

target_link_libraries( kmm_gpgfile
  PUBLIC
  Qt::Core
  $<$<BOOL:${WIN32}>:${KDEWIN_LIBRARIES}>
  $<$<BOOL:${Gpgmepp_FOUND}>:Gpgmepp>
  $<$<BOOL:${KF5Gpgmepp_FOUND}>:KF5::Gpgmepp>
)
set_target_properties(kmm_gpgfile PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
generate_export_header(kmm_gpgfile)


install(TARGETS
        kmm_printer
        kmm_webconnect
        kmm_selections
        kmm_gpgfile
        ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_printer_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_selections_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_webconnect_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kmm_gpgfile_export.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmymoney COMPONENT Devel)

