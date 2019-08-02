set(libmodels_a_SOURCES
    accountsproxymodel.cpp
    onlinebankingaccountsfilterproxymodel.cpp
    ../plugins/onlinetasks/interfaces/tasks/credittransfer.cpp
    models.cpp
    payeeidentifiercontainermodel.cpp
    equitiesmodel.cpp
    securitiesfilterproxymodel.cpp
    delegateproxy.cpp
)

if(ENABLE_UNFINISHEDFEATURES)
  list(APPEND libmodels_a_SOURCES
    ledgermodel.cpp
    ledgeritem.cpp
    ledgerproxymodel.cpp
    ledgerschedule.cpp
    ledgersplit.cpp
    ledgertransaction.cpp
    splitmodel.cpp
  )
endif()

if (USE_MODELTEST)
  list(APPEND libmodels_a_SOURCES modeltest.cpp)
endif()

add_library(kmm_models SHARED ${libmodels_a_SOURCES})

generate_export_header(kmm_models BASE_NAME kmm_models)

target_link_libraries(kmm_models PUBLIC
    Qt5::Core
    kmm_mymoney
    kmm_settings
    KF5::ItemModels
    $<$<BOOL:${ENABLE_UNFINISHEDFEATURES}>:kmymoney_common>
)
set_target_properties(kmm_models PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

if (USE_MODELTEST)
  target_compile_definitions(kmm_models PRIVATE -DKMM_MODELTEST)
  target_link_libraries(kmm_models PRIVATE Qt5::Test)
endif()

add_dependencies(kmm_models kmm_settings)

install(TARGETS kmm_models ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
