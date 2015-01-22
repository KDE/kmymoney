include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${GPGME_INCLUDES} ${Boost_INCLUDE_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} )

########### next target ###############

set(kgpgfile_STAT_SRCS
   kgpgfile.cpp
   )

add_library(kgpgfile STATIC ${kgpgfile_STAT_SRCS})
target_link_libraries(kgpgfile PUBLIC Qt5::Core)
if(KF5Gpgmepp_FOUND)
  target_link_libraries(kgpgfile PUBLIC KF5::Gpgmepp)
endif()

