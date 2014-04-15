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
  konlinejoboutbox.cpp
  kbankaccountsview.cpp
  kbankaccountedit.cpp
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
  konlinejoboutbox.ui
  kbankaccountsviewdecl.ui
  kbankaccounteditdecl.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
kde4_add_ui_files(libviews_a_SOURCES ${libviews_a_UI})

KDE4_ADD_LIBRARY(views STATIC ${libviews_a_SOURCES})

# TODO: Remove this dependency. But it is needed as long as the payee editor uses these objects directly
# This should be replaced by virtual methods in a pure abstract object.
target_link_libraries( views
  payeeidentifier_iban_bic
  payeeidentifier_nationalAccount
)

# we rely on some of the dialogs to be generated
ADD_DEPENDENCIES(views dialogs newinvestmentwizard newloanwizard endingbalancedlg)

########### install files ###############

INSTALL(FILES ${libviews_a_HEADER}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
