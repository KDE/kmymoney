add_subdirectory( settings )

########### next target ###############

set(libdialogs_a_SOURCES
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
  keditscheduledlg.cpp
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
  kpayeereassigndlg.cpp
  ktagreassigndlg.cpp
  kreportconfigurationfilterdlg.cpp
  kselectdatabasedlg.cpp
  kselecttransactionsdlg.cpp
  ksplittransactiondlg.cpp
  ktemplateexportdlg.cpp
  kupdatestockpricedlg.cpp
  mymoneyqifprofileeditor.cpp
  transactioneditor.cpp
  transactionmatcher.cpp
  konlinetransferform.cpp
)

set(dialogs_HEADERS
  investtransactioneditor.h kcurrencycalculator.h transactioneditor.h
  )

set(dialogs_UI
  kaccountselectdlgdecl.ui kbackupdlgdecl.ui
  kcategoryreassigndlgdecl.ui kchooseimportexportdlgdecl.ui
  kconfirmmanualenterdlgdecl.ui
  kcurrencycalculatordecl.ui kcurrencyeditdlgdecl.ui
  keditscheduledlgdecl.ui
  kenterscheduledlgdecl.ui
  kequitypriceupdatedlgdecl.ui kexportdlgdecl.ui
  kfindtransactiondlgdecl.ui kgeneratesqldlgdecl.ui kgncimportoptionsdlgdecl.ui
  kgncpricesourcedlgdecl.ui kimportdlgdecl.ui kloadtemplatedlgdecl.ui
  kmymoneyfileinfodlgdecl.ui kmymoneypricedlgdecl.ui
  knewaccountdlgdecl.ui knewbankdlgdecl.ui knewbudgetdlgdecl.ui
  knewequityentrydecl.ui knewfiledlgdecl.ui kpayeereassigndlgdecl.ui
  ktagreassigndlgdecl.ui
  kselectdatabasedlgdecl.ui kselecttransactionsdlgdecl.ui
  ksortoptiondlg.ui ksplitcorrectiondlg.ui ksplittransactiondlgdecl.ui
  ktemplateexportdlg.ui
  kupdatestockpricedlgdecl.ui mymoneyqifprofileeditordecl.ui
  ../widgets/kaccounttemplateselectordecl.ui ../widgets/transactionsortoptiondecl.ui
  konlinetransferformdecl.ui
)

kde4_add_ui_files(libdialogs_a_SOURCES ${dialogs_UI} )

# in order to use add_dependencies, we need to add this custom target
# for all generated header files.
# (see http://www.vtk.org/Wiki/CMake_FAQ#How_can_I_add_a_dependency_to_a_source_file_which_is_generated_in_a_subdirectory.3F )
add_custom_target(generate_dialogs_ui_srcs DEPENDS ${dialogs_ui_srcs})

kde4_add_library(dialogs STATIC ${libdialogs_a_SOURCES} ${dialogs_ui_srcs})
target_link_libraries(dialogs ${LIBALKIMIA_LIBRARIES})

# we rely on the widgets to be generated
add_dependencies(dialogs widgets)
target_link_libraries(dialogs LINK_PUBLIC
  kmm_widgets
  kmm_mymoney
  onlinetask_interfaces
)


########### install files ###############

install(FILES  ${dialogs_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
