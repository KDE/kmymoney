set(kgpgfile_STAT_SRCS
  kgpgfile.cpp
)

add_library(kgpgfile STATIC ${kgpgfile_STAT_SRCS})
target_link_libraries(kgpgfile PUBLIC Qt5::Core)

if(Gpgmepp_FOUND)
  if(WIN32)
    find_package(KDEWin REQUIRED)
    target_include_directories(kgpgfile PUBLIC ${KDEWIN_INCLUDES})
    target_link_libraries(kgpgfile PUBLIC ${KDEWIN_LIBRARIES})
    target_compile_definitions(kgpgfile PUBLIC ${KDEWIN_DEFINITIONS})
  endif()
  target_link_libraries(kgpgfile PUBLIC Gpgmepp)
endif()
