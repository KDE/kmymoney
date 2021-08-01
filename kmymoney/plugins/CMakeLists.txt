add_subdirectory( onlinetasks )
add_subdirectory( plugin-interfaces )
add_subdirectory( csv )
add_subdirectory( qif )
add_subdirectory( gnc )
add_subdirectory( ofx )
add_subdirectory( icalendar )
add_subdirectory( reconciliationreport )
add_subdirectory( checkprinting )
add_subdirectory( views )
add_subdirectory( xml )
add_subdirectory( xmlhelper )

if(ENABLE_SQLSTORAGE)
  add_subdirectory(sql)
endif()

if(ENABLE_IBANBICDATA)
  add_subdirectory(ibanbicdata)
endif()

if (ENABLE_KBANKING)
  add_subdirectory( kbanking )
endif (ENABLE_KBANKING)

if (ENABLE_SQLCIPHER)
  add_subdirectory(sqlcipher)
endif()

option(ENABLE_ONLINEJOBPLUGINMOCKUP "Enable onlineJob-plugin mockup (only for developers)" OFF)
if (ENABLE_ONLINEJOBPLUGINMOCKUP)
  add_subdirectory(onlinejobpluginmockup)
endif()

if (ENABLE_WOOB)
  add_subdirectory( woob )
endif ()

