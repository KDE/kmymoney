ADD_SUBDIRECTORY( settings ) 

########### next target ###############

SET(libdialogs_a_SOURCES
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
  keditloanwizard.cpp
  keditscheduledlg.cpp
  kendingbalancedlg.cpp 
  kenterscheduledlg.cpp
  kequitypriceupdatedlg.cpp 
  kexportdlg.cpp 
  kfindtransactiondlg.cpp 
  kgeneratesqldlg.cpp
  kgncimportoptionsdlg.cpp 
  kgncpricesourcedlg.cpp
  kgpgkeyselectiondlg.cpp 
  kimportdlg.cpp
  kloadtemplatedlg.cpp
  kmergetransactionsdlg.cpp 
  kmymoneyfileinfodlg.cpp
  kmymoneypricedlg.cpp 
  kmymoneysplittable.cpp 
  knewaccountdlg.cpp
  knewbankdlg.cpp 
  knewbudgetdlg.cpp 
  knewequityentrydlg.cpp
  knewfiledlg.cpp 
  knewinvestmentwizard.cpp 
  knewloanwizard.cpp 
  konlinequoteconfigurationdlg.cpp
  kpayeereassigndlg.cpp 
  kreconciledlg.cpp
  kreportconfigurationfilterdlg.cpp
  kselectdatabasedlg.cpp
  kselecttransactionsdlg.cpp 
  ksplittransactiondlg.cpp 
  kupdatestockpricedlg.cpp 
  mymoneyqifprofileeditor.cpp
  transactioneditor.cpp 
  transactionmatcher.cpp
  )

SET(dialogs_HEADERS
  investtransactioneditor.h kcurrencycalculator.h transactioneditor.h
  )

KMM_CREATE_LINKS( ${dialogs_HEADERS} )

SET(dialogs_UI
  kaccountselectdlgdecl.ui kbackupdlgdecl.ui
  kcategoryreassigndlgdecl.ui kchooseimportexportdlgdecl.ui
  kconfirmmanualenterdlgdecl.ui
  kcurrencycalculatordecl.ui kcurrencyeditdlgdecl.ui
  keditscheduledlgdecl.ui
  kendingbalancedlgdecl.ui kenterscheduledlgdecl.ui
  kequitypriceupdatedlgdecl.ui kexportdlgdecl.ui
  kfindtransactiondlgdecl.ui kgeneratesqldlgdecl.ui kgncimportoptionsdlgdecl.ui
  kgncpricesourcedlgdecl.ui kimportdlgdecl.ui kloadtemplatedlgdecl.ui
  kmymoneyfileinfodlgdecl.ui kmymoneypricedlgdecl.ui
  knewaccountdlgdecl.ui knewbankdlgdecl.ui knewbudgetdlgdecl.ui
  knewequityentrydecl.ui knewfiledlgdecl.ui
  knewinvestmentwizarddecl.ui knewloanwizarddecl.ui
  konlinequoteconfigurationdecl.ui kpayeereassigndlgdecl.ui
  kreconciledlgdecl.ui
  kselectdatabasedlgdecl.ui kselecttransactionsdlgdecl.ui
  ksortoptiondlg.ui ksplitcorrectiondlg.ui ksplittransactiondlgdecl.ui
  kupdatestockpricedlgdecl.ui mymoneyqifprofileeditordecl.ui
  ../widgets/kaccounttemplateselectordecl.ui ../widgets/transactionsortoptiondecl.ui
)

kde4_add_ui_files(libdialogs_a_SOURCES ${dialogs_UI} )

# in order to use add_dependencies, we need to add this custom target
# for all generated header files.
# (see http://www.vtk.org/Wiki/CMake_FAQ#How_can_I_add_a_dependency_to_a_source_file_which_is_generated_in_a_subdirectory.3F )
ADD_CUSTOM_TARGET(generate_dialogs_ui_srcs DEPENDS ${dialogs_ui_srcs})

KDE4_ADD_LIBRARY(dialogs STATIC ${libdialogs_a_SOURCES} ${dialogs_ui_srcs})

# we rely on the widgets to be generated
ADD_DEPENDENCIES(dialogs widgets)

########### install files ###############

INSTALL(FILES  ${dialogs_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
