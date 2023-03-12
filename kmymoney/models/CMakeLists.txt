set(libmodels_a_SOURCES
    accountsproxymodel.cpp
    comboboxmodels.cpp
    delegateproxy.cpp
    equitiesmodel.cpp
    idfilter.cpp
    institutionsproxymodel.cpp
    itemrenameproxymodel.cpp
    ledgerconcatenatemodel.cpp
    ledgerfilter.cpp
    ledgerfilterbase.cpp
    ledgersortorder.cpp
    ledgersortproxymodel.cpp
    onlinebankingaccountsfilterproxymodel.cpp
    onlinebalanceproxymodel.cpp
    payeeidentifiercontainermodel.cpp
    scheduleproxymodel.cpp
    securitiesfilterproxymodel.cpp
    securityaccountsproxymodel.cpp
    specialledgeritemfilter.cpp
)

add_library(kmm_models ${libmodels_a_SOURCES})

generate_export_header(kmm_models BASE_NAME kmm_models)

target_link_libraries(kmm_models PUBLIC
    Qt::Core
    kmm_mymoney
    kmm_settings
    kmymoney_common
    onlinetask_interfaces
    KF5::ItemModels
)
set_target_properties(kmm_models PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

if (USE_MODELTEST)
  target_compile_definitions(kmm_models PRIVATE -DKMM_MODELTEST)
  target_link_libraries(kmm_models PRIVATE Qt::Test)
endif()

add_dependencies(kmm_models kmm_settings)

install(TARGETS kmm_models ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
