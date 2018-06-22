if(ENABLE_FORECASTVIEW)
  add_subdirectory(forecast)
endif()

if(ENABLE_REPORTSVIEW OR BUILD_TESTING)
  add_subdirectory(reports)
endif()

if(ENABLE_BUDGETVIEW)
  add_subdirectory(budget)
endif()

if(ENABLE_ONLINEJOBOUTBOXVIEW)
  add_subdirectory(onlinejoboutbox)
endif()
