set(weboob_interface_SOURCES
    weboobinterface.cpp
    )

include_directories(${PYTHON_INCLUDE_DIRS})

# begin of a hack (read more on https://bugs.kde.org/show_bug.cgi?id=335965)
get_filename_component(PYTHON_LIBRARY_REALPATH "${PYTHON_LIBRARY}" REALPATH)
get_filename_component(LIB_PYTHON ${PYTHON_LIBRARY_REALPATH} NAME)
add_definitions(-DLIB_PYTHON="${LIB_PYTHON}")
# end of a hack

add_library(weboob_interface STATIC ${weboob_interface_SOURCES})
target_link_libraries (
  weboob_interface
  PUBLIC
  Qt5::Core
  Alkimia::alkimia
  PRIVATE
  ${PYTHON_LIBRARIES}
  )

install(FILES kmymoneyweboob.py
        DESTINATION "${DATA_INSTALL_DIR}/kmymoney/weboob")
