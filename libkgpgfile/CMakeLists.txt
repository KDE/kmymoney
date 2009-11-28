INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${GPGME_INCLUDES} ${Boost_INCLUDE_DIR} )

########### next target ###############

SET(kgpgfile_STAT_SRCS
   kgpgfile.cpp
   )

KMM_CREATE_LINKS(kgpgfile.h)

KDE4_ADD_LIBRARY(kgpgfile STATIC ${kgpgfile_STAT_SRCS})
TARGET_LINK_LIBRARIES(kgpgfile kleo)


