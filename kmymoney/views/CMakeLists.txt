INCLUDE_DIRECTORIES( ${KMyMoney_SOURCE_DIR}/libkdchart/include)

############# next target (views) STATIC ###################
SET(libviews_a_SOURCES
  kaccountsview.cpp
  kbudgetview.cpp
  kcategoriesview.cpp
  kforecastview.cpp
  kgloballedgerview.cpp
  kwelcomepage.cpp
  khomeview.cpp
  kinstitutionsview.cpp
  kinvestmentview.cpp
  kmymoneyfile.cpp
  kmymoneyview.cpp
  kpayeesview.cpp
  kreportsview.cpp
  kscheduledview.cpp
  tocitem.cpp
  tocitemgroup.cpp
  tocitemreport.cpp
  kscheduletreeitem.cpp
  ktagsview.cpp
  )

SET(libviews_a_HEADER kmymoneyview.h)

SET(libviews_a_UI
  kaccountsviewdecl.ui
  kbudgetviewdecl.ui
  kcategoriesviewdecl.ui
  kforecastviewdecl.ui
  kinstitutionsviewdecl.ui
  kinvestmentviewdecl.ui
  kpayeesviewdecl.ui
  kscheduledviewdecl.ui
  ktagsviewdecl.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
qt5_wrap_ui(libviews_a_SOURCES ${libviews_a_UI})

add_library(views STATIC ${libviews_a_SOURCES})

target_link_libraries(views PUBLIC KF5::Archive KF5::KHtml Qt5::Sql)

# we rely on some of the dialogs to be generated
ADD_DEPENDENCIES(views dialogs newinvestmentwizard newloanwizard endingbalancedlg)

########### install files ###############

INSTALL(FILES ${libviews_a_HEADER}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
