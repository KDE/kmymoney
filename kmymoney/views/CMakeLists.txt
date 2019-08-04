include_directories( ${KMyMoney_SOURCE_DIR}/libkdchart/include)

############# next target (views) STATIC ###################
set(libviews_a_SOURCES
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
  kpayeeidentifierview.cpp
  payeeidentifierselectiondelegate.cpp
  )

set(libviews_a_HEADER kmymoneyview.h)

set(libviews_a_UI
  kaccountsviewdecl.ui
  kbudgetviewdecl.ui
  kcategoriesviewdecl.ui
  kforecastviewdecl.ui
  kinstitutionsviewdecl.ui
  kinvestmentviewdecl.ui
  kpayeesviewdecl.ui
  kreportsviewdecl.ui
  kscheduledviewdecl.ui
  ktagsviewdecl.ui
  konlinejoboutbox.ui
  kpayeeidentifierview.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
kde4_add_ui_files(libviews_a_SOURCES ${libviews_a_UI})

kde4_add_library(views STATIC ${libviews_a_SOURCES})

# TODO: Remove this dependency. But it is needed as long as the payee editor uses these objects directly
# This should be replaced by virtual methods in a pure abstract object.
target_link_libraries( views
  payeeidentifier_iban_bic
  payeeidentifier_nationalAccount
  kmm_mymoney # needed to load payeeIdentifier
)

# we rely on some of the dialogs to be generated
add_dependencies(views dialogs newinvestmentwizard newloanwizard endingbalancedlg)

########### install files ###############

install(FILES ${libviews_a_HEADER}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
