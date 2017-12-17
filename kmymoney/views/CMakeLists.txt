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
  kmymoneywebpage.cpp
)

if(ENABLE_UNFINISHEDFEATURES)
  list(APPEND libviews_a_SOURCES
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
  )
endif()

set(libviews_a_UI
  kaccountsview.ui
  kbudgetview.ui
  kcategoriesview.ui
  kforecastviewdecl.ui
  kinstitutionsview.ui
  kinvestmentview.ui
  kpayeesview.ui
  kscheduledview.ui
  ktagsview.ui
  konlinejoboutbox.ui
  kpayeeidentifierview.ui
)

if(ENABLE_UNFINISHEDFEATURES)
  list(APPEND libviews_a_UI 
    simpleledgerview.ui
    ledgerview.ui
    ledgerviewpage.ui
    splitdialog.ui
    newspliteditor.ui
    newtransactioneditor.ui
    newtransactionform.ui
  )
endif()

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
ki18n_wrap_ui(libviews_a_SOURCES ${libviews_a_UI})

add_library(views STATIC ${libviews_a_SOURCES})

target_link_libraries(views PUBLIC newaccountwizard KChart KF5::KIOFileWidgets KF5::Notifications KF5::Archive KF5::TextWidgets Qt5::Sql Qt5::PrintSupport Alkimia::alkimia)

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
add_dependencies(views dialogs newinvestmentwizard newaccountwizard newloanwizard endingbalancedlg)
