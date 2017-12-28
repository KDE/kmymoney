if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

########### next target ###############

set(csvimportercore_PART_SRCS
  csvimportercore.cpp
  convdate.cpp
  csvutil.cpp
)

add_library(kmm_csvimportercore SHARED ${csvimportercore_PART_SRCS})
generate_export_header(kmm_csvimportercore)
target_link_libraries(kmm_csvimportercore
  PUBLIC
    kmm_mymoney
)

########### install files ###############

install(TARGETS kmm_csvimportercore
  ${INSTALL_TARGETS_DEFAULT_ARGS} )
