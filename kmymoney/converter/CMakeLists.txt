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
    KF${QT_MAJOR_VERSION}::Service
    KF${QT_MAJOR_VERSION}::XmlGui
    KF${QT_MAJOR_VERSION}::Completion
    KF${QT_MAJOR_VERSION}::TextWidgets
    KF${QT_MAJOR_VERSION}::WidgetsAddons
    Alkimia::alkimia
    KF${QT_MAJOR_VERSION}::KIOWidgets
    kmm_csvimportercore
    kmm_base_dialogs
    kmm_models
  PRIVATE
    KF${QT_MAJOR_VERSION}::I18n
    kmymoney_common
    newaccountwizard
)

# we rely on some dialogs to be generated
# add_dependencies(converter)

########### install files ###############

