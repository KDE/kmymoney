add_subdirectory( settings )

########### next target ###############

set(kmm_base_dialogs_SOURCES
  accountcreator.cpp
  amounteditcurrencyhelper.cpp
  editpersonaldatadlg.cpp
  kaccountselectdlg.cpp
  kavailablecurrencydlg.cpp
  kbackupdlg.cpp
  kbalancewarning.cpp
  kcategoryreassigndlg.cpp
  kchooseimportexportdlg.cpp
  kconfirmmanualenterdlg.cpp
  kcurrencycalculator.cpp
  kcurrencyeditdlg.cpp
  kcurrencyeditordlg.cpp
  kmymoneyfileinfodlg.cpp
  knewaccountdlg.cpp
  knewinstitutiondlg.cpp
  knewequityentrydlg.cpp
  kpayeereassigndlg.cpp
  ksaveasquestion.cpp
  ktagreassigndlg.cpp
  kupdatestockpricedlg.cpp
  payeecreator.cpp
  transactionmatcher.cpp
)

set(base_dialogs_UI
  editpersonaldatadlg.ui
  kaccountselectdlg.ui
  kavailablecurrencydlg.ui
  kbackupdlg.ui
  kcategoryreassigndlg.ui
  kchooseimportexportdlg.ui
  kconfirmmanualenterdlg.ui
  kcurrencycalculator.ui
  kcurrencyeditdlg.ui
  kcurrencyeditordlg.ui
  kmymoneyfileinfodlg.ui
  knewaccountdlg.ui
  knewinstitutiondlg.ui
  knewequityentrydlg.ui
  kpayeereassigndlg.ui
  ksaveasquestion.ui
  ktagreassigndlg.ui
  kupdatestockpricedlg.ui
  splitadjustdialog.ui
)

set(base_dialogs_HEADERS
  amounteditcurrencyhelper.h
  kcurrencycalculator.h
  splitadjustdialog.h
)

ki18n_wrap_ui(kmm_base_dialogs_SOURCES ${base_dialogs_UI} )

add_library(kmm_base_dialogs ${kmm_base_dialogs_SOURCES})

generate_export_header(kmm_base_dialogs BASE_NAME kmm_base_dialogs)

target_link_libraries(kmm_base_dialogs
  PUBLIC
    KF5::ItemViews
    KF5::I18n
    KF5::TextWidgets
    KF5::Completion
    Qt::Widgets
    Alkimia::alkimia
    kmm_mymoney
    onlinetask_interfaces
    kmm_base_widgets
    kmm_utils_platformtools
  PRIVATE
    KF5::KIOGui
)

target_link_libraries(kmm_base_dialogs LINK_PUBLIC
  kmm_base_widgets
  kmm_mymoney
  onlinetask_interfaces
)

set_target_properties(kmm_base_dialogs PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})


########### next target ###############

set(kmm_extended_dialogs_SOURCES
  kequitypriceupdateconfdlg.cpp
  kequitypriceupdatedlg.cpp
  kmymoneypricedlg.cpp
  splitadjustdialog.cpp
)

set(kmm_extended_dialogs_UI
  kequitypriceupdateconfdlg.ui
  kequitypriceupdatedlg.ui
  kmymoneypricedlg.ui
  ksplitcorrectiondlg.ui
)

ki18n_wrap_ui(kmm_extended_dialogs_SOURCES ${kmm_extended_dialogs_UI})

add_library(kmm_extended_dialogs ${kmm_extended_dialogs_SOURCES})

generate_export_header(kmm_extended_dialogs BASE_NAME kmm_extended_dialogs)

target_link_libraries(kmm_extended_dialogs
  PUBLIC
    KF5::ItemViews
    KF5::I18n
    KF5::TextWidgets
    KF5::Completion
    Qt::Widgets
    Alkimia::alkimia
    kmm_mymoney
    onlinetask_interfaces
    kmm_base_widgets
    kmm_utils_platformtools
    kmm_webconnect
    kmm_widgets
  PRIVATE
    KF5::KIOGui
)

target_link_libraries(kmm_extended_dialogs LINK_PUBLIC
  kmm_base_widgets
  kmm_mymoney
  onlinetask_interfaces
)

set_target_properties(kmm_extended_dialogs PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})




########### install files ###############

install(TARGETS
  kmm_base_dialogs
  kmm_extended_dialogs
  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
  ${base_dialogs_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
