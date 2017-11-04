############# next target (views) STATIC ###################
set(libviews_a_SOURCES
  kaccountsview.cpp
  kbudgetview.cpp
  kcategoriesview.cpp
  kforecastview.cpp
  kgloballedgerview.cpp
  ledgerview.cpp
  kwelcomepage.cpp
  khomeview.cpp
  kinstitutionsview.cpp
  kinvestmentview.cpp
  kmymoneyfile.cpp
  kmymoneyview.cpp
  kmymoneyviewbase.cpp
  kmymoneyaccountsviewbase.cpp
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
  simpleledgerview.cpp
  ledgerviewpage.cpp
  ledgerview.cpp
  ledgerdelegate.cpp
  newspliteditor.cpp
  newtransactioneditor.cpp
  newtransactionform.cpp
  splitdialog.cpp
  splitdelegate.cpp
  widgethintframe.cpp
  kmymoneywebpage.cpp
)

set(libviews_a_HEADER kmymoneyview.h)

set(libviews_a_UI
  kaccountsview.ui
  kbudgetview.ui
  kcategoriesview.ui
  kforecastviewdecl.ui
  kinstitutionsview.ui
  kinvestmentview.ui
  kpayeesviewdecl.ui
  kscheduledviewdecl.ui
  ktagsview.ui
  konlinejoboutbox.ui
  kpayeeidentifierview.ui
  simpleledgerview.ui
  ledgerview.ui
  ledgerviewpage.ui
  simpleledgerview.ui
  splitdialog.ui
  newspliteditor.ui
  newtransactioneditor.ui
  newtransactionform.ui
)

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
ki18n_wrap_ui(libviews_a_SOURCES ${libviews_a_UI})

add_library(views STATIC ${libviews_a_SOURCES})

target_link_libraries(views PUBLIC KChart KF5::KIOFileWidgets KF5::Notifications KF5::Archive KF5::TextWidgets Qt5::Sql Alkimia::alkimia)

if(ENABLE_WEBENGINE)
 target_link_libraries(views PUBLIC Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(views PUBLIC KF5::WebKit)
endif(ENABLE_WEBENGINE)

if(KF5Activities_FOUND)
 target_link_libraries(views PRIVATE KF5::Activities)
endif()

# TODO: Remove this dependency. But it is needed as long as the payee editor uses these objects directly
# This should be replaced by virtual methods in a pure abstract object.
target_link_libraries( views PUBLIC
  payeeidentifier_iban_bic
  payeeidentifier_nationalAccount
  kmm_mymoney # needed to load payeeIdentifier
)

# we rely on some of the dialogs to be generated
add_dependencies(views dialogs newinvestmentwizard newloanwizard endingbalancedlg)

########### install files ###############

install(FILES ${libviews_a_HEADER}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
