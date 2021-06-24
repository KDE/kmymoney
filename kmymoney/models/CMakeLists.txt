set(libmodels_a_SOURCES
    accountsproxymodel.cpp
    institutionsproxymodel.cpp
    onlinebankingaccountsfilterproxymodel.cpp
    payeeidentifiercontainermodel.cpp
    equitiesmodel.cpp
    securitiesfilterproxymodel.cpp
    delegateproxy.cpp
    itemrenameproxymodel.cpp
    onlinebalanceproxymodel.cpp
    ledgermodel.cpp
    ledgeritem.cpp
    ledgerfilterbase.cpp
    ledgerschedule.cpp
    ledgersplit.cpp
    ledgertransaction.cpp
    specialdatesfilter.cpp
    idfilter.cpp
    scheduleproxymodel.cpp
    ledgerfilter.cpp
    comboboxmodels.cpp
)

add_library(kmm_models SHARED ${libmodels_a_SOURCES})

generate_export_header(kmm_models BASE_NAME kmm_models)

target_link_libraries(kmm_models PUBLIC
    Qt5::Core
    kmm_mymoney
    kmm_settings
    kmymoney_common
    onlinetask_interfaces
    KF5::ItemModels
)
set_target_properties(kmm_models PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

if (USE_MODELTEST)
  target_compile_definitions(kmm_models PRIVATE -DKMM_MODELTEST)
  target_link_libraries(kmm_models PRIVATE Qt5::Test)
endif()

add_dependencies(kmm_models kmm_settings)

install(TARGETS kmm_models ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
