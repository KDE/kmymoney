if(BUILD_TESTING)
  add_subdirectory( tests )
endif()

set (libconverter_a_SOURCES
  mymoneystatementreader.cpp
  transactionmatchfinder.cpp
  existingtransactionmatchfinder.cpp
  scheduledtransactionmatchfinder.cpp
  ../widgets/kmymoneymoneyvalidator.cpp
)

add_library(converter STATIC ${libconverter_a_SOURCES})

# TODO: clean dependencies
target_link_libraries(converter
  PUBLIC
    KF5::Service
    KF5::XmlGui
    KF5::Completion
    KF5::TextWidgets
    KF5::WidgetsAddons
    Alkimia::alkimia
    KF5::KIOWidgets
    kmm_csvimportercore
    # kmm_dialogs
  PRIVATE
    KF5::I18n
    kmymoney_common
    newaccountwizard
)

# we rely on some dialogs to be generated
# add_dependencies(converter)

########### install files ###############

