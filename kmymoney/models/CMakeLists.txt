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
    models.cpp
    payeeidentifiercontainermodel.cpp
    onlinejobmessagesmodel.cpp
    equitiesmodel.cpp
    securitiesmodel.cpp
)

if (USE_MODELTEST)
  add_definitions( -DKMM_MODELTEST )
  set(libmodels_a_SOURCES ${libmodels_a_SOURCES} modeltest.cpp)
endif(USE_MODELTEST)

add_library(models STATIC ${libmodels_a_SOURCES})

target_link_libraries(models LINK_PUBLIC
    kmm_mymoney
    kmymoney_common
    KF5::ItemModels
)

add_dependencies(models kmm_config)
