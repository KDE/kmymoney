if(ENABLE_QIFIMPORTER)
  add_subdirectory( import )
endif()

if(ENABLE_QIFEXPORTER)
  add_subdirectory( export )
endif()

if(ENABLE_QIFIMPORTER OR ENABLE_QIFEXPORTER)
  add_subdirectory( config )
endif()
