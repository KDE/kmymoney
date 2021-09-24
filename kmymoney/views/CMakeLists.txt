############# next target (views) STATIC ###################
set(libviews_a_SOURCES
  kaccountsview.cpp
  kcategoriesview.cpp
  accountdelegate.cpp
  kwelcomepage.cpp
  khomeview.cpp
  kqmlview.cpp
  kinstitutionsview.cpp
  kinvestmentview.cpp
  kmymoneyfile.cpp
  kmymoneyview.cpp
  kpayeesview.cpp
  kscheduledview.cpp
  keditscheduledlg.cpp
  kenterscheduledlg.cpp
  ktagsview.cpp
  kpayeeidentifierview.cpp
  payeeidentifierselectiondelegate.cpp
  kmymoneywebpage.cpp
  simpleledgerview.cpp
  kmmstyleditemdelegate.cpp
  ledgerviewpage.cpp
  ledgerview.cpp
  ledgerviewsettings.cpp
  ledgeraccountfilter.cpp
  ledgerjournalidfilter.cpp
  ledgerpayeefilter.cpp
  ledgertagfilter.cpp
  journaldelegate.cpp
  onlinebalancedelegate.cpp
  reconciliationdelegate.cpp
  reconciliationledgerviewpage.cpp
  specialdatedelegate.cpp
  investtransactioneditor.cpp
  investactivities.cpp
  newspliteditor.cpp
  transactioneditorbase.cpp
  ksearchtransactiondlg.cpp
  ktransactionselectdlg.cpp
  newtransactioneditor.cpp
  newtransactionform.cpp
  splitview.cpp
  splitdialog.cpp
  splitdelegate.cpp
  widgethintframe.cpp
)

set(libviews_a_UI
  kaccountsview.ui
  kcategoriesview.ui
  kinstitutionsview.ui
  kinvestmentview.ui
  keditscheduledlg.ui
  kenterscheduledlg.ui
  kpayeesview.ui
  kscheduledview.ui
  ktagsview.ui
  kpayeeidentifierview.ui
  ksearchtransactiondlg.ui
  ktransactionselectdlg.ui
  simpleledgerview.ui
  ledgerviewpage.ui
  splitdialog.ui
  newspliteditor.ui
  newtransactioneditor.ui
  newtransactionform.ui
  investtransactioneditor.ui
)

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
ki18n_wrap_ui(libviews_a_SOURCES ${libviews_a_UI})

add_library(views STATIC ${libviews_a_SOURCES})

if(ENABLE_WEBENGINE)
 target_link_libraries(views PUBLIC Qt5::WebEngineWidgets)
else(ENABLE_WEBENGINE)
 target_link_libraries(views PUBLIC KF5::WebKit)
endif(ENABLE_WEBENGINE)

target_link_libraries(views PUBLIC
    Qt5::Quick
    Qt5::QuickWidgets
)

# TODO: Remove this dependency. But it is needed as long as the payee editor uses these objects directly
# This should be replaced by virtual methods in a pure abstract object.
target_link_libraries( views PUBLIC
  newaccountwizard
  KF5::TextWidgets
  Qt5::PrintSupport
  kmm_base_dialogs
  kmm_mymoney # needed to load payeeIdentifier
  kmm_widgets
  kmm_printer
)

# we rely on some of the dialogs to be generated
add_dependencies(views newinvestmentwizard newaccountwizard newloanwizard endingbalancedlg)
