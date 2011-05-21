PROJECT(widgets)

########### custom command #############

########### create links ###############
SET(kmymoney_STAT_HEADERS
  kaccounttemplateselector.h kbudgetvalues.h kguiutils.h
  kmymoneyaccountcombo.h kmymoneyaccountcompletion.h
  kmymoneyaccountselector.h
  kmymoneycategory.h
  kmymoneycombo.h kmymoneymvccombo.h kmymoneycompletion.h
  kmymoneycurrencyselector.h  kmymoneydateinput.h kmymoneyedit.h
  kmymoneylineedit.h kmymoneyselector.h
  kmymoneytitlelabel.h kmymoneywizard.h register.h registeritem.h
  scheduledtransaction.h selectedtransaction.h stdtransactiondownloaded.h
  stdtransactionmatched.h transactioneditorcontainer.h
  transactionform.h transaction.h transactionsortoptionimpl.h
  kmymoneyreportconfigtabimpl.h kmymoneyreportcontrolimpl.h
  kmymoney_export.h
  )

########### Shared widget library ###########
SET(kmm_widgets_sources
  kmymoneydateinput.cpp
  )

KDE4_ADD_LIBRARY(kmm_widgets SHARED ${kmm_widgets_sources})
TARGET_LINK_LIBRARIES(kmm_widgets ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS})
SET_TARGET_PROPERTIES(kmm_widgets PROPERTIES
   COMPILE_FLAGS "-DKMM_BUILD_WIDGETS_LIB"
   VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
INSTALL(TARGETS kmm_widgets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### Basic Widget Library (kmymoney_base) STATIC #################

# Common sources for libkmymoney.so and libwidgets.a that do not
# contain the KMM_DESIGNER flag
SET(_uncritial_common_sources
  kbudgetvalues.cpp
  kguiutils.cpp
  kmymoneyaccountcombo.cpp
  kmymoneyaccountselector.cpp
  kmymoneyaccounttreeview.cpp
  kmymoneycalculator.cpp
  kmymoneycombo.cpp
  kmymoneycompletion.cpp
  kmymoneydatetbl.cpp
  kmymoneyedit.cpp
  kmymoneylineedit.cpp
  kmymoneymvccombo.cpp
  kmymoneyselector.cpp
  kmymoneytitlelabel.cpp
  kmymoneywizard.cpp
  kpricetreeitem.cpp
  registeritem.cpp
  scheduledtransaction.cpp
  selectedtransaction.cpp
  stdtransactiondownloaded.cpp
  stdtransactionmatched.cpp 
  transaction.cpp
  transactionform.cpp
  transactionsortoptionimpl.cpp
  )

# sources that contain the KMM_DESIGNER flag
SET (_critial_common_sources
  kaccounttemplateselector.cpp
  kmymoneycurrencyselector.cpp
  kmymoneyaccountcompletion.cpp
  kmymoneycategory.cpp
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

# We can compile the uncritical sources without KMM_DESIGNER flags
KDE4_ADD_LIBRARY(kmymoney_base STATIC ${_uncritial_common_sources})

########### QtDesigner Widget Library (kmymoneywidgets) #################
# we never link against this library,
# but it is needed for uic and QtDesigner
IF( USE_QT_DESIGNER )
  SET(kmymoneywidgets_PART_SRCS
      ${CMAKE_CURRENT_BINARY_DIR}/kmymoneywidgets.cpp)

  KDE4_ADD_WIDGET_FILES(${kmymoneywidgets_PART_SRCS} kmymoney.widgets)

  SET(kmymoneywidgets_PART_SRCS
    ${_critial_common_sources}
    ${kmymoneywidgets_PART_SRCS})

  KDE4_ADD_PLUGIN(kmymoneywidgets ${kmymoneywidgets_PART_SRCS})

  INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

  # The option -DKMM_DESIGNER will leave away any code that breaks uic.
  SET_TARGET_PROPERTIES(kmymoneywidgets PROPERTIES
    COMPILE_FLAGS "-DKMM_DESIGNER")

  TARGET_LINK_LIBRARIES(kmymoneywidgets kmymoney_base kmm_mymoney kmymoney_common kmm_config dialogs models converter ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

  INSTALL(TARGETS kmymoneywidgets DESTINATION ${QT_PLUGINS_DIR}/designer )
ENDIF( USE_QT_DESIGNER )

########### Widget Library (widgets) STATIC #################

SET(libwidgets_a_SOURCES
  ${_critial_common_sources}
  kmymoneybriefschedule.cpp
  kmymoneycalendar.cpp
  kmymoneyscheduledcalendar.cpp
  kmymoneyscheduleddatetbl.cpp
  registersearchline.cpp
  transactioneditorcontainer.cpp
  kmymoneyreportconfigtabimpl.cpp
  kmymoneyreportcontrolimpl.cpp
  )

SET(libwidgets_a_UI
  kschedulebriefwidget.ui kmymoneyreportcontroldecl.ui
  kmymoneyreportconfigtab1decl.ui kmymoneyreportconfigtab2decl.ui
  kmymoneyreportconfigtab3decl.ui kmymoneyreportconfigtabchartdecl.ui
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
  )
TARGET_LINK_LIBRARIES(widgets kmymoney_base)

ADD_DEPENDENCIES(widgets kmm_config)

########### install files ###############

INSTALL(FILES ${kmymoney_STAT_HEADERS} 
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

