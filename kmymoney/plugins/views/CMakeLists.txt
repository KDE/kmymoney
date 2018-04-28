if(ENABLE_FORECASTVIEW)
  add_subdirectory(forecast)
endif()

if(ENABLE_REPORTSVIEW)
  add_subdirectory(reports)
endif()

if(ENABLE_ONLINEJOBOUTBOXVIEW)
  add_subdirectory(onlinejoboutbox)
endif()
