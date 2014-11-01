include_directories(
    ../kmymoney/mymoney
    ${CMAKE_BINARY_DIR}/kmymoney/mymoney
    ../kmymoney
)
add_executable(xea2kmt xea2kmt.cpp)
target_link_libraries(xea2kmt ${QT_QTCORE_LIBRARIES} ${QT_QTXMLPATTERNS_LIBRARIES} kmm_mymoney)
