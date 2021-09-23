if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

########### next target ###############

set(csvimportercore_PART_SRCS
  csvimportercore.cpp
  convdate.cpp
  csvutil.cpp
)

add_library(kmm_csvimportercore ${csvimportercore_PART_SRCS})
generate_export_header(kmm_csvimportercore)
target_link_libraries(kmm_csvimportercore
  PUBLIC
    kmm_mymoney
)

set_target_properties(kmm_csvimportercore PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)

########### install files ###############

install(TARGETS kmm_csvimportercore
  ${INSTALL_TARGETS_DEFAULT_ARGS} )
