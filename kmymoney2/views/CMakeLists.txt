############# next target (views) STATIC ###################
SET(libviews_a_SOURCES
	#kaccountsview.cpp
	kbudgetview.cpp
	kcategoriesview.cpp
	kforecastview.cpp
	#kgloballedgerview.cpp
	#khomeview.cpp
   kinstitutionsview.cpp
  kinvestmentlistitem.cpp
  kinvestmentview.cpp
  kmymoneyfile.cpp
  kmymoneytransaction.cpp
  #kmymoneyview.cpp
  #kpayeesview.cpp
  #kreportsview.cpp
  kscheduledlistitem.cpp
  kscheduledview.cpp
  )

SET(libviews_a_HEADER kmymoneyview.h)

SET(libviews_a_UI
  kaccountsviewdecl.ui
  kbudgetviewdecl.ui
  kcategoriesviewdecl.ui
  kforecastviewdecl.ui
  kinstitutionsviewdecl.ui
  kinvestmentviewdecl.ui
  kpayeesviewdecl.ui
  kscheduledviewdecl.ui
  )

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
kde4_add_ui_files(libviews_a_SOURCES ${libviews_a_UI})

KDE4_ADD_LIBRARY(views STATIC ${libviews_a_SOURCES})

ADD_DEPENDENCIES(views generate_dialogs_ui_srcs generate_base_ui_srcs
  generate_widgets_ui_srcs)

########### install files ###############

INSTALL(FILES ${libviews_a_HEADER}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
