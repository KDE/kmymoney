ADD_SUBDIRECTORY( storage )

########### next target ###############

SET(kmm_mymoney_LIB_SRCS
  mymoneymoney.cpp mymoneyfinancialcalculator.cpp 
  mymoneytransactionfilter.cpp
  mymoneyobject.cpp mymoneykeyvaluecontainer.cpp 
  mymoneysplit.cpp mymoneyinstitution.cpp
  mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyutils.cpp
  mymoneysecurity.cpp mymoneytransaction.cpp mymoneyscheduled.cpp
  mymoneypayee.cpp 
  mymoneyfile.cpp 
  mymoneycategory.cpp 
  mymoneyaccount.cpp
  mymoneyreport.cpp mymoneystatement.cpp mymoneyprice.cpp mymoneybudget.cpp
  mymoneyobjectcontainer.cpp 
  mymoneyforecast.cpp
  )


SET(mymoney_HEADERS kmm_mymoney_export.h
  mymoneyobject.h mymoneyaccount.h mymoneycategory.h mymoneyexception.h
  mymoneyfile.h mymoneyfinancialcalculator.h mymoneyinstitution.h
  mymoneyinvesttransaction.h mymoneykeyvaluecontainer.h mymoneymoney.h
  mymoneypayee.h mymoneyprice.h mymoneyreport.h
  mymoneyscheduled.h mymoneysecurity.h mymoneysplit.h mymoneystatement.h
  mymoneytransactionfilter.h mymoneytransaction.h
  mymoneyutils.h mymoneybudget.h mymoneyobjectcontainer.h mymoneyforecast.h
  )

KDE4_ADD_LIBRARY(kmm_mymoney SHARED 
	${kmm_mymoney_LIB_SRCS})

TARGET_LINK_LIBRARIES(kmm_mymoney ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS} kmm_storage)

SET_TARGET_PROPERTIES(kmm_mymoney PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

########### install files ###############

INSTALL(TARGETS kmm_mymoney
	${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL(FILES ${mymoney_HEADERS}
	DESTINATION ${INCLUDE_INSTALL_DIR}/kmymoney COMPONENT Devel)
