############# next target (views) STATIC ###################
set(libviews_a_SOURCES
  kaccountsview.cpp
  kcategoriesview.cpp
  accountdelegate.cpp
  kwelcomepage.cpp
  khomeview.cpp
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
  simpleledgerview.cpp
  kmmstyleditemdelegate.cpp
  ledgerviewpage.cpp
  ledgerview.cpp
  ledgerviewsettings.cpp
  ledgeraccountfilter.cpp
  ledgerjournalidfilter.cpp
  ledgerpayeefilter.cpp
  ledgertagfilter.cpp
  multitransactioneditor.cpp
  journaldelegate.cpp
  onlinebalancedelegate.cpp
  reconciliationdelegate.cpp
  reconciliationledgerviewpage.cpp
  specialdatedelegate.cpp
  investtransactioneditor.cpp
  investactivities.cpp
  ksearchtransactiondlg.cpp
  ktransactionselectdlg.cpp
  newtransactioneditor.cpp
  newtransactionform.cpp
  securityaccountnamedelegate.cpp
)

set(ledgerviews_SOURCES
  newspliteditor.cpp
  splitview.cpp
  splitdelegate.cpp
  tabordereditor.cpp
  transactioneditorbase.cpp
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
  newspliteditor.ui
  newtransactioneditor.ui
  newtransactionform.ui
  investtransactioneditor.ui
)

set(ledgerviews_UI
  tabordereditor.ui
)

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
ki18n_wrap_ui(libviews_a_SOURCES ${libviews_a_UI})
ki18n_wrap_ui(ledgerviews_SOURCES ${ledgerviews_UI})

add_library(views STATIC ${libviews_a_SOURCES})

# TODO: Remove this dependency. But it is needed as long as the payee editor uses these objects directly
# This should be replaced by virtual methods in a pure abstract object.
target_link_libraries( views PUBLIC
  newaccountwizard
  KF${QT_MAJOR_VERSION}::TextWidgets
  Qt::PrintSupport
  kmm_base_dialogs
  kmm_extended_dialogs
  kmm_ledgerviews
  kmm_mymoney # needed to load payeeIdentifier
  kmm_widgets
  kmm_printer
  kmm_textbrowser
)

# we rely on some of the dialogs to be generated
add_dependencies(views newinvestmentwizard newaccountwizard newloanwizard endingbalancedlg)

add_library(kmm_ledgerviews ${ledgerviews_SOURCES})

generate_export_header(kmm_ledgerviews BASE_NAME kmm_ledgerviews)

target_link_libraries(kmm_ledgerviews PUBLIC
  KF${QT_MAJOR_VERSION}::TextWidgets
  kmm_mymoney # needed to load payeeIdentifier
  kmm_base_widgets
  kmm_base_dialogs
  PRIVATE
  settings
)

########### install files ###############

install(TARGETS
  kmm_ledgerviews
  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

