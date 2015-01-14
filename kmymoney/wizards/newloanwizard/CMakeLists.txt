include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (libnewloanwizard_a_SOURCES
  additionalfeeswizardpage.cpp       lendborrowwizardpage.cpp
  assetaccountwizardpage.cpp         loanamountwizardpage.cpp
  loanattributeswizardpage.cpp
  calculationoverviewwizardpage.cpp  namewizardpage.cpp
  durationwizardpage.cpp             newcalculateloanwizardpage.cpp
  editintrowizardpage.cpp            newgeneralinfowizardpage.cpp
  editselectionwizardpage.cpp        newintrowizardpage.cpp
  effectivedatewizardpage.cpp        newpaymentswizardpage.cpp
  finalpaymentwizardpage.cpp         paymenteditwizardpage.cpp
  firstpaymentwizardpage.cpp         paymentfrequencywizardpage.cpp
  interestcalculationwizardpage.cpp  paymentwizardpage.cpp
  interestcategorywizardpage.cpp     previouspaymentswizardpage.cpp
  interesteditwizardpage.cpp         recordpaymentwizardpage.cpp
  interesttypewizardpage.cpp         schedulewizardpage.cpp
  interestwizardpage.cpp             summaryeditwizardpage.cpp
  keditloanwizard.cpp                summarywizardpage.cpp
  knewloanwizard.cpp                 variableinterestdatewizardpage.cpp
  )

set (libnewloanwizard_a_UI
  knewloanwizarddecl.ui
  additionalfeeswizardpagedecl.ui       loanamountwizardpagedecl.ui
  loanattributeswizardpagedecl.ui
  assetaccountwizardpagedecl.ui         namewizardpagedecl.ui
  calculationoverviewwizardpagedecl.ui  newcalculateloanwizardpagedecl.ui
  durationwizardpagedecl.ui             newgeneralinfowizardpagedecl.ui
  editintrowizardpagedecl.ui            newintrowizardpagedecl.ui
  editselectionwizardpagedecl.ui        newpaymentswizardpagedecl.ui
  effectivedatewizardpagedecl.ui        paymenteditwizardpagedecl.ui
  finalpaymentwizardpagedecl.ui         paymentfrequencywizardpagedecl.ui
  firstpaymentwizardpagedecl.ui         paymentwizardpagedecl.ui
  interestcalculationwizardpagedecl.ui  previouspaymentswizardpagedecl.ui
  interestcategorywizardpagedecl.ui     recordpaymentwizardpagedecl.ui
  interesteditwizardpagedecl.ui         schedulewizardpagedecl.ui
  interesttypewizardpagedecl.ui         summaryeditwizardpagedecl.ui
  interestwizardpagedecl.ui             summarywizardpagedecl.ui
  knewloanwizarddecl.ui                 variableinterestdatewizardpagedecl.ui
  lendborrowwizardpagedecl.ui
  )

qt5_wrap_ui(libnewloanwizard_a_SOURCES ${libnewloanwizard_a_UI} )

add_library(newloanwizard STATIC ${libnewloanwizard_a_SOURCES})
target_link_libraries(newloanwizard KF5::KDELibs4Support Qt5::Sql Alkimia::alkimia)

# we rely on some dialogs to be generated
add_dependencies(newloanwizard dialogs)
