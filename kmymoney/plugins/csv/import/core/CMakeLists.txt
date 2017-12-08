if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

########### next target ###############

set(kmm_csvimport_core_PART_SRCS
  csvimporter.cpp
  convdate.cpp
  csvutil.cpp
)

add_library(kmm_csvimport_core SHARED ${kmm_csvimport_core_PART_SRCS})
generate_export_header(kmm_csvimport_core)
target_link_libraries(kmm_csvimport_core
  PUBLIC
    kmm_mymoney
)

########### install files ###############

install(TARGETS kmm_csvimport_core
  ${INSTALL_TARGETS_DEFAULT_ARGS} )
