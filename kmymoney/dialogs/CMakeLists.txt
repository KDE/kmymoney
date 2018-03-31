add_subdirectory( settings )

########### next target ###############

set(libdialogs_a_SOURCES
  splitadjustdialog.cpp
  investactivities.cpp
  investtransactioneditor.cpp
  kaccountselectdlg.cpp
  kbackupdlg.cpp
  kbalancechartdlg.cpp
  kbalancewarning.cpp
  kcategoryreassigndlg.cpp
  kchooseimportexportdlg.cpp
  kconfirmmanualenterdlg.cpp
  kcurrencycalculator.cpp
  kcurrencyeditdlg.cpp
  kavailablecurrencydlg.cpp
  kcurrencyeditordlg.cpp
  keditscheduledlg.cpp
  kenterscheduledlg.cpp
  kequitypriceupdatedlg.cpp
  kequitypriceupdateconfdlg.cpp
  kfindtransactiondlg.cpp
  kgpgkeyselectiondlg.cpp
  kloadtemplatedlg.cpp
  kmergetransactionsdlg.cpp
  kmymoneyfileinfodlg.cpp
  kmymoneypricedlg.cpp
  kmymoneysplittable.cpp
  knewaccountdlg.cpp
  hierarchyfilterproxymodel.cpp
  knewbankdlg.cpp
  knewbudgetdlg.cpp
  knewequityentrydlg.cpp
  editpersonaldatadlg.cpp
  kpayeereassigndlg.cpp
  ktagreassigndlg.cpp
  kselecttransactionsdlg.cpp
  ksplittransactiondlg.cpp
  ktemplateexportdlg.cpp
  kupdatestockpricedlg.cpp
  transactioneditor.cpp
  stdtransactioneditor.cpp
  transactionmatcher.cpp
  konlinetransferform.cpp
)

set(dialogs_HEADERS
  splitadjustdialog.h
  investtransactioneditor.h kcurrencycalculator.h transactioneditor.h stdtransactioneditor.h
  )

set(dialogs_UI
  splitadjustdialog.ui
  kaccountselectdlg.ui kbackupdlg.ui
  kcategoryreassigndlg.ui kchooseimportexportdlg.ui
  kconfirmmanualenterdlg.ui
  kcurrencycalculator.ui kcurrencyeditdlg.ui kavailablecurrencydlg.ui kcurrencyeditordlg.ui
  keditscheduledlg.ui
  kenterscheduledlg.ui
  kequitypriceupdatedlg.ui kequitypriceupdateconfdlg.ui
  kfindtransactiondlg.ui
  kgpgkeyselectiondlg.ui
  kloadtemplatedlg.ui
  kmymoneyfileinfodlg.ui kmymoneypricedlg.ui
  knewaccountdlg.ui knewbankdlg.ui knewbudgetdlg.ui
  knewequityentrydlg.ui editpersonaldatadlg.ui kpayeereassigndlg.ui
  ktagreassigndlg.ui
  kselecttransactionsdlg.ui
  ksortoptiondlg.ui ksplitcorrectiondlg.ui ksplittransactiondlg.ui
  ktemplateexportdlg.ui
  kupdatestockpricedlg.ui
  konlinetransferform.ui
)

ki18n_wrap_ui(libdialogs_a_SOURCES ${dialogs_UI} )

add_library(dialogs STATIC ${libdialogs_a_SOURCES})

target_link_libraries(dialogs
  PUBLIC
    KChart
    KF5::ItemViews
    KF5::I18n
    KF5::TextWidgets
    KF5::Completion
    Qt5::Widgets
    Qt5::Sql
    Alkimia::alkimia
    kmm_mymoney
    onlinetask_interfaces
    kmm_widgets
    kmm_utils_platformtools
)

target_link_libraries(dialogs LINK_PUBLIC
  kmm_widgets
  kmm_mymoney
  onlinetask_interfaces
)


########### install files ###############

install(FILES  ${dialogs_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
