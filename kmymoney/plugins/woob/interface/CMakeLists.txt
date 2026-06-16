set(woob_interface_SOURCES
        woobinterface.cpp
    )

include_directories(${Python3_INCLUDE_DIRS})

qt_add_resources(woob_interface_SOURCES woobinterface.qrc)

add_library(woob_interface STATIC ${woob_interface_SOURCES})

# Extract the Python library path for dlopen with RTLD_GLOBAL at runtime.
# Python C extension modules (e.g. _struct) have undefined references to symbols
# in libpython, which must be resolved from the global symbol table.
# Since woob.so is a dynamically loaded plugin, libpython is loaded with
# RTLD_LOCAL by default. We pass the library name so we can re-open it
# with RTLD_GLOBAL before calling Py_Initialize().
list(GET Python3_LIBRARIES 0 PYTHON_LIBRARY_FIRST)
get_filename_component(PYTHON_LIBRARY_NAME "${PYTHON_LIBRARY_FIRST}" NAME)
target_compile_definitions(woob_interface PRIVATE
    PYTHON_LIBRARY_NAME="${PYTHON_LIBRARY_NAME}"
)

target_link_libraries (
  woob_interface
  PUBLIC
  Qt::Core
  Alkimia::alkimia
  PRIVATE
  ${Python3_LIBRARIES}
  )
