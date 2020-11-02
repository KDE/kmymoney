
########### create links ###############
set(kmm_widgets_HEADERS
  amountedit.h
  amountvalidator.h
  creditdebithelper.h
  kaccounttemplateselector.h
  kguiutils.h
  kmymoneyaccountcombo.h
  kmymoneyaccountcompletion.h
  kmymoneyaccountselector.h
  kmymoneycategory.h
  kmymoneycombo.h
  kmymoneycompletion.h
  kmymoneycurrencyselector.h
  kmymoneydateedit.h
  kmymoneydateinput.h
  kmymoneylineedit.h
  kmymoneymvccombo.h
  kmymoneyselector.h
  kmymoneytitlelabel.h
  kmymoneyvalidationfeedback.h
  onlinejobmessagesview.h
  )

########### Shared widget library ###########
set(kmm_base_widgets_SOURCES
  amountedit.cpp
  amountvalidator.cpp
  columnselector.cpp
  creditdebithelper.cpp
  kaccounttemplateselector.cpp
  kguiutils.cpp
  kmymoneyaccountcombo.cpp
  kmymoneyaccountcompletion.cpp
  kmymoneyaccountselector.cpp
  kmymoneyaccounttreeview.cpp
  kmymoneyactivitycombo.cpp
  kmymoneycalculator.cpp
  kmymoneycashflowcombo.cpp
  kmymoneycategory.cpp
  kmymoneycombo.cpp
  kmymoneycompletion.cpp
  kmymoneycurrencyselector.cpp
  kmymoneydateedit.cpp
  kmymoneydateinput.cpp
  kmymoneyfrequencycombo.cpp
  kmymoneygeneralcombo.cpp
  kmymoneylineedit.cpp
  kmymoneymoneyvalidator.cpp
  kmymoneymvccombo.cpp
  kmymoneyoccurrencecombo.cpp
  kmymoneyoccurrenceperiodcombo.cpp
  kmymoneypayeecombo.cpp
  kmymoneyperiodcombo.cpp
  kmymoneyreconcilecombo.cpp
  kmymoneyselector.cpp
  kmymoneytextedit.cpp
  kmymoneytextedithighlighter.cpp
  kmymoneytitlelabel.cpp
  kmymoneyvalidationfeedback.cpp
  ktagcontainer.cpp
  ktaglabel.cpp
  passwordtoggle.cpp
  styleditemdelegateforwarder.cpp
  tabbar.cpp
  )

set(kmm_widgets_SOURCES
  budgetviewproxymodel.cpp
  daterangedlg.cpp
  kmymoneybriefschedule.cpp
  kmymoneyviewbase.cpp
  kpricetreeitem.cpp
  ktransactionfilter.cpp
  ktreewidgetfilterlinewidget.cpp
  onlinejobmessagesview.cpp
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

list(APPEND kmm_base_widgets_SOURCES ${nationalAccountWidget_SOURCES})
list(APPEND kmm_widgets_HEADERS ${nationalAccountWidget_HEADERS})

list(APPEND kmm_base_widgets_SOURCES ${IBANBICWidget_SOURCES})
list(APPEND kmm_widgets_HEADERS ${IBANBICWidget_HEADERS})

ki18n_wrap_ui(kmm_base_widgets_SOURCES
  kaccounttemplateselector.ui
  kmymoneyvalidationfeedback.ui
  ./payeeidentifier/nationalaccount/nationalaccountedit.ui
  ./payeeidentifier/ibanbic/ibanbicitemedit.ui
)


ki18n_wrap_ui(kmm_widgets_SOURCES
  kmymoneybriefschedule.ui
  onlinejobmessagesview.ui
  daterangedlg.ui
  ktransactionfilter.ui
)

add_library(kmm_base_widgets SHARED ${kmm_base_widgets_SOURCES})

target_link_libraries(kmm_base_widgets PUBLIC
  KF5::Completion
  KF5::Notifications
  KF5::TextWidgets
#  KF5::KIOWidgets
#  KF5::ItemViews
#  KF5::I18n
#  KF5::XmlGui
#  KF5::IconThemes
#  KF5::ConfigWidgets
#  KF5::ConfigCore
#  KF5::ItemModels
#  KF5::Service
#  Qt5::Gui
#  Qt5::Core
#  Qt5::Widgets
#  Qt5::Xml
  Alkimia::alkimia
  kmm_selections
  kmm_mymoney
  kmm_settings
  kmm_models
)

set_target_properties(kmm_base_widgets PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)

generate_export_header(kmm_base_widgets BASE_NAME kmm_base_widgets)







add_library(kmm_widgets SHARED ${kmm_widgets_SOURCES})

target_link_libraries(kmm_widgets PUBLIC
  KF5::TextWidgets
  KF5::KIOWidgets
  KF5::Completion
  KF5::Notifications
  KF5::ItemViews
  KF5::I18n
  KF5::XmlGui
  KF5::IconThemes
  KF5::ConfigWidgets
  KF5::ConfigCore
  KF5::ItemModels
  KF5::Service
  Qt5::Gui
  Qt5::Core
  Qt5::Widgets
  Qt5::Xml
  Alkimia::alkimia
  converter
  kmm_mymoney
  kmm_models
  kmm_plugin
  kmm_settings
  kmm_base_dialogs
  kmm_base_widgets
  kmm_selections
)

set_target_properties(kmm_widgets PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
)
#   COMPILE_FLAGS "-DKMM_BUILD_WIDGETS_LIB"

generate_export_header(kmm_widgets BASE_NAME kmm_widgets)

########### install files ###############
install(TARGETS kmm_base_widgets kmm_widgets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ${kmm_widgets_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/kmm_widgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)

############## tests ####################

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
