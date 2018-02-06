project(widgets)

########### create links ###############
set(kmymoney_STAT_HEADERS
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
  kmymoneyvalidationfeedback.h
  onlinejobmessagesview.h
  )

########### Shared widget library ###########
set(kmm_widgets_sources
  kmymoneydateinput.cpp
  kmymoneyvalidationfeedback.cpp
  styleditemdelegateforwarder.cpp
  kmymoneyedit.cpp
  kmymoneylineedit.cpp
  kmymoneytextedit.cpp
  kmymoneymvccombo.cpp
  kmymoneyselector.cpp
  kmymoneycalculator.cpp
  ktreewidgetfilterlinewidget.cpp
  kguiutils.cpp
  onlinejobmessagesview.cpp
  )

kde4_add_ui_files(kmm_widgets_sources
  kmymoneyvalidationfeedback.ui
  onlinejobmessagesview.ui
)

kde4_add_library(kmm_widgets SHARED ${kmm_widgets_sources})
target_link_libraries(kmm_widgets LINK_PUBLIC ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS}
  kmm_settings
  kmm_mymoney
)

set_target_properties(kmm_widgets PROPERTIES
   COMPILE_FLAGS "-DKMM_BUILD_WIDGETS_LIB"
   VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

generate_export_header(kmm_widgets)

install(TARGETS kmm_widgets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### Basic Widget Library (kmymoney_base) STATIC #################

# Common sources for libkmymoney.so and libwidgets.a that do not
# contain the KMM_DESIGNER flag
set(_uncritial_common_sources
  fixedcolumntreeview.cpp
  kbudgetvalues.cpp
  kmymoneyaccountcombo.cpp
  kmymoneyaccountselector.cpp
  kmymoneyaccounttreeview.cpp
  kmymoneycombo.cpp
  kmymoneycompletion.cpp
  kmymoneydatetbl.cpp
  kmymoneytitlelabel.cpp
  kmymoneywizard.cpp
  kpricetreeitem.cpp
  registeritem.cpp
  scheduledtransaction.cpp
  selectedtransaction.cpp
  stdtransactiondownloaded.cpp
  stdtransactionmatched.cpp
  transactionform.cpp
  transactionsortoptionimpl.cpp
  )

# sources that contain the KMM_DESIGNER flag
set (_critial_common_sources
  kaccounttemplateselector.cpp
  kmymoneycurrencyselector.cpp
  kmymoneyaccountcompletion.cpp
  kmymoneycategory.cpp
  register.cpp
  transaction.cpp
  transactioneditorcontainer.cpp
  )


set (kmymoney_base_UI
  kbudgetvaluesdecl.ui transactionsortoptiondecl.ui kaccounttemplateselectordecl.ui
  )

kde4_add_ui_files(kmymoney_base_ui_srcs ${kmymoney_base_UI})
set(_uncritial_common_sources ${_uncritial_common_sources}
  ${kmymoney_base_ui_srcs})

# in order to use add_dependencies, we need to add this custom target
# for all generated header files.
# (see http://www.vtk.org/Wiki/CMake_FAQ#How_can_I_add_a_dependency_to_a_source_file_which_is_generated_in_a_subdirectory.3F )
add_custom_target(generate_base_ui_srcs DEPENDS
  ${kmymoney_base_ui_srcs})

# We can compile the uncritical sources without KMM_DESIGNER flags
kde4_add_library(kmymoney_base STATIC ${_uncritial_common_sources})

add_dependencies(kmymoney_base kmm_settings)

########### QtDesigner Widget Library (kmymoneywidgets) #################
# we never link against this library,
# but it is needed for uic and QtDesigner
if( USE_QT_DESIGNER )
  set(kmymoneywidgets_PART_SRCS
      ${CMAKE_CURRENT_BINARY_DIR}/kmymoneywidgets.cpp)

  kde4_add_widget_files(kmymoneywidgets_PART_SRCS kmymoney.widgets)

  set(kmymoneywidgets_PART_SRCS
    ${_critial_common_sources}
    ${kmymoneywidgets_PART_SRCS})

  kde4_add_plugin(kmymoneywidgets ${kmymoneywidgets_PART_SRCS})

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

  # The option -DKMM_DESIGNER will leave away any code that breaks uic.
  set_target_properties(kmymoneywidgets PROPERTIES
    COMPILE_FLAGS "-DKMM_DESIGNER")

  # The qt-designer widget library shouldn't need to link against the
  # dialogs and converter libraries.  If a widget references something
  # from one of these libraries it is most likely due to code that needs
  # to be excluded with a KMM_DESIGNER ifndef.
  target_link_libraries(kmymoneywidgets kmymoney_base kmm_mymoney kmymoney_common kmm_settings models ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

  install(TARGETS kmymoneywidgets DESTINATION ${QT_PLUGINS_DIR}/designer )
endif( USE_QT_DESIGNER )

########### Widget Library (widgets) STATIC #################

set(libwidgets_a_SOURCES
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

set(libwidgets_a_UI
  kschedulebriefwidget.ui kmymoneyreportcontroldecl.ui
  kmymoneyreportconfigtab1decl.ui kmymoneyreportconfigtab2decl.ui
  kmymoneyreportconfigtab3decl.ui kmymoneyreportconfigtabchartdecl.ui
  )

# using uic on the above UI files DEPENDS on libkmymoney.so. If uic
# does not find libkmymoney.so, gcc will fail compiling
# kmymoneyreportconfigtab2decl.cpp and throw errors like "invalid use
# of undefined type `struct KMyMoneyGeneralCombo'"


kde4_add_ui_files(widgets_ui_srcs ${libwidgets_a_UI})
add_custom_target(generate_widgets_ui_srcs DEPENDS ${widgets_ui_srcs})

kde4_add_library(widgets STATIC
  ${libwidgets_a_SOURCES}
  ${widgets_ui_srcs}
  )
target_link_libraries(widgets kmymoney_base)

add_dependencies(widgets kmm_settings)

########### install files ###############

install(FILES ${kmymoney_STAT_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/kmm_widgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
