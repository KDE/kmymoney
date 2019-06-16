include(ECMAddTests)

set(testwidgets_SOURCES
    ../amountvalidator.cpp
    )

add_library(testwidgets STATIC ${testwidgets_SOURCES})
target_link_libraries(testwidgets PUBLIC
    Qt5::Gui
    Qt5::Core
)

file(GLOB tests_sources "*-test.cpp")
ecm_add_tests(${tests_sources}
  LINK_LIBRARIES
    Qt5::Gui
    Qt5::Core
    Qt5::Test
    Qt5::Xml
    testwidgets
)
