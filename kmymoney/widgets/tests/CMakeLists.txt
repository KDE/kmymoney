include(ECMAddTests)

set(testwidgets_SOURCES
    ../amountvalidator.cpp
    )

add_library(testwidgets STATIC ${testwidgets_SOURCES})
target_link_libraries(testwidgets PUBLIC
    Qt::Gui
    Qt::Core
    kmm_utils_platformtools
)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt::Gui
    Qt::Core
    Qt::Test
    Qt::Xml
    testwidgets
)
