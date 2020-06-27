############# next target (kmm_oldregister) SHARED ###################
set(kmm_oldregister_SOURCES
  investactivities.cpp
  investtransaction.cpp
  investtransactioneditor.cpp
  keditscheduledlg.cpp
  kenterscheduledlg.cpp
  kfindtransactiondlg.cpp
  kmergetransactionsdlg.cpp
  kmymoneysplittable.cpp
  kselecttransactionsdlg.cpp
  ksplittransactiondlg.cpp
  stdtransaction.cpp
  stdtransactiondownloaded.cpp
  stdtransactioneditor.cpp
  stdtransactionmatched.cpp
  transactioneditor.cpp
  fancydategroupmarker.cpp
  fancydategroupmarkers.cpp
  groupmarker.cpp
  groupmarkers.cpp
  itemptrvector.cpp
  qwidgetcontainer.cpp
  register.cpp
  registerfilter.cpp
  registeritem.cpp
  registeritemdelegate.cpp
  registersearchline.cpp
  scheduledtransaction.cpp
  selectedtransaction.cpp
  selectedtransactions.cpp
  transaction.cpp
  transactioneditorcontainer.cpp
  transactionform.cpp
  transactionformitemdelegate.cpp
  transactionsortoption.cpp
)

set(kmm_oldregister_HEADERS
  fancydategroupmarker.h
  groupmarker.h
  investtransaction.h
  investtransactioneditor.h
  qwidgetcontainer.h
  register.h
  registeritem.h
  scheduledtransaction.h
  selectedtransaction.h
  selectedtransactions.h
  stdtransaction.h
  stdtransactiondownloaded.h
  stdtransactioneditor.h
  stdtransactionmatched.h
  transactioneditor.h
  transactioneditorcontainer.h
  transactionform.h
  transaction.h
  transactionsortoption.h
)

set(kmm_oldregister_UI
  ksortoptiondlg.ui
  keditscheduledlg.ui
  kenterscheduledlg.ui
  kfindtransactiondlg.ui
  kselecttransactionsdlg.ui
  ksplittransactiondlg.ui
  transactionsortoption.ui
)

ki18n_wrap_ui(kmm_oldregister_SOURCES ${kmm_oldregister_UI})

add_library(kmm_oldregister SHARED ${kmm_oldregister_SOURCES})

generate_export_header(kmm_oldregister BASE_NAME kmm_oldregister)

target_link_libraries(kmm_oldregister PUBLIC
    newloanwizard
    newaccountwizard
    newinvestmentwizard
    kmm_wizard
    wizardpages
    kmm_extended_dialogs
    kmm_widgets
    kmm_base_dialogs
    kmm_base_widgets
    kmm_mymoney
)

set_target_properties(kmm_oldregister PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS kmm_oldregister ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
  ${kmm_oldregister_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
