
########### create links ###############
set(kmymoney_STAT_HEADERS
  kaccounttemplateselector.h kguiutils.h
  kmymoneyaccountcombo.h kmymoneyaccountcompletion.h
  kmymoneyaccountselector.h
  kmymoneycategory.h
  kmymoneycombo.h kmymoneymvccombo.h kmymoneycompletion.h
  kmymoneycurrencyselector.h  kmymoneydateinput.h kmymoneyedit.h
  kmymoneylineedit.h kmymoneyselector.h
  kmymoneytitlelabel.h register.h registeritem.h groupmarker.h fancydategroupmarker.h
  scheduledtransaction.h selectedtransaction.h selectedtransactions.h stdtransactiondownloaded.h
  stdtransactionmatched.h transactioneditorcontainer.h
  transactionform.h transaction.h investtransaction.h stdtransaction.h
  transactionsortoption.h
  kmymoneyvalidationfeedback.h
  onlinejobmessagesview.h
  kmymoneydateedit.h
  amountedit.h amountvalidator.h creditdebithelper.h
  )

########### Shared widget library ###########
set(kmm_widgets_sources
  kmymoneydateinput.cpp
  kmymoneyvalidationfeedback.cpp
  styleditemdelegateforwarder.cpp
  kmymoneyedit.cpp
  kmymoneylineedit.cpp
  kmymoneytextedit.cpp
  kmymoneytextedithighlighter.cpp
  kmymoneymvccombo.cpp
  kmymoneygeneralcombo.cpp
  kmymoneyactivitycombo.cpp
  kmymoneycashflowcombo.cpp
  kmymoneyfrequencycombo.cpp
  kmymoneyoccurrencecombo.cpp
  kmymoneyoccurrenceperiodcombo.cpp
  kmymoneypayeecombo.cpp
  kmymoneyperiodcombo.cpp
  kmymoneyreconcilecombo.cpp
  kmymoneytagcombo.cpp
  kmymoneyaccountselector.cpp
  ktagcontainer.cpp
  ktaglabel.cpp
  kmymoneyselector.cpp
  kmymoneycalculator.cpp
  ktreewidgetfilterlinewidget.cpp
  kguiutils.cpp
  onlinejobmessagesview.cpp
  kmymoneydateedit.cpp
  kmymoneymoneyvalidator.cpp
  amountedit.cpp
  amountvalidator.cpp
  creditdebithelper.cpp
  daterangedlg.cpp
  ktransactionfilter.cpp
  kmymoneyaccounttreeview.cpp
  accountsviewproxymodel.cpp
  budgetviewproxymodel.cpp
  kmymoneyviewbase.cpp
  kmymoneyaccountsviewbase.cpp
  passwordtoggle.cpp
  )

set(nationalAccountWidget_SOURCES
  ./payeeidentifier/nationalaccount/nationalaccountedit.cpp
  ./payeeidentifier/nationalaccount/nationalaccountdelegate.cpp
)

set(nationalAccountWidget_HEADERS
  ./payeeidentifier/nationalaccount/nationalaccountdelegate.h
  ./payeeidentifier/nationalaccount/nationalaccountedit.h
)

set(IBANBICWidget_SOURCES
  ./payeeidentifier/ibanbic/kibanlineedit.cpp
  ./payeeidentifier/ibanbic/kbicedit.cpp
  ./payeeidentifier/ibanbic/ibanvalidator.cpp
  ./payeeidentifier/ibanbic/bicvalidator.cpp
  ./payeeidentifier/ibanbic/ibanbicitemdelegate.cpp
  ./payeeidentifier/ibanbic/ibanbicitemedit.cpp
)

set(IBANBICWidget_HEADERS
  ./payeeidentifier/ibanbic/kibanlineedit.h
  ./payeeidentifier/ibanbic/kbicedit.h
  ./payeeidentifier/ibanbic/ibanvalidator.h
  ./payeeidentifier/ibanbic/bicvalidator.h
  ./payeeidentifier/ibanbic/ibanbicitemdelegate.h
)

list(APPEND kmm_widgets_sources ${nationalAccountWidget_SOURCES})
list(APPEND kmymoney_STAT_HEADERS ${nationalAccountWidget_HEADERS})

list(APPEND kmm_widgets_sources ${IBANBICWidget_SOURCES})
list(APPEND kmymoney_STAT_HEADERS ${IBANBICWidget_HEADERS})

ki18n_wrap_ui(kmm_widgets_sources
  kmymoneyvalidationfeedback.ui
  onlinejobmessagesview.ui
  daterangedlg.ui
  ktransactionfilter.ui
  ./payeeidentifier/nationalaccount/nationalaccountedit.ui
  ./payeeidentifier/ibanbic/ibanbicitemedit.ui
)

