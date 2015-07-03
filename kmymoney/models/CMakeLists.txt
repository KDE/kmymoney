set(libmodels_a_SOURCES
    accountsmodel.cpp
    onlinejobmodel.cpp
    onlinebankingaccountsfilterproxymodel.cpp
    models.cpp
    payeeidentifiercontainermodel.cpp
    onlinejobmessagesmodel.cpp
   )

add_library(models STATIC ${libmodels_a_SOURCES})

target_link_libraries(models LINK_PUBLIC
    kmm_mymoney
    kmymoney_common
    KF5::KDELibs4Support
    Alkimia::alkimia
)

add_dependencies(models kmm_config)
