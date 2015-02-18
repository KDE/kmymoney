set(libmodels_a_SOURCES
    accountsmodel.cpp
    onlinejobmodel.cpp
    onlinebankingaccountsfilterproxymodel.cpp
    models.cpp
    payeeidentifiermodel.cpp
    onlinejobmessagesmodel.cpp
   )

add_library(models STATIC ${libmodels_a_SOURCES})
target_link_libraries(models KF5::KDELibs4Support Alkimia::alkimia)

target_link_libraries(models
    kmm_mymoney
)

add_dependencies(models kmm_config)
