include_directories(
    ../kmymoney/mymoney
    ${CMAKE_BINARY_DIR}/kmymoney/mymoney
    ../kmymoney
    # Todo: this alkimia should not be needed here
    ${LIBALKIMIA_INCLUDE_DIR}
)
add_executable(xea2kmt xea2kmt.cpp)
target_link_libraries(xea2kmt ${QT_QTCORE_LIBRARIES} ${QT_QTXMLPATTERNS_LIBRARIES} kmm_mymoney)