add_library(kmm_widgets SHARED ${kmm_widgets_sources})
target_link_libraries(kmm_widgets PUBLIC
  kmm_settings
  KF5::TextWidgets
  KF5::KIOWidgets
  KF5::Completion
  KF5::Notifications
  KF5::ItemViews
  KF5::I18n
  Qt5::Gui
  Qt5::Core
  Alkimia::alkimia
  kmm_mymoney
  kmm_models
  kmm_plugin
)

set_target_properties(kmm_widgets PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
  COMPILE_FLAGS "-DKMM_BUILD_WIDGETS_LIB"
)

generate_export_header(kmm_widgets)

install(TARGETS kmm_widgets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### Basic Widget Library (kmymoney_base) STATIC #################

# Common sources for libkmymoney.so and libwidgets.a that do not
# contain the KMM_DESIGNER flag
set(_uncritial_common_sources
  kmymoneyaccountcombo.cpp
  kmymoneycombo.cpp
  kmymoneycompletion.cpp
  kmymoneytitlelabel.cpp
  kmymoneydateedit.cpp
  kpricetreeitem.cpp
  registeritem.cpp
  registerfilter.cpp
  scheduledtransaction.cpp
  selectedtransaction.cpp
  selectedtransactions.cpp
  stdtransactiondownloaded.cpp
  stdtransactionmatched.cpp
  transactionform.cpp
  tabbar.cpp
  transactionformitemdelegate.cpp
  transactionsortoption.cpp
  )

# sources that contain the KMM_DESIGNER flag
set (_critial_common_sources
  kaccounttemplateselector.cpp
  kmymoneycurrencyselector.cpp
  kmymoneyaccountcompletion.cpp
  kmymoneycategory.cpp
  groupmarker.cpp
  groupmarkers.cpp
  fancydategroupmarker.cpp
  fancydategroupmarkers.cpp
  register.cpp
  itemptrvector.cpp
  qwidgetcontainer.cpp
  registeritemdelegate.cpp
  transaction.cpp
  stdtransaction.cpp
  investtransaction.cpp
  transactioneditorcontainer.cpp
  )


set (kmymoney_base_UI
  transactionsortoption.ui kaccounttemplateselector.ui
  )

ki18n_wrap_ui(kmymoney_base_ui_srcs ${kmymoney_base_UI})
set(_uncritial_common_sources ${_uncritial_common_sources}
  ${kmymoney_base_ui_srcs})

# in order to use add_dependencies, we need to add this custom target
# for all generated header files.
# (see http://www.vtk.org/Wiki/CMake_FAQ#How_can_I_add_a_dependency_to_a_source_file_which_is_generated_in_a_subdirectory.3F )
add_custom_target(generate_base_ui_srcs DEPENDS
  ${kmymoney_base_ui_srcs})

# We can compile the uncritical sources without KMM_DESIGNER flags
add_library(kmymoney_base STATIC ${_uncritial_common_sources})
# TODO: fix dependencies
target_link_libraries(kmymoney_base KF5::XmlGui KF5::TextWidgets KF5::IconThemes KF5::I18n KF5::ConfigWidgets KF5::ConfigCore KF5::Completion KF5::Service Qt5::Gui Qt5::Widgets Qt5::Xml kmm_settings Alkimia::alkimia)

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

  add_library(kmymoneywidgets MODULE ${kmymoneywidgets_PART_SRCS})

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

  # The option -DKMM_DESIGNER will leave away any code that breaks uic.
  set_target_properties(kmymoneywidgets PROPERTIES
    COMPILE_FLAGS "-DKMM_DESIGNER")

  # The qt-designer widget library shouldn't need to link against the
  # dialogs and converter libraries.  If a widget references something
  # from one of these libraries it is most likely due to code that needs
  # to be excluded with a KMM_DESIGNER ifndef.
  target_link_libraries(kmymoneywidgets kmymoney_base kmm_mymoney kmymoney_common kmm_settings models)

  install(TARGETS kmymoneywidgets DESTINATION ${QT_PLUGINS_DIR}/designer )
endif( USE_QT_DESIGNER )

########### Widget Library (widgets) STATIC #################

set(libwidgets_a_SOURCES
  ${_critial_common_sources}
  kmymoneybriefschedule.cpp
  registersearchline.cpp
  transactioneditorcontainer.cpp
  )

set(libwidgets_a_UI
  kmymoneybriefschedule.ui
  )

# using uic on the above UI files DEPENDS on libkmymoney.so. If uic
# does not find libkmymoney.so, gcc will fail compiling
# kmymoneyreportconfigtab2decl.cpp and throw errors like "invalid use
# of undefined type `struct KMyMoneyGeneralCombo'"


ki18n_wrap_ui(widgets_ui_srcs ${libwidgets_a_UI})
add_custom_target(generate_widgets_ui_srcs DEPENDS ${widgets_ui_srcs})

add_library(widgets STATIC
  ${libwidgets_a_SOURCES}
  ${widgets_ui_srcs}
  )
target_link_libraries(widgets KF5::XmlGui kmymoney_base)

add_dependencies(widgets kmm_settings)

########### install files ###############

install(FILES ${kmymoney_STAT_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/kmm_widgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
