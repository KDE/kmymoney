set(libmodels_a_SOURCES
    ledgermodel.cpp
    ledgeritem.cpp
    ledgerproxymodel.cpp
    ledgerschedule.cpp
    ledgersplit.cpp
    ledgertransaction.cpp
    splitmodel.cpp
    costcentermodel.cpp
    payeesmodel.cpp
    accountsmodel.cpp
    accountsproxymodel.cpp
    onlinejobmodel.cpp
    onlinebankingaccountsfilterproxymodel.cpp
    ../mymoney/onlinejobmessage.cpp
    ../plugins/onlinetasks/interfaces/tasks/credittransfer.cpp
    models.cpp
    payeeidentifiercontainermodel.cpp
    onlinejobmessagesmodel.cpp
    equitiesmodel.cpp
    securitiesmodel.cpp
)

if (USE_MODELTEST)
  list(APPEND libmodels_a_SOURCES modeltest.cpp)
endif()

add_library(models STATIC ${libmodels_a_SOURCES})

target_link_libraries(models PUBLIC
    kmm_mymoney
    kmymoney_common
    KF5::ItemModels
)

if (USE_MODELTEST)
  target_compile_definitions(models PRIVATE -DKMM_MODELTEST)
  target_link_libraries(models PRIVATE Qt5::Test)
endif()

add_dependencies(models kmm_config)
