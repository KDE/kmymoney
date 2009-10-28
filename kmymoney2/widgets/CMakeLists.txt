PROJECT(widgets)

########### custom command #############

########### create links ###############
SET(kmymoney_STAT_HEADERS
  kaccounttemplateselector.h kbudgetvalues.h kguiutils.h
  kmymoneyaccountcombo.h kmymoneyaccountcompletion.h
  kmymoneyaccountselector.h kmymoneyaccounttreebase.h
  kmymoneyaccounttreebudget.h kmymoneyaccounttreeforecast.h
  kmymoneyaccounttree.h kmymoneycategory.h kmymoneychecklistitem.h
  kmymoneycombo.h kmymoneycompletion.h kmymoneycurrencyselector.h
  kmymoneydateinput.h kmymoneyedit.h kmymoneylineedit.h
  kmymoneylistviewitem.h kmymoneyselector.h kmymoneytitlelabel.h
  kmymoneywizard.h register.h registeritem.h scheduledtransaction.h
  selectedtransaction.h stdtransactiondownloaded.h
  stdtransactionmatched.h transactioneditorcontainer.h
  transactionform.h transaction.h transactionsortoptionimpl.h
  kmymoneyreportconfigtabimpl.h kmymoneyreportcontrolimpl.h
  kmymoney_export.h
  )

KMM_CREATE_LINKS( ${kmymoney_STAT_HEADERS} )
KMM_CREATE_LINKS_BIN( transactionsortoption.h )

########### Basic Widget Library (kmymoney_base) STATIC #################

# Common sources for libkmymoney.so and libwidgets.a that do not
# contain the KMM_DESIGNER flag
SET(_uncritial_common_sources
  kbudgetvalues.cpp
  kguiutils.cpp
  kmymoneyaccountselector.cpp
  kmymoneyaccounttreebudget.cpp
  kmymoneyaccounttree.cpp
  kmymoneyaccounttreeforecast.cpp
  kmymoneycalculator.cpp
  kmymoneychecklistitem.cpp
  kmymoneycombo.cpp
  kmymoneycompletion.cpp
  kmymoneycurrencyselector.cpp
  kmymoneydateinput.cpp
  kmymoneydatetbl.cpp
  kmymoneyedit.cpp
  kmymoneylineedit.cpp
  kmymoneylistviewitem.cpp
  kmymoneytitlelabel.cpp
  kmymoneywizard.cpp
  registeritem.cpp
  scheduledtransaction.cpp
  stdtransactiondownloaded.cpp
  stdtransactionmatched.cpp 
  transaction.cpp
  transactionform.cpp
  )

# sources that contain the KMM_DESIGNER flag
SET (_critial_common_sources
  kaccounttemplateselector.cpp
  kmymoneyaccountcombo.cpp
  kmymoneyaccountcompletion.cpp
  kmymoneyaccounttreebase.cpp
  kmymoneycategory.cpp
  kmymoneyselector.cpp
  register.cpp
  )


SET (kmymoney_base_UI
  kbudgetvaluesdecl.ui transactionsortoptiondecl.ui kaccounttemplateselectordecl.ui
  )

KDE4_ADD_UI_FILES(kmymoney_base_ui_srcs ${kmymoney_base_UI})
SET(_uncritial_common_sources ${_uncritial_common_sources}
  ${kmymoney_base_ui_srcs})

# in order to use add_dependencies, we need to add this custom target
# for all generated header files.
# (see http://www.vtk.org/Wiki/CMake_FAQ#How_can_I_add_a_dependency_to_a_source_file_which_is_generated_in_a_subdirectory.3F )
ADD_CUSTOM_TARGET(generate_base_ui_srcs DEPENDS
  ${kmymoney_base_ui_srcs})

KDE4_ADD_KCFG_FILES(_uncritial_common_sources ../kmymoneysettings.kcfgc)

# We can compile the uncritical sources without KMM_DESIGNER flags
KDE4_ADD_LIBRARY(kmymoney_base STATIC ${_uncritial_common_sources})

########### QtDesigner Widget Library (kmymoneywidgets) #################
# we never link against this library,
# but it is needed for uic and QtDesigner
set(kmymoneywidgets_PART_SRCS
   ${CMAKE_CURRENT_BINARY_DIR}/kmymoneywidgets.cpp
)

kde4_add_widget_files(${kmymoneywidgets_PART_SRCS} kmymoney.widgets)

set(kmymoneywidgets_PART_SRCS
   ${_critial_common_sources} 
   ${_uncritial_common_sources}
   ${kmymoneywidgets_PART_SRCS}
)

kde4_add_plugin(kmymoneywidgets ${kmymoneywidgets_PART_SRCS})

# For now, be NOT pedantic about this library.
# Some undefined references remain.
SET_TARGET_PROPERTIES(kmymoneywidgets PROPERTIES
  LINK_FLAGS "-Wl,--undefined")

##### Is the option -DKMM_DESIGNER needed??
# The option -DKMM_DESIGNER will leave away any code that breaks uic.
SET_TARGET_PROPERTIES(kmymoneywidgets PROPERTIES
  COMPILE_FLAGS "-U_CHECK_MEMORY -DKMM_DESIGNER")
#####

target_link_libraries(kmymoneywidgets kmymoney_base kmm_mymoney kmymoney_common dialogs widgets converter ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

if( USE_QT_DESIGNER )
  install(TARGETS kmymoneywidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )
  INSTALL(FILES kmymoneytitlelabel.png DESTINATION ${DATA_INSTALL_DIR}/kmymoney2/pics)
endif( USE_QT_DESIGNER )

########### Widget Library (widgets) STATIC #################
 
SET(libwidgets_a_SOURCES
  ${_critial_common_sources}
  kmymoneybriefschedule.cpp
  kmymoneycalendar.cpp
  kmymoneyforecastlistviewitem.cpp
  kmymoneyonlinequoteconfig.cpp
  kmymoneypriceview.cpp
  kmymoneyscheduledcalendar.cpp
  kmymoneyscheduleddatetbl.cpp
  registersearchline.cpp
  selectedtransaction.cpp
  transactioneditorcontainer.cpp
  transactionsortoptionimpl.cpp
  kmymoneyreportconfigtabimpl.cpp
  kmymoneyreportcontrolimpl.cpp
  )

SET(libwidgets_a_UI
  kschedulebriefwidget.ui kmymoneyreportcontroldecl.ui
  kmymoneyreportconfigtab1decl.ui kmymoneyreportconfigtab2decl.ui
  kmymoneyreportconfigtab3decl.ui kmymoneyreportconfigtabchartdecl.ui
  kmymoneyonlinequoteconfigdecl.ui
  )

# using uic on the above UI files DEPENDS on libkmymoney.so. If uic
# does not find libkmymoney.so, gcc will fail compiling
# kmymoneyreportconfigtab2decl.cpp and throw errors like "invalid use
# of undefined type `struct KMyMoneyGeneralCombo'"


kde4_add_ui_files(widgets_ui_srcs ${libwidgets_a_UI})
ADD_CUSTOM_TARGET(generate_widgets_ui_srcs DEPENDS ${widgets_ui_srcs})

KDE4_ADD_LIBRARY(widgets STATIC
  ${libwidgets_a_SOURCES}
  ${widgets_ui_srcs}
  ${_uncritial_common_sources}
  )
TARGET_LINK_LIBRARIES(widgets kmymoney_base)

########### install files ###############

INSTALL(FILES ${kmymoney_STAT_HEADERS} 
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

