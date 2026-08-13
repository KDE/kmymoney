set(mymoneyqifreader_SRCS ../mymoneyqifreader.cpp)
add_library(test_mymoneyqifreader STATIC ${mymoneyqifreader_SRCS})
target_link_libraries(test_mymoneyqifreader
  PUBLIC
    Qt::Core
    Alkimia::alkimia
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::KIOWidgets
    kmm_mymoney
    kmm_settings
)

set(test_mymoneyqifreader_SRCS test-mymoneyqifreader.cpp)

kmm_add_tests(${test_mymoneyqifreader_SRCS}
    LINK_LIBRARIES
        test_mymoneyqifreader
        KF${QT_MAJOR_VERSION}::Completion
        Qt::Test
)

target_include_directories(test-mymoneyqifreader
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)
