include(ECMAddTests)

set(test_mymoneyqifreader_SRCS
    test-mymoneyqifreader.cpp
    ../mymoneyqifreader.cpp
)

ecm_add_test(${test_mymoneyqifreader_SRCS}
    TEST_NAME test-mymoneyqifreader
    LINK_LIBRARIES
        kmm_mymoney
        kmm_settings
        KF${QT_MAJOR_VERSION}::Completion
        KF${QT_MAJOR_VERSION}::I18n
        KF${QT_MAJOR_VERSION}::KIOCore
        KF${QT_MAJOR_VERSION}::KIOWidgets
        Qt${QT_MAJOR_VERSION}::Test
)

target_include_directories(test-mymoneyqifreader
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)
