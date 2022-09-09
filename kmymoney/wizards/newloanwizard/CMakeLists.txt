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
  knewloanwizard.ui
  additionalfeeswizardpage.ui       loanamountwizardpage.ui
  loanattributeswizardpage.ui
  assetaccountwizardpage.ui         namewizardpage.ui
  calculationoverviewwizardpage.ui  newcalculateloanwizardpage.ui
  durationwizardpage.ui             newgeneralinfowizardpage.ui
  editintrowizardpage.ui            newintrowizardpage.ui
  editselectionwizardpage.ui        newpaymentswizardpage.ui
  effectivedatewizardpage.ui        paymenteditwizardpage.ui
  finalpaymentwizardpage.ui         paymentfrequencywizardpage.ui
  firstpaymentwizardpage.ui         paymentwizardpage.ui
  interestcalculationwizardpage.ui  previouspaymentswizardpage.ui
  interestcategorywizardpage.ui     recordpaymentwizardpage.ui
  interesteditwizardpage.ui         schedulewizardpage.ui
  interesttypewizardpage.ui         summaryeditwizardpage.ui
  interestwizardpage.ui             summarywizardpage.ui
  knewloanwizard.ui                 variableinterestdatewizardpage.ui
  lendborrowwizardpage.ui
  )

ki18n_wrap_ui(libnewloanwizard_a_SOURCES ${libnewloanwizard_a_UI} )

add_library(newloanwizard STATIC ${libnewloanwizard_a_SOURCES})
# TODO: clean dependencies
target_link_libraries(newloanwizard newaccountwizard KF5::XmlGui KF5::Service KF5::I18n KF5::Completion KF5::WidgetsAddons KF5::TextWidgets Qt::Widgets Qt::Xml Alkimia::alkimia)

# we rely on some dialogs to be generated
# add_dependencies(newloanwizard)
